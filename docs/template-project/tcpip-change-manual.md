****# Manual de Cambios TCP/IP (ST-LIB + ExampleTCPIP)

Este documento explica en detalle lo que se ha cambiado en:

- `Server`
- `ServerSocket`
- `Socket`
- `DatagramSocket`
- `ExampleTCPIP`
- scripts de test/soak y entorno host

Objetivo principal de los cambios:

- reducir fallos intermitentes bajo carga
- hacer recuperación automática ante desconexiones
- eliminar rutas peligrosas de memoria/estado
- mejorar trazabilidad de fallos con telemetría

## 1. Arquitectura de alto nivel

### 1.1 Bloques principales

- `Server` administra varias conexiones TCP entrantes (`ServerSocket`).
- `ServerSocket` representa una conexión TCP aceptada por el server.
- `Socket` representa un cliente TCP saliente (board -> host).
- `DatagramSocket` maneja UDP board <-> host.
- `ExampleTCPIP` orquesta todo y expone comandos de control para tests.

### 1.2 Flujo operativo en ExampleTCPIP

- Host envía comandos TCP (`CMD_PING`, `CMD_BURST_SERVER`, etc.).
- Firmware responde por `TCPIP_RESPONSE_ORDER_ID`.
- Se prueban:
- integridad de payload TCP
- ráfagas server->host
- ráfagas client(board)->host
- roundtrip UDP
- desconexión/reconexión forzada

## 2. Cambios en `OrderProtocol`

Archivo: `deps/ST-LIB/Inc/HALAL/Models/Packets/OrderProtocol.hpp`

Cambio:

- se añadió destructor virtual.

Por qué:

- `OrderProtocol` es clase base polimórfica.
- sin destructor virtual, un `delete` vía puntero base puede ser UB/fuga.

## 3. Cambios en `Server`

Archivos:

- `deps/ST-LIB/Inc/ST-LIB_LOW/Communication/Server/Server.hpp`
- `deps/ST-LIB/Src/ST-LIB_LOW/Communication/Server/Server.cpp`

### 3.1 Gestión de memoria y ciclo de vida

Antes:

- había llamadas explícitas a destructores (`obj->~ServerSocket()`), peligrosas.

Ahora:

- se usa `delete` real.
- se limpian punteros a `nullptr`.
- se elimina el server de `running_servers` de forma segura.

### 3.2 Lógica de `update()` más robusta

Ahora:

- si `status == CLOSED`, no hace nada.
- si el listener (`open_connection`) no está conectado ni escuchando, se recrea.
- si llega una conexión y hay capacidad, se mueve a `running_connections`.
- si no hay capacidad, se cierra esa nueva conexión sin romper sesiones actuales.
- se compacta el array de conexiones activas y se borran desconectadas.

Efecto:

- evita estados zombis.
- evita crecimiento de conexiones inválidas.
- evita caer a fault por una desconexión normal.

### 3.3 `broadcast_order` ahora devuelve `bool`

Antes:

- era `void`; no se podía saber si se envió a alguien.

Ahora:

- devuelve `true` si al menos una conexión aceptó el envío.

Efecto:

- `ExampleTCPIP` puede decidir si la respuesta realmente salió por el canal esperado.

## 4. Cambios en `ServerSocket`

Archivos:

- `deps/ST-LIB/Inc/HALAL/Services/Communication/Ethernet/LWIP/TCP/ServerSocket.hpp`
- `deps/ST-LIB/Src/HALAL/Services/Communication/Ethernet/LWIP/TCP/ServerSocket.cpp`

### 4.1 TX queue y envío

Cambios:

- `MAX_TX_QUEUE_DEPTH` subido a `64` (antes `24`).
- `send_order()` usa `add_order_to_queue()`.
- si cola llena momentáneamente, intenta un flush (`send()`) y reintenta una vez.

Efecto:

- menos falsos fallos bajo ráfagas.
- mejor aprovechamiento del buffer TCP.

### 4.2 Parsing RX por stream

Cambio clave:

- se introdujo `rx_stream_buffer` y parser incremental (`process_order_stream`).

Por qué:

- TCP no preserva framing de mensajes.
- un `Order` puede llegar fragmentado o varios `Order` juntos.

Comportamiento:

- acumula bytes.
- busca `order_id`.
- valida tamaño.
- procesa solo frames completos.
- resincroniza si encuentra basura/ID desconocido.
- limita buffer a `8192` bytes para no crecer infinito.

### 4.3 Callbacks lwIP reforzados

Cambios:

