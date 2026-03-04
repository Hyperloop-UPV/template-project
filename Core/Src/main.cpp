#include "main.h"

#include "ST-LIB.hpp"
#include "VCU/VCU.hpp"

int main(void) {
    
    VCU::init();
    

    while (1) {
        VCU::update();
    }
}

extern "C" void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
