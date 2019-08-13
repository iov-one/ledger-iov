#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

#
# Setup python
#
python --version
pip --version

#
# Install dependencies
#
case "$(uname -s)" in
  Linux*)
    sudo apt-get update
    sudo apt-get install -y build-essential git wget cmake libssl-dev libgmp-dev autoconf libtool
    make deps
    ;;
  Darwin*)
    brew install libusb
    make deps
    ;;
  *)
    echo "OS not recognized"
    ;;
esac

#
# Build Ledger app
#
cmake .
make

#
# Run tests
#
export GTEST_COLOR=1
ctest -VV