- comprobaciones nulas en callbacks.
- en errores transitorios de `receive_callback`, no mata conexión agresivamente.
- `error_callback` asume que lwIP ya liberó PCB y limpia estado local.
- `poll_callback` y `send_callback` drenan TX/RX y cierran limpio en `CLOSING`.

### 4.4 Estados adicionales útiles

Cambio:

- nuevo `is_listening() const`.

Uso:

- `Server::update()` detecta listener inválido y lo recrea.

## 5. Cambios en `Socket` (cliente TCP board->host)

Archivos:

- `deps/ST-LIB/Inc/HALAL/Services/Communication/Ethernet/LWIP/TCP/Socket.hpp`
- `deps/ST-LIB/Src/HALAL/Services/Communication/Ethernet/LWIP/TCP/Socket.cpp`

### 5.1 Inicialización y punteros seguros

Cambios:

- `connection_control_block` y `socket_control_block` inicializados a `nullptr`.
- limpieza consistente en `close()`, destructor y `operator=`.

### 5.2 TX queue y envío

Cambios:

- `MAX_TX_QUEUE_DEPTH` a `64`.
- `send_order()`:
- verifica `state == CONNECTED`.
- si no conectado, intenta `reconnect()`.
- enqueue con `add_order_to_queue()`.
- flush oportunista + reintento cuando cola momentáneamente llena.

### 5.3 Parsing RX por stream (igual filosofía que ServerSocket)

Cambio:

- `rx_stream_buffer` + parser incremental para soportar fragmentación TCP.

### 5.4 Reconexión y watchdog de handshake

Cambios:

- `pending_connection_reset` y `connect_poll_ticks`.
- `connection_poll_callback()` incrementa ticks cuando queda en `SYN_SENT`.
- si se estanca (`>=20` ticks), marca reset pendiente.
- `reconnect()` dispara `reset()` cuando hay reset pendiente o no hay PCB válido.

Efecto:

- menos sockets atascados en handshake.
- recuperación autónoma sin reiniciar firmware.

### 5.5 Limpieza de código redundante/muerto

Hecho:

- se eliminaron rutas de abortado duplicadas tras `close()`.
- se simplificó `connection_error_callback` para limpiar estado sin ramas inútiles.

## 6. Cambios en `DatagramSocket` (UDP)

Archivos:

- `deps/ST-LIB/Inc/HALAL/Services/Communication/Ethernet/LWIP/UDP/DatagramSocket.hpp`
- `deps/ST-LIB/Src/HALAL/Services/Communication/Ethernet/LWIP/UDP/DatagramSocket.cpp`

### 6.1 Robustez de ciclo de vida

Cambios:

- `udp_control_block` inicializado a `nullptr`.
- checks de `udp_new()` y `udp_bind()` con rollback completo.
- `close()` y `reconnect()` idempotentes (si no hay PCB, no rompen).
- move ctor/assignment corregidos para transferencia de ownership real.

### 6.2 RX seguro con pbuf chain

Antes:

- se parseaba directo `packet_buffer->payload` (peligroso si `pbuf` encadenado).

Ahora:

- copia con `pbuf_copy_partial()` a buffer continuo.
- parsea solo si tamaño minimo valido.

Efecto:

- evita leer memoria incompleta/corrupta en UDP segmentado.

## 7. Cambios en lwIP config relevantes

Archivo: `deps/ST-LIB/LWIP/Target/lwipopts.h`

Cambio:

- se añadieron macros explícitas:
- `CHECKSUM_GEN_ICMP 0`
- `CHECKSUM_CHECK_ICMP 0`

Interpretación:

- coherente con `CHECKSUM_BY_HARDWARE=1` y resto de checksums en `0` (offload HW).

## 8. Cambios en `ExampleTCPIP` (firmware de pruebas)

Archivo: `Core/Src/Examples/ExampleTCPIP.cpp`

### 8.1 Respuesta de control fiable

Cambio clave:

- `try_send_tcp_response()` distingue envío por canal cliente vs canal server.
- si hay conexiones server activas, considera éxito solo si respondió por server.

Por qué:

- evita “ACK enviado” por el socket cliente cuando el control real iba por server.
- corrige timeouts falsos de comandos en el script.

### 8.2 Cola de respuestas pendientes

Cambios:

- `queue_tcp_response()` + `flush_pending_tcp_response()`.
- no se pierde respuesta si en ese instante no hay ventana TCP disponible.

### 8.3 Burst server/client con decremento correcto

Cambio:

- `server_burst_remaining` y `client_burst_remaining` decrementan solo en envío OK.

Efecto:

- evita contar como enviados paquetes que realmente no salieron.

