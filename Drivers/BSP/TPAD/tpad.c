#include "BSP/TPAD/tpad.h"

TIM_HandleTypeDef g_timx_cap_chy_handle;
uint16_t g_tpad_default_val;
static uint8_t keyen;

static void tpad_timx_cap_init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    TIM_IC_InitTypeDef timx_cap_chy_handle = {0};

    TPAD_GPIO_CLK_ENABLE();
    TPAD_TIMX_CAP_CHY_CLK_ENABLE();

    gpio_init_struct.Pin = TPAD_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(TPAD_GPIO_PORT, &gpio_init_struct);

    g_timx_cap_chy_handle.Instance = TPAD_TIMX_CAP;
    g_timx_cap_chy_handle.Init.Prescaler = psc;
    g_timx_cap_chy_handle.Init.Period = arr;
    g_timx_cap_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    g_timx_cap_chy_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_IC_Init(&g_timx_cap_chy_handle);

    timx_cap_chy_handle.ICPolarity = TIM_ICPOLARITY_RISING;
    timx_cap_chy_handle.ICSelection = TIM_ICSELECTION_DIRECTTI;
    timx_cap_chy_handle.ICPrescaler = TIM_ICPSC_DIV1;
    timx_cap_chy_handle.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&g_timx_cap_chy_handle, &timx_cap_chy_handle, TPAD_TIMX_CAP_CHY);

    HAL_TIM_IC_Start(&g_timx_cap_chy_handle, TPAD_TIMX_CAP_CHY);
}

static void tpad_reset(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    gpio_init_struct.Pin = TPAD_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(TPAD_GPIO_PORT, &gpio_init_struct);

    HAL_GPIO_WritePin(TPAD_GPIO_PORT, TPAD_GPIO_PIN, GPIO_PIN_RESET);

    HAL_Delay(5);

    __HAL_TIM_CLEAR_FLAG(&g_timx_cap_chy_handle, TIM_FLAG_CC2);
    g_timx_cap_chy_handle.Instance->CNT = 0;

    gpio_init_struct.Pin = TPAD_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(TPAD_GPIO_PORT, &gpio_init_struct);
}

static uint16_t tpad_get_val(void)
{
    tpad_reset();

    while (__HAL_TIM_GET_FLAG(&g_timx_cap_chy_handle, TIM_FLAG_CC2) == RESET)
    {
        if (g_timx_cap_chy_handle.Instance->CNT > (65535 - 500))
        {
            return g_timx_cap_chy_handle.Instance->CNT;
        }
    }

    return HAL_TIM_ReadCapturedValue(&g_timx_cap_chy_handle, TPAD_TIMX_CAP_CHY);
}

uint8_t tpad_init(uint16_t psc)
{
    /* 时钟假设: SYSCLK=72MHz -> APB1=36MHz -> TIM5 倍频至 72MHz
       psc=72 时定时器频率 = 72MHz/72 = 1MHz, 每 tick = 1us */
    uint8_t i, j;
    uint16_t temp = 0;
    uint32_t sum = 0;
    uint16_t buf[10];

    if (psc == 0) return 1;

    tpad_timx_cap_init(65535, psc - 1);

    for (i = 0; i < 10; i++)
    {
        buf[i] = tpad_get_val();
        HAL_Delay(10);
    }

    for (i = 0; i < 10 - 1; i++)
    {
        for (j = 0; j < 10 - 1 - i; j++)
        {
            if (buf[j] > buf[j + 1])
            {
                temp = buf[j];
                buf[j] = buf[j + 1];
                buf[j + 1] = temp;
            }
        }
    }

    for (i = 2; i < 8; i++)
    {
        sum += buf[i];
    }
    g_tpad_default_val = sum / 6;

    if (g_tpad_default_val > (65535 / 2))
    {
        return 1;
    }

    return 0;
}

uint16_t tpad_get_maxval(uint8_t n)
{
    uint16_t temp = 0;
    uint16_t maxval = 0;

    if (n == 0) return 0;

    while (n--)
    {
        temp = tpad_get_val();
        if (maxval < temp)
        {
            maxval = temp;
        }
    }
    return maxval;
}

uint8_t tpad_scan(uint8_t mode)
{
    uint8_t res = 0;
    uint8_t sample = 3;
    uint16_t rval = 0;

    if (mode)
    {
        sample = 6;
        keyen = 0;
    }

    rval = tpad_get_maxval(sample);

    if (rval > (g_tpad_default_val + TPAD_GATE_VAL))
    {
        if (keyen == 0)
        {
            res = 1;
        }
        keyen = 3;
    }

    if (keyen)
    {
        keyen--;
    }

    return res;
}
