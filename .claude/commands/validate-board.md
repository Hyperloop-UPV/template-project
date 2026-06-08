Validate all JSON configuration files for board: $ARGUMENTS

1. Read `Core/Inc/Code_generation/JSON_ADE/boards.json` — confirm the board key exists

2. Read the board's main JSON file (`boards/[BOARD]/[BOARD].json`) and identify all referenced files

3. Read all referenced files: measurements, packets, orders, sockets

4. Run these checks and report each issue clearly (file + field):

   **Measurements**
   - No duplicate `id` values
   - Each entry has `id`, `name`, `type`
   - `type` is one of: `bool`, `uint8`, `uint16`, `uint32`, `uint64`, `int8`, `int16`, `int32`, `int64`, `float`, `double`

   **Packets / Orders**
   - No duplicate `id` values across all packets AND orders combined
   - Each entry has `type`, `name`, `id`
   - All `variables` entries exist as `id` in a measurements file
   - `socket` references (if present) exist in `sockets.json`
   - `period_type` (if present) is `"ms"` or `"us"`

   **Sockets**
   - Each entry has `type` and `name`
   - `type` is one of: `ServerSocket`, `Socket`, `DatagramSocket`
   - Required fields by type:
     - `ServerSocket`: `port`
     - `Socket`: `local_port`, `remote_ip`, `remote_port`
     - `DatagramSocket`: `port`, `remote_ip`

5. Attempt actual generation to catch Python-level errors:
   ```bash
   python3 Core/Inc/Code_generation/Generator.py [BOARD]
   ```

6. Report: PASS (with a summary) or FAIL (with each error listed)
