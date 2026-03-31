#include "main.h"

#include <cstdio>

#include "ST-LIB.hpp"

using namespace ST_LIB;

// --- LED ---
constexpr auto led = DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

// --- DFSDM Clock Output: PE9 (DFSDM1_CKOUT, AF3) ---
// APB2 @ 137.5 MHz, clk_divider=14 → CKOUT = 137.5/14 ≈ 9.82 MHz
// NOTE: PC2 was AF6 in the library but DFSDM1_CKOUT needs AF3 — use PE9 instead
constexpr auto dfsdm_clk = DFSDM_CLK_DOMAIN::DFSDM_CLK(ST_LIB::PE9, 14);

// --- DFSDM Channel 1: PC3 (DFSDM1_DATIN1) ---
// Clock source: CKOUT from PE9, data sampled on rising edge
// Sinc3 filter, OSR=64, integrator=1 → ODR ≈ 153.5 kHz
// Regular, continuous conversion, no DMA (ISR-driven buffer)
constexpr DFSDM_CHANNEL_DOMAIN::Config_Channel sdm_ch_cfg{
    .spi_clock_sel = DFSDM_CHANNEL_DOMAIN::SPICKSel::NORMAL_CLK_OUT,
    .spi_type = DFSDM_CHANNEL_DOMAIN::SPI_Type::SPI_RISING,
};

constexpr DFSDM_CHANNEL_DOMAIN::Config_Filter sdm_flt_cfg{
    .filter = 0,
    .filter_type = DFSDM_CHANNEL_DOMAIN::Filter_Type::Sinc3,
    .oversampling = 64,
    .integrator = 1,
    .type_conv = DFSDM_CHANNEL_DOMAIN::Type_Conversion::Regular,
    .rcont = DFSDM_CHANNEL_DOMAIN::Regular_Mode::Continuous,
};

constexpr auto dfsdm_ch =
    DFSDM_CHANNEL_DOMAIN::DFSDM_CHANNEL(ST_LIB::PC3, sdm_ch_cfg, sdm_flt_cfg, 1);

// --- PWM Test Output: PD12 (TIM4_CH1, AF2) ---
constexpr ST_LIB::TimerPin pwm_out_pin{
    .af = ST_LIB::TimerAF::PWM,
    .pin = ST_LIB::PD12,
    .channel = ST_LIB::TimerChannel::CHANNEL_1,
};

constexpr ST_LIB::TimerDomain::Timer pwm_out_timer{
    ST_LIB::TimerRequest::GeneralPurpose_4,
    ST_LIB::TimerDomain::EMPTY_TIMER_NAME,
    pwm_out_pin,
};

constexpr uint32_t pwm_out_frequency_hz = 9821428U;
constexpr float pwm_out_duty_percent = 50.0f;

using MainBoard = ST_LIB::Board<led, dfsdm_clk, dfsdm_ch, pwm_out_timer>;

