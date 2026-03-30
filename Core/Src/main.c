/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "ssd1306.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define KEYS_MAX	4
#define KEY_PLUS_FREQ		GPIOF, GPIO_PIN_2
#define KEY_MINUS_FREQ		GPIOC, GPIO_PIN_14
#define KEY_PLUS_TUNN		GPIOA, GPIO_PIN_13
#define KEY_MINUS_TUNN		GPIOA, GPIO_PIN_14

#define MAX_RADIO_CHANNEL	24
#define SIZE_RADIO_NAME		30
#define DIV_ACT	2
#define STEP_FREQ	12.000000
#define DIV_FREQ	75.000000
#define INTER_F1	10.700000

#define WAIT_FOR_ALL_RELEASE	while(INPUT_IsAnyPress(port,KEYS_MAX))

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct{
	char radioName[SIZE_RADIO_NAME];
	double freq;
	uint16_t freqStep;
	uint16_t freqDiv;
	double freqOffs;
} RADIOPARAM;

typedef struct{
	char *pName[MAX_RADIO_CHANNEL];
	uint8_t selRadio;
	RADIOPARAM Radio[MAX_RADIO_CHANNEL];
} STRUCT_VISUALPARAM;

static INPUT_PIN port[KEYS_MAX]={{KEY_MINUS_FREQ},{KEY_PLUS_FREQ},{KEY_MINUS_TUNN},{KEY_PLUS_TUNN}};
static STRUCT_VISUALPARAM Test;
static uint16_t toogleVar[3]={0x01};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static int RADIO_SetFreq(int nr){ 	/* PCF8575_Init(); */
	INIT(temp1,PCF8575_Write(0,Test.Radio[nr].freqStep));
	INIT(temp2,PCF8575_Write(1,Test.Radio[nr].freqDiv));
	if(EQUAL2_OR(-1,temp1,temp2)) return 0; else return 1;
}

static void RADIO_CalcFreqDiv(int nr){
	Test.Radio[nr].freqDiv = ((double)Test.Radio[nr].freqStep+1) * (((Test.Radio[nr].freq+Test.Radio[nr].freqOffs)-INTER_F1-DIV_FREQ)/(DIV_ACT*STEP_FREQ)) - 1;
/*	Test.Radio[i].freq 	  = INTER_F1 + DIV_FREQ + DIV_ACT*STEP_FREQ*((Test.Radio[i].freqDiv+1)/(Test.Radio[i].freqStep+1)); */
}

char tempBuff[40];
char tempBuff2[40];

#include <stdint.h>
#include <math.h>
const double round_nums[8] =
{ 0.5, 0.05, 0.005, 0.0005, 0.00005,0.000005,0.0000005,0.00000005};
void float2stri(char *buffer, float value, unsigned int dec_digits)
{
	int idx;
	int64_t dbl_int, dbl_frac;
	int64_t mult = 1;
	char *output = buffer;
	char tbfr[40];

	if (isfinite(value))
	{
		if ((value <= -99999999999999) || (value >= 99999999999999))
		{
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
			*output++ = '-';
		}
		else
		{
			if (value < 0.0)
			{
				*output++ = '-';
				value *= -1.0;
			}

			if (dec_digits <8)
			{
				value += round_nums[dec_digits];
				for (idx = 0; idx < dec_digits; idx++)
					mult *= 10;
			}
			else
			{
				dec_digits = 7;
				value += round_nums[7];
				mult = 10000000;
			}

			dbl_int = (int64_t) value;
			dbl_frac = (int64_t) ((value - (float) dbl_int) * (float) mult);

			idx = 0;
			while (dbl_int != 0)
			{
				tbfr[idx++] = '0' + (dbl_int % 10);
				dbl_int /= 10;
			}

			if (idx == 0)
				*output++ = '0';
			else
			{
				while (idx > 0)
				{
					*output++ = tbfr[idx - 1];
					idx--;
				}
			}

			if (dec_digits > 0)
			{
				*output++ = '.';

				idx = 0;
				while (dbl_frac != 0)
				{
					tbfr[idx++] = '0' + (dbl_frac % 10);
					dbl_frac /= 10;
				}
				while (idx < dec_digits)
					tbfr[idx++] = '0';

				if (idx == 0)
					*output++ = '0';
				else
				{
					while (idx > 0)
					{
						*output++ = tbfr[idx - 1];
						idx--;
					}
				}
			}
		}
	}
	else
	{
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
		*output++ = '-';
	}
	*output = 0;
}

