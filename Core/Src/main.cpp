#if 0

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

using MainBoard = ST_LIB::Board<led>;

#if !defined(EXAMPLE_ADC) && !defined(EXAMPLE_ETHERNET) &&                     \
    !defined(EXAMPLE_MPU) && !defined(EXAMPLE_HARDFAULT)
int main(void) {
  MainBoard::init();

  auto &led_instance = MainBoard::instance_of<led>();

  while (1) {
    led_instance.toggle();
    HAL_Delay(200);
  }
}
#endif

extern "C" void Error_Handler(void) {
  ErrorHandler("HAL error handler triggered");
  while (1) {
  }
}
#endif

#include "ST-LIB.hpp"
#include "main.h"

#include "Communications/Packets/DataPackets.hpp"
#include "Communications/Packets/OrderPackets.hpp"

using namespace ST_LIB;

double slope{1.0};
double offset{0.0};

double raw_value{0.0};
double value{0.0};

double real_value{0.0};

constinit float sensor_value{0.0f};
constexpr auto sensor =
    ADCDomain::ADC(ST_LIB::PA0, sensor_value, ADCDomain::Resolution::BITS_12,
                   ADCDomain::SampleTime::CYCLES_8_5);

#if defined(USE_PHY_LAN8742)
constexpr auto eth =
    EthernetDomain::Ethernet(EthernetDomain::PINSET_H10, "00:80:e1:00:01:07",
                             "192.168.1.7", "255.255.0.0");
#elif defined(USE_PHY_LAN8700)
constexpr auto eth =
    EthernetDomain::Ethernet(EthernetDomain::PINSET_H10, "00:80:e1:00:01:07",
                             "192.168.1.7", "255.255.0.0");
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth =
    EthernetDomain::Ethernet(EthernetDomain::PINSET_H11, "00:80:e1:00:01:07",
                             "192.168.1.7", "255.255.0.0");
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

  // Comms
  OrderPackets::characterize_init(real_value);
  DataPackets::characterization_init(slope, offset);
  DataPackets::value_init(raw_value, value);
  DataPackets::start();

  // Instances
  auto &eth_instance = ExampleEthernetBoard::instance_of<eth>();
  auto &sensor_instance = ExampleEthernetBoard::instance_of<sensor>();

  while (1) {
    eth_instance.update();

    const float raw = sensor_instance.get_raw();
    raw_value = static_cast<double>(raw);
    value = static_cast<double>(sensor_instance.get_value_from_raw(raw));
    sensor_value = static_cast<float>(value);

    if (OrderPackets::characterize_flag) {
      OrderPackets::characterize_flag = false;
      characterize(raw, real_value);
      DataPackets::packets_socket->send_packet(*DataPackets::characterization);
    }
  }
}
