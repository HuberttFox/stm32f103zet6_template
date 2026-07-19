/**
 * oled.c - OLED 8080 并行模式驱动
 *
 * 硬件接口（8080 并行 8-bit）：
 *   D0..D7  → PC0..PC7  （8 位数据总线）
 *   RS      → PD3       （命令/数据选择：0=命令，1=数据）
 *   CS      → PD6       （片选，低电平有效）
 *   RD      → PG13      （读使能，低电平有效）
 *   WR      → PG14      （写使能，上升沿锁存数据）
 *   RST     → PG15      （硬件复位，低电平有效）
 *
 * 驱动方式：
 *   使用 128×8 字节帧缓冲区（g_oled_gram），每个 bit 对应一个像素。
 *   先写缓冲区，再通过 oled_refresh_gram() 批量刷入 OLED（页寻址模式，8 页 × 128 列）。
 *
 * 参考型号：SSD1306 驱动 0.96" OLED 128×64
 */

#include "BSP/OLED/oled.h"
#include "BSP/OLED/oledfont.h"
#include "SYSTEM/delay/delay.h"

/* 帧缓冲区：128 列 × 8 页（每页 8 像素，共 64 行） */
static uint8_t g_oled_gram[128][8];

static void oled_wr_byte(uint8_t data, uint8_t cmd);

/**
 * 刷缓冲区到 OLED 显存（页寻址模式）
 *
 * SSD1306 页寻址：每页 128 列，共 8 页（page 0-7，每页 8 行）。
 * 依次设置页地址 → 列低 4 位 → 列高 4 位 → 连续 128 字节数据。
 */
void oled_refresh_gram(void)
{
    uint8_t i, n;

    for (i = 0; i < 8; i++)
    {
        oled_wr_byte(0xB0 + i, OLED_CMD);       /* 设置页地址（0xB0 + page#） */
        oled_wr_byte(0x00, OLED_CMD);            /* 列起始地址低 4 位 */
        oled_wr_byte(0x10, OLED_CMD);            /* 列起始地址高 4 位 */

        for (n = 0; n < 128; n++)
        {
            oled_wr_byte(g_oled_gram[n][i], OLED_DATA);
        }
    }
}

/**
 * 在缓冲区中绘制一个像素点
 * @param x   列坐标（0-127）
 * @param y   行坐标（0-63）
 * @param dot 1=点亮，0=熄灭
 * @note 只修改缓冲区，需调用 oled_refresh_gram() 刷入 OLED
 */
void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot)
{
    uint8_t pos, bx, temp = 0;

    if (x > 127 || y > 63)
        return;

    pos = y / 8;                         /* 目标页索引 */
    bx = y % 8;                          /* 该页内 bit 偏移 */
    temp = 1 << bx;

    if (dot)
        g_oled_gram[x][pos] |= temp;
    else
        g_oled_gram[x][pos] &= ~temp;
}

/**
 * 在缓冲区显示一个 ASCII 字符
 * @param x,y  左上角坐标
 * @param chr  字符（0x20-0x7E）
 * @param size  字高：12/16/24
 * @param mode  0=白底黑字，1=黑底白字
 *
 * 字宽 = size/2。字库来自 oledfont.h：
 *   size=12 → oled_asc2_1206[95][12]
 *   size=16 → oled_asc2_1608[95][16]
 *   size=24 → oled_asc2_2412[95][36]
 */
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t *pfont = 0;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' ';

    if (size == 12)
    {
        pfont = (uint8_t *)oled_asc2_1206[chr];
    }
    else if (size == 16)
    {
        pfont = (uint8_t *)oled_asc2_1608[chr];
    }
    else if (size == 24)
    {
        pfont = (uint8_t *)oled_asc2_2412[chr];
    }
    else
    {
        return;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                oled_draw_point(x, y, mode);
            else
                oled_draw_point(x, y, !mode);

            temp <<= 1;
            y++;

            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/* 10 的 n 次幂（用于 oled_show_num 按位提取数字） */
static uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
        result *= m;

    return result;
}

/**
 * 在缓冲区显示一个无符号整数
 * @param x,y  左上角坐标
 * @param num  待显示数值
 * @param len  总显示位数（不足前补空格）
 * @param size 字高（12/16/24）
 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                oled_show_char(x + (size / 2) * t, y, ' ', size, 1);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }

        oled_show_char(x + (size / 2) * t, y, temp + '0', size, 1);
    }
}

/**
 * 在缓冲区显示一个字符串（ASCII 可打印字符 0x20-0x7E）
 * @param x,y  起始坐标
 * @param p    '\0' 结尾的字符串指针
 * @param size 字高（12/16/24）；字宽自动为 size/2
 *
 * 自动换行逻辑：
 *   - 当前行剩余宽度不足一个字符时，折到下一行（x=0, y+=size）
 *   - 超出屏幕高度时，清屏回到 (0,0)
 *   - 完成后需调用 oled_refresh_gram() 将缓冲区刷到 OLED
 */
void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size)
{
    while ((*p <= '~') && (*p >= ' '))
    {
        if (x > (128 - (size / 2)))
        {
            x = 0;
            y += size;
        }

        if (y > (64 - size))
        {
            y = x = 0;
            oled_clear();
        }

        oled_show_char(x, y, *p, size, 1);
        x += size / 2;
        p++;
    }
}

