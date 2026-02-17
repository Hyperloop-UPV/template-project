#!/usr/bin/env python3
"""
Pre-flash check script.
Checks for uncommitted changes when the binary was compiled with BOARD symbol.
"""

import subprocess
import sys
from pathlib import Path

def get_script_dir() -> Path:
    return Path(__file__).parent.resolve()

def get_workspace_dir() -> Path:
    return get_script_dir().parent

def check_board_symbol(build_dir: Path) -> bool:
    """Check if the binary was compiled with BOARD symbol."""
    marker_file = build_dir / "board_build_marker"
    return marker_file.exists()

def has_uncommitted_changes(workspace_dir: Path) -> bool:
    """Check if there are uncommitted changes in the repository."""
    result = subprocess.run(
        ["git", "status", "--porcelain"],
        cwd=workspace_dir,
        capture_output=True,
        text=True
    )
    return bool(result.stdout.strip())

def main():
    workspace_dir = get_workspace_dir()

    # Default build directory - can be overridden by command line argument
    if len(sys.argv) > 1:
        build_dir = Path(sys.argv[1])
    else:
        build_dir = workspace_dir / "out" / "build"

    print(f"Workspace directory: {workspace_dir}")
    print(f"Build directory: {build_dir}")

    # Check if this is a BOARD build
    if not check_board_symbol(build_dir):
        print("Binary was not compiled with BOARD symbol (or marker not found). Skipping pre-flash check.")
        sys.exit(0)

    print("Binary was compiled with BOARD symbol. Checking for uncommitted changes...")

    if has_uncommitted_changes(workspace_dir):
        print("Uncommitted changes detected. Aborting Flash...")
        print("Please before flashing a board make sure all changes are commited");
        sys.exit(1)
    else:
        print("No uncommitted changes. Proceeding with flash.")

    sys.exit(0)

if __name__ == "__main__":
    main()
