/*
    This file is part of coreminer.

    coreminer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    coreminer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with coreminer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <CLI/CLI.hpp>

#include <coreminer/buildinfo.h>
#include <condition_variable>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#include <libethcore/Farm.h>
#include <libethash-cpu/CPUMiner.h>
#include <libpoolprotocols/PoolManager.h>
#include "RandomY/src/randomx.h"

#if API_CORE
#include <libapicore/ApiServer.h>
#include <regex>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

using namespace std;
using namespace dev;
using namespace dev::eth;


// Global vars
bool g_running = false;
bool g_exitOnError = false;  // Whether or not coreminer should exit on mining threads errors

condition_variable g_shouldstop;
boost::asio::io_service g_io_service;  // The IO service itself

struct MiningChannel : public LogChannel
{
    static const char* name() { return EthGreen " m"; }
    static const int verbosity = 2;
};

#define minelog clog(MiningChannel)

#if ETH_DBUS
#include <coreminer/DBusInt.h>
#endif

class MinerCLI
{
public:
    enum class OperationMode
    {
        None,
        Simulation,
        Mining
    };

    MinerCLI() : m_cliDisplayTimer(g_io_service), m_io_strand(g_io_service)
    {
        // Initialize display timer as sleeper
        m_cliDisplayTimer.expires_from_now(boost::posix_time::pos_infin);
        m_cliDisplayTimer.async_wait(m_io_strand.wrap(boost::bind(
            &MinerCLI::cliDisplayInterval_elapsed, this, boost::asio::placeholders::error)));

        // Start io_service in it's own thread
        m_io_thread = std::thread{boost::bind(&boost::asio::io_service::run, &g_io_service)};

        // Io service is now live and running
        // All components using io_service should post to reference of g_io_service
        // and should not start/stop or even join threads (which heavily time consuming)
    }

    virtual ~MinerCLI()
    {
        m_cliDisplayTimer.cancel();
        g_io_service.stop();
        m_io_thread.join();
    }

    void cliDisplayInterval_elapsed(const boost::system::error_code& ec)
    {
        if (!ec && g_running)
        {
            string logLine =
                PoolManager::p().isConnected() ? Farm::f().Telemetry().str() : "Not connected";
            minelog << logLine;

#if ETH_DBUS
            dbusint.send(Farm::f().Telemetry().str().c_str());
#endif
            // Resubmit timer
            m_cliDisplayTimer.expires_from_now(boost::posix_time::seconds(m_cliDisplayInterval));
            m_cliDisplayTimer.async_wait(m_io_strand.wrap(boost::bind(
                &MinerCLI::cliDisplayInterval_elapsed, this, boost::asio::placeholders::error)));
        }
    }

    static void signalHandler(int sig)
    {
        dev::setThreadName("main");

        switch (sig)
        {
        case (999U):
            // Compiler complains about the lack of
            // a case statement in Windows
            // this makes it happy.
            break;
        default:
            cnote << "Got interrupt ...";
            g_running = false;
            g_shouldstop.notify_all();
            break;
        }
    }

#if API_CORE

    static void ParseBind(
        const std::string& inaddr, std::string& outaddr, int& outport, bool advertise_negative_port)
    {
        std::regex pattern("([\\da-fA-F\\.\\:]*)\\:([\\d\\-]*)");
        std::smatch matches;

        if (std::regex_match(inaddr, matches, pattern))
        {
            // Validate Ip address
            boost::system::error_code ec;
            outaddr = boost::asio::ip::address::from_string(matches[1], ec).to_string();
            if (ec)
                throw std::invalid_argument("Invalid Ip Address");

            // Parse port ( Let exception throw )
            outport = std::stoi(matches[2]);
            if (advertise_negative_port)
            {
                if (outport < -65535 || outport > 65535 || outport == 0)
                    throw std::invalid_argument(
                        "Invalid port number. Allowed non zero values in range [-65535 .. 65535]");
            }
            else
            {
                if (outport < 1 || outport > 65535)
                    throw std::invalid_argument(
                        "Invalid port number. Allowed non zero values in range [1 .. 65535]");
            }
        }
        else
        {
            throw std::invalid_argument("Invalid syntax");
        }
    }
#endif
    bool validateArgs(int argc, char** argv)
    {
        std::queue<string> warnings;

        CLI::App app("Coreminer - CPU Ethash miner");

        bool bhelp = false;
        string shelpExt;

        app.set_help_flag();
        app.add_flag("-h,--help", bhelp, "Show help");

        app.add_set("-H,--help-ext", shelpExt,
            {
                "con", "test", "cp",
#if API_CORE
                    "api",
#endif
                    "misc", "env"
            },
            "", true);

        bool version = false;

        app.add_option("--ergodicity", m_FarmSettings.ergodicity, "", true)->check(CLI::Range(0, 2));

        app.add_flag("-V,--version", version, "Show program version");

        app.add_option("-v,--verbosity", g_logOptions, "", true)->check(CLI::Range(LOG_NEXT - 1));

        app.add_option("--farm-recheck", m_PoolSettings.getWorkPollInterval, "", true)->check(CLI::Range(1, 99999));

        app.add_option("--farm-retries", m_PoolSettings.connectionMaxRetries, "", true)->check(CLI::Range(0, 99999));

        app.add_option("--retry-delay", m_PoolSettings.delayBeforeRetry, "", true)
            ->check(CLI::Range(1, 999));
        
        app.add_option("--work-timeout", m_PoolSettings.noWorkTimeout, "", true)
            ->check(CLI::Range(180, 99999));

        app.add_option("--response-timeout", m_PoolSettings.noResponseTimeout, "", true)
            ->check(CLI::Range(2, 999));

        app.add_flag("-R,--report-hashrate,--report-hr", m_PoolSettings.reportHashrate, "");

        app.add_option("--display-interval", m_cliDisplayInterval, "", true)
            ->check(CLI::Range(1, 1800));

        app.add_flag("--exit", g_exitOnError, "");

        vector<string> pools;
        app.add_option("-P,--pool", pools, "");

        app.add_option("--failover-timeout", m_PoolSettings.poolFailoverTimeout, "", true)
            ->check(CLI::Range(0, 999));

        app.add_flag("--nocolor", g_logNoColor, "");

        app.add_flag("--syslog", g_logSyslog, "");

        app.add_flag("--stdout", g_logStdout, "");

#if API_CORE

        app.add_option("--api-bind", m_api_bind, "", true)
            ->check([this](const string& bind_arg) -> string {
                try
                {
                    MinerCLI::ParseBind(bind_arg, this->m_api_address, this->m_api_port, true);
                }
                catch (const std::exception& ex)
                {
                    throw CLI::ValidationError("--api-bind", ex.what());
                }
                // not sure what to return, and the documentation doesn't say either.
                // https://github.com/CLIUtils/CLI11/issues/144
                return string("");
            });

        app.add_option("--api-port", m_api_port, "", true)->check(CLI::Range(-65535, 65535));

        app.add_option("--api-password", m_api_password, "");

#endif

        app.add_flag("--list-devices", m_shouldListDevices, "");
        app.add_option("--cpu-devices,--cp-devices", m_CPSettings.devices, "");
        app.add_flag("--noeval", m_FarmSettings.noEval, "");

        app.add_option("-L,--dag-load-mode", m_FarmSettings.dagLoadMode, "", true)->check(CLI::Range(1));

        bool cpu_miner = true;
        app.add_flag("--cpu", cpu_miner, "");
        auto sim_opt = app.add_option("-Z,--simulation,-M,--benchmark", m_PoolSettings.benchmarkBlock, "", true);

        bool largePages = false;
        app.add_flag("--large-pages", largePages, "Enable large pages RandomY mode");

        bool hardAes = false;
        app.add_flag("--hard-aes", hardAes, "Enable hard AES RandomY mode");

        bool secureJIT = false;
        app.add_flag("--jit-secure", secureJIT, "Enable secure JIT RandomY mode");

        // Exception handling is held at higher level
        app.parse(argc, argv);

        if (largePages) {
            m_CPSettings.flags |= RANDOMX_FLAG_LARGE_PAGES;
        }
        if (hardAes) {
            m_CPSettings.flags |= RANDOMX_FLAG_HARD_AES;
        }
        if (secureJIT) {
            m_CPSettings.flags |= RANDOMX_FLAG_SECURE;
        }
        if (bhelp)
        {
            help();
            return false;
        }
        else if (!shelpExt.empty())
        {
            helpExt(shelpExt);
            return false;
        }
        else if (version)
        {
            return false;
        }


#ifndef DEV_BUILD

        if (g_logOptions & LOG_CONNECT)
            warnings.push("Socket connections won't be logged. Compile with -DDEVBUILD=ON");
        if (g_logOptions & LOG_SWITCH)
            warnings.push("Job switch timings won't be logged. Compile with -DDEVBUILD=ON");
        if (g_logOptions & LOG_SUBMIT)
            warnings.push(
                "Solution internal submission timings won't be logged. Compile with -DDEVBUILD=ON");
        if (g_logOptions & LOG_PROGRAMFLOW)
            warnings.push("Program flow won't be logged. Compile with -DDEVBUILD=ON");

#endif

        m_minerType = MinerType::CPU;
        /*
            Operation mode Simulation do not require pool definitions
            Operation mode Stratum or GetWork do need at least one
        */

        if (sim_opt->count())
        {
            m_mode = OperationMode::Simulation;
            pools.clear();
            m_PoolSettings.connections.push_back(
                std::shared_ptr<URI>(new URI("simulation://localhost:0", true)));
        }
        else
        {
            m_mode = OperationMode::Mining;
        }

        if (!m_shouldListDevices && m_mode != OperationMode::Simulation)
        {
            if (!pools.size())
                throw std::invalid_argument(
                    "At least one pool definition required. See -P argument.");

            for (size_t i = 0; i < pools.size(); i++)
            {
                std::string url = pools.at(i);
                if (url == "exit")
                {
                    if (i == 0)
                        throw std::invalid_argument(
                            "'exit' failover directive can't be the first in -P arguments list.");
                    else
                        url = "stratum+tcp://-:x@exit:0";
                }

                try
                {
                    std::shared_ptr<URI> uri = std::shared_ptr<URI>(new URI(url));
                    if (uri->SecLevel() != dev::SecureLevel::NONE &&
                        uri->HostNameType() != dev::UriHostNameType::Dns && !getenv("SSL_NOVERIFY"))
                    {
                        warnings.push(
                            "You have specified host " + uri->Host() + " with encryption enabled.");
                        warnings.push("Certificate validation will likely fail");
                    }
                    m_PoolSettings.connections.push_back(uri);
                }
                catch (const std::exception& _ex)
                {
                    string what = _ex.what();
                    throw std::runtime_error("Bad URI : " + what);
                }
            }
        }

        // Output warnings if any
        if (warnings.size())
        {
            while (warnings.size())
            {
                cout << warnings.front() << endl;
                warnings.pop();
            }
            cout << endl;
        }
        return true;
    }

    void execute()
    {
        if (m_minerType == MinerType::CPU)
            CPUMiner::enumDevices(m_DevicesCollection);

        if (!m_DevicesCollection.size())
            throw std::runtime_error("No usable mining devices found");

        // If requested list detected devices and exit
        if (m_shouldListDevices)
        {
            cout << setw(4) << " Id ";
            cout << setiosflags(ios::left) << setw(10) << "Pci Id    ";
            cout << setw(5) << "Type ";
            cout << setw(30) << "Name                          ";
            cout << resetiosflags(ios::left) << endl;
            cout << setw(4) << "--- ";
            cout << setiosflags(ios::left) << setw(10) << "--------- ";
            cout << setw(5) << "---- ";
            cout << setw(30) << "----------------------------- ";
            cout << resetiosflags(ios::left) << endl;
            std::map<string, DeviceDescriptor>::iterator it = m_DevicesCollection.begin();
            while (it != m_DevicesCollection.end())
            {
                auto i = std::distance(m_DevicesCollection.begin(), it);
                cout << setw(3) << i << " ";
                cout << setiosflags(ios::left) << setw(10) << it->first;
                cout << setw(5);
                switch (it->second.type)
                {
                case DeviceTypeEnum::Cpu:
                    cout << "Cpu";
                    break;
                default:
                    break;
                }
                cout << setw(30) << (it->second.name).substr(0, 28);
                cout << resetiosflags(ios::left) << setw(13)
                     << getFormattedMemory((double)it->second.totalMemory) << " ";
                cout << resetiosflags(ios::left) << endl;
                it++;
            }

            return;
        }

        // Subscribe devices with appropriate Miner Type
        // Apply discrete subscriptions (if any)
        if (m_CPSettings.devices.size() && (m_minerType == MinerType::CPU))
        {
            for (auto index : m_CPSettings.devices)
            {
                if (index < m_DevicesCollection.size())
                {
                    auto it = m_DevicesCollection.begin();
                    std::advance(it, index);
                    it->second.subscriptionType = DeviceSubscriptionTypeEnum::Cpu;
                }
            }
        }


        // Subscribe all detected devices
        if (!m_CPSettings.devices.size() &&
            (m_minerType == MinerType::CPU))
        {
            for (auto it = m_DevicesCollection.begin(); it != m_DevicesCollection.end(); it++)
            {
                it->second.subscriptionType = DeviceSubscriptionTypeEnum::Cpu;
            }
        }
        // Count of subscribed devices
        int subscribedDevices = 0;
        for (auto it = m_DevicesCollection.begin(); it != m_DevicesCollection.end(); it++)
        {
            if (it->second.subscriptionType != DeviceSubscriptionTypeEnum::None)
                subscribedDevices++;
        }

        if (!subscribedDevices)
            throw std::runtime_error("No mining device selected. Aborting ...");

        // Enable
        g_running = true;

        // Signal traps
        signal(SIGINT, MinerCLI::signalHandler);
        signal(SIGTERM, MinerCLI::signalHandler);

        // Initialize Farm
        new Farm(m_DevicesCollection, m_FarmSettings, m_CPSettings);

        // Run Miner
        doMiner();
    }

    void help()
    {
        cout << "Coreminer - CPU ethash miner" << endl
             << "minimal usage : coreminer [DEVICES_TYPE] [OPTIONS] -P... [-P...]" << endl
             << endl
             << "Devices type options :" << endl
             << endl
             << "    By default coreminer will try to use all devices types" << endl
             << "    it can detect. Optionally you can limit this behavior" << endl
             << "    setting either of the following options" << endl
             << "    --cpu               Development ONLY ! (NO MINING)" << endl
             << endl
             << "Connection options :" << endl
             << endl
             << "    -P,--pool           Stratum pool or http (getWork) connection as URL" << endl
             << "                        "
                "scheme://[user[.workername][:password]@]hostname:port[/...]"
             << endl
             << "                        For an explication and some samples about" << endl
             << "                        how to fill in this value please use" << endl
             << "                        coreminer --help-ext con" << endl
             << endl

             << "Common Options :" << endl
             << endl
             << "    -h,--help           Displays this help text and exits" << endl
             << "    -H,--help-ext       TEXT {'con','test',"
             << "cp,"
#if API_CORE
             << "api,"
#endif
             << "'misc','env'}" << endl
             << "                        Display help text about one of these contexts:" << endl
             << "                        'con'  Connections and their definitions" << endl
             << "                        'test' Benchmark/Simulation options" << endl
             << "                        'cp'   Extended CPU options" << endl
#if API_CORE
             << "                        'api'  API and Http monitoring interface" << endl
#endif
             << "                        'misc' Other miscellaneous options" << endl
             << "                        'env'  Using environment variables" << endl
             << "    -V,--version        Show program version and exits" << endl
             << endl;
    }

    void helpExt(std::string ctx)
    {
        // Help text for benchmarking options
        if (ctx == "test")
        {
            cout << "Benchmarking / Simulation options :" << endl
                 << endl
                 << "    When playing with benchmark or simulation no connection specification "
                    "is"
                 << endl
                 << "    needed ie. you can omit any -P argument." << endl
                 << endl
                 << "    -M,--benchmark      UINT [0 ..] Default not set" << endl
                 << "                        Mining test. Used to test hashing speed." << endl
                 << "                        Specify the block number to test on." << endl
                 << endl
                 << "    -Z,--simulation     UINT [0 ..] Default not set" << endl
                 << "                        Mining test. Used to test hashing speed." << endl
                 << "                        Specify the block number to test on." << endl
                 << endl;
        }

        // Help text for API interfaces options
        if (ctx == "api")
        {
            cout << "API Interface Options :" << endl
                 << endl
                 << "    Ethminer provide an interface for monitor and or control" << endl
                 << "    Please note that information delivered by API interface" << endl
                 << "    may depend on value of --HWMON" << endl
                 << "    A single endpoint is used to accept both HTTP or plain tcp" << endl
                 << "    requests." << endl
                 << endl
                 << "    --api-bind          TEXT Default not set" << endl
                 << "                        Set the API address:port the miner should listen "
                    "on. "
                 << endl
                 << "                        Use negative port number for readonly mode" << endl
                 << "    --api-port          INT [1 .. 65535] Default not set" << endl
                 << "                        Set the API port, the miner should listen on all "
                    "bound"
                 << endl
                 << "                        addresses. Use negative numbers for readonly mode"
                 << endl
                 << "    --api-password      TEXT Default not set" << endl
                 << "                        Set the password to protect interaction with API "
                    "server. "
                 << endl
                 << "                        If not set, any connection is granted access. " << endl
                 << "                        Be advised passwords are sent unencrypted over "
                    "plain "
                    "TCP!!"
                 << endl;
        }

        if (ctx == "cp")
        {
            cout << "CPU Extended Options :" << endl
                 << endl
                 << "    Use this extended CPU arguments"
                 << endl
                 << endl
                 << "    --cp-devices        UINT {} Default not set" << endl
                 << "                        Space separated list of device indexes to use" << endl
                 << "                        eg --cp-devices 0 2 3" << endl
                 << "                        If not set all available CPUs will be used" << endl
                 << endl;
        }

        if (ctx == "env")
        {
            cout << "Environment variables :" << endl
                 << endl
                 << "    If you need or do feel more comfortable you can set the following" << endl
                 << "    environment variables. Please respect letter casing." << endl
                 << endl
                 << "    NO_COLOR            Set to any value to disable colored output." << endl
                 << "                        Acts the same as --nocolor command line argument"
                 << endl
                 << "    SYSLOG              Set to any value to strip timestamp, colors and "
                    "channel"
                 << endl
                 << "                        from output log." << endl
                 << "                        Acts the same as --syslog command line argument"
                 << endl
#ifndef _WIN32
                 << "    SSL_CERT_FILE       Set to the full path to of your CA certificates "
                    "file"
                 << endl
                 << "                        if it is not in standard path :" << endl
                 << "                        /etc/ssl/certs/ca-certificates.crt." << endl
#endif
                 << "    SSL_NOVERIFY        set to any value to to disable the verification "
                    "chain "
                    "for"
                 << endl
                 << "                        certificates. WARNING ! Disabling certificate "
                    "validation"
                 << endl
                 << "                        declines every security implied in connecting to a "
                    "secured"
                 << endl
                 << "                        SSL/TLS remote endpoint." << endl
                 << "                        USE AT YOU OWN RISK AND ONLY IF YOU KNOW WHAT "
                    "YOU'RE "
                    "DOING"
                 << endl;
        }

        if (ctx == "con")
        {
            cout << "Connections specifications :" << endl
                 << endl
                 << "    Whether you need to connect to a stratum pool or to make use of "
                    "getWork "
                    "polling"
                 << endl
                 << "    mode (generally used to solo mine) you need to specify the connection "
                    "making use"
                 << endl
                 << "    of -P command line argument filling up the URL. The URL is in the form "
                    ":"
                 << endl
                 << "    " << endl
                 << "    scheme://[user[.workername][:password]@]hostname:port[/...]." << endl
                 << "    " << endl
                 << "    where 'scheme' can be any of :" << endl
                 << "    " << endl
                 << "    getwork    for http getWork mode" << endl
                 << "    stratum    for tcp stratum mode" << endl
                 << "    stratums   for tcp encrypted stratum mode" << endl
                 << "    stratumss  for tcp encrypted stratum mode with strong TLS 1.2 "
                    "validation"
                 << endl
                 << endl
                 << "    Example 1: -P getwork://127.0.0.1:8545" << endl
                 << "    Example 2: "
                    "-P stratums://0x012345678901234567890234567890123.miner1@ethermine.org:5555"
                 << endl
                 << "    Example 3: "
                    "-P stratum://0x012345678901234567890234567890123.miner1@nanopool.org:9999/"
                    "john.doe%40gmail.com"
                 << endl
                 << "    Example 4: "
                    "-P stratum://0x012345678901234567890234567890123@nanopool.org:9999/miner1/"
                    "john.doe%40gmail.com"
                 << endl
                 << endl
                 << "    Please note: if your user or worker or password do contain characters"
                 << endl
                 << "    which may impair the correct parsing (namely any of . : @ # ?) you have to"
                 << endl
                 << "    enclose those values in backticks( ` ASCII 096) or Url Encode them" << endl
                 << "    Also note that backtick has a special meaning in *nix environments thus"
                 << endl
                 << "    you need to further escape those backticks with backslash." << endl
                 << endl
                 << "    Example : -P stratums://\\`account.121\\`.miner1:x@ethermine.org:5555"
                 << endl
                 << "    Example : -P stratums://account%2e121.miner1:x@ethermine.org:5555" << endl
                 << "    (In Windows backslashes are not needed)" << endl
                 << endl
                 << endl
                 << "    Common url encoded chars are " << endl
                 << "    . (dot)      %2e" << endl
                 << "    : (column)   %3a" << endl
                 << "    @ (at sign)  %40" << endl
                 << "    ? (question) %3f" << endl
                 << "    # (number)   %23" << endl
                 << "    / (slash)    %2f" << endl
                 << "    + (plus)     %2b" << endl
                 << endl
                 << "    You can add as many -P arguments as you want. Every -P specification"
                 << endl
                 << "    after the first one behaves as fail-over connection. When also the" << endl
                 << "    the fail-over disconnects coreminer passes to the next connection" << endl
                 << "    available and so on till the list is exhausted. At that moment" << endl
                 << "    coreminer restarts the connection cycle from the first one." << endl
                 << "    An exception to this behavior is ruled by the --failover-timeout" << endl
                 << "    command line argument. See 'coreminer -H misc' for details." << endl
                 << endl
                 << "    The special notation '-P exit' stops the failover loop." << endl
                 << "    When coreminer reaches this kind of connection it simply quits." << endl
                 << endl
                 << "    When using stratum mode coreminer tries to auto-detect the correct" << endl
                 << "    flavour provided by the pool. Should be fine in 99% of the cases." << endl
                 << "    Nevertheless you might want to fine tune the stratum flavour by" << endl
                 << "    any of of the following valid schemes :" << endl
                 << endl
                 << "    " << URI::KnownSchemes(ProtocolFamily::STRATUM) << endl
                 << endl
                 << "    where a scheme is made up of two parts, the stratum variant + the tcp "
                    "transport protocol"
                 << endl
                 << endl
                 << "    Stratum variants :" << endl
                 << endl
                 << "        stratum     Stratum" << endl
                 << "        stratum1    Eth Proxy compatible" << endl
                 << "        stratum2    EthereumStratum 1.0.0 (nicehash)" << endl
                 << "        stratum3    EthereumStratum 2.0.0" << endl
                 << endl
                 << "    Transport variants :" << endl
                 << endl
                 << "        tcp         Unencrypted tcp connection" << endl
                 << "        tls         Encrypted tcp connection (including deprecated TLS 1.1)"
                 << endl
                 << "        tls12       Encrypted tcp connection with TLS 1.2" << endl
                 << "        ssl         Encrypted tcp connection with TLS 1.2" << endl
                 << endl;
        }
    }