/**
 * OLED 硬件初始化（8080 并行模式）
 *
 * 引脚分配：
 *   PC0-PC7  数据总线     （推挽输出）
 *   PD3      RS 命令/数据 （推挽输出）
 *   PD6      CS 片选       （推挽输出）
 *   PG13     RD 读使能     （推挽输出）
 *   PG14     WR 写使能     （推挽输出）
 *   PG15     RST 硬件复位  （推挽输出）
 *
 * 初始化流程：GPIO 配置 → RST 复位序列 → SSD1306 寄存器初始化
 */
void oled_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    OLED_DATA_CLK_ENABLE();
    OLED_CS_CLK_ENABLE();
    OLED_RS_CLK_ENABLE();
    OLED_RD_CLK_ENABLE();
    OLED_WR_CLK_ENABLE();
    OLED_RST_CLK_ENABLE();

    gpio_init_struct.Pin = OLED_DATA_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_DATA_PORT, &gpio_init_struct);
    HAL_GPIO_WritePin(OLED_DATA_PORT, OLED_DATA_PIN, GPIO_PIN_RESET);

    gpio_init_struct.Pin = OLED_RS_PIN | OLED_CS_PIN;
    HAL_GPIO_Init(OLED_CS_PORT, &gpio_init_struct);
    HAL_GPIO_WritePin(OLED_CS_PORT, OLED_RS_PIN | OLED_CS_PIN, GPIO_PIN_SET);

    gpio_init_struct.Pin = OLED_RD_PIN | OLED_WR_PIN | OLED_RST_PIN;
    HAL_GPIO_Init(OLED_RD_PORT, &gpio_init_struct);
    HAL_GPIO_WritePin(OLED_RD_PORT, OLED_RD_PIN | OLED_WR_PIN | OLED_RST_PIN, GPIO_PIN_SET);

    OLED_WR(1);
    OLED_RD(1);
    OLED_CS(1);
    OLED_RS(1);

    OLED_RST(0);
    delay_ms(100);
    OLED_RST(1);

    /* ---------- SSD1306 初始化命令序列 ---------- */
    oled_wr_byte(0xAE, OLED_CMD);  /* 关闭显示 */
    oled_wr_byte(0xD5, OLED_CMD);  /* 设置显示时钟分频/振荡频率 */
    oled_wr_byte(80,   OLED_CMD);  /*   默认值 0x80 */
    oled_wr_byte(0xA8, OLED_CMD);  /* 设置多路复用比 */
    oled_wr_byte(0x3F, OLED_CMD);  /*   64 (128x64 模式) */
    oled_wr_byte(0xD3, OLED_CMD);  /* 设置显示偏移 */
    oled_wr_byte(0x00, OLED_CMD);  /*   无偏移 */
    oled_wr_byte(0x40, OLED_CMD);  /* 设置显示起始行 = 0 */
    oled_wr_byte(0x8D, OLED_CMD);  /* 电荷泵设置 */
    oled_wr_byte(0x14, OLED_CMD);  /*   开启电荷泵 */
    oled_wr_byte(0x20, OLED_CMD);  /* 设置内存寻址模式 */
    oled_wr_byte(0x02, OLED_CMD);  /*   页寻址模式 */
    oled_wr_byte(0xA1, OLED_CMD);  /* 列段重映射（左右镜像：取消镜像） */
    oled_wr_byte(0xC8, OLED_CMD);  /* COM 扫描方向（上下镜像：从 COM[N-1] 到 COM0） */
    oled_wr_byte(0xDA, OLED_CMD);  /* COM 引脚硬件配置 */
    oled_wr_byte(0x12, OLED_CMD);  /*   替代 COM 引脚配置 */
    oled_wr_byte(0x81, OLED_CMD);  /* 设置对比度 */
    oled_wr_byte(0xEF, OLED_CMD);  /*   最大对比度 */
    oled_wr_byte(0xD9, OLED_CMD);  /* 预充电周期 */
    oled_wr_byte(0xF1, OLED_CMD);  /*   1 时钟相位 1，15 时钟相位 2 */
    oled_wr_byte(0xDB, OLED_CMD);  /* VCOMH 电压倍率 */
    oled_wr_byte(0x30, OLED_CMD);  /*   0.83 × VCC */
    oled_wr_byte(0xA4, OLED_CMD);  /* 全局显示：全部像素跟随 RAM 内容 */
    oled_wr_byte(0xA6, OLED_CMD);  /* 正常显示（非反色） */
    oled_wr_byte(0xAF, OLED_CMD);  /* 打开显示 */
    oled_clear();
}

/**
 * 向 8 位数据总线输出一个字节
 * 保持高 8 位（PC8-PC15）不变
 *
 * 注意：Read-Modify-Write 操作，若中断或其它代码使用 PC8-PC15
 * 作为输出，此处会覆盖高 8 位状态，必要时添加关中断保护
 */
static void oled_data_out(uint8_t data)
{
    GPIOC->ODR = (GPIOC->ODR & 0xFF00) | (data & 0x00FF);
}

/**
 * 通过 8080 并行接口向 OLED 写入一个字节
 * @param data 待写入字节
 * @param cmd  0=命令，1=数据
 *
 * 时序：RS(cmd) → CS↓ → 数据输出至 D0-D7 → WR↓ → WR↑ → CS↑
 */
static void oled_wr_byte(uint8_t data, uint8_t cmd)
{
    OLED_RS(cmd);
    OLED_CS(0);
    oled_data_out(data);
    OLED_WR(0);
    OLED_WR(1);
    OLED_CS(1);
}

/**
 * 清空缓冲区并刷入 OLED
 */
void oled_clear(void)
{
    uint8_t x, page;

    for (x = 0; x < 128; x++)
    {
        for (page = 0; page < 8; page++)
        {
            g_oled_gram[x][page] = 0;
        }
    }

    oled_refresh_gram();
}
