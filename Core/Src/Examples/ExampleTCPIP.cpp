#ifdef EXAMPLE_TCPIP

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

#ifdef STLIB_ETH

#ifndef TCPIP_TEST_BOARD_IP
#define TCPIP_TEST_BOARD_IP 192.168.1.7
#endif

#ifndef TCPIP_TEST_HOST_IP
#define TCPIP_TEST_HOST_IP 192.168.1.9
#endif

#define TCPIP_STRINGIFY_IMPL(value) #value
#define TCPIP_STRINGIFY(value) TCPIP_STRINGIFY_IMPL(value)

#ifndef TCPIP_TEST_TCP_SERVER_PORT
#define TCPIP_TEST_TCP_SERVER_PORT 40000
#endif

#ifndef TCPIP_TEST_TCP_CLIENT_LOCAL_PORT
#define TCPIP_TEST_TCP_CLIENT_LOCAL_PORT 40001
#endif

#ifndef TCPIP_TEST_TCP_CLIENT_REMOTE_PORT
#define TCPIP_TEST_TCP_CLIENT_REMOTE_PORT 40002
#endif

#ifndef TCPIP_TEST_UDP_LOCAL_PORT
#define TCPIP_TEST_UDP_LOCAL_PORT 40003
#endif

#ifndef TCPIP_TEST_UDP_REMOTE_PORT
#define TCPIP_TEST_UDP_REMOTE_PORT 40004
#endif

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

#if defined(USE_PHY_LAN8742)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP),
    "255.255.0.0"
);
#elif defined(USE_PHY_LAN8700)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP),
    "255.255.0.0"
);
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H11,
    "00:80:e1:00:01:07",
    TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP),
    "255.255.0.0"
);
#else
#error "No PHY selected for Ethernet pinset selection"
#endif

using ExampleTCPIPBoard = ST_LIB::Board<eth, led>;

namespace {

constexpr uint16_t TCPIP_CMD_ORDER_ID = 0x7101;
constexpr uint16_t TCPIP_RESPONSE_ORDER_ID = 0x7102;
constexpr uint16_t TCPIP_PAYLOAD_ORDER_ID = 0x7103;
constexpr uint16_t TCPIP_CLIENT_STREAM_ORDER_ID = 0x7104;
constexpr uint16_t TCPIP_SERVER_STREAM_ORDER_ID = 0x7105;

constexpr uint16_t TCPIP_UDP_PROBE_PACKET_ID = 0x7201;
constexpr uint16_t TCPIP_UDP_STATUS_PACKET_ID = 0x7202;

constexpr uint32_t CMD_RESET = 1;
constexpr uint32_t CMD_PING = 2;
constexpr uint32_t CMD_GET_STATS = 3;
constexpr uint32_t CMD_FORCE_DISCONNECT = 4;
constexpr uint32_t CMD_BURST_SERVER = 5;
constexpr uint32_t CMD_BURST_CLIENT = 6;
constexpr uint32_t CMD_FORCE_CLIENT_RECONNECT = 7;
constexpr uint32_t CMD_GET_HEALTH = 8;
constexpr uint32_t CMD_RESET_HEALTH = 9;

constexpr uint32_t STREAMS_PER_LOOP = 6;
constexpr uint32_t CLIENT_HEARTBEAT_MS = 200;
constexpr uint32_t CLIENT_RECONNECT_MS = 500;
constexpr uint32_t CLIENT_RECONNECT_RECREATE_EVERY = 60;
constexpr uint32_t STREAM_RETRY_BACKOFF_MS = 3;
constexpr uint32_t STREAM_SUCCESS_SPACING_MS = 1;
constexpr uint32_t STREAM_DISCONNECTED_BACKOFF_MS = 20;
constexpr uint32_t STREAM_FAIL_STREAK_BACKOFF_MS = 40;
constexpr uint32_t CLIENT_SEND_FAIL_RECREATE_THRESHOLD = 256;
constexpr uint32_t CLIENT_SEND_FAIL_RECREATE_MIN_INTERVAL_MS = 1000;
constexpr uint32_t LED_TOGGLE_MS = 250;
constexpr uint32_t HEALTH_PAGE_COUNT = 6;

enum HealthReason : uint32_t {
    HEALTH_REASON_NONE = 0,
    HEALTH_REASON_BOOT = 1,
    HEALTH_REASON_CMD_FORCE_DISCONNECT = 2,
    HEALTH_REASON_SERVER_RECREATE = 3,
    HEALTH_REASON_CLIENT_RECREATE_CMD = 4,
    HEALTH_REASON_CLIENT_RECREATE_WATCHDOG = 5,
    HEALTH_REASON_CLIENT_RECONNECT_POLL = 6,
    HEALTH_REASON_CLIENT_SEND_FAIL = 7
};

struct RuntimeStats {
    uint32_t tcp_commands_rx = 0;
    uint32_t tcp_payload_rx = 0;
    uint32_t tcp_payload_bad = 0;
    uint32_t tcp_responses_tx = 0;
    uint32_t tcp_client_tx_ok = 0;
    uint32_t tcp_client_tx_fail = 0;
    uint32_t udp_probe_rx = 0;
    uint32_t udp_probe_bad = 0;
    uint32_t udp_status_tx_ok = 0;
    uint32_t udp_status_tx_fail = 0;
    uint32_t forced_disconnects = 0;
};

RuntimeStats stats{};

struct HealthTelemetry {
    uint32_t loop_iterations = 0;
    uint32_t last_cmd_opcode = 0;
    uint32_t last_cmd_at_ms = 0;

