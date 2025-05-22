# CoreMiner Autostart

You can configure CoreMiner to start automatically on Linux distributions using the `systemd` service manager.

## Installation

Follow these steps to set up automatic startup. The steps may vary depending on your operating system.

You can choose one of the following installation methods:

- [CoreMiner Autostart](#coreminer-autostart)
  - [Installation](#installation)
  - [Manual Installation](#manual-installation)
    - [Optional Steps](#optional-steps)
  - [Automatic Installation](#automatic-installation)
  - [Troubleshooting](#troubleshooting)

## Manual Installation

1. Download the latest release of CoreMiner and run the initial setup using the `mine.sh` script.
2. Create a systemd service file with the following content. Replace `WorkingDirectory` with your miner location and `ExecStart` with the `mine.sh` script location. (The example shows a fresh Kali Linux installation.) Use the `pwd` command to print your current directory.

   ```ini
   [Unit]
   Description=CoreMiner Service
   After=network.target
   StartLimitIntervalSec=0

   [Service]
   Type=simple
   WorkingDirectory=$(pwd)
   ExecStart=/bin/bash $(pwd)/mine.sh
   Restart=always
   RestartSec=3
   TimeoutStartSec=0

   [Install]
   WantedBy=multi-user.target
   ```

3. Save the service file as `coreminer.service` in `/etc/systemd/system/`.
4. Reload the systemd daemon:

   ```bash
   sudo systemctl daemon-reload
   ```

5. Enable the service:

   ```bash
   sudo systemctl enable coreminer.service
   ```

6. Start the service:

   ```bash
   sudo systemctl start coreminer.service
   ```

7. Restart your machine (optional but recommended).

### Optional Steps

1. Verify the service is registered:

   ```bash
   systemctl --all | grep coreminer.service
   ```

2. Configure log rotation to keep logs for one day:

   ```bash
   sudo journalctl --rotate && journalctl --vacuum-time=1d
   ```

## Automatic Installation

The automatic installation is included in the `mine.sh` script. You can use it for both initial setup and reinstallation.

## Troubleshooting

1. Check the service logs for detailed information:

   ```bash
   journalctl -u coreminer.service
   ```

2. If issues persist:
   - Open a thread in the [discussion board](https://github.com/catchthatrabbit/coreminer/discussions)
   - Or [raise an issue](https://github.com/catchthatrabbit/coreminer/issues/new/choose)
