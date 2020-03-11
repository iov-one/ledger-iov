#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

#
# Travis helpers
#
function fold_start() {
  export CURRENT_FOLD_NAME="$1"

  if [[ "${TRAVIS_COMMIT:-}" != "" ]]; then
    travis_fold start "$CURRENT_FOLD_NAME"
    travis_time_start "$CURRENT_FOLD_NAME"
  else
    echo "Starting $CURRENT_FOLD_NAME"
  fi
}

function fold_end() {
  if [[ "${TRAVIS_COMMIT:-}" != "" ]]; then
    travis_time_finish "$CURRENT_FOLD_NAME"
    travis_fold end "$CURRENT_FOLD_NAME"
  else
    echo "Done with $CURRENT_FOLD_NAME"
  fi
}

#
# Install apt packages
#
fold_start "install-apt-packages"
if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
  sudo apt-get update
  sudo apt-get install -y \
    python3 python3-pip \
    build-essential git wget cmake libssl-dev libgmp-dev autoconf libtool \
    libusb-1.0.0 libudev-dev
fi
fold_end

#
# Setup python
#
fold_start "setup-python"
if [[ "${TRAVIS_OS_NAME:-}" == "linux" ]]; then
  export PATH="$PWD/scripts/python3-ubuntu:$PATH"
else
  export PATH="/usr/local/opt/python/libexec/bin:$PATH"
fi
python --version
pip --version
fold_end

if [[ "$NET" == "testnet" ]]; then
  export TESTNET_ENABLED=1
fi

if [[ "$MODE" == "unit" ]]; then
  #
  # Install dependencies
  #
  fold_start "install-dependencies"
  make deps
  fold_end

  #
  # Build
  #
  fold_start "build"
  cmake -DDISABLE_DOCKER_BUILDS=ON -DCMAKE_BUILD_TYPE=Release .
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
  #
  # Build
  #
  fold_start "build"
  make
  echo "Debug compile output..."
  sha256sum app/bin/*
  fold_end

  #
  # Pack
  #
  fold_start "pack"
  VERSION=$(echo "${TRAVIS_TAG:-v0.0.0}" | cut -d "v" -f 2 )
  echo "Packing version $VERSION ..."
  ./scripts/pack.sh "$NET" "$VERSION"
  fold_end

  if [[ "$TRAVIS_TAG" != "" ]]; then
    #
    # Export
    #
    fold_start "export"
    echo "Copy to exports dir ..."
    mkdir exports
    cp out/*.zip exports
    sha256sum exports/*
    fold_end
  fi
else
  echo "Unsupported MODE value"
  exit 1
fi
