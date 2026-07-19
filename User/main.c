#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/LED/led.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();

    printf("STM32F103ZET6 Template\r\n");

    while (1)
    {
        LED0_TOGGLE();
        delay_ms(500);
    }
}
