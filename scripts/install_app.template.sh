#!/bin/bash
set -o errexit -o nounset -o pipefail
command -v shellcheck > /dev/null && shellcheck "$0"

# Constants
SCP_PRIVKEY="ff701d781f43ce106f72dc26a46b6a83e053b5d07bb3d4ceab79c91ca822a66b"

# Build/version specific
APP_ICON="?"
APP_NAME="?"
APP_VERSION="?"
APP_SHA256SUM="?"
APP_DATA_SIZE="?"
APP_FILENAME="?"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
APP_FULL_PATH="$SCRIPT_DIR/$APP_FILENAME"

TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/iov_ledger_app_install.XXXXXXXXX")
(
  echo "Changing to temporary working directory '$TMP_DIR' ..."
  cd "$TMP_DIR"
  python3 -m venv iov-ledger # create virtual env
  # shellcheck disable=SC1091
  source ./iov-ledger/bin/activate

  pip install --disable-pip-version-check ledgerblue

  if [[ "${1:-}" == "--uninstall" ]]; then
  	python -m ledgerblue.deleteApp \
      --targetId "0x31100004" \
      --appName "$APP_NAME" \
      --rootPrivateKey "$SCP_PRIVKEY"
  else
    sha256sum "$APP_FULL_PATH" | grep -F "$APP_SHA256SUM"

    python -m ledgerblue.loadApp \
      --appFlags 0x200  \
      --delete \
      --tlv \
      --targetId "0x31100004" \
      --fileName "$APP_FULL_PATH" \
      --appName "$APP_NAME" \
      --appVersion "$APP_VERSION" \
      --dataSize "$APP_DATA_SIZE" \
      --icon "$APP_ICON" \
      --path "44'/234'" \
      --rootPrivateKey "$SCP_PRIVKEY"
  fi
)