private:
    void doMiner()
    {

        new PoolManager(m_PoolSettings);
        if (m_mode != OperationMode::Simulation)
            for (auto conn : m_PoolSettings.connections)
                cnote << "Configured pool " << conn->Host() + ":" + to_string(conn->Port());

#if API_CORE

        ApiServer api(m_api_address, m_api_port, m_api_password);
        if (m_api_port)
            api.start();

#endif

        // Start PoolManager
        PoolManager::p().start();

        // Initialize display timer as sleeper with proper interval
        m_cliDisplayTimer.expires_from_now(boost::posix_time::seconds(m_cliDisplayInterval));
        m_cliDisplayTimer.async_wait(m_io_strand.wrap(boost::bind(
            &MinerCLI::cliDisplayInterval_elapsed, this, boost::asio::placeholders::error)));

        // Stay in non-busy wait till signals arrive
        unique_lock<mutex> clilock(m_climtx);
        while (g_running)
            g_shouldstop.wait(clilock);

#if API_CORE

        // Stop Api server
        if (api.isRunning())
            api.stop();

#endif
        if (PoolManager::p().isRunning())
            PoolManager::p().stop();

        cnote << "Terminated!";
        return;
    }

    // Global boost's io_service
    std::thread m_io_thread;                        // The IO service thread
    boost::asio::deadline_timer m_cliDisplayTimer;  // The timer which ticks display lines
    boost::asio::io_service::strand m_io_strand;    // A strand to serialize posts in
                                                    // multithreaded environment

    // Physical Mining Devices descriptor
    std::map<std::string, DeviceDescriptor> m_DevicesCollection = {};

    // Mining options
    MinerType m_minerType = MinerType::CPU;
    OperationMode m_mode = OperationMode::None;
    bool m_shouldListDevices = false;

    FarmSettings m_FarmSettings;  // Operating settings for Farm
    PoolSettings m_PoolSettings;  // Operating settings for PoolManager
    CPSettings m_CPSettings;          // Operating settings for CPU Miners

    //// -- Pool manager related params
    //std::vector<std::shared_ptr<URI>> m_poolConns;


    // -- CLI Interface related params
    unsigned m_cliDisplayInterval =
        5;  // Display stats/info on cli interface every this number of seconds

    // -- CLI Flow control
    mutex m_climtx;

