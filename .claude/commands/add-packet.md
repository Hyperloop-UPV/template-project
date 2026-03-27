Add a new packet to the board JSON configuration. Arguments: $ARGUMENTS
Expected format: `[board_key] [packet_name] [data|order]`

Steps:

1. Read `Core/Inc/Code_generation/JSON_ADE/boards.json` — confirm the board key exists and find its config path
2. Read `[board_path]/[BOARD].json` — find which measurement files and packet files it references
3. Read the board's `[BOARD]_measurements.json` — check if all variables for the new packet already exist
   - If not, add the missing variables with correct types: `bool`, `uint8`, `uint16`, `uint32`, `uint64`, `int8`, `int16`, `int32`, `int64`, `float`, `double`
   - Each measurement entry: `{"id": "snake_case_name", "name": "Human Name", "type": "..."}`
4. Add the new packet to `packets.json` (data) or `orders.json` (order):

   Data packet format:
   ```json
   {
     "type": "data",
     "name": "packet_name",
     "variables": ["var1", "var2"],
     "id": 20XXX,
     "socket": "socket_name",
     "period": 50,
     "period_type": "ms"
   }
   ```

   Order packet format:
   ```json
   {
     "type": "order",
     "name": "packet_name",
     "variables": ["var1", "var2"],
     "id": 20XXX
   }
   ```

   - `id` must be unique across all packets and orders — check existing IDs to avoid collision
   - `socket` must match a name defined in `sockets.json` (only for data packets with telemetry)

5. Regenerate headers:
   ```bash
   python3 Core/Inc/Code_generation/Generator.py [BOARD]
   ```

6. Show the relevant generated section from `Core/Inc/Communications/Packets/DataPackets.hpp` or `OrderPackets.hpp` so the developer knows the exact C++ names to use in firmware code
