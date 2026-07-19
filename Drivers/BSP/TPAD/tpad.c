/**
 * tpad.c - 电容触摸按键驱动（RC 充电计时法）
 *
 * 原理：
 *   触摸焊盘对地存在分布电容 Cp。触摸时，人体电容 Ch 并联到 Cp，
 *   总电容增加 → RC 充电时间变长。通过 TIM5_CH2 输入捕获测量充电
 *   上升沿时间，与无触摸时的基准值比较，判断是否按下。
 *
 * 硬件接口：
 *   TPAD 焊盘 → PA1 (TIM5_CH2)
 *   充电电阻为 PCB 上拉电阻（典型 1MΩ ~ 10MΩ）
 *
 * 时序：
 *   1. 拉低 PA1 放电 5ms
 *   2. 切为浮空输入，TIM5 计数器从 0 开始
 *   3. 外部 RC 电路充电，PA1 电平上升
 *   4. TIM5_CH2 上升沿捕获，记录充电时间
 *
 * 时钟假设：SYSCLK=72MHz, APB1=36MHz, TIM5 倍频至 72MHz
 *   psc=72 时定时器频率 = 1MHz，每 tick = 1µs
 */

#include "BSP/TPAD/tpad.h"

/* TIM5 输入捕获句柄 */
TIM_HandleTypeDef g_timx_cap_chy_handle;

/* 无触摸时的基准充电时间（tpad_init 时校准）
 * 仅在 tpad.c 内部使用，加 static 封装，避免外部意外引用 */
static uint16_t g_tpad_default_val;

/* 按键消抖计数器：按下后连按抑制 3 次扫描 */
static uint8_t keyen;

/**
 * 初始化 TIM5_CH2 输入捕获
 * @param arr  自动重装值（建议 65535，最大化测量范围）
 * @param psc  预分频值（决定 tick 精度，通常 psc-1）
 *
 * PA1 配置为浮空输入，TIM5_CH2 上升沿捕获，
 * 无预分频、无滤波，直接 TI2 信号。
 */
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

    timx_cap_chy_handle.ICPolarity = TIM_ICPOLARITY_RISING;       /* 上升沿捕获 */
    timx_cap_chy_handle.ICSelection = TIM_ICSELECTION_DIRECTTI;   /* 直接 TI2 输入 */
    timx_cap_chy_handle.ICPrescaler = TIM_ICPSC_DIV1;             /* 无预分频 */
    timx_cap_chy_handle.ICFilter = 0;                             /* 无滤波 */
    HAL_TIM_IC_ConfigChannel(&g_timx_cap_chy_handle, &timx_cap_chy_handle, TPAD_TIMX_CAP_CHY);

    HAL_TIM_IC_Start(&g_timx_cap_chy_handle, TPAD_TIMX_CAP_CHY);
}

/**
 * 复位触摸焊盘并启动一次充电测量
 *
 * 将 PA1 设为推挽输出并拉低 5ms（确保完全放电），
 * 清除捕获标志、计数器归零，再切回浮空输入。
 * 此时外部 RC 电路开始充电，TIM5_CH2 等待上升沿。
 */
static void tpad_reset(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    gpio_init_struct.Pin = TPAD_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(TPAD_GPIO_PORT, &gpio_init_struct);

    HAL_GPIO_WritePin(TPAD_GPIO_PORT, TPAD_GPIO_PIN, GPIO_PIN_RESET);

    HAL_Delay(5);   /* 放电 5ms */

    __HAL_TIM_CLEAR_FLAG(&g_timx_cap_chy_handle, TIM_FLAG_CC2);
    g_timx_cap_chy_handle.Instance->CNT = 0;

    gpio_init_struct.Pin = TPAD_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(TPAD_GPIO_PORT, &gpio_init_struct);
}

/**
 * 获取一次充电时间测量值
 * @return 充电时间（TIM5 计数值），单位取决于 psc
 *
 * 放电并复位后等待上升沿捕获。若超时（计数器 > 65535-500 即
 * 接近溢出仍未捕获），则直接返回当前计数值避免死循环。
 *
 * 正常返回值在几百 ~ 几千范围内；触摸时因电容增大，返回数值显著偏高。
 */
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

/**
 * 电容触摸按键初始化 & 基准值校准
 * @param psc  定时器预分频值（72 → 1µs/tick，36 → 0.5µs/tick，依此类推）
 * @return 0=成功，1=初始化失败（无触摸基准值异常偏高）
 *
 * 校准流程：
 *   1. 初始化 TIM5_CH2 输入捕获
 *   2. 连续采样 10 次充电时间
 *   3. 冒泡排序，去除最小 2 个和最大 2 个
 *   4. 取中间 6 个的均值作为基准值 g_tpad_default_val
 *   5. 若基准值 > 32767 则返回失败（硬件可能未连接）
 */
uint8_t tpad_init(uint16_t psc)
{
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

    /* 冒泡排序（升序） */
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

    /* 去头尾各 2 个极值，取中间 6 个平均 */
    for (i = 2; i < 8; i++)
    {
        sum += buf[i];
    }
    g_tpad_default_val = sum / 6;

    if (g_tpad_default_val > (65535 / 2))
    {
        return 1;   /* 基准异常：可能硬件未正确连接 */
    }

    return 0;
}

/**
 * 连续采样 n 次，取充电时间的最大值
 * @param n  采样次数
 * @return   其中最大的计数值
 *
 * 用于 tpad_scan 中判断是否触摸：触摸时充电变慢，
 * 多次采样取最大值能提高检测灵敏度。
 */
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

/**
 * 电容触摸按键扫描
 * @param mode  0=单次模式（按下返回 1 后需松开才能再次触发）
 *              1=连续模式（一直按下时持续返回 1）
 * @return 0=未按下，1=按下
 *
 * 检测逻辑：
 *   - 采样充电时间（单次 3 次 / 连续 6 次），取最大值
 *   - 若 > (基准值 + TPAD_GATE_VAL) 则判为按下
 *   - 按下后 keyen 置 3，每扫描一次减 1；只有 keyen==0 时
 *     才允许触发新的按键事件（硬件消抖 + 防止连按）
 *
 * 阈值 TPAD_GATE_VAL 在 tpad.h 中定义（默认 100），
 * 可根据实际焊盘灵敏度调整。
 */
uint8_t tpad_scan(uint8_t mode)
{
    uint8_t res = 0;
    uint8_t sample = 3;
    uint16_t rval = 0;

    if (mode)
    {
        sample = 6;    /* 连续模式多采样，抗抖动 */
        keyen = 0;
    }

    rval = tpad_get_maxval(sample);

    if (rval > (g_tpad_default_val + TPAD_GATE_VAL))
    {
        if (keyen == 0)
        {
            res = 1;   /* 首次检测到按下 */
        }
        keyen = 3;     /* 启动消抖计数器 */
    }

    if (keyen)
    {
        keyen--;
    }

    return res;
}
