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
#include "string_oper.h"
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
#define MAX_DISPLAYs			2

#define SIZE_MAIN_BUFF	 100

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
static uint8_t display[MAX_DISPLAYs]={0x7A,0x7A};
static char mainBuff[SIZE_MAIN_BUFF]={0};

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

static void ERROR_pcf8575(void){
	SSD1306_DispBK(display[0],NoInvert);
	SSD1306_TxtMidd(0,0,"ERROR",Font_16x26,NoInvert);
	SSD1306_TxtMiddX(0,SSD1306_posY()+Font_16x26.FontHeight+3," PCF8575 ",Font_7x10,NoInvert);
	ssd1306_UpdateScreen();
}
static void ERROR_ssd1306(void){
	;
}

static void RADIO_InitScreen(void){
	SSD1306_DispBK(display[0],NoInvert);
	SSD1306_Txt(0,0,"Radio",Font_16x26,NoInvert);	SSD1306_Txt(SSD1306_posX()+5,0,"FM",Font_16x26,NoInvert);
	SSD1306_TxtMiddX(0,SSD1306_posY()+Font_16x26.FontHeight+6,"Synteza",Font_7x10,NoInvert);
	SSD1306_TxtMiddX(0,SSD1306_posY()+Font_7x10.FontHeight+3,"czestotliwosci",Font_7x10,NoInvert);
	ssd1306_UpdateScreen();
}

static int RADIO_SetFreq(int nr){ 	/* PCF8575_Init(); */
	return PCF8575_SetVAl( Test.Radio[nr].freqStep, Test.Radio[nr].freqDiv);
}

static void RADIO_CalcFreqDiv(int nr){
	Test.Radio[nr].freqDiv = ((double)Test.Radio[nr].freqStep+1) * (((Test.Radio[nr].freq+Test.Radio[nr].freqOffs)-INTER_F1-DIV_FREQ)/(DIV_ACT*STEP_FREQ)) - 1;
}
static void RADIO_CalcFreq(int nr){
	Test.Radio[nr].freq    = INTER_F1 + DIV_FREQ + DIV_ACT*STEP_FREQ*(((double)Test.Radio[nr].freqDiv+1)/((double)Test.Radio[nr].freqStep+1));
}

static int RADIO_CorrectFreqForSelChannel(uint8_t nrChannel,uint16_t newFreqDiv){
	Test.selRadio = nrChannel;
	int i = Test.selRadio;
	Test.Radio[i].freqDiv = newFreqDiv;
	RADIO_CalcFreq(i);
	if(PCF8575_SetVAl( Test.Radio[i].freqStep, Test.Radio[i].freqDiv ))
		return 1;
	return 0;
}

static char* GetFreqToBuff(uint16_t idx){
	mainBuff[idx]=' ';
	if(Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs < 100.00) 	float2stri(mainBuff+idx+1,Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs,2);
	else																			float2stri(mainBuff+idx,  Test.Radio[Test.selRadio].freq+Test.Radio[Test.selRadio].freqOffs,2);
	return mainBuff+idx;
}

static void DispFreqScreen(uint8_t nrDispl)
{
	uint8_t sizeRect=7;
	uint8_t rectPosY=24;
	uint8_t descrPosY=rectPosY-Font_7x10.FontHeight-1;
	uint16_t value;

	SSD1306_DispBK(nrDispl,NoInvert);
	SSD1306_Txt(0,0, Test.Radio[Test.selRadio].radioName, Font_7x10,NoInvert);




	toogleVar[1]=2;


	if(toogleVar[1])
	{
		if(1==toogleVar[1]) value=Test.Radio[Test.selRadio].freqStep;
		else				value=Test.Radio[Test.selRadio].freqDiv;

		SSD1306_Txt(0, descrPosY, HexToAscii(value,	  0) /*Int2Str(val_1,Space,3,Sign_none)*/, Font_7x10,NoInvert);
		SSD1306_Txt(64,descrPosY, HexToAscii(value>>8,0) /*Int2Str(val_2,Space,3,Sign_none)*/, Font_7x10,NoInvert);
		LOOP_FOR(i,16){
			if(CHECK_bit(value,i)) SSD1306_roundRect (8*i,rectPosY,sizeRect,sizeRect);
			else   				   SSD1306_roundFrame(8*i,rectPosY,sizeRect,sizeRect);
		}
	}





	SSD1306_Txt(0,37,GetFreqToBuff(0), Font_16x26,NoInvert);
	SSD1306_Txt( SSD1306_posX()+10, MIDDLE(37,Font_16x26.FontHeight,Font_7x10.FontHeight), "MHz", Font_7x10,NoInvert);
	ssd1306_UpdateScreen();
}

static void RADIO_DispFreq(void){
	RADIO_CalcFreqDiv(Test.selRadio);
	if(RADIO_SetFreq(Test.selRadio)) ERROR_pcf8575();
	DispFreqScreen(display[0]);
}

static void FUNC_TunningFreq(int k){ switch(k){
  case -1: return;
  case  0: INCR( Test.Radio[Test.selRadio].freqOffs, 0.01, 1.00 ); break;
  case  1: DECR( Test.Radio[Test.selRadio].freqOffs, 0.01,-1.00 ); break; }
	RADIO_DispFreq();
}

