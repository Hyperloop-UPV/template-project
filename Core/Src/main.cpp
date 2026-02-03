#include "Examples/ExampleMPU.cpp"
#include "Examples/ExamplesHardFault.cpp"

#include "main.h"
#include "ST-LIB.hpp"
#include "Datagram_Packets.hpp"
using ST_LIB::TimerDomain;
using ST_LIB::TimerRequest;

uint32_t val = 5;
HeapPacket* pack1 = new HeapPacket(900,&val);
ServerSocket *ControlStationSocket;
DatagramSocket *datagramSocket;
void send_packets_(void* data){
  datagramSocket->send_packet(*pack1);
}
constexpr TimerDomain::Timer general_purpose_timer{{
    .name = {'t','i','m','g','p','0','0','\0'},
    .request = TimerRequest::AnyGeneralPurpose,
  } /*, aquí van los pines */};
int main(void) { 
  //Hard_fault_check();
  using myBoard = ST_LIB::Board<general_purpose_timer>;
  myBoard::init();
  STLIB::start("69:64:69:34:08:36","192.168.1.7","255.255.0.0","192.168.0.1");
  ControlStationSocket = new ServerSocket("192.168.1.7",50500);
  datagramSocket = new DatagramSocket("192.168.1.7",50400,"192.168.0.9",50400);

  [[maybe_unused]] const auto& orders = create_all_orders();
  auto packets = create_all_packets();
  
  auto tim_gp0 = get_timer_instance(myBoard,general_purpose_timer);

  tim_gp0.configure16bit<200*100>(send_packets_,&packets,20000);
  tim_gp0.enable_nvic();
  tim_gp0.enable_update_interrupt();

  while (1) {
    STLIB::update();
  }
}
void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
