# ethminer

> Corecoin CPU miner with stratum support

**Corecoin** is an RandomY CPU mining worker: with coreminer you can mine every coin which relies on an RandomY Proof of Work.

## Table of Contents

* [Install](#install)
* [Usage](#usage)
    * [Examples connecting to pools](#examples-connecting-to-pools)
* [Build](#build)
    * [Continuous Integration and development builds](#continuous-integration-and-development-builds)
    * [Building from source](#building-from-source)
* [Contribute](#contribute)
* [F.A.Q.](#faq)


## Install

[![Releases](https://img.shields.io/github/downloads/catchthatrabbit/coreminer/releases/total.svg)][Releases]

Standalone **executables** for *Linux*, *macOS* and *Windows* are provided in
the [Releases] section.
Download an archive for your operating system and unpack the content to a place
accessible from command line. The ethminer is ready to go.

| Builds | Release | Date |
| ------ | ------- | ---- |
| Last   | [![GitHub release](https://img.shields.io/github/release/catchthatrabbit/coreminer/all.svg)](https://github.com/catchthatrabbit/coreminer/releases) | [![GitHub Release Date](https://img.shields.io/github/release-date-pre/catchthatrabbit/coreminer.svg)](https://github.com/ethereum-mining/ethminer/releases) |


## Usage

The **ethminer** is a command line program. This means you launch it either
from a Windows command prompt or Linux console, or create shortcuts to
predefined command lines using a Linux Bash script or Windows batch/cmd file.
For a full list of available command, please run:

```sh
ethminer --help
```

### Examples connecting to pools

Check our [samples](docs/POOL_EXAMPLES_ETH.md) to see how to connect to different pools.

## Build

### Continuous Integration and development builds

![example workflow](https://github.com/catchthatrabbit/coreminer/actions/workflows/build.yml/badge.svg)

| CI            | OS            | Status  | Development builds |
| ------------- | ------------- | -----   | -----------------  |
| [Travis CI]   | Linux, macOS  | [![Travis CI](https://img.shields.io/travis/ethereum-mining/ethminer/master.svg)][Travis CI]    | ✗ No build artifacts, [Amazon S3 is needed] for this |
| [AppVeyor]    | Windows       | [![AppVeyor](https://img.shields.io/appveyor/ci/ethereum-mining/ethminer/master.svg)][AppVeyor] | ✓ Build artifacts available for all PRs and branches |

The AppVeyor system automatically builds a Windows .exe for every commit. The latest version is always available [on the landing page](https://ci.appveyor.com/project/ethereum-mining/ethminer) or you can [browse the history](https://ci.appveyor.com/project/ethereum-mining/ethminer/history) to access previous builds.

### Building from source

See [docs/BUILD.md](docs/BUILD.md) for build/compilation details.


## License

Licensed under the [GNU General Public License, Version 3](LICENSE).


## F.A.Q

[Releases]: https://github.com/catchthatrabbit/coreminer/releases
