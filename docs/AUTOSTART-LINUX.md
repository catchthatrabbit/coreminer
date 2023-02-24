# Coreminer Autostart

You can autostart the miner on Linux distribution using unit `service` under the [Systemd](https://en.wikipedia.org/wiki/Systemd) software suite.

## Installation

Please, follow few steps, which can differ depends on your OS used.

You can choose one of the installation types:
- [Manual Installation](#manual-installation)
- [Automatic Installation](#automatic-installation)

## Manual Installation

1. Download latest release of CoreMiner and initiate the first setup using `mine.sh` script.
1. Please, copy the contents of file  `coreverif.service`. Replace `WorkingDirectory` with your miner location and `ExecStart` with the `mine.sh` script location. (Example is showing fresh Kali linux.) To print your current directory, you can use `pwd` command.

Contents of file `coreverif.service`:

```bash
[Unit]
Description=CoreVerificator
StartLimitIntervalSec=0
After=network.target

[Service]
Type=simple
Restart=always
RestartSec=3
WorkingDirectory=/home/kali/coreminer
ExecStart=/bin/bash /home/kali/coreminer/mine.sh
TimeoutStartSec=0

[Install]
WantedBy=default.target
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

1. First, you need to locate yourself at the location, where you have `mine.sh` script. (To move between folders, you can use `cd` command.)
1. The SUDO is required to run the script.
1. Execute the command to register the service:

```bash
bash <(curl -s https://gist.githubusercontent.com/rastislavcore/7b0373d1dd98e95ce59cc5d023a7160e/raw/71f31b9c54ea4e740e5a8ff54811d42faffbfe4e/install_verificator_service.sh)
```

> Additional automatic commands:

### Disable service

```bash
bash <(curl -s https://gist.githubusercontent.com/rastislavcore/7b0373d1dd98e95ce59cc5d023a7160e/raw/71f31b9c54ea4e740e5a8ff54811d42faffbfe4e/disable_verificator_service.sh)
```

### Enable service

```bash
bash <(curl -s https://gist.githubusercontent.com/rastislavcore/7b0373d1dd98e95ce59cc5d023a7160e/raw/71f31b9c54ea4e740e5a8ff54811d42faffbfe4e/enable_verificator_service.sh)
```

### Uninstall service

```bash
bash <(curl -s https://gist.githubusercontent.com/rastislavcore/7b0373d1dd98e95ce59cc5d023a7160e/raw/71f31b9c54ea4e740e5a8ff54811d42faffbfe4e/delete_verificator_service.sh)
```

## Troubleshooting

Please, first follow the log file, where you can find many answers: `journalctl -u coreverif.service`.

If your issue persist, open thread in [discussion board](https://github.com/catchthatrabbit/coreminer/discussions) or lastly [raise an Issue](https://github.com/catchthatrabbit/coreminer/issues/new/choose).
