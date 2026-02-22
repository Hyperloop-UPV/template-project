#ifdef EXAMPLE_LINEAR_SENSOR_CHARACTERIZATION

#include "Communications/Packets/DataPackets.hpp"
#include "Communications/Packets/OrderPackets.hpp"
#include "ST-LIB.hpp"
#include "main.h"

using namespace ST_LIB;

double slope{1.0};
double offset{0.0};

double value{0.0};
double sensor_value{0.0};

double real_value{0.0};

constinit float raw_value{0.0f};
constexpr auto sensor = ADCDomain::ADC(
    ST_LIB::PA0,
    raw_value,
    ADCDomain::Resolution::BITS_16,
    ADCDomain::SampleTime::CYCLES_8_5
);

#if defined(USE_PHY_LAN8742)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_LAN8700)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H11,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#else
#error "No PHY selected for Ethernet pinset selection"
#endif
using ExampleEthernetBoard = ST_LIB::Board<eth, sensor>;

extern "C" void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}

void characterize(float raw, double read) {
    // Incremental OLS accumulators for y = slope * x + offset.
    static uint64_t sample_count{0};
    static double sum_x{0.0};
    static double sum_y{0.0};
    static double sum_xx{0.0};
    static double sum_xy{0.0};

    const double x = static_cast<double>(raw);
    const double y = read;

    ++sample_count;
    sum_x += x;
    sum_y += y;
    sum_xx += x * x;
    sum_xy += x * y;

    if (sample_count < 2) {
        offset = y - (slope * x);
        return;
    }

    const double n = static_cast<double>(sample_count);
    const double denominator = (n * sum_xx) - (sum_x * sum_x);
    if (denominator == 0.0) {
        return;
    }

    slope = ((n * sum_xy) - (sum_x * sum_y)) / denominator;
    offset = (sum_y - (slope * sum_x)) / n;
}

int main(void) {
    Hard_fault_check();
    ExampleEthernetBoard::init();

    Scheduler::start();
    // Comms
    OrderPackets::characterize_init(real_value);
    DataPackets::characterization_init(slope, offset);
    DataPackets::Value_init(sensor_value, value);
    DataPackets::start();

    OrderPackets::start();

    // Instances
    auto& eth_instance = ExampleEthernetBoard::instance_of<eth>();
    auto& sensor_instance = ExampleEthernetBoard::instance_of<sensor>();

    while (1) {
        eth_instance.update();
        Scheduler::update();

        sensor_instance.read();
        sensor_value = static_cast<double>(raw_value);
        value = slope * sensor_value + offset;

        if (OrderPackets::characterize_flag) {
            OrderPackets::characterize_flag = false;
            characterize(raw_value, real_value);
            DataPackets::packets_socket->send_packet(*DataPackets::characterization_packet);
        }
    }
}
#endif
