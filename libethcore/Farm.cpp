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


#include <libethcore/Farm.h>
#include <libethash-cpu/CPUMiner.h>

namespace dev
{
namespace eth
{
Farm* Farm::m_this = nullptr;

Farm::Farm(std::map<std::string, DeviceDescriptor>& _DevicesCollection,
    FarmSettings _settings, CPSettings _CPSettings)
  : m_Settings(std::move(_settings)),
    m_CPSettings(std::move(_CPSettings)),
    m_io_strand(g_io_service),
    m_collectTimer(g_io_service),
    m_DevicesCollection(_DevicesCollection)
{
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::Farm() begin");

    m_this = this;

    // Initialize nonce_scrambler
    shuffle();

    // Start data collector timer
    // It should work for the whole lifetime of Farm
    // regardless it's mining state
    m_collectTimer.expires_from_now(boost::posix_time::milliseconds(m_collectInterval));
    m_collectTimer.async_wait(
        m_io_strand.wrap(boost::bind(&Farm::collectData, this, boost::asio::placeholders::error)));

    CPUMiner::getRandomYDataset(m_CPSettings.flags);
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::Farm() end");
}

Farm::~Farm()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::~Farm() begin");

    // Stop data collector (before monitors !!!)
    m_collectTimer.cancel();

    // Stop mining (if needed)
    if (m_isMining.load(std::memory_order_relaxed))
        stop();

