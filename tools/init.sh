#!/usr/bin/env bash
set -euo pipefail

# This script is supposed to be under ${REPO_PATH}/tools/init.sh

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

echo "Script directory: $SCRIPT_DIR"
echo "Repository directory: $REPO_DIR"

cd "$REPO_DIR"
if [[ "${HYPER_SKIP_PYTHON_INIT:-0}" != "1" ]]; then
    python3 -m venv virtual
    source ./virtual/bin/activate
    python -m pip install --upgrade pip
    pip install -r requirements.txt
else
    echo "Skipping Python environment setup: managed by hyper/uv"
fi

while read -r _ submodule_path; do
    if git -C "$submodule_path" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        if [[ -n "$(git -C "$submodule_path" status --porcelain)" ]]; then
            echo "Skipping dirty submodule: $submodule_path"
            continue
        fi
    fi
    git submodule update --init -- "$submodule_path"
done < <(git config --file .gitmodules --get-regexp path)

./deps/ST-LIB/tools/init-submodules.sh

echo "Setup complete."
