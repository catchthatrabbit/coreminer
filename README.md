# Core Miner

> Core CPU miner with stratum support

**Coreminer** is an RandomY CPU mining worker: with Coreminer you can mine every coin which relies on an RandomY Proof of Work.

## Table of Contents

* [Install](#install)
* [Usage](#usage)
* [Build](#build)

## Install

Standalone **executables** for *Linux*, *macOS* and *Windows* are provided in
the [Releases](https://github.com/catchthatrabbit/coreminer/releases) section.
Download an archive for your operating system and unpack the content to a place
accessible from command line. The coreminer is ready to go.

## Usage

The **Coreminer** is a command line program. This means you launch it either
from a Windows command prompt or Linux console, or create shortcuts to
predefined command lines using a Linux Bash script or Windows batch/cmd file.
For a full list of available command, please run:

```sh
coreminer --help
```

We are providing additional scripts to start mining easily:

- `entry.sh` check for large pages, hard aes & mine
- `mining.sh` mining inputs, additional checks of parameters & mine
- `pool.sh` additional checks of parameters & mine with docker

### How to start mining

Check our [instruction](https://catchthatrabbit.com/start-mining) to start mining process.

## Build

### Building from source

See [docs/BUILD.md](docs/BUILD.md) for build/compilation details.

## License

Licensed under the [GNU General Public License, Version 3](LICENSE).