static void RADIO_DispFreq(void){
	RADIO_CalcFreqDiv(Test.selRadio);
	if(RADIO_SetFreq(Test.selRadio));

	ssd1306_Fill(Black); ssd1306_UpdateScreen(&hi2c2); ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(Test.Radio[Test.selRadio].radioName, Font_7x10, White);
	ssd1306_SetCursor(0, 36);
	float2stri(tempBuff,Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs,2);
	ssd1306_WriteString(tempBuff, Font_16x26, White);
	ssd1306_UpdateScreen(&hi2c2);
}

static void FUNC_TunningFreq(int k){ switch(k){
  case -1: return;
  case  0: INCR( Test.Radio[Test.selRadio].freqOffs, 0.01, 1.00 ); break;
  case  1: DECR( Test.Radio[Test.selRadio].freqOffs, 0.01,-1.00 ); break; }
	RADIO_DispFreq();
}





static void VisualParam_LCD_Reset(void)
{
	Test.Radio[0].freq= 101.6;		strcpy(Test.Radio[0].radioName,"Radio Krakow");
	Test.Radio[1].freq=  87.8;		strcpy(Test.Radio[1].radioName,"RMF Classic");
	Test.Radio[2].freq=  89.4;		strcpy(Test.Radio[2].radioName,"Jedynka");
	Test.Radio[3].freq=  88.8;		strcpy(Test.Radio[3].radioName,"Eska 2");
	Test.Radio[4].freq=  99.4;		strcpy(Test.Radio[4].radioName,"Trojka");
	Test.Radio[5].freq=  96.0;		strcpy(Test.Radio[5].radioName,"RMF FM");
	Test.Radio[6].freq= 104.1;		strcpy(Test.Radio[6].radioName,"Radio Zet");
	Test.Radio[7].freq=  96.7;		strcpy(Test.Radio[7].radioName,"RMF MAXXX");
	Test.Radio[8].freq= 106.1;		strcpy(Test.Radio[8].radioName,"Radio Plus");
	Test.Radio[9].freq=  90.6;		strcpy(Test.Radio[9].radioName,"Radio Maryja");		//PCF8575_SetVAl( (0x12<<8)|0xBF, (0x2<<8)|0x1E );  //00000010  00011110 Radio Maryja
	Test.Radio[10].freq= 92.5;		strcpy(Test.Radio[10].radioName,"Zlote przeboje");
	Test.Radio[11].freq= 102.4;	strcpy(Test.Radio[11].radioName,"Radio Pogoda");
	Test.Radio[12].freq= 103.0;	strcpy(Test.Radio[12].radioName,"Radio Katowice");
	Test.Radio[13].freq= 102.9;	strcpy(Test.Radio[13].radioName,"TOK FM");
	Test.Radio[14].freq=  97.7;	strcpy(Test.Radio[14].radioName,"Eska");
	Test.Radio[15].freq=  93.7;	strcpy(Test.Radio[15].radioName,"Chillizet");
	Test.Radio[16].freq= 104.9;	strcpy(Test.Radio[16].radioName,"Eska Rock");
	Test.Radio[17].freq= 107.0;	strcpy(Test.Radio[17].radioName,"VOX FM");
	Test.Radio[18].freq= 101.0;	strcpy(Test.Radio[18].radioName,"AntyRadio");
	Test.Radio[19].freq= 102.0;	strcpy(Test.Radio[19].radioName,"Dwojka");
	Test.Radio[20].freq=  97.2;	strcpy(Test.Radio[20].radioName,"Polskie Radio 24");
	Test.Radio[21].freq= 103.8;	strcpy(Test.Radio[21].radioName,"Rock Radio");
	Test.Radio[22].freq=  95.2;	strcpy(Test.Radio[22].radioName,"Radio Wnet");
	Test.Radio[23].freq= 100.5;	strcpy(Test.Radio[23].radioName,"Radio Famka");

	LOOP_FOR(i,MAX_RADIO_CHANNEL){	Test.pName[i]		   = Test.Radio[i].radioName;
									Test.Radio[i].freqStep = (0x12<<8)|0xBF;
									Test.Radio[i].freqOffs = 0.0;
									RADIO_CalcFreqDiv(i);
	}
	Test.selRadio=0;
	RADIO_DispFreq();
}

