#include "main.h"
#include "ST-LIB.hpp"
#include "FDCBootloader/BootloaderTFTP.hpp"

void ExitBootloader(void) {
    const uint32_t MAIN_APP_ADDRESS = 0x08000000;

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    HAL_DeInit();

    SCB->VTOR = MAIN_APP_ADDRESS;

    uint32_t main_stack_pointer = *(__IO uint32_t*)MAIN_APP_ADDRESS;

    __set_MSP(main_stack_pointer);

    uint32_t jump_address = *(__IO uint32_t*)(MAIN_APP_ADDRESS + 4);
    void (*reset_handler)(void) = (void (*)(void))jump_address;

    reset_handler();
}



int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    // STLIB::start();
    STLIB::start(MAC::parse_string("00:80:e1:00:00:50"), IPV4::parse_string("192.168.0.27"), IPV4::parse_string("255.255.0.0"), IPV4::parse_string("192.168.1.1"));
    BTFTP::start();
    BTFTP::on(BTFTP::Mode::WRITE);
    

    while (1) {
        if( BTFTP::end_bootloader ) {
            ExitBootloader();
        }
        STLIB::update();
    }

}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
