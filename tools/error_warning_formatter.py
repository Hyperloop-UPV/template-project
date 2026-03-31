#!/usr/bin/env python3
"""
Git hook for managing ErrorHandler and InfoWarning IDs.
Each call site gets a unique composite ID: [origin_id].[local_id]
Each origin manages its own ID space independently.
"""

import re
import json
import sys
import os
import argparse
from pathlib import Path

# Define the root directory and paths
ROOT_DIR = Path(__file__).parent.parent

# Origin definitions and ID assignment
# Each origin gets a numeric ID prefix (origin_id)
# The origin_id is read from a config file in the origin directory
ORIGINS_CONFIG = {
    "core": {
        "paths": ["Core/"],
        "config_file": "Core/.error_warning.config",
        "registry_file": ".error_warning_registry_core.json",
        "json_output": "Core/Inc/Code_generation/JSON_ADE/errors_warnings.json",
        "description": "Core firmware",
    },
    "st-lib": {
        "paths": ["deps/ST-LIB/"],
        "config_file": "deps/ST-LIB/.error_warning.config",
        "registry_file": "deps/ST-LIB/.error_warning_registry.json",
        "json_output": "deps/ST-LIB/Inc/Code_generation/errors_warnings.json",
        "description": "ST-LIB dependency",
    },
    "lcu": {
        "paths": ["LCU/", "lcu/"],
        "config_file": ".error_warning.config.lcu",
        "registry_file": ".error_warning_registry_lcu.json",
        "json_output": "LCU/Inc/Code_generation/errors_warnings.json",
        "description": "LCU board code",
    },
}

# Patterns for matching ErrorHandler and WARNING macro calls
ERROR_HANDLER_PATTERN = r"ErrorHandler\s*\(\s*([^,)]+)\s*(?:,\s*[^)]*?)?\s*\)"
WARNING_PATTERN = r"WARNING\s*\(\s*([^,)]+)\s*(?:,\s*[^)]*?)?\s*\)"


def get_origin_id(origin):
    """Get the origin_id for a specific origin.

    Lookup order:
    1. Environment variable: ERROR_WARNING_{ORIGIN}_ID (e.g., ERROR_WARNING_CORE_ID)
    2. Config file: {origin}/.error_warning.config
    3. Fallback to default mapping
    """
    # Try environment variable first
    env_var = f"ERROR_WARNING_{origin.upper()}_ID"
    env_value = os.getenv(env_var)
    if env_value is not None:
        try:
            return int(env_value)
        except ValueError:
            print(
                f"Warning: Invalid {env_var}={env_value}, using config file",
                file=sys.stderr,
            )

    # Try config file
    if origin in ORIGINS_CONFIG:
        config_file = ROOT_DIR / ORIGINS_CONFIG[origin]["config_file"]
        if config_file.exists():
            try:
                with open(config_file, "r") as f:
                    content = f.read().strip()
                    # Parse lines like "origin_id=0" or just "0"
                    for line in content.split("\n"):
                        line = line.strip()
                        if line and not line.startswith("#"):
                            if "=" in line:
                                key, value = line.split("=", 1)
                                if key.strip() == "origin_id":
                                    return int(value.strip())
                            else:
                                # Try direct number
                                return int(line)
            except Exception as e:
                print(f"Warning: Failed to read {config_file}: {e}", file=sys.stderr)

    # Fallback default mapping
    default_mapping = {
        "core": 0,
        "st-lib": 1,
        "lcu": 2,
    }
    return default_mapping.get(origin, 0)
    """Detect the origin/namespace of a file based on its path."""
    filepath_str = str(filepath)
    for origin, config in ORIGINS_CONFIG.items():
        for path_pattern in config["paths"]:
            if path_pattern in filepath_str:
                return origin
    return "core"  # Default to core


def get_registry_path(origin):
    """Get the registry file path for an origin."""
    if origin not in ORIGINS_CONFIG:
        origin = "core"
    return ROOT_DIR / ORIGINS_CONFIG[origin]["registry_file"]


def get_json_output_path(origin):
    """Get the JSON output file path for an origin."""
    if origin not in ORIGINS_CONFIG:
        origin = "core"
    return ROOT_DIR / ORIGINS_CONFIG[origin]["json_output"]