### 8.4 Backoff/pacing y heartbeats

Cambios:

- pacing con `next_*_burst_attempt_ms`.
- heartbeats del cliente solo cuando no hay burst activo.
- reduce interferencia entre tráfico de control y tráfico de estrés.

### 8.5 Reconexión del `Socket` cliente más fina

Cambios:

- `CLIENT_RECONNECT_MS` a `500ms`.
- recreación pesada por contador (`CLIENT_RECONNECT_RECREATE_EVERY=60`) en vez de agresiva.
- watchdog adicional por racha de fallos de envío.

Efecto:

- menor inestabilidad en `tcp_client_stream`.

### 8.6 Health telemetry

Se añadieron/estandarizaron páginas de health (`CMD_GET_HEALTH`) para diagnosticar:

- loops, comandos, payload rx/bad
- tx ok/fail del cliente TCP
- conteo de recreaciones y reconexiones
- último motivo de evento
- máximos de burst y ticks desconectado

## 9. Cambios en `example_tcpip_stress.py`

Archivo: `tools/example_tcpip_stress.py`

### 9.1 Matching fuerte de respuestas

Cambio:

- `wait_response_matching(...)` permite validar `value0/1/2` esperados.

Efecto:

- evita confundir respuestas viejas con respuestas del comando actual.

### 9.2 Integridad payload más estable

Cambio:

- tras enviar payloads, hace polling de stats con ventana de asentamiento.

Efecto:

- reduce falsos negativos por latencia de actualización de contadores en firmware.

### 9.3 Ventanas dinámicas en burst/client-stream

Cambio:

- tiempo de colección ajustado al tamaño de burst.

Efecto:

- menos sensibilidad a jitter temporal.

### 9.4 Check client-stream más robusto

Cambios:

- nudge de conexión cuando sink está inactivo.
- reintentos controlados.
- validación por ventana agregada (`aggregate_window_pass`) para tolerar ruido de microventanas.

## 10. Cambios en scripts de ejecución

### 10.1 Host network seguro en macOS

Archivo: `tools/configure_nucleo_host_network_macos.sh`

Qué añade:

- prioriza Wi-Fi y mantiene USB-Ethernet para la placa.
- valida ruta default (internet) y ruta a board por interfaz correcta.
- detecta bloqueo de permiso "Local Network" y lo reporta claramente.

### 10.2 Runner end-to-end de Nucleo

Archivo: `tools/run_example_tcpip_nucleo.sh`

Qué añade:

- preflight de rutas y diagnóstico local-network.
- build con `TCPIP_TEST_HOST_IP` y `TCPIP_TEST_BOARD_IP`.
- flash con `stm32prog` o `openocd`.
- fallback de OpenOCD si `verify` falla por mismatch típico de secciones RAM.

### 10.3 Quality gate y soak

Archivos:

- `tools/example_tcpip_quality_gate.sh`
- `tools/example_tcpip_soak.sh`
- `tools/example_tcpip_soak_hours.sh`

Qué añade:

- matriz base/agresiva repetible con logs por run.
- preflight ping entre runs.
- soak largo con resumen de ratio PASS/FAIL y breakdown por test fallido.

## 11. Qué problemas se buscaba solucionar

Síntomas observados antes:

- fallos intermitentes en `tcp_client_stream`.
- timeouts puntuales en respuestas de comando (`tcp_server_burst`).
- reconexiones no deterministas.
- posibles riesgos de lifecycle/ownership en sockets.

Estado tras cambios:

- mejora clara de estabilidad.
- siguen existiendo fallos aislados bajo soak estricto (baja frecuencia).

## 12. Riesgos residuales y próximos pasos recomendados

Pendiente para casi "bulletproof":

- incluir token de correlación explícito request/response a nivel protocolo de control.
- exponer métricas internas de cola TX y errores lwIP (`ERR_MEM`, `ERR_RST`, etc.) por health page.
- ejecutar soak nocturno y usar baseline de ratio para detectar regresiones automáticamente.

## 13. Guía rápida de lectura del código

Orden recomendado para entenderlo sin perderse:

1. `ExampleTCPIP.cpp` (qué se prueba y cómo se orquesta).
2. `Server.cpp` + `ServerSocket.cpp` (camino de control server-side).
3. `Socket.cpp` (camino board->host y reconexión).
4. `DatagramSocket.cpp` (UDP).
5. `example_tcpip_stress.py` (cómo se verifica desde host).
6. `example_tcpip_quality_gate.sh` / `example_tcpip_soak*.sh` (cómo se automatiza a largo plazo).
