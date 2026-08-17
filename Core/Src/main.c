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
#include "stdlib.h"
#include "stdbool.h"
#include "enc28j60.h"
#include "mcp2515.h"
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
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define MCP_CMD_CONNECT			0
#define MCP_CMD_DRIVE			1
#define MCP_CMD_IN_PING			2
#define MCP_CMD_OUT_PING		3
#define MCP_CMD_DATA			4
uint8_t txData[14] = {0x01, 0x00};
uint8_t rxData[14];
mcp_def mcp = {spi: &hspi1, port: GPIOA, pin: GPIO_PIN_15};
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
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_9);
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
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

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

  mcp_activate_rx(&mcp);

  struct ID *initID = createID(0);
  struct ID *List_ID = initID;
  current_order_ID = List_ID;
  current_ID = List_ID;
  uint8_t count_time=0, counter = 0;
//  uint8_t byte=0;
  enc_def enc = {&hspi2, GPIOB, GPIO_PIN_12};
  enc_init(enc);
  uint8_t dummy_cnt = 0;
  enc_clear_bits(enc, ECON1, 0x03);
  enc_set_bits(enc, ECON1, 0x01);
  enc_read_byte(enc, EPKTCNT, &dummy_cnt);
  while(dummy_cnt > 0) {
      enc_set_bits(enc, ECON2, 0x40);
      enc_read_byte(enc, EPKTCNT, &dummy_cnt);
  }
  uint8_t buffer[64];
  uint8_t next_byte_low = 0x00, next_byte_high = 0x0C;
  while(1) {
	  HAL_Delay(50);

	  if (is_enc_rx) {
	      is_enc_rx = 0;

	      uint8_t eir;
	      uint8_t header[6];
	      uint16_t nextPacket;
	      uint16_t length;
	      uint16_t status;
	      uint16_t erxrdpt;

	      enc_read_byte(enc, EIR, &eir);
	      if (eir & 0x40) {
//	          enc_clear_bits(enc, ECON1, 0x03);
//	    	  enc_set_bits(enc, ECON1, 0x01);
//	          enc_read_byte(enc, EPKTCNT, &dummy_cnt);
//	    	  HAL_UART_Transmit(&huart2, &dummy_cnt, 1, 100);

	    	  enc_clear_bits(enc, ECON1, 0x03);
	          enc_write_byte(enc, ERDPTL, next_byte_low);
	          enc_write_byte(enc, ERDPTH, next_byte_high);

	          enc_read_buffer(enc, header, 6);

	          nextPacket = header[0] | (header[1] << 8);
	          length     = header[2] | (header[3] << 8);
	          status     = header[4] | (header[5] << 8);

	          next_byte_low  = header[0];
	          next_byte_high = header[1];

//			  HAL_UART_Transmit(&huart2, header, 6, 100);
	          if (status & 0x80) {
	              if (length > sizeof(buffer)) length = sizeof(buffer);
	              enc_read_buffer(enc, buffer, length);
				  if (buffer[12] == 0x08 && buffer[13] == 0x06) {
					  HAL_UART_Transmit(&huart2, &buffer[14], length-14, 100);
			  		  mcp_set_ID_standard(&mcp, buffer[14], 0x07);
					  if(buffer[15] == 1) mcp_send(&mcp, &data[MCP_CMD_DRIVE], 1);
					  else if(buffer[15] == 2) mcp_send(&mcp, &data[MCP_CMD_IN_PING], 1);

					  HAL_Delay(20);
				  }
	          }

	          if (nextPacket <= 0x0C00) erxrdpt = 0x1FFF;
	          else erxrdpt = nextPacket - 1;

	          enc_write_byte(enc, ERXRDPTL, erxrdpt & 0xFF);
	          enc_write_byte(enc, ERXRDPTH, erxrdpt >> 8);

	          enc_set_bits(enc, ECON2, 0x40);
	      }
		  enc_clear_bits(enc, EIR, 0x41);
		  enc_clear_bits(enc, ESTAT, 0x40);
	  }

	  if(is_next == 1) {
		  if(current_ID->id != 0) {
			  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
			  mcp_send(&mcp, &data[MCP_CMD_OUT_PING], 1);
			  HAL_Delay(50);
		  }
		  current_ID = current_ID->next;
		  if(current_ID != NULL) {
			  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
			  mcp_send(&mcp, &data[MCP_CMD_IN_PING], 1);
			  HAL_Delay(50);
		  } else current_ID = List_ID;
		  is_next = 0;
	  }
	  if(is_send == 1) {
		  mcp_set_ID_standard(&mcp, current_ID->id, 0x07);
		  mcp_send(&mcp, &data[MCP_CMD_DRIVE], 1);
		  HAL_Delay(20);
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
//		  HAL_UART_Transmit(&huart2, rxData, 13, 100);
		  txData[0] = rxData[0];
		  txData[1] = rxData[6];
		  txData[2] = rxData[7];
		  if(txData[0] != 0) enc_transmit(enc, txData, 3);
		  HAL_Delay(10);
	  }






//	  mcp_read_byte(&mcp, CANINTF, &byte);
//	  HAL_UART_Transmit(&huart1, &byte, 1, 100);
//	  mcp_read_byte(&mcp, EFLG, &byte);
//	  HAL_UART_Transmit(&huart1, &byte, 1, 100);
//	  HAL_UART_Transmit(&huart1, rxData, 13, 100);
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 PA10 PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