#ifndef EXAMPLE_SELECTED
int main(void) {
    MainBoard::init();

#ifdef HAL_UART_MODULE_ENABLED
    if (!UART::set_up_printf(UART::uart3)) {
        ErrorHandler("Unable to set up UART printf");
    }
    UART::start();
#endif

    auto& led_inst = MainBoard::instance_of<led>();
    auto& ch = MainBoard::instance_of<dfsdm_ch>();
    auto& pwm_inst = MainBoard::instance_of<pwm_out_timer>();

    ST_LIB::TimerWrapper<pwm_out_timer> pwm_timer(&pwm_inst);
    auto pwm_out = pwm_timer.get_pwm<pwm_out_pin>();

    // Override the default low-speed GPIO drive for the timer output.
    MODIFY_REG(GPIOD->OSPEEDR, GPIO_OSPEEDR_OSPEED12_Msk, (0x3U << GPIO_OSPEEDR_OSPEED12_Pos));

    pwm_out.configure(pwm_out_frequency_hz, pwm_out_duty_percent);
    pwm_out.turn_on();

    const float pwm_actual_hz = static_cast<float>(pwm_timer.get_clock_frequency()) /
                                static_cast<float>(pwm_timer.get_period() + 1U);

    // Calculated values:
    //   CKOUT   = APB2 / clk_divider = 137.5 MHz / 14 ≈ 9.82 MHz
    //   ODR     = CKOUT / OSR        = 9.82 MHz / 64 ≈ 153.5 kHz
    //   Latency (Sinc3, OSR=64, IOSR=1): OSR*(IOSR-1+ford)+ford = 64*(0+3)+3 = 195 cycles
    printf("\r\n=== DFSDM Sigma-Delta Modulator Debug ===\r\n");
    printf("CKOUT pin   : PE9 (DFSDM1_CKOUT, AF3), clk_div=14, ~9.82 MHz\r\n");
    printf("DATA  pin   : PC3 (DFSDM1_DATIN1), channel=1\r\n");
    printf("Filter      : Sinc3, OSR=64, integrator=1\r\n");
    printf("ODR         : ~153.5 kHz\r\n");
    printf(
        "PWM out pin : PD12 (TIM4_CH1), %.3f MHz, %.1f%% duty\r\n",
        pwm_actual_hz / 1.0e6f,
        pwm_out.get_duty_cycle()
    );
    printf("Conv mode   : Regular, Continuous, no DMA\r\n");
    printf("latency_cfg : %lu cycles\r\n\r\n", ch.latency_cycles);

    // Register dump before starting conversion
    printf("--- Register state after init ---\r\n");
    printf(
        "DFSDMEN        : %lu\r\n",
        (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN_Msk) >> DFSDM_CHCFGR1_DFSDMEN_Pos
    );
    {
        const uint32_t ckoutdiv =
            (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_CKOUTDIV_Msk) >> DFSDM_CHCFGR1_CKOUTDIV_Pos;
        printf("CKOUTDIV       : %lu  (~%.2f MHz)\r\n", ckoutdiv, 137.5f / (float)(ckoutdiv + 1));
    }
    printf("CH1 CHCFGR1    : 0x%08lX\r\n", ch.channel_regs->CHCFGR1);
    printf("CH1 CHCFGR2    : 0x%08lX\r\n", ch.channel_regs->CHCFGR2);
    printf("FLT0 FLTCR1    : 0x%08lX\r\n", ch.filter_regs->FLTCR1);
    printf("FLT0 FLTCR2    : 0x%08lX\r\n", ch.filter_regs->FLTCR2);
    printf("FLT0 FLTFCR    : 0x%08lX\r\n", ch.filter_regs->FLTFCR);
    printf("ch.is_enabled  : %d\r\n", ch.is_enabled());
    printf("\r\n");

    auto gpio_mode_to_str = [](uint32_t mode) -> const char* {
        switch (mode) {
        case 0:
            return "INPUT!";
        case 1:
            return "OUTPUT!";
        case 2:
            return "AF-OK";
        default:
            return "ANALOG!";
        }
    };

    // GPIO debug: PE9 (DFSDM_CKOUT), PC3 (DFSDM_DATIN1) and PD12 (PWM_OUT)
    printf("--- GPIO state (PE9=CKOUT, PC3=DATIN1, PD12=PWM_OUT) ---\r\n");

    const uint32_t pe9_mode = (GPIOE->MODER >> (2 * 9)) & 0x3;
    const uint32_t pe9_af = (GPIOE->AFR[1] >> (4 * (9 - 8))) & 0xF;
    const uint32_t pc3_mode = (GPIOC->MODER >> (2 * 3)) & 0x3;
    const uint32_t pc3_af = (GPIOC->AFR[0] >> (4 * 3)) & 0xF;
    const uint32_t pd12_mode = (GPIOD->MODER >> (2 * 12)) & 0x3;
    const uint32_t pd12_af = (GPIOD->AFR[1] >> (4 * (12 - 8))) & 0xF;
    const uint32_t pd12_speed = (GPIOD->OSPEEDR >> (2 * 12)) & 0x3;

    printf("PE9  MODER=%lu (%s)  AF=%lu\r\n", pe9_mode, gpio_mode_to_str(pe9_mode), pe9_af);
    printf("PC3  MODER=%lu (%s)  AF=%lu\r\n", pc3_mode, gpio_mode_to_str(pc3_mode), pc3_af);
    printf(
        "PD12 MODER=%lu (%s)  AF=%lu  SPEED=%lu\r\n",
        pd12_mode,
        gpio_mode_to_str(pd12_mode),
        pd12_af,
        pd12_speed
    );
    printf("\r\n");

    // Trigger first regular conversion (continuous mode auto-restarts after this)
    DFSDM_CHANNEL_DOMAIN::start_reg_conv_filter(0);

    printf("%-10s  %-12s  %-8s  %-10s  %-12s\r\n", "t_ms", "raw", "enabled", "lat_hw", "FLTISR");
    printf("--------------------------------------------------------------\r\n");

    uint32_t loop_count = 0;
    while (1) {
        const int32_t raw = ch.read();
        const uint32_t fltisr = ch.filter_regs->FLTISR;
        const uint32_t lat_hw = ch.check_latency_cycles();

        printf(
            "%-10lu  %-12ld  %-8d  %-10lu  0x%08lX\r\n",
            HAL_GetTick(),
            raw,
            ch.is_enabled(),
            lat_hw,
            fltisr
        );

        // Periodic full register snapshot every 20 prints (~2 s)
        ++loop_count;
        if ((loop_count % 20U) == 0U) {
            printf("\r\n--- Register snapshot (t=%lu ms) ---\r\n", HAL_GetTick());
            printf("CH1 CHCFGR1  : 0x%08lX\r\n", ch.channel_regs->CHCFGR1);
            printf("CH1 CHCFGR2  : 0x%08lX\r\n", ch.channel_regs->CHCFGR2);
            printf("FLT0 FLTCR1  : 0x%08lX\r\n", ch.filter_regs->FLTCR1);
            printf("FLT0 FLTCR2  : 0x%08lX\r\n", ch.filter_regs->FLTCR2);
            printf("FLT0 FLTFCR  : 0x%08lX\r\n", ch.filter_regs->FLTFCR);
            printf("FLT0 FLTISR  : 0x%08lX\r\n", ch.filter_regs->FLTISR);
            printf("FLT0 FLTRDATAR: 0x%08lX\r\n", ch.filter_regs->FLTRDATAR);
            printf("FLT0 FLTEXMIN: 0x%08lX\r\n", ch.filter_regs->FLTEXMIN);
            printf("FLT0 FLTEXMAX: 0x%08lX\r\n", ch.filter_regs->FLTEXMAX);
            printf("------------------------------------\r\n\r\n");
        }

        led_inst.toggle();
        HAL_Delay(100);
    }
}
#endif

extern "C" void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
