/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define INIT(name,val)				int name=val
#define EQUAL2_OR(val,eq1,eq2)		(((eq1)==(val))||((eq2)==(val)))
#define LOOP_FOR(cnt,max) 			for(int (cnt)=0;(cnt)<(max);++(cnt))
#define INCR(x,step,max)  			(((x)+(step)>(max)) ? ((x)=(max)) : ((x)+=(step)))
#define DECR(x,step,min)  			(((x)-(step)<(min)) ? ((x)=(min)) : ((x)-=(step)))
#define INCR_WRAP(x,step,min,max)  	(((x)+(step)>(max)) ? ((x)=(min)) : ((x)+=(step)))
#define DECR_WRAP(x,step,min,max)  	(((x)-(step)<(min)) ? ((x)=(max)) : ((x)-=(step)))
#define STRUCT_TAB_SIZE(_struct) 	(sizeof(_struct) / sizeof(&_struct[0]))		/* if every element of table or structure is regular */
#define IS_RANGE(val,min,max) 		(((val)>=(min))&&((val)<=(max)))
#define TOOGLE(x) 					((x)=1-(x))

#define PRESSED_NONE	0
#define PRESSED_ONE		1
#define PRESSED_TWO		2
#define PRESSED_THREE	3
#define PRESSED_FOUR	4

typedef enum{
	press_0,press_1,press_2,press_3,press_4,press_5,press_6,press_7,press_8,press_9,press_10,press_11,press_12,press_13,press_14,
	press_15,press_16,press_17,press_18,press_19,press_20,press_21,press_22,press_23,press_24,press_25,press_26,press_27,press_28,
	press_29,press_30,press_31,press_32,press_33,press_34,press_35,press_36,press_37,press_38,press_39,press_40,press_41,press_42,
}KEYS_PRES;

typedef struct{
	GPIO_TypeDef* gpio;
	uint16_t pin;
}INPUT_PIN;

typedef struct{
	uint8_t mode;		/* number of the pressed pins */
	uint8_t code;		/* code for the number of the pressed pins */
}INPUT_PRESS;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
