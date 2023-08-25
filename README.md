# Core Miner

> Core CPU miner with stratum protocol support

**CoreMiner** is a RandomY CPU mining worker - with CoreMiner you can mine every coin which relies on a RandomY Proof of Work.

## Table of Contents

* [Install](#install)
   * [Manual Installation](#manual-installation)
   * [Automatic Installation](#automatic-installation)
* [Usage](#usage)
* [Limitations](#limitations)
* [Post-installation steps](#post-installation-steps)
* [Config file](#config-file)
* [Build](#build)
* [Pools](#pools)

## Install

### Manual Installation

Standalone **executables** for *Linux*, *macOS* and *Windows* are provided in
the [Releases](https://github.com/catchthatrabbit/coreminer/releases) section.
Download an archive for your operating system and unpack the content to a place
accessible from command line. The CoreMiner is ready to go.

### Automatic Installation

You can initiate automatic installation by running the script in a Linux terminal:

```bash
bash <(wget -qO- https://raw.githubusercontent.com/catchthatrabbit/coreminer/master/mine.sh)
```

We are recommending running the [post-installation steps](docs/AUTOSTART-LINUX.md) to remain in high availability.

## Usage

The **CoreMiner** is a command line program. This means you launch it either
from a Windows command prompt or Linux console, or create shortcuts to
predefined command lines using a Linux Bash script or Windows batch/cmd file.
For a full list of available command, please run:

```sh
coreminer --help
```

We are providing additional scripts to start mining easily:

- `mine.sh` update of miner, mining inputs, additional checks of parameters & mine
- `pool.sh` additional checks of parameters & mine with docker

### How to start mining

1. Download latest release of [CoreMiner](https://github.com/catchthatrabbit/coreminer/releases).
1. You have several options how to run it: `standalone`, `docker container`

#### Standalone

1. Navigate to the directory, where is `CoreMiner` with `cd …` command.
1. Make the script `mine.sh` executable with the command: `chmod +x mine.sh`.
1. Start the mine script in the terminal with `bash mine.sh`.
1. Follow the steps in the terminal.

## Limitations

The Coreminer is released for Linux distributions and require `GLIBC_2.33`.

## Post-installation steps

Follow the [post-installation steps](docs/AUTOSTART-LINUX.md) from documentation.

## Config file

After running the `mine.sh` in the same folder produced setting file `pool.cfg`.

This file is loaded from two possible sources:
1. Same directory as `mine.sh` file with name `pool.cfg`.
1. Connected flash drive named `coredrive` and `pool.cfg` is located in the root folder.

Example of the contents of `pool.cfg` file:

```bash
wallet=cb…
worker=Rabbit
server[1]=eu.catchthatrabbit.com
port[1]=8008
server[2]=as.catchthatrabbit.com
port[2]=8008
server[3]=eu1.catchthatrabbit.com
port[3]=8008
server[4]=as1.catchthatrabbit.com
port[4]=8008
```

## Build

### Building from source

See [docs/BUILD.md](docs/BUILD.md) for build/compilation details.

### API documentation

You can read more about [API](docs/API.md) in the documentation.

## Pools

CTR [instructions](https://catchthatrabbit.com/start-mining) to start mining process.

Read more about [Stratum server](docs/STRATUM.md) in the documentation.

## License

Licensed under the [GNU General Public License, Version 3](LICENSE).

*Cryptoni confidimus*
