#include "main.h"
#include "ST-LIB.hpp"

#include "ST-LIB_LOW/DigitalInput2.hpp"
#include "ST-LIB_LOW/DigitalOutput2.hpp"

using ST_LIB::DigitalInputDomain;
using ST_LIB::DigitalOutputDomain;

constexpr DigitalOutputDomain::DigitalOutput led1{ST_LIB::PB0};
constexpr DigitalOutputDomain::DigitalOutput led2{ST_LIB::PE1};
constexpr DigitalOutputDomain::DigitalOutput led3{ST_LIB::PB14};
int main(void) {
  STLIB::start();

  using myBoard = ST_LIB::Board<led1, led2, led3>;
  myBoard::init();
  auto &green_led = myBoard::instance_of<led1>();
  auto &yellow_led = myBoard::instance_of<led2>();
  auto &gred_led = myBoard::instance_of<led3>();

  Time::register_low_precision_alarm(100, [&]() {
    green_led.toggle();
    yellow_led.toggle();
    gred_led.toggle();
  });

  while (1) {
    STLIB::update();
  }
}

void Error_Handler(void) {
  ErrorHandler("HAL error handler triggered");
  while (1) {
  }
}

extern "C" {
void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
  }
}
}