    uint32_t reason_last = HEALTH_REASON_BOOT;
    uint32_t reason_arg_last = 0;
    uint32_t reason_update_count = 1;

    uint32_t tcp_server_recreate_count = 0;
    uint32_t tcp_client_recreate_count = 0;
    uint32_t tcp_client_reconnect_calls = 0;
    uint32_t tcp_client_not_connected_ticks = 0;

    uint32_t tcp_client_send_fail_streak = 0;
    uint32_t tcp_client_send_fail_streak_max = 0;
    uint32_t tcp_client_send_fail_events = 0;
    uint32_t tcp_client_send_ok_events = 0;

    uint32_t server_burst_requested_max = 0;
    uint32_t client_burst_requested_max = 0;
};

HealthTelemetry health{};

Server* tcp_server = nullptr;
Socket* tcp_client = nullptr;
DatagramSocket* udp_socket = nullptr;

uint32_t command_opcode = 0;
uint32_t command_arg0 = 0;
uint32_t command_arg1 = 0;
bool pending_command = false;
bool force_disconnect_requested = false;
uint32_t server_burst_remaining = 0;
uint32_t client_burst_remaining = 0;
uint32_t next_server_burst_attempt_ms = 0;
uint32_t next_client_burst_attempt_ms = 0;
bool tcp_response_retry_pending = false;
uint32_t pending_response_code = 0;
uint32_t pending_response_value0 = 0;
uint32_t pending_response_value1 = 0;
uint32_t pending_response_value2 = 0;
bool client_send_failed_last = false;

uint32_t response_code = 0;
uint32_t response_value0 = 0;
uint32_t response_value1 = 0;
uint32_t response_value2 = 0;
StackOrder tcp_response_order(
    TCPIP_RESPONSE_ORDER_ID,
    &response_code,
    &response_value0,
    &response_value1,
    &response_value2
);

uint32_t tcp_payload_sequence = 0;
uint32_t tcp_payload_checksum = 0;
array<uint8_t, 128> tcp_payload_bytes = {};

uint32_t tcp_client_stream_sequence = 0;
uint32_t tcp_client_stream_ok = 0;
uint32_t tcp_client_stream_fail = 0;
StackOrder tcp_client_stream_order(
    TCPIP_CLIENT_STREAM_ORDER_ID,
    &tcp_client_stream_sequence,
    &tcp_client_stream_ok,
    &tcp_client_stream_fail
);

uint32_t tcp_server_stream_sequence = 0;
array<uint8_t, 64> tcp_server_stream_bytes = {};
StackOrder tcp_server_stream_order(
    TCPIP_SERVER_STREAM_ORDER_ID,
    &tcp_server_stream_sequence,
    &tcp_server_stream_bytes
);

uint32_t udp_probe_sequence = 0;
uint32_t udp_probe_checksum = 0;
array<uint8_t, 128> udp_probe_bytes = {};
StackPacket udp_probe_packet(
    TCPIP_UDP_PROBE_PACKET_ID,
    &udp_probe_sequence,
    &udp_probe_checksum,
    &udp_probe_bytes
);

uint32_t udp_status_sequence = 0;
uint32_t udp_status_ok = 0;
uint32_t udp_status_bad = 0;
StackPacket udp_status_packet(
    TCPIP_UDP_STATUS_PACKET_ID,
    &udp_status_sequence,
    &udp_status_ok,
    &udp_status_bad
);

uint32_t last_udp_probe_sequence = 0;
bool udp_probe_seen = false;

uint32_t checksum32(const uint8_t* data, size_t size) {
    uint32_t checksum = 2166136261u;
    for (size_t i = 0; i < size; i++) {
        checksum ^= data[i];
        checksum *= 16777619u;
    }
    return checksum;
}

void fill_pattern(uint8_t* data, size_t size, uint32_t seed) {
    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<uint8_t>((seed + (i * 17u)) & 0xFFu);
    }
}

