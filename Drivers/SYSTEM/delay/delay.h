#ifndef _DELAY_H
#define _DELAY_H

#include "SYSTEM/sys/sys.h"

void delay_init(uint16_t sysclk);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

void HAL_Delay(uint32_t Delay);

#endif