def load_registry(origin):
    """Load the existing registry for a specific origin."""
    registry_path = get_registry_path(origin)
    if registry_path.exists():
        with open(registry_path, "r") as f:
            return json.load(f)

    # Initialize new registry for this origin
    origin_id = get_origin_id(origin)
    registry = {
        "origin": origin,
        "origin_id": origin_id,
        "next_local_id": 1,
        "assignments": {},
    }
    return registry


def save_registry(registry, origin):
    """Save the registry for a specific origin."""
    registry_path = get_registry_path(origin)
    registry_path.parent.mkdir(parents=True, exist_ok=True)
    with open(registry_path, "w") as f:
        json.dump(registry, f, indent=2)


def get_next_local_id(registry):
    """Get the next local ID for this origin."""
    local_id = registry["next_local_id"]
    registry["next_local_id"] = local_id + 1
    return local_id


def format_error_id(origin, local_id):
    """Format a composite error ID using origin name and local ID.

    Returns: "ORIGIN.LOCAL" (for comments/semantic representation)
    """
    origin_upper = origin.upper()
    return f"{origin_upper}.{local_id}"


def process_file(filepath, registry, origin, current_origin_id):
    """Process a single file and return updated content."""

    with open(filepath, "r", errors="ignore") as f:
        lines = f.readlines()

    updated_lines = []
    modified = False
    i = 0

    while i < len(lines):
        line = lines[i]

        # Look for ErrorHandler or WARNING calls
        error_match = re.search(ERROR_HANDLER_PATTERN, line)
        warning_match = re.search(WARNING_PATTERN, line)

        if error_match or warning_match:
            match = error_match or warning_match
            call_type = "error" if error_match else "warning"
            first_arg = match.group(1).strip()

            # Check if it's already using an ID (numeric or composite) or needs conversion
            if first_arg.isdigit() or "." in first_arg or first_arg.isupper():
                # Already has an ID, constant, or composite ID - keep as is
                updated_lines.append(line)
                i += 1
            else:
                # Extract format string (this is the old format without ID)
                full_call_match = re.search(
                    r'(ErrorHandler|WARNING)\s*\(\s*"([^"]*)"\s*(?:,\s*([^)]*))?\s*\)',
                    line,
                )

                if full_call_match:
                    format_string = full_call_match.group(2)
                    args = full_call_match.group(3) if full_call_match.group(3) else ""

                    # Generate key for this location (file + line)
                    location_key = f"{filepath}:{i+1}"

                    # Assign ID if not already assigned
                    if location_key not in registry["assignments"]:
                        local_id = get_next_local_id(registry)
                        composite_id = format_error_id(
                            origin, local_id
                        )  # Use origin name, not ID
                        registry["assignments"][location_key] = {
                            "composite_id": composite_id,
                            "local_id": local_id,
                            "origin_id": current_origin_id,
                            "origin_name": origin,
                            "format": format_string,
                            "type": call_type,
                            "file": str(filepath),
                            "line": i + 1,
                        }
                        modified = True
                        new_id = composite_id
                    else:
                        new_id = registry["assignments"][location_key]["composite_id"]

                    # Replace the call with ID-based version
                    macro_call = f"{full_call_match.group(1)}({new_id}"
                    if args:
                        macro_call += f", {args}"
                    macro_call += ")"

                    new_line = (
                        line[: full_call_match.start()]
                        + macro_call
                        + line[full_call_match.end() :]
                    )

                    # Add or update comment with composite ID
                    indent = len(line) - len(line.lstrip())
                    new_line = (
                        " " * indent
                        + f'// @{call_type}_id "{format_string}"  [{new_id}]\n'
                        + new_line
                    )

                    updated_lines.append(new_line)
                    i += 1
                else:
                    updated_lines.append(line)
                    i += 1
        else:
            updated_lines.append(line)
            i += 1

    return "".join(updated_lines), modified


def generate_json(registry, origin, current_origin_id):
    """Generate the errors_warnings.json from registry for this origin.

    Args:
        registry: The registry dict from load_registry()
        origin: The origin name (core, st-lib, etc)
        current_origin_id: Fresh origin_id read from config (not from registry)

    The numeric_id is regenerated using current_origin_id so it updates when config changes.
    """
    entries = []

    for location_key, assignment in registry["assignments"].items():
        entry = {
            "id": assignment["composite_id"],  # Semantic: "CORE.1" (never changes)
            "numeric_id": f"{current_origin_id}.{assignment['local_id']}",  # Uses CURRENT origin_id
            "local_id": assignment["local_id"],
            "origin_id": current_origin_id,  # Use current, not stored
            "origin_name": assignment.get("origin_name", origin),
            "category": assignment["type"],
            "format": assignment["format"],
            "argument_count": assignment["format"].count("%"),
            "location": f"{assignment['file']}:{assignment['line']}",
        }
        entries.append(entry)

    # Sort by local ID
    entries.sort(key=lambda x: x["local_id"])

    return entries


