/*
This file is part of ethminer.

ethminer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ethminer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ethminer.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 CPUMiner simulates mining devices but does NOT real mine!
 USE FOR DEVELOPMENT ONLY !
*/

#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* we need sched_setaffinity() */
#endif
#include <sched.h>
#include <unistd.h>
#endif

#include <libethcore/Farm.h>
#include <ethash/ethash.hpp>
#include <boost/version.hpp>

#if 0
#include <boost/fiber/numa/pin_thread.hpp>
#include <boost/fiber/numa/topology.hpp>
#endif

#include "CPUMiner.h"
#include "picosha3.h"
#include "endian.hpp"
#include "RandomY/src/randomx.h"

/* Sanity check for defined OS */
#if defined(__APPLE__) || defined(__MACOSX)
/* MACOSX */
#include <mach/mach.h>
#elif defined(__linux__)
/* linux */
#elif defined(_WIN32)
/* windows */
#endif

using namespace std;
using namespace dev;
using namespace eth;
using namespace ethash;

/* ################## OS-specific functions ################## */

/*
 * returns physically available memory (no swap)
 */
static size_t getTotalPhysAvailableMemory()
{
#if defined(__APPLE__) || defined(__MACOSX)
    vm_statistics64_data_t	vm_stat;
    vm_size_t page_size;
    host_name_port_t host = mach_host_self();
    kern_return_t rv = host_page_size(host, &page_size);
    if( rv != KERN_SUCCESS) {
        cwarn << "Error in func " << __FUNCTION__ << " at host_page_size(...) \""
              << "\"\n";
        mach_error("host_page_size(...) error :", rv);
        return 0;
    }
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    rv = host_statistics (host, HOST_VM_INFO, (host_info_t)&vm_stat, &count);
    if (rv != KERN_SUCCESS) {
        cwarn << "Error in func " << __FUNCTION__ << " at host_statistics(...) \""
              << "\"\n";
        mach_error("host_statistics(...) error :", rv);
        return 0;
    }
    return vm_stat.free_count*page_size;
#elif defined(__linux__)
    long pages = sysconf(_SC_AVPHYS_PAGES);
    if (pages == -1L)
    {
        cwarn << "Error in func " << __FUNCTION__ << " at sysconf(_SC_AVPHYS_PAGES) \""
              << strerror(errno) << "\"\n";
        return 0;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1L)
    {
        cwarn << "Error in func " << __FUNCTION__ << " at sysconf(_SC_PAGESIZE) \""
              << strerror(errno) << "\"\n";
        return 0;
    }

    return (size_t)pages * (size_t)page_size;
#else
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo) == 0)
    {
        // Handle Errorcode (GetLastError) ??
        return 0;
    }
    return memInfo.ullAvailPhys;
#endif
}

/*
 * return numbers of available CPUs
 */
unsigned CPUMiner::getNumDevices()
{
#if 0
    static unsigned cpus = 0;

    if (cpus == 0)
    {
        std::vector< boost::fibers::numa::node > topo = boost::fibers::numa::topology();
        for (auto n : topo) {
            cpus += n.logical_cpus.size();
        }
    }
    return cpus;
#elif defined(__APPLE__) || defined(__MACOSX)
    unsigned int cpus_available = std::thread::hardware_concurrency();
    if (cpus_available <= 0)
    {
        cwarn << "Error in func " << __FUNCTION__ << " at std::thread::hardware_concurrency \""
              << cpus_available << " were found." << "\"\n";
        return 0;
    }
    return cpus_available;

#elif defined(__linux__)
    long cpus_available;
    cpus_available = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpus_available == -1L)
    {
        cwarn << "Error in func " << __FUNCTION__ << " at sysconf(_SC_NPROCESSORS_ONLN) \""
              << strerror(errno) << "\"\n";
        return 0;
    }
    return cpus_available;
#else
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#endif
}


/* ######################## CPU Miner ######################## */

struct CPUChannel : public LogChannel
{
    static const char* name() { return EthOrange "cp"; }
    static const int verbosity = 2;
};
#define cpulog clog(CPUChannel)

CPUMiner::CPUMiner(unsigned _index, CPSettings _settings, DeviceDescriptor& _device)
  : Miner("cpu-", _index), m_settings(_settings), m_vm(nullptr)
{
    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::CPUMiner() begin");
    m_deviceDescriptor = _device;

    auto dataset = getRandomYDataset(m_settings.flags);
    if (dataset) {
        m_vm = randomx_create_vm(m_settings.flags, nullptr, dataset);
    }

    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::CPUMiner() begin");
}


CPUMiner::~CPUMiner()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::~CPUMiner() begin");
    stopWorking();
    kick_miner();

    if (m_vm != nullptr) {
        randomx_destroy_vm(m_vm);
    }

    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::~CPUMiner() end");
}

