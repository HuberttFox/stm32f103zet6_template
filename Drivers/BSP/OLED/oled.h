#ifndef _OLED_H
#define _OLED_H

/* OLED 驱动头文件，适用于 8080 并口模式 OLED。
 * 提供引脚定义、时钟使能宏、控制信号宏和公共接口声明。
 */

#include <stdint.h>
#include "SYSTEM/sys/sys.h"

/* OLED 8080 模式引脚定义 */

/* 片选脚 (CS)
 * 用于使能/禁用 OLED 器件，低电平通常为选中（取决于硬件接线）。
 * 这里将 CS 置于 PD6，并提供对应的时钟使能宏。
 */
#define OLED_CS_PORT GPIOD
#define OLED_CS_PIN GPIO_PIN_6
#define OLED_CS_CLK_ENABLE()          \
    do                                \
    {                                 \
        __HAL_RCC_GPIOD_CLK_ENABLE(); \
    } while (0) /* PD 口时钟使能 */

/* 命令/数据选择脚 (RS)
 * RS = 0 表示写命令，RS = 1 表示写数据。
 * 这里将 RS 连接到 PD3。
 */
#define OLED_RS_PORT GPIOD
#define OLED_RS_PIN GPIO_PIN_3
#define OLED_RS_CLK_ENABLE()          \
    do                                \
    {                                 \
        __HAL_RCC_GPIOD_CLK_ENABLE(); \
    } while (0) /* PD 口时钟使能 */

/* 读取使能脚 (RD)
 * RD 低电平时 OLED 允许将数据从 D0..D7 读回 MCU。
 * 当前驱动主要用于写操作，因此 RD 只作为可选读取控制线。
 */
#define OLED_RD_PORT GPIOG
#define OLED_RD_PIN GPIO_PIN_13
#define OLED_RD_CLK_ENABLE()          \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* PG 口时钟使能 */

/* 向 OLED 写入数据脚 (WR)
 * WR 产生下降沿时，OLED 读取数据总线上的并行数据。
 * 这里连接到 PG14。
 */
#define OLED_WR_PORT GPIOG
#define OLED_WR_PIN GPIO_PIN_14
#define OLED_WR_CLK_ENABLE()          \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* PG 口时钟使能 */

/* 复位脚 (RST)
 * RST 低电平有效，用于对 OLED 进行硬件复位。
 * 驱动初始化时通常先拉低复位再拉高恢复正常运行。
 */
#define OLED_RST_PORT GPIOG
#define OLED_RST_PIN GPIO_PIN_15
#define OLED_RST_CLK_ENABLE()         \
    do                                \
    {                                 \
        __HAL_RCC_GPIOG_CLK_ENABLE(); \
    } while (0) /* PG 口时钟使能 */

/* 数据总线 (D0..D7)
 * 使用 PC0..PC7 作为 8 位并行数据线。为防止宏展开后的优先级问题，
 * 将引脚掩码用括号包裹以确保在表达式中安全使用。
 */
#define OLED_DATA_PORT GPIOC
#define OLED_DATA_PIN (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define OLED_DATA_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOC_CLK_ENABLE(); \
    } while (0) /* PC 口时钟使能 */

/* OLED 8080 模式相关端口控制函数 定义 */
/* 控制引脚操作宏（将引脚设置为 0 或 1）
 * 使用三元表达式在宏内部调用 HAL 写引脚函数，调用处使用示例：OLED_CS(1);
 */
#define OLED_RST(x)                                                                                                                        \
    do                                                                                                                                     \
    {                                                                                                                                      \
        x ? HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_RESET); \
    } while (0) /* 设置 RST 引脚 */

#define OLED_CS(x)                                                                                                                     \
    do                                                                                                                                 \
    {                                                                                                                                  \
        x ? HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_RESET); \
    } while (0) /* 设置 CS 引脚 */
#define OLED_RS(x)                                                                                                                     \
    do                                                                                                                                 \
    {                                                                                                                                  \
        x ? HAL_GPIO_WritePin(OLED_RS_PORT, OLED_RS_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_RS_PORT, OLED_RS_PIN, GPIO_PIN_RESET); \
    } while (0) /* 设置 RS 引脚 */

#define OLED_WR(x)                                                                                                                     \
    do                                                                                                                                 \
    {                                                                                                                                  \
        x ? HAL_GPIO_WritePin(OLED_WR_PORT, OLED_WR_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_WR_PORT, OLED_WR_PIN, GPIO_PIN_RESET); \
    } while (0) /* 设置 WR 引脚 */

#define OLED_RD(x)                                                                                                                     \
    do                                                                                                                                 \
    {                                                                                                                                  \
        x ? HAL_GPIO_WritePin(OLED_RD_PORT, OLED_RD_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(OLED_RD_PORT, OLED_RD_PIN, GPIO_PIN_RESET); \
    } while (0) /* 设置 RD 引脚 */

/* 命令/数据 标志
 * OLED_CMD = 0 表示后续字节为命令，
 * OLED_DATA = 1 表示后续字节为显示数据。
 */
#define OLED_CMD 0  /* 写命令 */
#define OLED_DATA 1 /* 写数据 */

/* OLED 公共接口声明 */
void oled_init(void);                                                               /* OLED 初始化 */
void oled_refresh_gram(void);                                                       /* 刷新显示缓存到屏幕 */
void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot);                            /* 缓存层绘制单像素 */
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode); /* 显示单字符 */
void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size);           /* 显示字符串 */
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);  /* 显示数字 */
void oled_clear(void);                                                              /* 清屏 */

#endif