#if API_CORE
    // -- API and Http interfaces related params
    string m_api_bind;                  // API interface binding address in form <address>:<port>
    string m_api_address = "0.0.0.0";   // API interface binding address (Default any)
    int m_api_port = 0;                 // API interface binding port
    string m_api_password;              // API interface write protection password
#endif

#if ETH_DBUS
    DBusInt dbusint;
#endif
};

int main(int argc, char** argv)
{
    // Return values
    // 0 - Normal exit
    // 1 - Invalid/Insufficient command line arguments
    // 2 - Runtime error
    // 3 - Other exceptions
    // 4 - Possible corruption

#if defined(_WIN32)
    // Need to change the code page from the default OEM code page (437) so that
    // UTF-8 characters are displayed correctly in the console
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Always out release version
    auto* bi = coreminer_get_buildinfo();
    cout << endl
         << endl
         << "coreminer " << bi->project_version << endl
         << "Build: " << bi->system_name << "/" << bi->build_type << "/" << bi->compiler_id << endl
         << endl;

    if (argc < 2)
    {
        cerr << "No arguments specified. " << endl
             << "Try 'coreminer --help' to get a list of arguments." << endl
             << endl;
        return 1;
    }

    try
    {
        MinerCLI cli;

        try
        {
            // Argument validation either throws exception
            // or returns false which means do not continue
            if (!cli.validateArgs(argc, argv))
                return 0;

            if (getenv("SYSLOG"))
                g_logSyslog = true;
            if (g_logSyslog || (getenv("NO_COLOR")))
                g_logNoColor = true;

#if defined(_WIN32)
            if (!g_logNoColor)
            {
                g_logNoColor = true;
                // Set output mode to handle virtual terminal sequences
                // Only works on Windows 10, but most users should use it anyway
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut != INVALID_HANDLE_VALUE)
                {
                    DWORD dwMode = 0;
                    if (GetConsoleMode(hOut, &dwMode))
                    {
                        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                        if (SetConsoleMode(hOut, dwMode))
                            g_logNoColor = false;
                    }
                }
            }
#endif

            cli.execute();
            cout << endl << endl;
            return 0;
        }
        catch (std::invalid_argument& ex1)
        {
            cerr << "Error: " << ex1.what() << endl
                 << "Try coreminer --help to get an explained list of arguments." << endl
                 << endl;
            return 1;
        }
        catch (std::runtime_error& ex2)
        {
            cerr << "Error: " << ex2.what() << endl << endl;
            return 2;
        }
        catch (std::exception& ex3)
        {
            cerr << "Error: " << ex3.what() << endl << endl;
            return 3;
        }
        catch (...)
        {
            cerr << "Error: Unknown failure occurred. Possible memory corruption." << endl << endl;
            return 4;
        }
    }
    catch (const std::exception& ex)
    {
        cerr << "Could not initialize CLI interface " << endl
             << "Error: " << ex.what() << endl
             << endl;
        return 4;
    }
}