bool CPUMiner::initDevice()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::initDevice begin");

    cpulog << "Using CPU: " << m_deviceDescriptor.cpCpuNumer
           << " Memory : " << dev::getFormattedMemory((double)m_deviceDescriptor.totalMemory);

#if defined(__APPLE__) || defined(__MACOSX)
/* Not supported on MAC OSX. See https://developer.apple.com/library/archive/releasenotes/Performance/RN-AffinityAPI/ */
#elif defined(__linux__)
    cpu_set_t cpuset;
    int err;

    CPU_ZERO(&cpuset);
    CPU_SET(m_deviceDescriptor.cpCpuNumer, &cpuset);

    err = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    if (err != 0)
    {
        cwarn << "Error in func " << __FUNCTION__ << " at sched_setaffinity() \"" << strerror(errno)
              << "\"\n";
        cwarn << "cp-" << m_index << "could not bind thread to cpu" << m_deviceDescriptor.cpCpuNumer
              << "\n";
    }
#else
    DWORD_PTR dwThreadAffinityMask = 1i64 << m_deviceDescriptor.cpCpuNumer;
    DWORD_PTR previous_mask;
    previous_mask = SetThreadAffinityMask(GetCurrentThread(), dwThreadAffinityMask);
    if (previous_mask == NULL)
    {
        cwarn << "cp-" << m_index << "could not bind thread to cpu" << m_deviceDescriptor.cpCpuNumer
              << "\n";
        // Handle Errorcode (GetLastError) ??
    }
#endif
    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::initDevice end");
    return true;
}

/*
 * A new epoch was receifed with last work package (called from Miner::initEpoch())
 *
 * If we get here it means epoch has changed so it's not necessary
 * to check again dag sizes. They're changed for sure
 * We've all related infos in m_epochContext (.dagSize, .dagNumItems, .lightSize, .lightNumItems)
 */
bool CPUMiner::initEpoch_internal()
{
    return true;
}


/*
   Miner should stop working on the current block
   This happens if a
     * new work arrived                       or
     * miner should stop (eg exit ethminer)   or
     * miner should pause
*/
void CPUMiner::kick_miner()
{
    m_new_work.store(true, std::memory_order_relaxed);
    m_new_work_signal.notify_one();
}

static std::string to_hex(const hash256 h)
{
    static const auto hex_chars = "0123456789abcdef";
    std::string str;
    str.reserve(sizeof(h) * 2);
    for (auto b : h.bytes)
    {
        str.push_back(hex_chars[uint8_t(b) >> 4]);
        str.push_back(hex_chars[uint8_t(b) & 0xf]);
    }
    return str;
}

static bool is_less_or_equal(const hash256& a, const hash256& b) noexcept
{
    for (size_t i = 0; i < (sizeof(a) / sizeof(a.word64s[0])); ++i)
    {
        if (be::uint64(a.word64s[i]) > be::uint64(b.word64s[i]))
            return false;
        if (be::uint64(a.word64s[i]) < be::uint64(b.word64s[i]))
            return true;
    }
    return true;
}

void CPUMiner::showRandomYFlags(randomx_flags flags) {
    if (flags & RANDOMX_FLAG_LARGE_PAGES) {
        cpulog << "RandomY large pages mode is enabled";
    }
    if (flags & RANDOMX_FLAG_HARD_AES) {
        cpulog << "RandomY hard aes mode is enabled";
    }
    if (flags & RANDOMX_FLAG_FULL_MEM) {
        cpulog << "RandomY full mem mode is enabled";
    }
    if (flags & RANDOMX_FLAG_JIT) {
        cpulog << "RandomY JIT mode is enabled";
    }
}

randomx_dataset* CPUMiner::getRandomYDataset(randomx_flags flags) {
    static randomx_dataset* dataset = nullptr;
    if (dataset == nullptr) {
        showRandomYFlags(flags);
        std::vector<std::thread> threads;
        auto initThreadCount = std::thread::hardware_concurrency();
        auto cache = randomx_alloc_cache(flags);
        if (cache == nullptr) {
          DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "CPUMiner::getDataset cache is nullptr");
          return nullptr;
        }

        unsigned char coreKey[] = {53, 54, 55, 56, 57};
        randomx_init_cache(cache, &coreKey, sizeof coreKey);
        dataset = randomx_alloc_dataset(flags);
        if (dataset == nullptr) {
          DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "CPUMiner::getDataset dataset is nullptr");
          return nullptr;
        }
        uint32_t datasetItemCount = randomx_dataset_item_count();
        if (initThreadCount > 1) {
          auto perThread = datasetItemCount / initThreadCount;
          auto remainder = datasetItemCount % initThreadCount;
          uint32_t startItem = 0;
          for (auto i = 0; i < initThreadCount; ++i) {
            auto count = perThread + (i == initThreadCount - 1 ? remainder : 0);
            threads.push_back(std::thread(&randomx_init_dataset, dataset, cache, startItem, count));
            startItem += count;
          }
          for (auto i = 0; i < threads.size(); ++i) {
            threads[i].join();
          }
        }
        else {
          randomx_init_dataset(dataset, cache, 0, datasetItemCount);
        }
        randomx_release_cache(cache);
        cache = nullptr;
        threads.clear();
    }
    return dataset;
}

