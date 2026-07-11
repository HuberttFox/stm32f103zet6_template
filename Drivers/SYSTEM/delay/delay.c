#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"

static uint16_t  g_fac_us = 0;

void delay_init(uint16_t sysclk)
{
    /* This layer owns SysTick after initialization; do not rely on stock HAL tick timeout semantics. */
    SysTick->CTRL = 0;
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);

    g_fac_us = sysclk / 8;
}

void delay_us(uint32_t nus)
{
    uint32_t temp;
    uint32_t max_nus = 0xFFFFFF / g_fac_us;

    if (nus > max_nus)
    {
        nus = max_nus;
    }

    SysTick->LOAD = nus * g_fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= 1 << 0;

    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));

    SysTick->CTRL &= ~(1 << 0);
    SysTick->VAL = 0x00;
}

void delay_ms(uint32_t nms)
{
    uint32_t repeat = nms / 1000;
    uint32_t remain = nms % 1000;

    while (repeat)
    {
        delay_us(1000 * 1000);
        repeat--;
    }

    if (remain)
    {
        delay_us(remain * 1000);
    }
}

void HAL_Delay(uint32_t Delay)
{
     delay_ms(Delay);
}
