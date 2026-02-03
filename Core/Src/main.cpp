#include "Examples/ExampleMPU.cpp"
#include "Examples/ExamplesHardFault.cpp"

#include "main.h"
#include "ST-LIB.hpp"
#include "Datagram_Packets.hpp"

using ST_LIB::TimerDomain;
using ST_LIB::TimerRequest;

uint32_t val = 5;
 [[maybe_unused]]auto packets = create_all_packets();
ServerSocket *ControlStationSocket;
DatagramSocket *datagramSocket;
void send_packets_(){
  for(auto x : packets){
    datagramSocket->send_packet(*x);
  }
  
}

int main(void) { 
  using myBoard = ST_LIB::Board<>;
  myBoard::init();
  Hard_fault_check();
  STLIB::start("69:64:69:34:08:36","192.168.1.7","255.255.0.0","192.168.0.1");
  ControlStationSocket = new ServerSocket("192.168.1.7",50500);
  datagramSocket = new DatagramSocket("192.168.1.7",50400,"192.168.0.9",50400);

  [[maybe_unused]] const auto& orders = create_all_orders();
 
  Scheduler::register_task(100000,send_packets_);
  Scheduler::start();

  while (1) {
    STLIB::update();
    Scheduler::update();
  }
}
void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
