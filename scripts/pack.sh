#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

NET="$1"
VERSION="$2"

APP_ICON=$(python ./deps/nanos-secure-sdk/icon.py src/ledger/nanos_icon.gif hexbitmaponly)
if [[ "$NET" == "mainnet" ]]; then
  APP_NAME="IOV"
else
  APP_NAME="IOVTEST"
fi
APP_FILENAME="iov-$NET.hex"
APP_VERSION=$(echo "$VERSION" | cut -d "-" -f 1)
APP_SHA256SUM=$(sha256sum src/ledger/bin/app.hex | cut -d " " -f 1)
APP_DATA_SIZE=$(grep -F _nvram_data_size src/ledger/debug/app.map | tr -s ' ' | cut -f2 -d' ')

rm -rf "out/iov-$NET-ledger"
mkdir -p "out/iov-$NET-ledger"
cp src/ledger/bin/app.hex "out/iov-$NET-ledger/$APP_FILENAME"
cp scripts/install_app.template.sh "out/iov-$NET-ledger/install_app.sh"
chmod +x "out/iov-$NET-ledger/install_app.sh"

(
  cd out

  sed \
    -e "s|APP_ICON=.*|APP_ICON=\"$APP_ICON\"|" \
    -e "s|APP_NAME=.*|APP_NAME=\"$APP_NAME\"|" \
    -e "s|APP_VERSION=.*|APP_VERSION=\"$APP_VERSION\"|" \
    -e "s|APP_SHA256SUM=.*|APP_SHA256SUM=\"$APP_SHA256SUM\"|" \
    -e "s|APP_DATA_SIZE=.*|APP_DATA_SIZE=\"$APP_DATA_SIZE\"|" \
    -e "s|APP_FILENAME=.*|APP_FILENAME=\"$APP_FILENAME\"|" \
    -i "" "iov-$NET-ledger/install_app.sh"

  zip -r "iov-$NET-ledger-$VERSION.zip" "iov-$NET-ledger"
)
