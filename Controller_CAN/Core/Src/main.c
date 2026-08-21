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
#include "header.h"
#include "stdlib.h"
#include "spi.h"
#include "stdbool.h"
#include "mcp2515.h"
#include "enc28j60.h"
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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

__attribute__((section(".header"))) const header_s header = {
	.magic = MAGIC_NUMBER,
	.ota_flag = 0,
	.Size = 8996,
	.Crc = 0x189111F1,
	.version = 1,
};


#define MCP_CMD_CONNECT			0
#define MCP_CMD_DRIVE			1
#define MCP_CMD_IN_PING			2
#define MCP_CMD_OUT_PING		3
#define MCP_CMD_DATA			4

uint8_t txData[14];
uint8_t rxData[14];
uint8_t is_rxd = 0;
uint8_t is_send = 0;
uint8_t is_next = 0;
uint8_t is_enc_rx= 0;

struct ID {
	uint8_t id;
    bool ping;
    bool drive;
    struct ID *next;
};

uint8_t amount_id = 0;
struct ID *current_ID, *current_order_ID;

void EXTI9_5_IRQHandler(void) {
	if(EXTI->PR & (0x01<<6)) {
		is_rxd = 1;
		EXTI->PR |= (0x01<<6);
	}
	if(EXTI->PR & (0x01<<8)) {
		is_enc_rx = 1;
		EXTI->PR |= (0x01<<8);
	}
	if(EXTI->PR & (0x01<<9)) {
		is_send = 1;
		EXTI->PR |= (0x01<<9);
	}
}

void EXTI15_10_IRQHandler(void) {
	if(EXTI->PR & (0x01<<10)) {
		is_next = 1;
		EXTI->PR |= (0x01<<10);
	}
	if(EXTI->PR & (0x01<<11)) {
		is_send = 1;
		EXTI->PR |= (0x01<<11);
	}
}

void delay_us(uint16_t us) {
	TIM4->CNT = 0;
	while(us > TIM4->CNT);
}

void delay_ms(uint16_t ms) {
	while((ms--) > 0) delay_us(1000);
}

struct ID *createID(int value) {
    struct ID *newID = (struct ID*)malloc(sizeof(struct ID));
    newID->id = value;
    newID->next = NULL;
    newID->ping = 0;
    newID->drive = 0;
    return newID;
}

void insertID(struct ID *List_ID, int value) {
	if(value == 0 || List_ID == NULL) return;
    struct ID *current = List_ID;

    while(current->next != NULL) {
        current = current->next;
    	if(current->id == value) return;
    }
    amount_id += 1;
    struct ID *newID = createID(value);
    current->next = newID;
}

struct ID *getIDbyIndex(struct ID *List_ID, int index) {
	if(index == 0 || List_ID == NULL) return List_ID;
	int i=0;
    struct ID *current = List_ID;

	while(i != index) {
        if(current->next == NULL) return List_ID;
		current = current->next;
        i += 1;
    }
	return current;
}

