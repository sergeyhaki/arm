
/*
 * main.h
 *
 *  Created on: Jul 26, 2014
 *      Author: qwer
 */

#ifndef MAIN_H_
#define MAIN_H_

#define F_HSE       8000000uL
#define F_CPU       48000000uL
#define HSE_VALUE   F_HSE

#define bin2bcd_u32 bin2bcd_u32_fdiv

#include <string.h>

#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_flash.h"
#include "stm32f0xx_wwdg.h"

#include "ul_other.h"
#include "ul_timing.h"
#include "ul_ptx.h"

#include "uart.h"
#include "pwm.h"
#include "adc.h"
#include "fft.h"
#include "setup.h"

#endif /* MAIN_H_ */
