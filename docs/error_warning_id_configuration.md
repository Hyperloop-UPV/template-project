# Error/Warning ID Origin Configuration

## Overview

Error and warning IDs use a composite format: `[origin_id].[local_id]`

Each board/module/dependency gets a unique `origin_id` to prevent ID collisions between different systems. The `local_id` is automatically incremented within each origin.

## Default Configuration

| Origin | File | Default ID | Used For |
|--------|------|------------|----------|
| core | `Core/.error_warning.config` | 0 | Core firmware (THIS_BOARD) |
| st-lib | `deps/ST-LIB/.error_warning.config` | 1 | ST-LIB dependency |
| lcu | `.error_warning.config.lcu` | 2 | LCU board code |

## Changing Origin IDs

You have two options to configure origin IDs:

### Option 1: Config File (Recommended)

Edit the `.error_warning.config` file in each origin's directory:

```bash
# Core/
cat Core/.error_warning.config
```

```ini
origin_id=0
```

Simply change the number and save. The formatter will use the new ID next time it runs.

### Option 2: Environment Variable

Override via environment variable before running the formatter:

```bash
# Use origin_id=5 for core
ERROR_WARNING_CORE_ID=5 python3 tools/error_warning_formatter.py --origin core

# Use origin_id=10 for st-lib
ERROR_WARNING_STLIB_ID=10 python3 tools/error_warning_formatter.py --origin st-lib

# Use origin_id=20 for lcu
ERROR_WARNING_LCU_ID=20 python3 tools/error_warning_formatter.py --origin lcu
```

Environment variables take precedence over config files.

## Example IDs

With the default configuration:

- Core errors: `0.1`, `0.2`, `0.3`, ...
- ST-LIB errors: `1.1`, `1.2`, `1.3`, ...
- LCU errors: `2.1`, `2.2`, `2.3`, ...

## Adding New Origins

If you add a new subsystem/board:

1. Update `tools/error_warning_formatter.py` - add to `ORIGINS_CONFIG`
2. Create `.error_warning.config` in the new origin's directory
3. Assign a unique `origin_id`

Example in `ORIGINS_CONFIG`:
```python
"my-board": {
    "paths": ["my-board/"],
    "config_file": "my-board/.error_warning.config",
    "registry_file": ".error_warning_registry_my_board.json",
    "json_output": "my-board/Inc/Code_generation/errors_warnings.json",
    "description": "My custom board"
},
```

Then create `my-board/.error_warning.config`:
```ini
# My custom board error/warning origin
origin_id=3
```