bool try_send_tcp_response(uint32_t code, uint32_t value0, uint32_t value1, uint32_t value2) {
    response_code = code;
    response_value0 = value0;
    response_value1 = value1;
    response_value2 = value2;
    bool client_sent = false;
    bool server_sent = false;
    const bool client_connected = (tcp_client != nullptr && tcp_client->is_connected());
    const bool server_has_connections =
        (tcp_server != nullptr && tcp_server->connections_count() > 0);

    if (client_connected) {
        client_sent = tcp_client->send_order(tcp_response_order);
    }
    if (tcp_server != nullptr) {
        server_sent = tcp_server->broadcast_order(tcp_response_order);
    }

    // Prefer ACKing through the server path whenever server connections exist, because
    // command/control in stress mode runs over TCP server and must receive every response.
    if (server_has_connections) {
        return server_sent;
    }
    if (client_connected) {
        return client_sent;
    }
    return false;
}

void queue_tcp_response(uint32_t code, uint32_t value0, uint32_t value1, uint32_t value2) {
    pending_response_code = code;
    pending_response_value0 = value0;
    pending_response_value1 = value1;
    pending_response_value2 = value2;
    tcp_response_retry_pending = true;
}

void flush_pending_tcp_response() {
    if (!tcp_response_retry_pending) {
        return;
    }
    if (try_send_tcp_response(
            pending_response_code,
            pending_response_value0,
            pending_response_value1,
            pending_response_value2
        )) {
        stats.tcp_responses_tx++;
        tcp_response_retry_pending = false;
    }
}

void tcp_command_callback() {
    stats.tcp_commands_rx++;
    pending_command = true;
}

void tcp_payload_callback() {
    stats.tcp_payload_rx++;
    if (checksum32(tcp_payload_bytes.data(), tcp_payload_bytes.size()) != tcp_payload_checksum) {
        stats.tcp_payload_bad++;
    }
}

StackOrder tcp_command_order(
    TCPIP_CMD_ORDER_ID,
    &tcp_command_callback,
    &command_opcode,
    &command_arg0,
    &command_arg1
);

StackOrder tcp_payload_order(
    TCPIP_PAYLOAD_ORDER_ID,
    &tcp_payload_callback,
    &tcp_payload_sequence,
    &tcp_payload_checksum,
    &tcp_payload_bytes
);

void reset_runtime_stats() {
    stats = {};
    server_burst_remaining = 0;
    client_burst_remaining = 0;
    next_server_burst_attempt_ms = HAL_GetTick();
    next_client_burst_attempt_ms = HAL_GetTick();
}

void set_health_reason(uint32_t reason, uint32_t arg = 0) {
    health.reason_last = reason;
    health.reason_arg_last = arg;
    health.reason_update_count++;
}

void reset_health_telemetry() {
    health = {};
    health.reason_last = HEALTH_REASON_NONE;
}

void send_health_page(uint32_t page) {
    const uint32_t uptime_ms = HAL_GetTick();
    switch (page) {
    case 0:
        queue_tcp_response(
            CMD_GET_HEALTH,
            uptime_ms,
            health.loop_iterations,
            stats.tcp_commands_rx
        );
        break;
    case 1:
        queue_tcp_response(
            CMD_GET_HEALTH,
            stats.tcp_payload_rx,
            stats.tcp_payload_bad,
            stats.tcp_responses_tx
        );
        break;
    case 2:
        queue_tcp_response(
            CMD_GET_HEALTH,
            stats.tcp_client_tx_ok,
            stats.tcp_client_tx_fail,
            health.tcp_client_send_fail_streak_max
        );
        break;
    case 3:
        queue_tcp_response(
            CMD_GET_HEALTH,
            health.tcp_server_recreate_count,
            health.tcp_client_recreate_count,
            health.tcp_client_reconnect_calls
        );
        break;
    case 4:
        queue_tcp_response(
            CMD_GET_HEALTH,
            health.reason_last,
            health.reason_arg_last,
            health.reason_update_count
        );
        break;
    case 5:
        queue_tcp_response(
            CMD_GET_HEALTH,
            health.server_burst_requested_max,
            health.client_burst_requested_max,
            health.tcp_client_not_connected_ticks
        );
        break;
    default:
        queue_tcp_response(CMD_GET_HEALTH, page, HEALTH_PAGE_COUNT, 0xDEAD0001u);
        break;
    }
}

