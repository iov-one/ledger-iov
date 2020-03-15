# Ledger-IOV

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Build Status](https://travis-ci.com/iov-one/ledger-iov.svg?branch=master)](https://travis-ci.com/iov-one/ledger-iov)

This repository contains the Ledger Nano S app for IOV (BOLOS app and unit tests).

For development purposes, this repo is recommended as it includes unit tests, tools, etc.

## Installing the release

> **Warning** The only safe and convenient way to install apps on the Leder is via the Ledger Live Store. Those instructions are meant for testers and developers only. We recommend dedicated testing devices.

### Prerequisites

- You run some kind of UNIX-like environment
- Python3 and venv module installed (check via `python3 --version`, `python3 -m venv --help`)
- A C compiler

#### Debian/Ubuntu

1. Ensure to add Universe to your apt repositories
`sudo apt install build-essential python3-dev python3-venv libusb-1.0-0-dev libudev-dev`
2. `wget https://raw.githubusercontent.com/LedgerHQ/udev-rules/master/20-hw1.rules`
3. `sudo mv 20-hw1.rules /etc/udev/rules.d/20-hw1.rules`
4. `sudo udevadm trigger && sudo udevadm control --reload-rules`

#### MacOS

1. Install dependencies: `brew bundle`
2. Install [XCode](https://apps.apple.com/us/app/xcode/id497799835) and XCode command line tools `xcode-select --install`

### Download

Find a recent release from [ledger-iov releases](https://github.com/iov-one/ledger-iov/releases) that includes a zip package (`e.g. iov-testnet-ledger-0.10.0+9.zip`). The mainnet/testnet parts in the file name tell you for which network the app works. You can install both apps in parallel.

Download and extract.

### Install / Uninstall

1. Navigate to the extracted folder, e.g. `cd ~/Downloads/iov-testnet-ledger`
2. Connect and unlock Ledger Nano S; navigate to the main menu.
3. To install run `./install_app.sh`; to uninstall run `./install_app.sh --uninstall`

### Further notes

The `+xyz` suffix in versions is build meta data, i.e. the same Ledger app in a different package

## Ledger Nano X support

Right now, there is no support for the Ledger Nano X. However, the source code is prepared for future Nano X support,
which is why there are Nano X references in some places.

## Building

The following document describes how to build the apps: [Build instructions](docs/BUILD.md)

## Specifications

- [APDU Protocol](https://github.com/iov-one/ledger-iov-app/tree/master/docs/APDUSPEC.md)
