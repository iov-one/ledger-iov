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
# Install apt packages
#
fold_start "install-apt-packages"
if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
  sudo apt-get update
  sudo apt-get install -y python3 python3-pip
fi
fold_end

#
# Setup python
#
fold_start "setup-python"
python3 --version
pip3 --version
fold_end

if [[ "$MODE" == "unit" ]]; then
  #
  # Install dependencies
  #
  fold_start "install-dependencies"
  if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
    sudo apt-get install -y build-essential git wget cmake libssl-dev libgmp-dev autoconf libtool
    sudo make deps
  else
    brew install libusb
    make deps
  fi
  fold_end

  #
  # Build
  #
  fold_start "build"
  cmake -DDISABLE_DOCKER_BUILDS=ON .
  make
  fold_end

  #
  # Run tests
  #
  fold_start "test"
  export GTEST_COLOR=1
  ctest -VV
  fold_end
elif [[ "$MODE" == "ledger" ]]; then
  echo "TODO"
else
  echo "Unsupported MODE value"
  exit 1
fi
