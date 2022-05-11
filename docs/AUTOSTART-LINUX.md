# Coreminer Autostart

You can autostart the miner on Linux distribution using unit `service` under the [Systemd](https://en.wikipedia.org/wiki/Systemd) software suite.

## Installation

Please, follow few steps, which can differ depends on your OS used.

### Required steps

1. Download latest release of CoreMiner and initiate the first setup using `mine.sh` script.
1. Please, copy the contents of file  `coreminer.service`. Replace `WorkingDirectory` with your miner location and `ExecStart` with the `mine.sh` script location. (Example is showing fresh Kali linux.)

Contents of file `coreminer.service`:

```bash
[Unit]
Description=CoreMiner
StartLimitIntervalSec=0
After=network.target

[Service]
Type=simple
Restart=always
RestartSec=3
WorkingDirectory=/home/kali/Desktop/coreminer
ExecStart=/bin/bash /home/kali/Desktop/coreminer/mine.sh
TimeoutStartSec=0

[Install]
WantedBy=default.target
```

1. Create file `coreminer.service` and save it into `/etc/systemd/system/` folder.
1. Execute the command for reloading the client: `sudo systemctl daemon-reload`.
1. Execute the command for enabling the service: `sudo systemctl enable coreminer.service`.
1. Execute the command for starting the service: `sudo systemctl start coreminer.service`.
1. Restart your machine. (Optional, but recommended.)

### Optional steps

1. Check if service is registered with the command: `systemctl --all | grep coreminer.service`.
1. Reduce log files to remember past for 1 day: `sudo journalctl --rotate && journalctl --vacuum-time=1d`.

## Troubleshooting

Please, first follow the log file, where you can find many answers: `journalctl -u coreminer.service`.

If your issue persist, open thread in [discussion board](https://coretalk.info/c/mining/11/) or lastly [raise an Issue](https://github.com/catchthatrabbit/coreminer/issues/new/choose).
