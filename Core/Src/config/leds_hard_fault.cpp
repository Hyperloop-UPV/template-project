#include "HALAL/HALAL.hpp"
extern "C" {

#ifdef NUCLEO
GPIO_TypeDef* ports_hard_fault[] = {GPIOB, GPIOB, GPIOE};
uint16_t pins_hard_fault[] = {GPIO_PIN_0, GPIO_PIN_14, GPIO_PIN_1};
// //don't touch the count
uint8_t hard_fault_leds_count =
    (sizeof(ports_hard_fault) / sizeof(GPIO_TypeDef*) == sizeof(pins_hard_fault) / sizeof(uint16_t))
        ? sizeof(pins_hard_fault) / sizeof(uint16_t)
        : 0;

#endif

#ifdef BOARD
GPIO_TypeDef* ports_hard_fault[] = {GPIOG, GPIOG, GPIOG, GPIOG};
uint16_t pins_hard_fault[] = {GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8};
// //don't touch the count
uint8_t hard_fault_leds_count =
    (sizeof(ports_hard_fault) / sizeof(GPIO_TypeDef*) == sizeof(pins_hard_fault) / sizeof(uint16_t))
        ? sizeof(pins_hard_fault) / sizeof(uint16_t)
        : 0;
#endif
}