void recreate_tcp_server(uint32_t reason = HEALTH_REASON_SERVER_RECREATE, uint32_t arg = 0) {
    if (tcp_server != nullptr) {
        delete tcp_server;
        tcp_server = nullptr;
    }
    tcp_server = new Server(IPV4(TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP)), TCPIP_TEST_TCP_SERVER_PORT);
    health.tcp_server_recreate_count++;
    set_health_reason(reason, arg);
}

void recreate_tcp_client(uint32_t reason, uint32_t arg = 0) {
    if (tcp_client != nullptr) {
        delete tcp_client;
        tcp_client = nullptr;
    }
    tcp_client = new Socket(
        IPV4(TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP)),
        TCPIP_TEST_TCP_CLIENT_LOCAL_PORT,
        IPV4(TCPIP_STRINGIFY(TCPIP_TEST_HOST_IP)),
        TCPIP_TEST_TCP_CLIENT_REMOTE_PORT
    );
    health.tcp_client_recreate_count++;
    health.tcp_client_send_fail_streak = 0;
    set_health_reason(reason, arg);
}

void process_pending_command() {
    if (tcp_response_retry_pending) {
        flush_pending_tcp_response();
        if (tcp_response_retry_pending) {
            return;
        }
    }

    if (!pending_command) {
        return;
    }
    pending_command = false;
    health.last_cmd_opcode = command_opcode;
    health.last_cmd_at_ms = HAL_GetTick();

    switch (command_opcode) {
    case CMD_RESET:
        reset_runtime_stats();
        queue_tcp_response(CMD_RESET, 0, 0, 0);
        break;

    case CMD_PING:
        queue_tcp_response(
            CMD_PING,
            command_arg0,
            (tcp_server != nullptr) ? tcp_server->connections_count() : 0,
            stats.tcp_payload_rx
        );
        break;

    case CMD_GET_STATS:
        queue_tcp_response(
            CMD_GET_STATS,
            stats.tcp_payload_rx,
            stats.tcp_payload_bad,
            stats.forced_disconnects
        );
        break;

    case CMD_FORCE_DISCONNECT:
        stats.forced_disconnects++;
        queue_tcp_response(CMD_FORCE_DISCONNECT, stats.forced_disconnects, 0, 0);
        force_disconnect_requested = true;
        set_health_reason(HEALTH_REASON_CMD_FORCE_DISCONNECT, stats.forced_disconnects);
        break;

    case CMD_BURST_SERVER:
        server_burst_remaining = command_arg0;
        if (server_burst_remaining > health.server_burst_requested_max) {
            health.server_burst_requested_max = server_burst_remaining;
        }
        next_server_burst_attempt_ms = HAL_GetTick();
        queue_tcp_response(CMD_BURST_SERVER, server_burst_remaining, 0, 0);
        break;

    case CMD_BURST_CLIENT:
        client_burst_remaining = command_arg0;
        if (client_burst_remaining > health.client_burst_requested_max) {
            health.client_burst_requested_max = client_burst_remaining;
        }
        next_client_burst_attempt_ms = HAL_GetTick();
        queue_tcp_response(CMD_BURST_CLIENT, client_burst_remaining, 0, 0);
        break;

    case CMD_FORCE_CLIENT_RECONNECT:
        recreate_tcp_client(HEALTH_REASON_CLIENT_RECREATE_CMD, command_arg0);
        queue_tcp_response(CMD_FORCE_CLIENT_RECONNECT, 1, 0, 0);
        break;

    case CMD_GET_HEALTH:
        send_health_page(command_arg0);
        break;

    case CMD_RESET_HEALTH:
        reset_health_telemetry();
        queue_tcp_response(CMD_RESET_HEALTH, 1, 0, 0);
        break;

    default:
        queue_tcp_response(0xFFFFFFFFu, command_opcode, command_arg0, command_arg1);
        break;
    }
}

