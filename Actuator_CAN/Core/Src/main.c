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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcp2515.h"
#include "stdbool.h"
#include "header.h"
#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

__attribute__((section(".header"))) const header_s header = {
	.magic = MAGIC_NUMBER,
	.ota_flag = 0,
	.Size = 4220,
	.Crc = 0xFF1DCD3F,
	.version = 1,
};


#define MCP_CMD_CONNECT			0
#define MCP_CMD_DRIVE			1
#define MCP_CMD_IN_PING			2
#define MCP_CMD_OUT_PING		3
#define MCP_CMD_DATA			4

uint8_t txData[8];
uint8_t rxData[14];

volatile bool is_rxd = 0;

void EXTI9_5_IRQHandler(void) {
	if((EXTI->PR>>6)&0x01) {
		is_rxd = 1;
		EXTI->PR |= 0b01<<6;
	}
}

void delay_us(uint16_t us) {
	TIM4->CNT = 0;
	while(us > TIM4->CNT);
}

void delay_ms(uint16_t ms) {
	while((ms--) > 0) delay_us(1000);
}

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
  SCB->VTOR = APP_START_ADDR;
  __enable_irq();
  /* USER CODE END Init */

  /* Configure the system clock */
//  SystemClock_Config();

  /* USER CODE BEGIN SysInit */


  while ((RCC->CR&(0b1<<1))== 0);
  RCC->CFGR = 0;
  RCC->CR |= 0b1<<16; //HSE
  while((RCC->CR&(0b1<<17)) == 0);
  RCC->APB1ENR |= 0b1<<28;
  FLASH->ACR |= 0b1<<4;
  FLASH->ACR &= ~0b111;
  FLASH->ACR |= 0b010;

//
  RCC->CFGR |= (0b10<<14) | (0b100<<8) | (0b0111<<18) | (0b1<<16);
  RCC->CR |= 0b1<<24; //PLL
  while((RCC->CR&(0b1<<25)) == 0);

  RCC->CFGR |= 0b10; // SW PPL
  while(((RCC->CFGR>>2)&0b11) != 0b10);
  RCC->APB2ENR |= 0b1<<5; // enable port D

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

//  set up GPIO
  RCC->APB2ENR |= (0b1<<12) | (0b1<<2) | (0b1<<3) | (0b1<<4); // SPI1 GPIOA GPIOB GPIOC
  GPIOA->CRL &= ~(0xFFFF<<16); // clear pin 4 5 6 7 port A
  GPIOA->CRH &= ~(0xF<<28); // clear pin 15 port A

  GPIOA->CRL |= (0b0011<<16) | (0b0011<<20) | (0b0011<<24); // pin 4 5 6 output
  GPIOA->CRL |= 0b0100<<28;  // pin 7 input
  GPIOA->CRH |= (0b0011<<28); // pin 15 output

  GPIOB->CRL &= ~(0xFFFFF<<12); // clear pin 3 4 5 _ 7 port B
  GPIOB->CRH &= ~(0xFF<<0); // clear pin 8 9 port B
  GPIOB->ODR |= 0b111<<7; // ngo ra pin 7 8 9 muc cao
  GPIOB->CRL |= (0b0011<<28); // pin 7 output
  GPIOB->CRH |= (0b0011<<0) | (0b0011<<4);  // pin 8 9 output
  GPIOB->CRL |= (0b1011<<12) | (0b0100<<16) | (0b1011<<20); // config pin SPI1 pin 3 4 5
  GPIOB->CRL |= (0b0100 << 24); // chan ngat reset pin 6

//  set up ngat ngoai
  AFIO->EXTICR[1] |= 0b0001<<8;
  EXTI->IMR |= 0b01<<6;
  EXTI->FTSR |= 0b01<<6;
  EXTI->PR |= 0b01<<6;
  NVIC->ISER[0] |= 0b01<<23; // kich hoat ngat EXTI6
//
//  set up SPI1
  AFIO->MAPR |= 0b01;
  AFIO->MAPR |= 0b010<<25;
  SPI1->CR1 = 0;
  SPI1->CR1 = (0b1<<9) | (0b1<<8) | (0b011<<3) | (0b1<<2);
  SPI1->CR1 |= 0b1<<6;

