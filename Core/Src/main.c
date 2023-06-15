/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "driver_uart.h"
#include "driver_timer.h"
#include "cmox_crypto.h"
#include "interface_esp8266.h"
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
 UART_HandleTypeDef huart2;
/*
COUNT = 0
QCAVSx = 700c48f77f56584c5cc632ca65640db91b6bacce3a4df6b42ce7cc838833d287 - Remote_Public_Key
QCAVSy = db71e509e3fd9b060ddb20ba5c51dcc5948d46fbf640dfe0441782cab85fa4ac - Remote_Public_Key
dIUT = 7d7dc5f71eb29ddaf80d6214632eeae03d9058af1fb6d22ed80badb62bc1a534 - Private_Key
QIUTx = ead218590119e8876b29146ff89ca61770c4edbbf97d38ce385ed281d8a6b230
QIUTy = 28af61281fd35e2fa7002523acc85a429cb06ee6648325389f59edfce1405141
ZIUT = 46fc62106420ff012e54a434fbdd2d25ccc5852060561e68040dd7778997bd7b - Expected_SecretX
 */
/* USER CODE BEGIN PV */
 const uint8_t Private_Key[] =
 {
   0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda, 0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
   0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e, 0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
 };
 const uint8_t Remote_Public_Key[] =
 {
   0x70, 0x0c, 0x48, 0xf7, 0x7f, 0x56, 0x58, 0x4c, 0x5c, 0xc6, 0x32, 0xca, 0x65, 0x64, 0x0d, 0xb9,
   0x1b, 0x6b, 0xac, 0xce, 0x3a, 0x4d, 0xf6, 0xb4, 0x2c, 0xe7, 0xcc, 0x83, 0x88, 0x33, 0xd2, 0x87,
   0xdb, 0x71, 0xe5, 0x09, 0xe3, 0xfd, 0x9b, 0x06, 0x0d, 0xdb, 0x20, 0xba, 0x5c, 0x51, 0xdc, 0xc5,
   0x94, 0x8d, 0x46, 0xfb, 0xf6, 0x40, 0xdf, 0xe0, 0x44, 0x17, 0x82, 0xca, 0xb8, 0x5f, 0xa4, 0xac
 };
 const uint8_t Expected_SecretX[] =
 {
   0x46, 0xfc, 0x62, 0x10, 0x64, 0x20, 0xff, 0x01, 0x2e, 0x54, 0xa4, 0x34, 0xfb, 0xdd, 0x2d, 0x25,
   0xcc, 0xc5, 0x85, 0x20, 0x60, 0x56, 0x1e, 0x68, 0x04, 0x0d, 0xd7, 0x77, 0x89, 0x97, 0xbd, 0x7b
 };

 /* Computed data buffer */
 uint8_t Computed_Secret[CMOX_ECC_SECP256R1_SECRET_LEN];

/* USER CODE END PV */
 /* ECC context */
 cmox_ecc_handle_t Ecc_Ctx;
 /* ECC working buffer */
 uint8_t Working_Buffer[2000];

 __IO TestStatus glob_status = FAILED;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void process_cli(void);
/* USER CODE BEGIN PFP */

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
	  cmox_ecc_retval_t retval;
	  size_t computed_size;
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
  //esp8266__initialize();
  uart__initialize(USART1, 115200);
  timer__initialize(DRIVER_TIMER2);
  uint16_t count = 0;

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  /* Initialize cryptographic library */
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }
  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
  retval = cmox_ecdh(&Ecc_Ctx,                                         /* ECC context */
                       CMOX_ECC_CURVE_SECP256R1,                         /* SECP256R1 ECC curve selected */
                       Private_Key, sizeof(Private_Key),                 /* Local Private key */
                       Remote_Public_Key, sizeof(Remote_Public_Key),     /* Remote Public key */
                       Computed_Secret, &computed_size);                 /* Data buffer to receive shared secret */

    /* Verify API returned value */
    if (retval != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }

    /* Verify generated data size is the expected one */
    if (computed_size != sizeof(Computed_Secret))
    {
      Error_Handler();
    }

    /* Verify generated data are the expected ones */
    if (memcmp(Computed_Secret, Expected_SecretX, sizeof(Expected_SecretX)) != 0)
    {
      Error_Handler();
    }
    /* Cleanup context */
    cmox_ecc_cleanup(&Ecc_Ctx);

    /* No more need of cryptographic services, finalize cryptographic library */
    if (cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
    {
      Error_Handler();
    }

    glob_status = PASSED;

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 /* esp8266__process();
	  if(timer__ms_elapsed(DRIVER_TIMER2))
	  {
		  if(++count == 1000)
		  {
			  count = 0;
			  esp8266__send_test_string();
		  }
	  }*/
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
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
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  //  // USART1_TX (A9)
  GPIOA->MODER &= ~(3 << 18);
  GPIOA->MODER |= (2 << 18); // alternate function
  GPIOA->OTYPER &= ~(1 << 9); // push-pull
  GPIOA->PUPDR &= ~(3 << 18); // no pull
  GPIOA->OSPEEDR &= ~(3 << 18); //
  GPIOA->OSPEEDR |= (3 << 18); // very high frequency
  GPIOA->AFR[1] |= (7 << 4);
  //
  //  // USART1_RX (A10)
  GPIOA->MODER &= ~(3 << 20);
  GPIOA->MODER |= (2 << 20); // alternate function
  GPIOA->OTYPER &= ~(1 << 10); // push-pull
  GPIOA->PUPDR &= ~(3 << 20); // no pull
  GPIOA->OSPEEDR &= ~(3 << 20);
  GPIOA->OSPEEDR |= (3 << 20); // very high frequency
  GPIOA->AFR[1] |= (7 << 8);

//  //  // USART2_TX (A2)
//  GPIOA->MODER &= ~(3 << 4);
//  GPIOA->MODER |= (2 << 4); // alternate function
//  GPIOA->OTYPER &= ~(1 << 2); // push-pull
//  GPIOA->PUPDR &= ~(3 << 4); // no pull
//  GPIOA->OSPEEDR &= ~(3 << 4); //
//  GPIOA->OSPEEDR |= (3 << 4); // very high frequency
//  GPIOA->AFR[1] |= (7 << 8);
//  //
//  //  // USART2_RX (A3)
//  GPIOA->MODER &= ~(3 << 6);
//  GPIOA->MODER |= (2 << 6); // alternate function
//  GPIOA->OTYPER &= ~(1 << 3); // push-pull
//  GPIOA->PUPDR &= ~(3 << 6); // no pull
//  GPIOA->OSPEEDR &= ~(3 << 6);
//  GPIOA->OSPEEDR |= (3 << 6); // very high frequency
//  GPIOA->AFR[1] |= (7 << 12);
}

static void process_cli(void)
{
	static uint16_t index = 0;
	static char buff[1024] = {0};
	uint8_t c = uart__get_char(DRIVER_UART1);
	if(c != NO_CHAR_AVAILABLE)
	{
	  // echo
	  uart__put(DRIVER_UART1, &c, 1);
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
