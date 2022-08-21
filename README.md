# Core Miner

> Core CPU miner with stratum support

**Coreminer** is an RandomY CPU mining worker: with Coreminer you can mine every coin which relies on an RandomY Proof of Work.

## Table of Contents

* [Install](#install)
* [Usage](#usage)
* [Limitations](#limitations)
* [Build](#build)
* [Pools](#pools)

## Install

Standalone **executables** for *Linux*, *macOS* and *Windows* are provided in
the [Releases](https://github.com/catchthatrabbit/coreminer/releases) section.
Download an archive for your operating system and unpack the content to a place
accessible from command line. The Coreminer is ready to go.

## Usage

The **Coreminer** is a command line program. This means you launch it either
from a Windows command prompt or Linux console, or create shortcuts to
predefined command lines using a Linux Bash script or Windows batch/cmd file.
For a full list of available command, please run:

```sh
coreminer --help
```

We are providing additional scripts to start mining easily:

- `mining.sh` mining inputs, additional checks of parameters & mine
- `pool.sh` additional checks of parameters & mine with docker

### How to start mining

1. Download latest release of [Coreminer](https://github.com/catchthatrabbit/coreminer/releases).
1. You have several options how to run it: `standalone`, `docker container`

#### Standalone

1. Navigate to the directory, where is `Coreminer` with `cd …` command.
1. Make the script `mine.sh` executable with the command: `chmod +x mine.sh`.
1. Start the mine script in the terminal with `bash mine.sh`.
1. Follow the steps in the terminal.

Optionally:

Follow the [post-installation steps](docs/AUTOSTART-LINUX.md) from documentation.

## Limitations

The Coreminer is released for Linux distributions and require `GLIBC_2.33`.

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
