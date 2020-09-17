/*
 * pwm.c
 *
 *  Created on: Sep 13, 2020
 *      Author: phil
 */

#include "bluenrg_x_device.h"
#include "BlueNRG1_conf.h"
#include "BlueNRG1_mft.h"
#include "beep.h"
#include "clock.h"
#define PWM0_PIN			GPIO_Pin_2
#define PWM1_PIN			GPIO_Pin_3


void beep_init() {
	GPIO_InitType GPIO_InitStructure;

	SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_GPIO, ENABLE);

	/* Configure PWM pins */
	GPIO_InitStructure.GPIO_Pin = PWM0_PIN | PWM1_PIN;
	GPIO_InitStructure.GPIO_Mode = Serial1_Mode;
	GPIO_InitStructure.GPIO_Pull = DISABLE;
	GPIO_InitStructure.GPIO_HighPwr = DISABLE;
	GPIO_Init( &GPIO_InitStructure);

	/* Enable MFTs */
	MFT_InitType timer_init;

	SysCtrl_PeripheralClockCmd(CLOCK_PERIPH_MTFX1 | CLOCK_PERIPH_MTFX2, ENABLE);

	MFT_StructInit(&timer_init);

	timer_init.MFT_Mode = MFT_MODE_1;
	timer_init.MFT_Prescaler = 16-1;      /* 1 us clock */

	/* MFT1 configuration */
	timer_init.MFT_Clock1 = MFT_PRESCALED_CLK;
	timer_init.MFT_Clock2 = MFT_NO_CLK;
	timer_init.MFT_CRA = 25000 - 1;       /* 250 ms positive duration */
	timer_init.MFT_CRB = 50000 - 1;       /* 500 ms negative duration */
	MFT_Init(MFT1, &timer_init);

	/* MFT2 configuration */
	timer_init.MFT_Clock1 = MFT_PRESCALED_CLK;
	timer_init.MFT_Clock2 = MFT_NO_CLK;
	timer_init.MFT_CRA = 5000 - 1;        /* 50 ms positive duration */
	timer_init.MFT_CRB = 10000 - 1;       /* 100 ms negative duration */
	MFT_Init(MFT2, &timer_init);

	/* Connect PWM output from MFT1/2 to TnA pin (PWM0/1) */
	MFT_TnXEN(MFT1, MFT_TnA, ENABLE);
	MFT_TnXEN(MFT2, MFT_TnA, ENABLE);
	beep_stop();
}

void beep_stop() {
	MFT_Cmd(MFT1, DISABLE);
	MFT_Cmd(MFT2, DISABLE);
	MFT1->TNCNT1 = 0;
	MFT1->TNCNT2 = 0;
	MFT2->TNCNT1 = 0;
	MFT2->TNCNT2 = 0;
	MFT1->TNMCTRL_b.TNAOUT = 0;
	MFT2->TNMCTRL_b.TNAOUT = 1;
}

void beep_start(int freq) {
	int delay = (1000000/2) / freq;
	MFT_SetCounter(MFT1, 0, 0);
	MFT_SetCounter(MFT2, 0, 0);
	MFT1->TNCRA = delay;
	MFT1->TNCRB = delay;
	MFT2->TNCRA = delay;
	MFT2->TNCRB = delay;
	MFT_Cmd(MFT1, ENABLE);
	MFT_Cmd(MFT2, ENABLE);
}
