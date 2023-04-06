
#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f1xx.h"
#include "stdbool.h"

#define REG_SIZE 256


// прототипы функций
void init_dma(void);
void init_tim2(void);
void initClk(void);
void init_gpio(void);



#endif /* MAIN_H_ */