def find_source_files(origin):
    """Find all C/C++ source files for a specific origin."""
    extensions = {".cpp", ".hpp", ".c", ".h"}
    files = []

    if origin not in ORIGINS_CONFIG:
        return files

    for path_pattern in ORIGINS_CONFIG[origin]["paths"]:
        search_path = ROOT_DIR / path_pattern
        if search_path.exists():
            for ext in extensions:
                files.extend(search_path.glob(f"**/*{ext}"))

    return files


def main():
    parser = argparse.ArgumentParser(
        description="Format ErrorHandler and WARNING calls with composite IDs [origin_id.local_id]"
    )
    parser.add_argument(
        "--origin",
        help="Process only files from specific origin (core, st-lib, lcu, etc.)",
        choices=list(ORIGINS_CONFIG.keys()),
        default=None,
    )
    parser.add_argument(
        "--list-origins",
        action="store_true",
        help="List configured origins and their IDs",
    )

    args = parser.parse_args()

    if args.list_origins:
        print("Configured error/warning ID origins:")
        print(
            "(Use .error_warning.config files or ERROR_WARNING_*_ID env vars to override)\n"
        )
        for origin, config in ORIGINS_CONFIG.items():
            origin_id = get_origin_id(origin)
            config_file = ROOT_DIR / config["config_file"]
            config_status = "✓" if config_file.exists() else "✗ (using default)"
            print(f"  {origin:15} : origin_id={origin_id}  ({config['description']})")
            print(f"                   config: {config['config_file']} {config_status}")
        return 0

    try:
        # Determine which origins to process
        origins_to_process = (
            [args.origin] if args.origin else list(ORIGINS_CONFIG.keys())
        )

        for origin in origins_to_process:
            print(f"\n{'='*60}", file=sys.stderr)
            print(f"Processing origin '{origin}'", file=sys.stderr)
            print(f"{'='*60}", file=sys.stderr)

            registry = load_registry(origin)
            # IMPORTANT: Read origin_id fresh from config, not from registry
            # This allows numeric_ids to regenerate when config changes
            current_origin_id = get_origin_id(origin)
            print(
                f"Loaded registry with {len(registry.get('assignments', {}))} assignments (origin_id={current_origin_id})",
                file=sys.stderr,
            )

            # Find source files for this origin
            source_files = list(find_source_files(origin))
            print(f"Found {len(source_files)} source files", file=sys.stderr)

            any_modified = False
            for filepath in source_files:
                try:
                    updated_content, modified = process_file(
                        filepath, registry, origin, current_origin_id
                    )
                    if modified:
                        with open(filepath, "w") as f:
                            f.write(updated_content)
                        any_modified = True
                        print(f"  Modified: {filepath}")
                except Exception as e:
                    print(f"Error processing {filepath}: {e}", file=sys.stderr)
                    import traceback

                    traceback.print_exc(file=sys.stderr)
                    continue

            # Save registry
            save_registry(registry, origin)
            print(
                f"Saved registry with {len(registry.get('assignments', {}))} assignments",
                file=sys.stderr,
            )

            # Generate JSON with current_origin_id (so numeric_ids update when config changes)
            json_entries = generate_json(registry, origin, current_origin_id)
            json_output_path = get_json_output_path(origin)
            json_output_path.parent.mkdir(parents=True, exist_ok=True)
            with open(json_output_path, "w") as f:
                json.dump(json_entries, f, indent=2)

            print(
                f"Generated: {json_output_path} with {len(json_entries)} entries",
                file=sys.stderr,
            )
            print(f"Registry:  {get_registry_path(origin)}", file=sys.stderr)

        print(f"\n{'='*60}", file=sys.stderr)
        print("✓ All origins processed successfully", file=sys.stderr)
        return 0
    except Exception as e:
        print(f"Fatal error: {e}", file=sys.stderr)
        import traceback

        traceback.print_exc(file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
