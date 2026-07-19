#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/LED/led.h"
#include "BSP/OLED/oled.h"

int main(void)
{
    uint8_t t = 0;

    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    oled_init();
    oled_show_string(0, 0, "Hello World", 24);
    oled_show_string(0, 24, "0.96' OLED TEST", 16);
    oled_show_string(64, 52, "CODE:", 12);
    oled_refresh_gram();

    printf("STM32F103ZET6 Template\r\n");
    t = ' ';
    while (1)
    {
        oled_show_char(36, 52, t, 12, 1);
        oled_show_num(94, 52, t, 3, 12);
        oled_refresh_gram();
        t++;

        if (t > '~')
        {
            t = ' ';
        }

        delay_ms(500);
        LED0_TOGGLE();
    }
}
