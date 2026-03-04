#pragma once
#include "ST-LIB.hpp"

using namespace ST_LIB;
using ST_LIB::DigitalOutputDomain;
using ST_LIB::EthernetDomain;

//---------------------------ETHERNET---------------------------

#if defined(USE_PHY_LAN8742)
constexpr auto eth = EthernetDomain::Ethernet(EthernetDomain::PINSET_H10, "01:80:e1:55:04:07",
                                              "192.168.1.3", "255.255.255.0");
#elif defined(USE_PHY_LAN8700)
constexpr auto eth = EthernetDomain::Ethernet(EthernetDomain::PINSET_H10, "01:80:e1:55:04:07",
                                              "192.168.1.3", "255.255.255.0");
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth = EthernetDomain::Ethernet(EthernetDomain::PINSET_H11, "01:80:e1:55:04:07",
                                              "192.168.1.3", "255.255.255.0");
#else
#error "Ethernet PHY not defined"

#endif

//--------------------------------------------------------------

//---------------------------GPIOS---------------------------

constexpr DigitalOutputDomain::DigitalOutput led{ST_LIB::PB0};
constexpr DigitalOutputDomain::DigitalOutput actuator{ST_LIB::PE7};
constexpr DigitalOutputDomain::DigitalOutput tape_enable{ST_LIB::PG1};
constexpr DigitalOutputDomain::DigitalOutput regulator_out{ST_LIB::PB9};

//--------------------------------------------------------------

using MyBoard = ST_LIB::Board<led, eth,actuator,tape_enable, regulator_out>;

void init_board();

ST_LIB::EthernetDomain::Instance& get_ethernet();
ST_LIB::DigitalOutputDomain::Instance& get_nucleo_led();
ST_LIB::DigitalOutputDomain::Instance& get_actuator();
ST_LIB::DigitalOutputDomain::Instance& get_tape_enable();
ST_LIB::DigitalOutputDomain::Instance& get_regulator_out();
