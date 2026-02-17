#!/usr/bin/env bash
set -euo pipefail

# This script is supposed to be under ${REPO_PATH}/tools/init.sh

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

echo "Script directory: $SCRIPT_DIR"
echo "Repository directory: $REPO_DIR"

cd "$REPO_DIR"
python3 -m venv virtual
source ./virtual/bin/activate
python -m pip install --upgrade pip
pip install -r requirements.txt

git submodule update --init
./deps/ST-LIB/tools/init-submodules.sh

echo "Setup complete."
