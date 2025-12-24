#include "main.h"
#include "ST-LIB.hpp"
#include "FDCBootloader/BootloaderTFTP.hpp"

int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    // STLIB::start();
    STLIB::start(MAC::parse_string("00:80:e1:00:00:50"), IPV4::parse_string("192.168.0.27"), IPV4::parse_string("255.255.0.0"), IPV4::parse_string("192.168.1.1"));
    BTFTP::start();
    BTFTP::on(BTFTP::Mode::WRITE);
    

    while (1) {
        STLIB::update();
    }

}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
