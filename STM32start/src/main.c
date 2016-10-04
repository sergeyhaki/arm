/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "stm32f0xx_conf.h"
#include "1.h"

/*
 * main.c
 *
 *  Created on: Jul 26, 2014
 *      Author: qwer
 */

u8 ps;

int main(){
	setup();

//	pwm_set16(0, 30);
//	pwm_set16(1, 0);
//	pwm_set16(2, 65535);
//	pwm_set16(3, 65535);
//	pwm_set16(4, 65535);
//	pwm_set16(5, 65535);
//	pwm_set16(6, 65535);
//	pwm_set16(7, 65535);

	while (1){

		t_sync();
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
			t_update();

//			static u16 d;
//			if (t.ms10){
//				pwm_set8(0xFF, d);
//				d += 1;
//			}

	//		if (t.ms500){
//				if (ps) GPIOB->BSRR = GPIO_Pin_1; else GPIOB->BRR = GPIO_Pin_1;
//				ps = !ps;

//				uart_tx_c(0, 'U');

//				uart_tx_h16(0, adc.raw); uart_tx_c(0,' ');
//				uart_tx_sh16(0, adc.min); uart_tx_c(0,' ');
//				uart_tx_sh16(0, adc.max); uart_tx_c(0,' ');

//				uart_tx_h32(0, adc.sum);

//				uart_tx_s32(0, adc.cur, 0);
//				uart_tx_c(0,'\r'); uart_tx_c(0,'\n');
	//		}
		}

		ptx_dispatch();
		uart_update();
	}

	return 0;
}








/*
**************************************************************************
int main(void)
{
  while(1)
  {

  }
}
*/
