#ifdef EXAMPLE_PACKETS

#include "Communications/Packets/DataPackets.hpp"
#include "Communications/Packets/OrderPackets.hpp"
#include "ST-LIB.hpp"
#include "main.h"

using namespace ST_LIB;

#ifndef STLIB_ETH
#error "EXAMPLE_PACKETS requires STLIB_ETH"
#endif

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

#if defined(USE_PHY_LAN8742)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:08",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_LAN8700)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:08",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H11,
    "00:80:e1:00:01:08",
    "192.168.1.7",
    "255.255.0.0"
);
#else
#error "No PHY selected for Ethernet pinset selection"
#endif

using ExamplePacketsBoard = ST_LIB::Board<eth, led>;

namespace {

constexpr uint16_t ORDER_ID_SET_SMALL_PROFILE = 0x5001;
constexpr uint16_t ORDER_ID_SET_LARGE_PROFILE = 0x5002;
constexpr uint16_t ORDER_ID_SET_EXTREMES = 0x5003;
constexpr uint16_t ORDER_ID_BUMP_STATE = 0x5004;
constexpr uint16_t ORDER_ID_SET_STATE_CODE = 0x5005;

constexpr uint32_t HEARTBEAT_PERIOD_MS = 250;
constexpr uint32_t SOCKET_RECONNECT_PERIOD_MS = 1000;

constexpr char BOARD_IP[] = "192.168.1.7";

bool enable_flag{false};
uint8_t small_counter{0};
uint16_t window_size{0};
uint32_t magic_value{0};
uint64_t big_counter{0};
int8_t trim_value{0};
int16_t offset_value{0};
int32_t position_value{0};
int64_t energy_value{0};
float ratio_value{0.0f};
double precise_value{0.0};

uint32_t tcp_order_count{0};
uint32_t udp_parse_count{0};
uint32_t heartbeat_ticks{0};
uint16_t last_order_code{0};

OrderPackets::order_mode order_mode{OrderPackets::order_mode::IDLE};
OrderPackets::order_state order_state{OrderPackets::order_state::BOOT};
DataPackets::mirror_mode mirror_mode{DataPackets::mirror_mode::IDLE};
DataPackets::mirror_state mirror_state{DataPackets::mirror_state::BOOT};

uint32_t probe_seq{0};
bool probe_toggle{false};
uint16_t probe_window{0};
float probe_ratio{0.0f};
DataPackets::probe_mode probe_mode{DataPackets::probe_mode::LOW};
uint32_t last_seen_probe_seq{0};

void sync_mirror_enums() {
    mirror_mode = static_cast<DataPackets::mirror_mode>(static_cast<uint8_t>(order_mode));
    mirror_state = static_cast<DataPackets::mirror_state>(static_cast<uint8_t>(order_state));
}

void process_orders() {
    if (OrderPackets::set_small_profile_flag) {
        OrderPackets::set_small_profile_flag = false;
        ++tcp_order_count;
        last_order_code = ORDER_ID_SET_SMALL_PROFILE;
    }

    if (OrderPackets::set_large_profile_flag) {
        OrderPackets::set_large_profile_flag = false;
        ++tcp_order_count;
        last_order_code = ORDER_ID_SET_LARGE_PROFILE;
    }

    if (OrderPackets::set_extremes_flag) {
        OrderPackets::set_extremes_flag = false;
        ++tcp_order_count;
        last_order_code = ORDER_ID_SET_EXTREMES;
    }

    if (OrderPackets::bump_state_flag) {
        OrderPackets::bump_state_flag = false;
        ++tcp_order_count;
        last_order_code = ORDER_ID_BUMP_STATE;
        const uint8_t next_state = (static_cast<uint8_t>(order_state) + 1U) % 4U;
        order_state = static_cast<OrderPackets::order_state>(next_state);
    }

    if (OrderPackets::set_state_code_flag) {
        OrderPackets::set_state_code_flag = false;
        ++tcp_order_count;
        last_order_code = ORDER_ID_SET_STATE_CODE;
    }

    sync_mirror_enums();
}

void process_probe_packet() {
    if (probe_seq == last_seen_probe_seq) {
        return;
    }
    last_seen_probe_seq = probe_seq;
    ++udp_parse_count;
}

} // namespace

int main(void) {
    Hard_fault_check();
    ExamplePacketsBoard::init();
    Scheduler::start();

    OrderPackets::set_small_profile_init(enable_flag, small_counter, offset_value, order_mode);
    OrderPackets::set_large_profile_init(
        window_size,
        magic_value,
        position_value,
        ratio_value,
        precise_value
    );
    OrderPackets::set_extremes_init(trim_value, energy_value, big_counter);
    OrderPackets::bump_state_init();
    OrderPackets::set_state_code_init(order_state);

    DataPackets::order_mirror_init(
        tcp_order_count,
        last_order_code,
        enable_flag,
        small_counter,
        offset_value,
        mirror_mode
    );
    DataPackets::numeric_mirror_init(
        window_size,
        magic_value,
        position_value,
        ratio_value,
        precise_value
    );
    DataPackets::extremes_mirror_init(trim_value, energy_value, big_counter, mirror_state);
    DataPackets::udp_probe_init(probe_seq, probe_toggle, probe_window, probe_ratio, probe_mode);
    DataPackets::udp_probe_echo_init(
        udp_parse_count,
        probe_seq,
        probe_toggle,
        probe_window,
        probe_ratio,
        probe_mode
    );
    DataPackets::heartbeat_snapshot_init(
        heartbeat_ticks,
        tcp_order_count,
        udp_parse_count,
        mirror_state
    );

    DataPackets::start();
    OrderPackets::start();

    auto& eth_instance = ExamplePacketsBoard::instance_of<eth>();
    auto& led_instance = ExamplePacketsBoard::instance_of<led>();

    sync_mirror_enums();

    uint32_t last_heartbeat_ms = HAL_GetTick();
    uint32_t last_reconnect_ms = HAL_GetTick();
    bool led_state = false;

    while (1) {
        eth_instance.update();
        Scheduler::update();

        process_orders();
        process_probe_packet();

        const uint32_t now = HAL_GetTick();
        if ((now - last_reconnect_ms) >= SOCKET_RECONNECT_PERIOD_MS) {
            last_reconnect_ms = now;
            if (OrderPackets::control_test_client != nullptr &&
                !OrderPackets::control_test_client->is_connected()) {
                OrderPackets::control_test_client->reconnect();
            }
            if (OrderPackets::control_test_tcp != nullptr &&
                !OrderPackets::control_test_tcp->is_connected() &&
                !OrderPackets::control_test_tcp->is_listening()) {
                delete OrderPackets::control_test_tcp;
                OrderPackets::control_test_tcp = new ServerSocket(BOARD_IP, 41000);
            }
        }

        if ((now - last_heartbeat_ms) >= HEARTBEAT_PERIOD_MS) {
            last_heartbeat_ms = now;
            ++heartbeat_ticks;
            sync_mirror_enums();
            led_state = !led_state;
            if (led_state) {
                led_instance.turn_on();
            } else {
                led_instance.turn_off();
            }
        }
    }
}

#endif // EXAMPLE_PACKETS