void CPUMiner::releaseRandomYDataset(randomx_flags flags) {
    auto dataset = getRandomYDataset(flags);
    if (dataset != nullptr) {
        randomx_release_dataset(dataset);
    }
}

void CPUMiner::search(const dev::eth::WorkPackage& w)
{
    constexpr size_t blocksize = 30;

    const auto header = hash256_from_bytes(w.header.data());
    const auto boundary = hash256_from_bytes(w.boundary.data());
    auto nonce = w.startNonce;

    while (true && m_vm != nullptr)
    {
        if (m_new_work.load(std::memory_order_relaxed))  // new work arrived ?
        {
            m_new_work.store(false, std::memory_order_relaxed);
            break;
        }

        if (shouldStop())
            break;

        const uint64_t end_nonce = nonce + blocksize;
        for (uint64_t n = nonce; n < end_nonce; ++n)
        {
            auto twisted = le::uint64(n);
            uint8_t init_data[sizeof header + sizeof twisted];
            std::memcpy(&init_data[0], &header, sizeof header);
            std::memcpy(&init_data[sizeof header], &twisted, sizeof twisted);
            std::vector<uint8_t> input(std::begin(init_data), std::end(init_data));

            auto sha3_512 = picosha3::get_sha3_generator<512>();
            std::array<uint8_t, picosha3::bits_to_bytes(512)> seed{};
            sha3_512(input.begin(), input.end(), seed.begin(), seed.end());

            char hash[RANDOMX_HASH_SIZE] = {};
            randomx_calculate_hash(m_vm, seed.data(), seed.size(), &hash);
            auto final_hash = hash256_from_bytes((const uint8_t*)hash);

            if (is_less_or_equal(final_hash, boundary)) {
                h256 mix{reinterpret_cast<dev::byte*>(final_hash.bytes), h256::ConstructFromPointer};
                auto sol = Solution{n, mix, w, std::chrono::steady_clock::now(), m_index};
                cpulog << EthWhite << "Job: " << w.header.abridged()
                       << " Sol: " << toHex(sol.nonce, HexPrefix::Add) << EthReset;
                Farm::f().submitProof(sol);
            }
        }

        nonce += blocksize;

        // Update the hash rate
        updateHashRate(blocksize, 1);
    }
}


/*
 * The main work loop of a Worker thread
 */
void CPUMiner::workLoop()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::workLoop() begin");

    WorkPackage current;
    current.header = h256();

    if (!initDevice())
        return;

    while (!shouldStop())
    {
        // Wait for work or 3 seconds (whichever the first)
        const WorkPackage w = work();
        if (!w)
        {
            boost::system_time const timeout =
                boost::get_system_time() + boost::posix_time::seconds(3);
            boost::mutex::scoped_lock l(x_work);
            m_new_work_signal.timed_wait(l, timeout);
            continue;
        }

        if (w.algo == "ethash")
        {
            // Epoch change ?
            if (current.epoch != w.epoch)
            {
                if (!initEpoch())
                    break;  // This will simply exit the thread

                // As DAG generation takes a while we need to
                // ensure we're on latest job, not on the one
                // which triggered the epoch change
                current = w;
                continue;
            }

            // Persist most recent job.
            // Job's differences should be handled at higher level
            current = w;

            // Start searching
            search(w);
        }
        else
        {
            throw std::runtime_error("Algo : " + w.algo + " not yet implemented");
        }
    }

    DEV_BUILD_LOG_PROGRAMFLOW(cpulog, "cp-" << m_index << " CPUMiner::workLoop() end");
}


void CPUMiner::enumDevices(std::map<string, DeviceDescriptor>& _DevicesCollection, unsigned _miningThreads)
{
    for (unsigned i = 0; i < _miningThreads; i++)
    {
        string uniqueId;
        ostringstream s;
        DeviceDescriptor deviceDescriptor;

        s << "cpu-" << i;
        uniqueId = s.str();
        if (_DevicesCollection.find(uniqueId) != _DevicesCollection.end())
            deviceDescriptor = _DevicesCollection[uniqueId];
        else
            deviceDescriptor = DeviceDescriptor();

        s.str("");
        s.clear();
        s << "eval()/boost " << (BOOST_VERSION / 100000) << "."
          << (BOOST_VERSION / 100 % 1000) << "." << (BOOST_VERSION % 100);
        deviceDescriptor.name = s.str();
        deviceDescriptor.uniqueId = uniqueId;
        deviceDescriptor.type = DeviceTypeEnum::Cpu;
        deviceDescriptor.totalMemory = getTotalPhysAvailableMemory();

        deviceDescriptor.cpCpuNumer = i;

        _DevicesCollection[uniqueId] = deviceDescriptor;
    }
}