void deleteByValue(struct ID *List_ID, int value) {
    if(List_ID == NULL || value == 0) return;
    struct ID *current = List_ID;
    struct ID *prev = NULL;

	while (current != NULL && current->id != value) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return;
    prev->next = current->next;
    free(current);
    amount_id -= 1;
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
  SystemClock_Config();

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
  /* USER CODE BEGIN 2 */
  //  set up GPIO
  RCC->APB2ENR |= (0b1<<12) | (0b1<<2) | (0b1<<3) | (0b1<<4); // SPI1 GPIOA GPIOB GPIOC


  GPIOA->CRL &= ~(0xFFFF<<16); // clear pin 4 5 6 7 port A
  GPIOA->CRH &= ~(0xFFFF<<0); // clear pin 8 9 10 11 port A
  GPIOA->CRH &= ~(0xF<<28); // clear pin 15 port A

  GPIOA->CRL |= (0b0011<<16) | (0b0011<<20) | (0b0011<<24); // pin 4 5 6 output
  GPIOA->CRL |= 0b0100<<28;  // pin 7 input
  GPIOA->CRH |= (0b0011<<28); // pin 15 output
  GPIOA->CRH |= (0x4444 << 0); // chan ngat reset pin 8 9 10 11

  GPIOB->CRL &= ~(0xFFFFF<<12); // clear pin 3 4 5 _ 7 port B
  GPIOB->CRH &= ~(0xFF<<0); // clear pin 8 9 port B
  GPIOB->CRH &= ~(0xFFFF<<16); // clear pin 12 13 14 15 port B

  GPIOB->CRL |= (0b0011<<28); // pin 7 output
  GPIOB->CRL |= (0b0100 << 24); // chan ngat reset pin 6
  GPIOB->CRH |= (0b0011<<0) | (0b0011<<4);  // pin 8 9 output
  GPIOB->CRH |= (0b0011<<16); // pin 12 output
  GPIOB->ODR |= 0b111<<7; // ngo ra pin 7 8 9 muc cao

  //  set up ngat ngoai

  AFIO->EXTICR[1] &= ~(0b1111<<8);
  AFIO->EXTICR[1] |= 0b0001<<8; // pin B6
  AFIO->EXTICR[2] &= ~(0b1111<<0); // pin A8
  AFIO->EXTICR[2] &= ~(0b1111<<4); // pin A9
  EXTI->IMR |= 0b111101<<6; // ngat chan B6 A8 A9 A10 A11
  EXTI->FTSR |= 0b111101<<6;
  EXTI->PR |= 0b01<<6;
  EXTI->PR |= 0b01111<<8; // A8 A9 A10 A11
  NVIC->ISER[0] |= 0b01<<23; // kich hoat ngat EXTI5-9
  NVIC->ISER[1] |= 0b01<<8;
  //
  //  set up SPI1
  GPIOB->CRL |= (0b1011<<12) | (0b0100<<16) | (0b1011<<20); // config pin SPI1 pin 3 4 5
  AFIO->MAPR |= 0b01;
  AFIO->MAPR |= 0b010<<25;
  SPI1->CR1 = 0;
  SPI1->CR1 = (0b1<<9) | (0b1<<8) | (0b011<<3) | (0b1<<2);
  SPI1->CR1 |= 0b1<<6;

  //  set up SPI2
  RCC->APB1ENR |= 0b1<<14;
  GPIOB->CRH |= (0b1011<<20) | (0b0100<<24) | (0b1011<<28); // config pin SPI2 pin 13 14 15
  SPI2->CR1 = 0;
  SPI2->CR1 = (0b1<<9) | (0b1<<8) | (0b011<<3) | (0b1<<2);
  SPI2->CR1 |= 0b1<<6;

  //  set up TIM4
  RCC->APB1ENR |= 0b1<<2; // TIM4
  TIM4->CNT = 0;
  TIM4->ARR = 0xFFFF;
  TIM4->PSC = 71;
  TIM4->EGR |= 0b1;
  TIM4->CR1 |= 0b1;

  //  set up watchdog wwdg
//  RCC->APB1ENR |= 0b1<<11;
//  WWDG->CFR = (0b11<<7) | (0x7F<<0);
     /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

  uint8_t data[8] = {
  	  MCP_CMD_CONNECT,
  	  MCP_CMD_DRIVE,
  	  MCP_CMD_IN_PING,
  	  MCP_CMD_OUT_PING,
  	  MCP_CMD_DATA
  };

  spi_s mcp = {SPI1, GPIOA, GPIO_PIN_15};
  spi_s enc = {SPI2, GPIOB, GPIO_PIN_12};
  mcp_activate_rx(&mcp);

  struct ID *initID = createID(0);
  struct ID *List_ID = initID;
  current_order_ID = List_ID;
  current_ID = List_ID;
  uint8_t count_time=0, counter = 0;
  enc_init(&enc);
  uint8_t dummy_cnt = 0;
  enc_clear_bits(&enc, ECON1, 0x03);
  enc_set_bits(&enc, ECON1, 0x01);
  enc_read_byte(&enc, EPKTCNT, &dummy_cnt);
  while(dummy_cnt > 0) {
      enc_set_bits(&enc, ECON2, 0x40);
      enc_read_byte(&enc, EPKTCNT, &dummy_cnt);
  }
  uint8_t buffer[64];
  uint8_t next_byte_low = 0x00, next_byte_high = 0x0C;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  delay_ms(50);
	  if (is_enc_rx) {
		  is_enc_rx = 0;

		  uint8_t eir;
		  uint8_t header[6];
		  uint16_t nextPacket;
		  uint16_t length;
		  uint16_t status;
		  uint16_t erxrdpt;

		  enc_read_byte(&enc, EIR, &eir);
		  if (eir & 0x40) {
			  enc_clear_bits(&enc, ECON1, 0x03);
			  enc_write_byte(&enc, ERDPTL, next_byte_low);
			  enc_write_byte(&enc, ERDPTH, next_byte_high);

			  enc_read_buffer(&enc, header, 6);

			  nextPacket = header[0] | (header[1] << 8);
			  length     = header[2] | (header[3] << 8);
			  status     = header[4] | (header[5] << 8);

			  next_byte_low  = header[0];
			  next_byte_high = header[1];

			  if (status & 0x80) {
				  if (length > sizeof(buffer)) length = sizeof(buffer);
				  enc_read_buffer(&enc, buffer, length);
				  if (buffer[12] == 0x08 && buffer[13] == 0x06) {
					  GPIOB->ODR &= ~(0x01<<8);
					  mcp_set_ID_standard(&mcp, buffer[14], 0x07);
					  if(buffer[15] == 1) mcp_send(&mcp, &data[MCP_CMD_DRIVE], 1);
					  else if(buffer[15] == 2) mcp_send(&mcp, &data[MCP_CMD_IN_PING], 1);

					  HAL_Delay(20);
					  GPIOB->ODR |= 0x01<<8;
				  }
			  }

			  if (nextPacket <= 0x0C00) erxrdpt = 0x1FFF;
			  else erxrdpt = nextPacket - 1;

			  enc_write_byte(&enc, ERXRDPTL, erxrdpt & 0xFF);
			  enc_write_byte(&enc, ERXRDPTH, erxrdpt >> 8);

			  enc_set_bits(&enc, ECON2, 0x40);
		  }
		  enc_clear_bits(&enc, EIR, 0x41);
		  enc_clear_bits(&enc, ESTAT, 0x40);
	  }

	  if(is_next == 1) {
		  if(current_ID->id != 0) {
			  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
			  mcp_send(&mcp, &data[MCP_CMD_OUT_PING], 1);
			  delay_ms(50);
		  }
		  current_ID = current_ID->next;
		  if(current_ID != NULL) {
			  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
			  mcp_send(&mcp, &data[MCP_CMD_IN_PING], 1);
			  delay_ms(50);
		  } else current_ID = List_ID;
		  is_next = 0;
	  }
	  if(is_send == 1) {
		  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
		  mcp_send(&mcp, &data[MCP_CMD_DRIVE], 1);
		  delay_ms(20);
		  is_send = 0;
	  }
	  if(count_time++ >= 40) {
		  current_order_ID = current_order_ID->next;
		  if(current_order_ID == NULL) {
			  current_order_ID = List_ID;
			  count_time = 0;
		  }
		  if(current_order_ID->id != 0x00) {
			  mcp_set_ID_standard(&mcp, current_order_ID->id, 0x07);
			  mcp_send(&mcp, &data[MCP_CMD_DATA], 1);
			  counter = 0;
			  while(counter++ < 50 && is_rxd == 0) HAL_Delay(10);
			  if(is_rxd == 0) deleteByValue(List_ID, current_order_ID->id);
		  }
	  }

	  if(is_rxd == 0) continue;
	  is_rxd = 0;
	  mcp_bit_modify(&mcp, CANINTF, 0xFF, 0x00);
	  mcp_read_buffer(&mcp, 0x00, rxData);
	  if(rxData[5] == MCP_CMD_CONNECT) {
		  insertID(List_ID, rxData[0]);
		  mcp_set_ID_standard(&mcp, rxData[0], 0x07);
		  mcp_send(&mcp, &data[MCP_CMD_CONNECT], 1);
	  } else if(rxData[5] == MCP_CMD_DATA) {
		  txData[0] = rxData[0];
		  txData[1] = rxData[6];
		  txData[2] = rxData[7];
		  if(txData[0] != 0) enc_transmit(&enc, txData, 3);
		  GPIOB->ODR &= ~(0b1<<7);
		  delay_ms(10);
		  GPIOB->ODR |= 0b1<<7;
	  }

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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
