#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

function fold_start() {
  export CURRENT_FOLD_NAME="$1"
  travis_fold start "$CURRENT_FOLD_NAME"
  travis_time_start
}

function fold_end() {
  travis_time_finish
  travis_fold end "$CURRENT_FOLD_NAME"
}

#
# Setup python
#
fold_start "setup-python"
if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
  alias python='/usr/bin/python3'
  alias pip='/usr/bin/pip3'
fi
python --version
pip --version
fold_end

#
# Install dependencies
#
fold_start "install-dependencies"
if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
  sudo apt-get update
  sudo apt-get install -y build-essential git wget cmake libssl-dev libgmp-dev autoconf libtool
  make deps
else
  brew install libusb
  make deps
fi
fold_end

#
# Build Ledger app
#
fold_start "build"
cmake .
make
fold_end

#
# Run tests
#
fold_start "test"
export GTEST_COLOR=1
ctest -VV
fold_end
