#ifndef _TPAD_H
#define _TPAD_H
#include "SYSTEM/sys/sys.h"

#define TPAD_GPIO_PORT                  GPIOA
#define TPAD_GPIO_PIN                   GPIO_PIN_1

#define TPAD_TIMX_CAP                   TIM5
#define TPAD_TIMX_CAP_CHY               TIM_CHANNEL_2

#define TPAD_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define TPAD_TIMX_CAP_CHY_CLK_ENABLE()  do{ __HAL_RCC_TIM5_CLK_ENABLE(); }while(0)

#define TPAD_GATE_VAL                   100

uint8_t tpad_init(uint16_t psc);
uint16_t tpad_get_maxval(uint8_t n);
uint8_t tpad_scan(uint8_t mode);

#endif
