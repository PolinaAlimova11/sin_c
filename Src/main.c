
#include <stdint.h>
#include "main.h"


uint16_t reg_ccr[REG_SIZE]={128,131,134,137,140,143,146,149,153,156,159,162,
		165,168,171,174,177,180,182,185,188,191,194,196,199,201,204,206,
		209,211,214,216,218,220,223,225,227,229,231,232,234,236,238,239,
		241,242,243,245,246,247,248,249,250,251,252,253,253,254,254,255,
		255,255,255,255,255,255,255,255,255,254,254,253,253,252,251,251,
		250,249,248,247,245,244,243,241,240,238,237,235,233,232,230,228,
		226,224,222,219,217,215,213,210,208,205,203,200,198,195,192,190,
		187,184,181,178,175,172,169,166,163,160,157,154,151,148,145,142,
		139,136,132,129,126,123,120,117,114,110,107,104,101,98,95,92,89,
		86,83,80,77,74,71,69,66,63,60,58,55,52,50,47,45,43,40,38,36,
		34, 32, 29, 27, 26, 24, 22, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 4, 4, 3,
		2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 13, 14, 16, 17, 19, 21, 22, 24, 26, 28, 30, 32, 34, 36, 39, 41, 43,
		46, 48, 51, 53, 56, 58, 61, 64, 67, 69, 72, 75, 78, 81, 84, 87, 90, 93, 96,	99, 102, 105, 108, 111, 115, 118, 121, 124, 127};//259
// регистр CCR2 размером в 16 бит, поэтому будем передавать значения в 16 бит


void init_dma(void)
{
	// включаем тактирование на шине
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// TIM2_UP расположен на 2 канале
	DMA1_Channel2->CPAR = (uint32_t)&TIM2->CCR2;	// указываем, куда будет идти запись из памяти
	DMA1_Channel2->CMAR = (uint32_t)reg_ccr;		// указываем, откуда будут браться данные
	DMA1_Channel2->CNDTR = REG_SIZE;				// указываем длину массива

	DMA1_Channel2->CCR = 0;							//зануляем весь регистр на всякий случай

	DMA1_Channel2->CCR &=~ DMA_CCR_MSIZE;			// зануляем 00 (память)
	DMA1_Channel2->CCR |= DMA_CCR_MSIZE_0;			// устанавливаем значение в 01, передача 16 бит

	DMA1_Channel2->CCR &=~ DMA_CCR_PSIZE;			// зануляем 00
	DMA1_Channel2->CCR |= DMA_CCR_PSIZE_0;			// устанавливаем 16 бит (периферия)

	DMA1_Channel2->CCR |= DMA_CCR_CIRC;				// устанавливаем режим цикличности

	DMA1_Channel2->CCR |= DMA_CCR_DIR;			// направление чтения из памяти в периферию
	DMA1_Channel2->CCR |= DMA_CCR_MINC;				// включаем инкремент в памяти


	//включаем DMA

	DMA1_Channel2->CCR |= DMA_CCR_EN;

}

void init_tim2(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;	// включаем тактирование шины

	//Частота APB1 для таймеров = APB1Clk * 2 = 32МГц * 2 = 64МГц
	TIM2-> PSC = 640 - 1;	// выставляем предделитель, частота 100 000 Гц
	TIM2 -> ARR = 256-1;	// выставляем значение, до которого считает таймер
	TIM2->CCR2 = 1;		// заполнение

	//настраиваем выход, разрешаем шим сигнал на выход на соответствующий пин, активный выход - высокий
	TIM2->CCER |= TIM_CCER_CC2E;		//шим включен на пин
	TIM2->CCER &=~ TIM_CCER_CC2P;		// высокий активный выход
	TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;	// прямой шим + буферизация регистра
	// 110: режим 1 PWM - при счёте вверх, канал 1 находится в активном состоянии пока
	//TIMx_CNT<TIMx_CCR2, иначе канал будет в неактивном состоянии;
	TIM2->CR1 &=~ TIM_CR1_DIR; // выставляем 0, то есть считать +1

	TIM2->DIER |= TIM_DIER_UDE;	// событие обновление счета активирует DMA

	TIM2->CR1 |= TIM_CR1_CEN; // включаем таймер

}

/**
  * @brief  Инициализация систем тактирования:
  * 		Источник тактирования: HSI
  * 		Частота: 64МГц
  * @param  None
  * @retval None
  */
void initClk(void)
{
	// Enable HSI
	RCC->CR |= RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY)){};

	// Enable Prefetch Buffer
	FLASH->ACR |= FLASH_ACR_PRFTBE;

	// Flash 2 wait state
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	// HCLK = SYSCLK
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

	// PCLK2 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

	// PCLK1 = HCLK
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

	// PLL configuration: PLLCLK = HSI/2 * 16 = 64 MHz
	RCC->CFGR &= ~RCC_CFGR_PLLSRC;
	RCC->CFGR |= RCC_CFGR_PLLMULL16;

	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;

	// Wait till PLL is ready
	while((RCC->CR & RCC_CR_PLLRDY) == 0) {};

	// Select PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	// Wait till PLL is used as system clock source
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){};
}

void init_gpio(void)
{
	//PA1
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN; //включаем тактирование на шину, включаем тактирование
	// альтернативных функций

	// 10-11
	GPIOA->CRL &=~ GPIO_CRL_CNF1;	//зануляем поле CNF
	GPIOA->CRL |= GPIO_CRL_CNF1_1;
	GPIOA->CRL |= GPIO_CRL_MODE1;	// записываем все 11 в поле mode



}




int main(void)
{
	initClk();
	init_tim2();
	init_gpio();
	init_dma();
    while(true){

    }
}