static void SERVICE_0press_1111(void){

}

static void SERVICE_1press_1110(void){
	INCR(Test.selRadio,1,MAX_RADIO_CHANNEL-1);
	RADIO_DispFreq();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_1press_1101(void){
	DECR(Test.selRadio,1,0);
	RADIO_DispFreq();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_1press_1011(void){
	FUNC_TunningFreq(0);
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_1press_0111(void){
	FUNC_TunningFreq(1);
	WAIT_FOR_ALL_RELEASE;
}

static void SERVICE_2press_1100(void){

}
static void SERVICE_2press_1010(void){

}
static void SERVICE_2press_0110(void){

}
static void SERVICE_2press_1001(void){

}
static void SERVICE_2press_0101(void){

}
static void SERVICE_2press_0011(void){
	if(PCF8575_SetVAl( (0x12<<8)|0xBF, (0x2<<8)|0x1E ))  //00000010  00011110 Radio Maryja
		PCF8575_Error();
	WAIT_FOR_ALL_RELEASE;
}

static void SERVICE_3press_1000(void){

}
static void SERVICE_3press_0100(void){

}
static void SERVICE_3press_0010(void){

}
static void SERVICE_3press_0001(void){

}

static void SERVICE_4press_0000(void){
	if(TOOGLE(toogleVar[0])){  if(PCF8575_SetVAl(0xFFFF,0xFFFF)) PCF8575_Error(); }
	else 					{  RADIO_DispFreq();								  }
	WAIT_FOR_ALL_RELEASE;
}

void SERVICE_InputPress(void){
	INPUT_PRESS press = INPUT_GetPressPin(port,KEYS_MAX);
	switch(press.mode){
	  	case PRESSED_NONE:
	  					SERVICE_0press_1111();
	  	  	break;
	   case PRESSED_ONE:
	  		switch(press.code){
	  		  	case 1: SERVICE_1press_1110(); break;
	  		  	case 2: SERVICE_1press_1101(); break;
	  		  	case 3: SERVICE_1press_1011(); break;
	  		  	case 4: SERVICE_1press_0111(); break;
	  		  	default: break; }
	  	  	break;
	  	case PRESSED_TWO:
	  		switch(press.code){
	  		  	case 5:  SERVICE_2press_1100(); break;
	  		  	case 6:  SERVICE_2press_1010(); break;
	  		  	case 7:  SERVICE_2press_0110(); break;
	  		  	case 9:  SERVICE_2press_1001(); break;
	  		  	case 11: SERVICE_2press_0101(); break;
	  		  	case 15: SERVICE_2press_0011(); break;
	  		  	default: break; }
	  	  	break;
	  	case PRESSED_THREE:
	  		switch(press.code){
	  		  	case 16: SERVICE_3press_1000(); break;
	  		  	case 18: SERVICE_3press_0100(); break;
	  		  	case 22: SERVICE_3press_0010(); break;
	  		  	case 34: SERVICE_3press_0001(); break;
	  		  	default: break; }
	  	  	break;
	  	case PRESSED_FOUR:
	  		switch(press.code){
	  		  	case 35: SERVICE_4press_0000(); break;
	  		  	default: break; }
	  	  	break;

	    default:
	     break;
	  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  VisualParam_LCD_Reset();
  MX_GPIO_ChangeConfigSWDpin(6000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // Init lcd using one of the stm32HAL i2c typedefs

  //HAL_Delay(2000);



  ssd1306_SetDevAddr(0x7A); if (ssd1306_Init(&hi2c2) != 0)/* Error_Handler()*/;
 // ssd1306_InvertColors();
  ssd1306_Fill(Black); ssd1306_UpdateScreen(&hi2c2); ssd1306_SetCursor(0, 0);
  ssd1306_WriteString(Test.Radio[Test.selRadio].radioName, Font_7x10, White);
  ssd1306_SetCursor(0, 36);
  float2stri(tempBuff,Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs,2);
  ssd1306_WriteString(tempBuff, Font_16x26, White);
  ssd1306_UpdateScreen(&hi2c2);


//  Test.selRadio++;
//  ssd1306_SetDevAddr(0x78); if (ssd1306_Init(&hi2c2) != 0)/* Error_Handler()*/;		ssd1306_Fill(White); ssd1306_UpdateScreen(&hi2c2); ssd1306_SetCursor(0, 0);
// // ssd1306_InvertColors();
//  ssd1306_WriteString(Test.Radio[Test.selRadio].radioName, Font_7x10, Black);
//  ssd1306_SetCursor(0, 36);
//  float2stri(tempBuff,Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs,2);
//  ssd1306_WriteString(tempBuff, Font_16x26, Black);
//  ssd1306_UpdateScreen(&hi2c2);



  // Draw rectangle on screen
//  for (uint8_t i=0; i<28; i++) {
//      for (uint8_t j=0; j<64; j++) {
//          ssd1306_DrawPixel(100+i, 0+j, White);
//      }
//  }


  // Copy all data from local screenbuffer to the screen


  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

//	  HAL_Delay(1000);
//	  PCF8575_Write(0,0xaa55);
//	  HAL_Delay(1000);
//	  PCF8575_Write(0,0x55aa);
//	  HAL_Delay(1000);
//	  PCF8575_Write(0,0xaa55);
//	  HAL_Delay(1000);
//	  PCF8575_Write(0,0x55aa);
//	  HAL_Delay(1000);
//	  PCF8575_Write(0,0xaa55);






	  SERVICE_InputPress();
	  //HAL_Delay(20);


//	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_13));
//	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_14));

//	  if(INPUT_ChekState(port,KEYS_MAX));
//
//	  if(MX_GPIO_IsPress(KEY_PLUS_FREQ)){
//		  INCR_WRAP(Test.selRadio,1,0,MAX_RADIO_CHANNEL);
//		  RADIO_DispFreq();
//		  MX_GPIO_WaitForRelease(KEY_PLUS_FREQ);
//	  }
//	  if(MX_GPIO_IsPress(KEY_MINUS_FREQ)){
//		  DECR_WRAP(Test.selRadio,1,0,MAX_RADIO_CHANNEL);
//		  RADIO_DispFreq();
//		  MX_GPIO_WaitForRelease(KEY_MINUS_FREQ);
//	  }

//
//	  if(MX_GPIO_IsPress(KEY_PLUS_TUNN)){
//		  INCR_WRAP(Test.selRadio,1,0,MAX_RADIO_CHANNEL);
//		  FUNC_TunningFreq(0);
//		  MX_GPIO_WaitForRelease(KEY_PLUS_TUNN);
//	  }
//	  if(MX_GPIO_IsPress(KEY_MINUS_TUNN)){
//		  DECR_WRAP(Test.selRadio,1,0,MAX_RADIO_CHANNEL);
//		  FUNC_TunningFreq(1);
//		  MX_GPIO_WaitForRelease(KEY_MINUS_TUNN);
//	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  asm("nop");
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
