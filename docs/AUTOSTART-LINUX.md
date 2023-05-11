# Coreminer Autostart

You can autostart the miner on Linux distribution using unit `service` under the [Systemd](https://en.wikipedia.org/wiki/Systemd) software suite.

## Installation

Please, follow few steps, which can differ depends on your OS used.

You can choose one of the installation types:
- [Coreminer Autostart](#coreminer-autostart)
  - [Installation](#installation)
  - [Manual Installation](#manual-installation)
    - [Optional steps](#optional-steps)
  - [Automatic Installation](#automatic-installation)
  - [Troubleshooting](#troubleshooting)

## Manual Installation

1. Download latest release of CoreMiner and initiate the first setup using `mine.sh` script.
1. Please, copy the contents of file  `coreverif.service`. Replace `WorkingDirectory` with your miner location and `ExecStart` with the `mine.sh` script location. (Example is showing fresh Kali linux.) To print your current directory, you can use `pwd` command.

Contents of file `coreverif.service`:

```bash
[Unit]
Description=CoreVerificator
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

1. Create file `coreverif.service` and save it into `/etc/systemd/system/` folder.
1. Execute the command for reloading the client: `sudo systemctl daemon-reload`.
1. Execute the command for enabling the service: `sudo systemctl enable coreverif.service`.
1. Execute the command for starting the service: `sudo systemctl start coreverif.service`.
1. Restart your machine. (Optional, but recommended.)

### Optional steps

1. Check if service is registered with the command: `systemctl --all | grep coreverif.service`.
1. Reduce log files to remember past for 1 day: `sudo journalctl --rotate && journalctl --vacuum-time=1d`.

## Automatic Installation

Automatic installation is included in the `mine.sh` script. You can use it for the first setup or for the reinstallation.

## Troubleshooting

Please, first follow the log file, where you can find many answers: `journalctl -u coreverif.service`.

If your issue persist, open thread in [discussion board](https://github.com/catchthatrabbit/coreminer/discussions) or lastly [raise an Issue](https://github.com/catchthatrabbit/coreminer/issues/new/choose).