void process_udp_probe() {
    if (!udp_probe_seen && udp_probe_sequence == 0 && udp_probe_checksum == 0) {
        return;
    }

    if (!udp_probe_seen || udp_probe_sequence != last_udp_probe_sequence) {
        udp_probe_seen = true;
        last_udp_probe_sequence = udp_probe_sequence;
        stats.udp_probe_rx++;

        if (checksum32(udp_probe_bytes.data(), udp_probe_bytes.size()) != udp_probe_checksum) {
            stats.udp_probe_bad++;
        }

        udp_status_sequence = udp_probe_sequence;
        udp_status_ok = stats.udp_probe_rx - stats.udp_probe_bad;
        udp_status_bad = stats.udp_probe_bad;

        if (udp_socket != nullptr && udp_socket->send_packet(udp_status_packet)) {
            stats.udp_status_tx_ok++;
        } else {
            stats.udp_status_tx_fail++;
        }
    }
}

uint32_t send_server_stream_burst(uint32_t budget) {
    if (tcp_server == nullptr) {
        return 0;
    }

    uint32_t sent_ok = 0;
    for (uint32_t sent = 0; sent < budget && server_burst_remaining > 0; sent++) {
        fill_pattern(
            tcp_server_stream_bytes.data(),
            tcp_server_stream_bytes.size(),
            tcp_server_stream_sequence
        );
        if (!tcp_server->broadcast_order(tcp_server_stream_order)) {
            break;
        }
        tcp_server_stream_sequence++;
        server_burst_remaining--;
        sent_ok++;
    }
    return sent_ok;
}

void note_client_send_failure() {
    stats.tcp_client_tx_fail++;
    health.tcp_client_send_fail_events++;
    health.tcp_client_send_fail_streak++;
    if (health.tcp_client_send_fail_streak > health.tcp_client_send_fail_streak_max) {
        health.tcp_client_send_fail_streak_max = health.tcp_client_send_fail_streak;
    }
    if (health.tcp_client_send_fail_streak == 1) {
        set_health_reason(HEALTH_REASON_CLIENT_SEND_FAIL, stats.tcp_client_tx_fail);
    }
}

uint32_t send_client_stream(uint32_t amount) {
    client_send_failed_last = false;
    if (tcp_client == nullptr) {
        return 0;
    }

    uint32_t sent_ok = 0;
    for (uint32_t sent = 0; sent < amount; sent++) {
        if (!tcp_client->is_connected()) {
            break;
        }

        tcp_client_stream_sequence++;
        tcp_client_stream_ok = stats.tcp_client_tx_ok;
        tcp_client_stream_fail = stats.tcp_client_tx_fail;

        if (tcp_client->send_order(tcp_client_stream_order)) {
            stats.tcp_client_tx_ok++;
            health.tcp_client_send_ok_events++;
            health.tcp_client_send_fail_streak = 0;
            sent_ok++;
        } else {
            tcp_client_stream_sequence--;
            note_client_send_failure();
            client_send_failed_last = true;
            break;
        }
    }
    return sent_ok;
}

} // namespace