//  set up TIM4
  RCC->APB1ENR |= 0b1<<2; // TIM4
  TIM4->CNT = 0;
  TIM4->ARR = 0xFFFF;
  TIM4->PSC = 71;
  TIM4->EGR |= 0b1;
  TIM4->CR1 |= 0b1;

//  set up watchdog wwdg
  RCC->APB1ENR |= 0b1<<11;
  WWDG->CFR = (0b11<<7) | (0x7F<<0);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  uint8_t byte = 0, ID = 0;
  uint8_t over_time_disconnect = 0, time_blink_ping = 0;
  bool is_ping = 0, is_connect = 0;

  uint8_t data[8] = {
	  MCP_CMD_CONNECT,
	  MCP_CMD_DRIVE,
	  MCP_CMD_IN_PING,
	  MCP_CMD_OUT_PING,
	  MCP_CMD_DATA
  };

  GPIOA->ODR &= ~(0x01<<5);

  // lay ID tu thanh gat 8 bit
  for(uint8_t i=0; i<8; i++) {
	  delay_ms(1);
  	  GPIOA->ODR |= 0x01<<6;
  	  GPIOA->ODR |= 0x01<<5;
  	  if(GPIOA->IDR&(0x01<<7)) ID |= 0x01<<i;
  	  delay_ms(1);
  	  GPIOA->ODR &= ~(0x01<<6);
  }
  // khoi tao mcp

  spi_s mcp = {SPI1, GPIOA, GPIO_PIN_15};

  mcp_activate_rx(&mcp);
  mcp_set_mask_standard(&mcp);
  mcp_set_filter_standard(&mcp, ID, 0x07);
  mcp_set_ID_standard(&mcp, ID, 0x07);
  mcp_send(&mcp, &data[MCP_CMD_CONNECT], 1);

  while (1) {
	  WWDG->CR = 0b01<<7 | 0x7F; // refresh watchdog
	  delay_ms(20);

	  // check ping tu master
	  if(is_ping) {
		  if(time_blink_ping++ >= 25) {
			  time_blink_ping = 0;
			  GPIOB->ODR ^= 0b1<<8;
		  }
	  } else GPIOB->ODR |= 0b1<<8;

	  // bat led xanh: bao da ket noi
	  if(is_connect) GPIOB->ODR &= ~(0b1<<7);

	  // tat led xanh: bao mat ket noi, gui yeu cau ket noi den master
	  if(over_time_disconnect++ > 150) {
		  over_time_disconnect = 0;
		  is_connect = 0;
		  is_ping = 0;
		  GPIOB->ODR |= 0b1<<8;
		  mcp_send(&mcp, &data[MCP_CMD_CONNECT], 1);
	  }

	  if(is_rxd == 0) continue;
	  is_rxd = 0;
	  mcp_read_byte(&mcp, CANINTF, &byte);
	  mcp_bit_modify(&mcp, CANINTF, 0xFF, 0x00);
	  mcp_read_buffer(&mcp, 0x00, rxData);

	  if(byte&0x80)continue;
	  if(rxData[5] == MCP_CMD_CONNECT) is_connect = 1;
	  else if(rxData[5] == MCP_CMD_IN_PING && is_ping == 1) is_ping = 0;
	  else if(rxData[5] == MCP_CMD_IN_PING) is_ping = 1;
	  else if(rxData[5] == MCP_CMD_OUT_PING) is_ping = 0;
	  else if(rxData[5] == MCP_CMD_DRIVE) GPIOA->ODR ^= 0b1<<4;
	  else if(rxData[5] == MCP_CMD_DATA) {
		  GPIOB->ODR &= ~(0b1<<9);
		  delay_ms(10);
		  GPIOB->ODR |= 0b1<<9;
		  is_connect = 1;
		  over_time_disconnect = 0;
		  txData[0] = MCP_CMD_DATA;
		  txData[1] = (GPIOA->IDR>>4)&0x01;
		  txData[2] = 0xFF;
		  mcp_send(&mcp, txData, 3);
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
