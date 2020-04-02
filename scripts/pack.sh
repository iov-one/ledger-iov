#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

NET="$1"
VERSION="$2"

# Get via `make dev_read_icon`
APP_ICON="0100000000ffffff00ffffffffffff7ffebffd9ff9fffffffffffff3cffbdfdbdbc3c3ffffffffffff"
if [[ "$NET" == "mainnet" ]]; then
  APP_NAME="IOV"
else
  APP_NAME="IOVTEST"
fi
APP_FILENAME="iov-$NET.hex"
APP_VERSION=$(echo "$VERSION" | cut -d "+" -f 1)
APP_SHA256SUM=$(sha256sum app/bin/app.hex | cut -d " " -f 1)
APP_DATA_SIZE=$((0x$(< app/debug/app.map grep _envram_data | tr -s ' ' | cut -f2 -d' '|cut -f2 -d'x') - 
                 0x$(< app/debug/app.map grep _nvram_data | tr -s ' ' | cut -f2 -d' '|cut -f2 -d'x')))

rm -rf "out/iov-$NET-ledger"
mkdir -p "out/iov-$NET-ledger"
cp app/bin/app.hex "out/iov-$NET-ledger/$APP_FILENAME"

sed \
  -e "s|APP_ICON=.*|APP_ICON=\"$APP_ICON\"|" \
  -e "s|APP_NAME=.*|APP_NAME=\"$APP_NAME\"|" \
  -e "s|APP_VERSION=.*|APP_VERSION=\"$APP_VERSION\"|" \
  -e "s|APP_SHA256SUM=.*|APP_SHA256SUM=\"$APP_SHA256SUM\"|" \
  -e "s|APP_DATA_SIZE=.*|APP_DATA_SIZE=\"$APP_DATA_SIZE\"|" \
  -e "s|APP_FILENAME=.*|APP_FILENAME=\"$APP_FILENAME\"|" \
  scripts/install_app.template.sh > "out/iov-$NET-ledger/install_app.sh"
shellcheck "out/iov-$NET-ledger/install_app.sh"
chmod +x "out/iov-$NET-ledger/install_app.sh"

(
  cd out
  zip -r "iov-$NET-ledger-$VERSION.zip" "iov-$NET-ledger"
)