int main(void) {
    Hard_fault_check();

    ExampleTCPIPBoard::init();

    auto& eth_instance = ExampleTCPIPBoard::instance_of<eth>();
    auto& led_instance = ExampleTCPIPBoard::instance_of<led>();

    recreate_tcp_server(HEALTH_REASON_BOOT, 0);
    recreate_tcp_client(HEALTH_REASON_BOOT, 0);
    udp_socket = new DatagramSocket(
        IPV4(TCPIP_STRINGIFY(TCPIP_TEST_BOARD_IP)),
        TCPIP_TEST_UDP_LOCAL_PORT,
        IPV4(TCPIP_STRINGIFY(TCPIP_TEST_HOST_IP)),
        TCPIP_TEST_UDP_REMOTE_PORT
    );

    uint32_t last_led_toggle_ms = HAL_GetTick();
    uint32_t last_client_heartbeat_ms = HAL_GetTick();
    uint32_t last_client_reconnect_ms = HAL_GetTick();
    uint32_t last_client_fail_recreate_ms = HAL_GetTick();

    led_instance.turn_on();

    while (1) {
        health.loop_iterations++;
        eth_instance.update();
        Server::update_servers();

        process_pending_command();
        flush_pending_tcp_response();
        process_udp_probe();

        const uint32_t now = HAL_GetTick();
        const bool control_tx_ready = !tcp_response_retry_pending;

        if (force_disconnect_requested) {
            force_disconnect_requested = false;
            recreate_tcp_server(HEALTH_REASON_SERVER_RECREATE, stats.forced_disconnects);
        }

        if (control_tx_ready && server_burst_remaining > 0 && now >= next_server_burst_attempt_ms) {
            const uint32_t sent_now = send_server_stream_burst(STREAMS_PER_LOOP);
            next_server_burst_attempt_ms =
                now + ((sent_now > 0) ? STREAM_SUCCESS_SPACING_MS : STREAM_RETRY_BACKOFF_MS);
        }

        if (control_tx_ready && client_burst_remaining > 0 && now >= next_client_burst_attempt_ms) {
            const uint32_t burst_budget = (client_burst_remaining > STREAMS_PER_LOOP)
                                              ? STREAMS_PER_LOOP
                                              : client_burst_remaining;
            const uint32_t sent_now = send_client_stream(burst_budget);
            if (sent_now <= client_burst_remaining) {
                client_burst_remaining -= sent_now;
            } else {
                client_burst_remaining = 0;
            }
            if (sent_now > 0) {
                next_client_burst_attempt_ms = now + STREAM_SUCCESS_SPACING_MS;
            } else if (tcp_client == nullptr || !tcp_client->is_connected()) {
                next_client_burst_attempt_ms = now + STREAM_DISCONNECTED_BACKOFF_MS;
            } else if (client_send_failed_last) {
                next_client_burst_attempt_ms = now + STREAM_FAIL_STREAK_BACKOFF_MS;
            } else {
                next_client_burst_attempt_ms = now + STREAM_SUCCESS_SPACING_MS;
            }
        }

        if (control_tx_ready && server_burst_remaining == 0 && client_burst_remaining == 0 &&
            now - last_client_heartbeat_ms >= CLIENT_HEARTBEAT_MS) {
            send_client_stream(1);
            last_client_heartbeat_ms = now;
        }

        if (tcp_client != nullptr) {
            if (!tcp_client->is_connected()) {
                health.tcp_client_not_connected_ticks++;
            }
            if (!tcp_client->is_connected() &&
                (now - last_client_reconnect_ms >= CLIENT_RECONNECT_MS)) {
                health.tcp_client_reconnect_calls++;
                set_health_reason(
                    HEALTH_REASON_CLIENT_RECONNECT_POLL,
                    health.tcp_client_reconnect_calls
                );
                tcp_client->reconnect();
                last_client_reconnect_ms = now;
                if (CLIENT_RECONNECT_RECREATE_EVERY > 0 &&
                    health.tcp_client_reconnect_calls % CLIENT_RECONNECT_RECREATE_EVERY == 0) {
                    recreate_tcp_client(
                        HEALTH_REASON_CLIENT_RECREATE_WATCHDOG,
                        health.tcp_client_reconnect_calls
                    );
                }
            }
            if (health.tcp_client_send_fail_streak >= CLIENT_SEND_FAIL_RECREATE_THRESHOLD &&
                (now - last_client_fail_recreate_ms >= CLIENT_SEND_FAIL_RECREATE_MIN_INTERVAL_MS)) {
                recreate_tcp_client(
                    HEALTH_REASON_CLIENT_RECREATE_WATCHDOG,
                    health.tcp_client_send_fail_streak
                );
                last_client_fail_recreate_ms = now;
                next_client_burst_attempt_ms = now + STREAM_DISCONNECTED_BACKOFF_MS;
            }
        }

        if (now - last_led_toggle_ms >= LED_TOGGLE_MS) {
            led_instance.toggle();
            last_led_toggle_ms = now;
        }
    }
}

#else

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);
using ExampleTCPIPBoard = ST_LIB::Board<led>;

int main(void) {
    ExampleTCPIPBoard::init();
    auto& led_instance = ExampleTCPIPBoard::instance_of<led>();

    while (1) {
        led_instance.toggle();
        HAL_Delay(200);
    }
}

#endif // STLIB_ETH
#endif // EXAMPLE_TCPIP