static void VisualParam_LCD_Reset(void)
{
	Test.Radio[0].freq= 101.6;		strcpy(Test.Radio[0].radioName,"Radio Krakow    ");
	Test.Radio[1].freq=  87.8;		strcpy(Test.Radio[1].radioName,"RMF Classic     ");
	Test.Radio[2].freq=  89.4;		strcpy(Test.Radio[2].radioName,"Jedynka         ");
	Test.Radio[3].freq=  88.8;		strcpy(Test.Radio[3].radioName,"Eska 2          ");
	Test.Radio[4].freq=  99.4;		strcpy(Test.Radio[4].radioName,"Trojka          ");
	Test.Radio[5].freq=  96.0;		strcpy(Test.Radio[5].radioName,"RMF FM          ");
	Test.Radio[6].freq= 104.1;		strcpy(Test.Radio[6].radioName,"Radio Zet       ");
	Test.Radio[7].freq=  96.7;		strcpy(Test.Radio[7].radioName,"RMF MAXXX       ");
	Test.Radio[8].freq= 106.1;		strcpy(Test.Radio[8].radioName,"Radio Plus      ");
	Test.Radio[9].freq=  90.6;		strcpy(Test.Radio[9].radioName,"Radio Maryja    ");		//PCF8575_SetVAl( (0x12<<8)|0xBF, (0x2<<8)|0x1E );  //00000010  00011110 Radio Maryja
	Test.Radio[10].freq= 92.5;	   strcpy(Test.Radio[10].radioName,"Zlote przeboje  ");
	Test.Radio[11].freq= 102.4;	   strcpy(Test.Radio[11].radioName,"Radio Pogoda    ");
	Test.Radio[12].freq= 103.0;	   strcpy(Test.Radio[12].radioName,"Radio Katowice  ");
	Test.Radio[13].freq= 102.9;	   strcpy(Test.Radio[13].radioName,"TOK FM          ");
	Test.Radio[14].freq=  97.7;	   strcpy(Test.Radio[14].radioName,"Eska            ");
	Test.Radio[15].freq=  93.7;	   strcpy(Test.Radio[15].radioName,"Chillizet       ");
	Test.Radio[16].freq= 104.9;	   strcpy(Test.Radio[16].radioName,"Eska Rock       ");
	Test.Radio[17].freq= 107.0;	   strcpy(Test.Radio[17].radioName,"VOX FM          ");
	Test.Radio[18].freq= 101.0;	   strcpy(Test.Radio[18].radioName,"AntyRadio       ");
	Test.Radio[19].freq= 102.0;	   strcpy(Test.Radio[19].radioName,"Dwojka          ");
	Test.Radio[20].freq=  97.2;	   strcpy(Test.Radio[20].radioName,"Polskie Radio 24");
	Test.Radio[21].freq= 103.8;	   strcpy(Test.Radio[21].radioName,"Rock Radio      ");
	Test.Radio[22].freq=  95.2;	   strcpy(Test.Radio[22].radioName,"Radio Wnet      ");
	Test.Radio[23].freq= 100.5;	   strcpy(Test.Radio[23].radioName,"Radio Famka     ");

	LOOP_FOR(i,MAX_RADIO_CHANNEL){	Test.pName[i]		   = Test.Radio[i].radioName;
									Test.Radio[i].freqStep = (0x12<<8)|0xBF;
									Test.Radio[i].freqOffs = 0.0;
									RADIO_CalcFreqDiv(i);
	}
	Test.selRadio=0;
	if(RADIO_SetFreq(Test.selRadio)) ERROR_pcf8575();
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
	RADIO_InitScreen();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_2press_1010(void){

}
static void SERVICE_2press_0110(void){
	EXAMPLE_DrawTxt(display);
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_2press_1001(void){
	INCR_WRAP(toogleVar[1],1,0,2);
	RADIO_DispFreq();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_2press_0101(void){
	if(toogleVar[1]){  INCR_WRAP(toogleVar[1],1,1,2);  }
	RADIO_DispFreq();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_2press_0011(void){
	if(RADIO_CorrectFreqForSelChannel(9, (0x2<<8)|0x1E))  ERROR_pcf8575();		/* Correct frequency for Radio Maryja */
	DispFreqScreen(display[0]);
	WAIT_FOR_ALL_RELEASE;
}

static void SERVICE_3press_1000(void){
	ERROR_pcf8575();
	WAIT_FOR_ALL_RELEASE;
}
static void SERVICE_3press_0100(void){
	ssd1306_InvertColors();
	ssd1306_Clr();
	DispFreqScreen(display[0]);
	HAL_Delay(500);
}
static void SERVICE_3press_0010(void){

}
static void SERVICE_3press_0001(void){

}

static void SERVICE_4press_0000(void){
	if(TOOGLE(toogleVar[0])){  if(PCF8575_SetVAl(0xFFFF,0xFFFF)) ERROR_pcf8575(); }
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
  HAL_Delay(200);
  if(SD1306_Inits(display)) ERROR_ssd1306();
  RADIO_InitScreen();

  VisualParam_LCD_Reset();
 // MX_GPIO_ChangeConfigSWDpin(6000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  DispFreqScreen(display[0]);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  SERVICE_InputPress();
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