    CPUMiner::releaseRandomYDataset(m_CPSettings.flags);
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::~Farm() end");
}

/**
 * @brief Randomizes the nonce scrambler
 */
void Farm::shuffle()
{
    // Given that all nonces are equally likely to solve the problem
    // we could reasonably always start the nonce search ranges
    // at a fixed place, but that would be boring. Provide a once
    // per run randomized start place, without creating much overhead.
    random_device engine;
    m_nonce_scrambler = uniform_int_distribution<uint64_t>()(engine);
}

void Farm::setWork(WorkPackage const& _newWp)
{
    // Set work to each miner giving it's own starting nonce
    Guard l(x_minerWork);

    // Retrieve appropriate EpochContext
    if (m_currentWp.epoch != _newWp.epoch)
    {
        ethash::epoch_context _ec = ethash::get_global_epoch_context(_newWp.epoch);
        m_currentEc.epochNumber = _newWp.epoch;
        m_currentEc.lightNumItems = _ec.light_cache_num_items;
        m_currentEc.lightSize = ethash::get_light_cache_size(_ec.light_cache_num_items);
        m_currentEc.dagNumItems = _ec.full_dataset_num_items;
        m_currentEc.dagSize = ethash::get_full_dataset_size(_ec.full_dataset_num_items);
        m_currentEc.lightCache = _ec.light_cache;

        for (auto const& miner : m_miners)
            miner->setEpoch(m_currentEc);
    }

    m_currentWp = _newWp;

    // Check if we need to shuffle per work (ergodicity == 2)
    if (m_Settings.ergodicity == 2 && m_currentWp.exSizeBytes == 0)
        shuffle();

    uint64_t _startNonce;
    if (m_currentWp.exSizeBytes > 0)
    {
        // Equally divide the residual segment among miners
        _startNonce = m_currentWp.startNonce;
        m_nonce_segment_with =
            (unsigned int)log2(pow(2, 64 - (m_currentWp.exSizeBytes * 4)) / m_miners.size());
    }
    else
    {
        // Get the randomly selected nonce
        _startNonce = m_nonce_scrambler;
    }

    for (unsigned int i = 0; i < m_miners.size(); i++)
    {
        m_currentWp.startNonce = _startNonce + ((uint64_t)i << m_nonce_segment_with);
        m_miners.at(i)->setWork(m_currentWp);
    }
}

/**
 * @brief Start a number of miners.
 */
bool Farm::start()
{
    // Prevent recursion
    if (m_isMining.load(std::memory_order_relaxed))
        return true;

    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::start() begin");
    Guard l(x_minerWork);

    // Start all subscribed miners if none yet
    if (!m_miners.size())
    {
        for (auto it = m_DevicesCollection.begin(); it != m_DevicesCollection.end(); it++)
        {
            TelemetryAccountType minerTelemetry;
            if (it->second.subscriptionType == DeviceSubscriptionTypeEnum::Cpu)
            {
                minerTelemetry.prefix = "cp";
                m_miners.push_back(std::shared_ptr<Miner>(
                    new CPUMiner(m_miners.size(), m_CPSettings, it->second)));
            }
            if (minerTelemetry.prefix.empty())
                continue;
            m_telemetry.miners.push_back(minerTelemetry);
            m_miners.back()->startWorking();
        }

        // Initialize DAG Load mode
        Miner::setDagLoadInfo(m_Settings.dagLoadMode, (unsigned int)m_miners.size());

        m_isMining.store(true, std::memory_order_relaxed);
    }
    else
    {
        for (auto const& miner : m_miners)
            miner->startWorking();
        m_isMining.store(true, std::memory_order_relaxed);
    }

    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::start() end");
    return m_isMining.load(std::memory_order_relaxed);
}

/**
 * @brief Stop all mining activities.
 */
void Farm::stop()
{
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::stop() begin");
    // Avoid re-entering if not actually mining.
    // This, in fact, is also called by destructor
    if (isMining())
    {
        {
            Guard l(x_minerWork);
            for (auto const& miner : m_miners)
            {
                miner->triggerStopWorking();
                miner->kick_miner();
            }

            m_miners.clear();
            m_isMining.store(false, std::memory_order_relaxed);
        }
    }
    DEV_BUILD_LOG_PROGRAMFLOW(cnote, "Farm::stop() end");
}

/**
 * @brief Pauses the whole collection of miners
 */
void Farm::pause()
{
    // Signal each miner to suspend mining
    Guard l(x_minerWork);
    m_paused.store(true, std::memory_order_relaxed);
    for (auto const& m : m_miners)
        m->pause(MinerPauseEnum::PauseDueToFarmPaused);
}

/**
 * @brief Returns whether or not this farm is paused for any reason
 */
bool Farm::paused()
{
    return m_paused.load(std::memory_order_relaxed);
}

/**
 * @brief Resumes from a pause condition
 */
void Farm::resume()
{
    // Signal each miner to resume mining
    // Note ! Miners may stay suspended if other reasons
    Guard l(x_minerWork);
    m_paused.store(false, std::memory_order_relaxed);
    for (auto const& m : m_miners)
        m->resume(MinerPauseEnum::PauseDueToFarmPaused);
}

/**
 * @brief Stop all mining activities and Starts them again
 */
void Farm::restart()
{
    if (m_onMinerRestart)
        m_onMinerRestart();
}

/**
 * @brief Stop all mining activities and Starts them again (async post)
 */
void Farm::restart_async()
{
    m_io_strand.get_io_service().post(m_io_strand.wrap(boost::bind(&Farm::restart, this)));
}

/**
 * @brief Spawn a reboot script (reboot.bat/reboot.sh)
 * @return false if no matching file was found
 */
bool Farm::reboot(const std::vector<std::string>& args)
{
#if defined(_WIN32)
    const char* filename = "reboot.bat";
#else
    const char* filename = "reboot.sh";
#endif

    return spawn_file_in_bin_dir(filename, args);
}

/**
 * @brief Account solutions for miner and for farm
 */
void Farm::accountSolution(unsigned _minerIdx, SolutionAccountingEnum _accounting)
{
    if (_accounting == SolutionAccountingEnum::Accepted)
    {
        m_telemetry.farm.solutions.accepted++;
        m_telemetry.farm.solutions.tstamp = std::chrono::steady_clock::now();
        m_telemetry.miners.at(_minerIdx).solutions.accepted++;
        m_telemetry.miners.at(_minerIdx).solutions.tstamp = std::chrono::steady_clock::now();
        return;
    }
    if (_accounting == SolutionAccountingEnum::Wasted)
    {
        m_telemetry.farm.solutions.wasted++;
        m_telemetry.farm.solutions.tstamp = std::chrono::steady_clock::now();
        m_telemetry.miners.at(_minerIdx).solutions.wasted++;
        m_telemetry.miners.at(_minerIdx).solutions.tstamp = std::chrono::steady_clock::now();
        return;
    }
    if (_accounting == SolutionAccountingEnum::Rejected)
    {
        m_telemetry.farm.solutions.rejected++;
        m_telemetry.farm.solutions.tstamp = std::chrono::steady_clock::now();
        m_telemetry.miners.at(_minerIdx).solutions.rejected++;
        m_telemetry.miners.at(_minerIdx).solutions.tstamp = std::chrono::steady_clock::now();
        return;
    }
    if (_accounting == SolutionAccountingEnum::Failed)
    {
        m_telemetry.farm.solutions.failed++;
        m_telemetry.farm.solutions.tstamp = std::chrono::steady_clock::now();
        m_telemetry.miners.at(_minerIdx).solutions.failed++;
        m_telemetry.miners.at(_minerIdx).solutions.tstamp = std::chrono::steady_clock::now();
        return;
    }
}

/**
 * @brief Gets the solutions account for the whole farm
 */

SolutionAccountType Farm::getSolutions()
{
    return m_telemetry.farm.solutions;
}

/**
 * @brief Gets the solutions account for single miner
 */
SolutionAccountType Farm::getSolutions(unsigned _minerIdx)
{
    try
    {
        return m_telemetry.miners.at(_minerIdx).solutions;
    }
    catch (const std::exception&)
    {
        return SolutionAccountType();
    }
}

/**
 * @brief Provides the description of segments each miner is working on
 * @return a JsonObject
 */
Json::Value Farm::get_nonce_scrambler_json()
{
    Json::Value jRes;
    jRes["start_nonce"] = toHex(m_nonce_scrambler, HexPrefix::Add);
    jRes["device_width"] = m_nonce_segment_with;
    jRes["device_count"] = (uint64_t)m_miners.size();

    return jRes;
}

void Farm::submitProof(Solution const& _s)
{
    g_io_service.post(m_io_strand.wrap(boost::bind(&Farm::submitProofAsync, this, _s)));
}

void Farm::submitProofAsync(Solution const& _s)
{
    if (!m_Settings.noEval)
    {
        Result r = EthashAux::eval(_s.work.epoch, _s.work.header, _s.nonce);
        if (r.value > _s.work.boundary)
        {
            accountSolution(_s.midx, SolutionAccountingEnum::Failed);
            cwarn << "CPU " << _s.midx
                  << " gave incorrect result. Lower overclocking values if it happens frequently.";
            return;
        }
        m_onSolutionFound(Solution{_s.nonce, r.mixHash, _s.work, _s.tstamp, _s.midx});
    }
    else
        m_onSolutionFound(_s);

#ifdef DEV_BUILD
    if (g_logOptions & LOG_SUBMIT)
        cnote << "Submit time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now() - _s.tstamp)
                     .count()
              << " us.";
#endif
}

// Collects data about hashing and hardware status
void Farm::collectData(const boost::system::error_code& ec)
{
    if (ec)
        return;

    // Reset hashrate (it will accumulate from miners)
    float farm_hr = 0.0f;

    // Process miners
    for (auto const& miner : m_miners)
    {
        int minerIdx = miner->Index();
        float hr = (miner->paused() ? 0.0f : miner->RetrieveHashRate());
        farm_hr += hr;
        m_telemetry.miners.at(minerIdx).hashrate = hr;
        m_telemetry.miners.at(minerIdx).paused = miner->paused();
        m_telemetry.farm.hashrate = farm_hr;
        miner->TriggerHashRateUpdate();
    }

    // Resubmit timer for another loop
    m_collectTimer.expires_from_now(boost::posix_time::milliseconds(m_collectInterval));
    m_collectTimer.async_wait(
        m_io_strand.wrap(boost::bind(&Farm::collectData, this, boost::asio::placeholders::error)));
}

bool Farm::spawn_file_in_bin_dir(const char* filename, const std::vector<std::string>& args)
{
    std::string fn = boost::dll::program_location().parent_path().string() +
                     "/" +  // boost::filesystem::path::preferred_separator
                     filename;
    try
    {
        if (!boost::filesystem::exists(fn))
            return false;

        /* anything in the file */
        if (!boost::filesystem::file_size(fn))
            return false;

#if defined(__linux)
        struct stat sb;
        if (stat(fn.c_str(), &sb) != 0)
            return false;
        /* just check if any exec flag is set.
           still execution can fail (not the uid, not in the group, selinux, ...)
         */
        if ((sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0)
            return false;
#endif
        /* spawn it (no wait,...) - fire and forget! */
        boost::process::spawn(fn, args);
        return true;
    }
    catch (...)
    {
    }
    return false;
}


}  // namespace eth
}  // namespace dev
