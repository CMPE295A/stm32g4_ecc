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
k = 58f741771620bdc428e91a32d86d230873e9140336fcfb1e122892ee1d501bdc - Known_Random - k-1 is ...db
R = 4a19274429e40522234b8785dc25fc524f179dcc95ff09b3c9770fc71f54ca0d - Known_Signature
S = 58982b79a65b7320f5b92d13bdaecdd1259e760f0f718ba933fd098f6f75d4b7 - Known_Signature
Msg = 68656c6c6f207468697320697320636d7065323935 - Message - hello this is cmpe295
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
 const uint8_t Message[] =
 {
		  0xff, 0x62, 0x4d, 0x0b, 0xa0, 0x2c, 0x7b, 0x63, 0x70, 0xc1, 0x62, 0x2e, 0xec, 0x3f, 0xa2, 0x18,
		  0x6e, 0xa6, 0x81, 0xd1, 0x65, 0x9e, 0x0a, 0x84, 0x54, 0x48, 0xe7, 0x77, 0xb7, 0x5a, 0x8e, 0x77,
		  0xa7, 0x7b, 0xb2, 0x6e, 0x57, 0x33, 0x17, 0x9d, 0x58, 0xef, 0x9b, 0xc8, 0xa4, 0xe8, 0xb6, 0x97,
		  0x1a, 0xef, 0x25, 0x39, 0xf7, 0x7a, 0xb0, 0x96, 0x3a, 0x34, 0x15, 0xbb, 0xd6, 0x25, 0x83, 0x39,
		  0xbd, 0x1b, 0xf5, 0x5d, 0xe6, 0x5d, 0xb5, 0x20, 0xc6, 0x3f, 0x5b, 0x8e, 0xab, 0x3d, 0x55, 0xde,
		  0xbd, 0x05, 0xe9, 0x49, 0x42, 0x12, 0x17, 0x0f, 0x5d, 0x65, 0xb3, 0x28, 0x6b, 0x8b, 0x66, 0x87,
		  0x05, 0xb1, 0xe2, 0xb2, 0xb5, 0x56, 0x86, 0x10, 0x61, 0x7a, 0xbb, 0x51, 0xd2, 0xdd, 0x0c, 0xb4,
		  0x50, 0xef, 0x59, 0xdf, 0x4b, 0x90, 0x7d, 0xa9, 0x0c, 0xfa, 0x7b, 0x26, 0x8d, 0xe8, 0xc4, 0xc2
		 /*0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x63, 0x6d,
	0x70, 0x65, 0x32, 0x39, 0x35*/
 };
 const uint8_t Known_Random[] = /* = k - 1 */
 {
   0x58, 0xf7, 0x41, 0x77, 0x16, 0x20, 0xbd, 0xc4, 0x28, 0xe9, 0x1a, 0x32, 0xd8, 0x6d, 0x23, 0x08,
   0x73, 0xe9, 0x14, 0x03, 0x36, 0xfc, 0xfb, 0x1e, 0x12, 0x28, 0x92, 0xee, 0x1d, 0x50, 0x1b, 0xdb
 };
 const uint8_t Known_Signature[] =
 {
   0x4a, 0x19, 0x27, 0x44, 0x29, 0xe4, 0x05, 0x22, 0x23, 0x4b, 0x87, 0x85, 0xdc, 0x25, 0xfc, 0x52,
   0x4f, 0x17, 0x9d, 0xcc, 0x95, 0xff, 0x09, 0xb3, 0xc9, 0x77, 0x0f, 0xc7, 0x1f, 0x54, 0xca, 0x0d,
   0x58, 0x98, 0x2b, 0x79, 0xa6, 0x5b, 0x73, 0x20, 0xf5, 0xb9, 0x2d, 0x13, 0xbd, 0xae, 0xcd, 0xd1,
   0x25, 0x9e, 0x76, 0x0f, 0x0f, 0x71, 0x8b, 0xa9, 0x33, 0xfd, 0x09, 0x8f, 0x6f, 0x75, 0xd4, 0xb7
 };

 /* Computed data buffer */
 uint8_t Computed_Secret[CMOX_ECC_SECP256R1_SECRET_LEN];
 uint8_t Computed_Hash[CMOX_SHA224_SIZE];
 uint8_t Computed_Signature[CMOX_ECC_SECP256R1_SIG_LEN];

/* USER CODE END PV */
 /* ECC context */
 cmox_ecc_handle_t Ecc_Ctx;
 /* ECC working buffer */
 uint8_t Working_Buffer[2000];

 /* Random data buffer */
 uint32_t Computed_Random[8];
 /* RNG peripheral handle */
 RNG_HandleTypeDef hrng;

 __IO TestStatus glob_status = FAILED;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void process_cli(void);
static void MX_USART2_UART_Init(void);
static void MX_RNG_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	  cmox_ecc_retval_t retval_ecc;
	  cmox_ecc_retval_t retval;
	  size_t computed_size;
	  size_t computed_size_ecdsa;
	  cmox_hash_retval_t hretval;

	  /* Fault check verification variable */
	  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

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
  esp8266__initialize();
  uart__initialize(USART1, 115200);
  timer__initialize(DRIVER_TIMER2);
  uint16_t count = 0;

  __HAL_RCC_RNG_CLK_ENABLE();

  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE BEGIN 2 */
  uint32_t randomNumber1;
   if (HAL_RNG_GenerateRandomNumber(&hrng, &randomNumber1) != HAL_OK)
   {
     Error_Handler();
   }

   // Define the size of the private key
   int size = 32;

   // Create an array to store the hexadecimal values
   uint8_t key[size];

   // Convert the random number to hexadecimal and store it in key
   toHex(randomNumber1, key, size);
/*
   // Print the key array
   printf("const uint8_t Private_Key[] = {");
   for (int i = 0; i < size; i++)
   {
     if (i != 0)
     {
       printf(", ");
     }
     printf("0x%02X", key[i]);
   }
   printf("};\n");
*/
  /* USER CODE END 2 */
  /* Initialize cryptographic library */
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }
  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
  retval_ecc = cmox_ecdh(&Ecc_Ctx,                                         /* ECC context */
                       CMOX_ECC_CURVE_SECP256R1,                         /* SECP256R1 ECC curve selected */
                       key, sizeof(key),                 /* Local Private key */
                       Remote_Public_Key, sizeof(Remote_Public_Key),     /* Remote Public key */
                       Computed_Secret, &computed_size);                 /* Data buffer to receive shared secret */


  /* Verify API returned value */
    if (retval_ecc != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }

    /* Verify generated data size is the expected one */
    if (computed_size != sizeof(Computed_Secret))
    {
      Error_Handler();
    }

    /* Verify generated data are the expected ones -
     * compare first 32 bytes of Computed_Secret with Expected_Secret */
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


    /* Compute directly the digest passing all the needed parameters */
    hretval = cmox_hash_compute(CMOX_SHA224_ALGO,         /* Use SHA224 algorithm */
                                Message, sizeof(Message), /* Message to digest */
                                Computed_Hash,            /* Data buffer to receive digest data */
                                CMOX_SHA224_SIZE,         /* Expected digest size */
                                &computed_size_ecdsa);          /* Size of computed digest */

    /* Verify API returned value */
    if (hretval != CMOX_HASH_SUCCESS)
    {
      Error_Handler();
    }

    /* Verify generated data size is the expected one */
    if (computed_size_ecdsa != CMOX_SHA224_SIZE)
    {
      Error_Handler();
    }
    /* --------------------------------------------------------------------------
       * KNOWN RANDOM USAGE
       * --------------------------------------------------------------------------
       */

      /* Construct a ECC context, specifying mathematics implementation and working buffer for later processing */
      /* Note: CMOX_ECC256_MATH_FUNCS refer to the default mathematics implementation
       * selected in cmox_default_config.h. To use a specific implementation, user can
       * directly choose:
       * - CMOX_MATH_FUNCS_SMALL to select the mathematics small implementation
       * - CMOX_MATH_FUNCS_FAST to select the mathematics fast implementation
       * - CMOX_MATH_FUNCS_SUPERFAST256 to select the mathematics fast implementation optimized for 256 bits computation
       */
      cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

      /* Compute directly the signature passing all the needed parameters */
      /* Note: CMOX_ECC_CURVE_SECP256R1 refer to the default SECP256R1 definition
       * selected in cmox_default_config.h. To use a specific definition, user can
       * directly choose:
       * - CMOX_ECC_SECP256R1_LOWMEM to select the low RAM usage definition (slower computing)
       * - CMOX_ECC_SECP256R1_HIGHMEM to select the high RAM usage definition (faster computing)
       */
      retval = cmox_ecdsa_sign(&Ecc_Ctx,                                 /* ECC context */
                               CMOX_ECC_CURVE_SECP256R1,                 /* SECP256R1 ECC curve selected */
                               Known_Random, sizeof(Known_Random),       /* Random data buffer */
                               Private_Key, sizeof(Private_Key),         /* Private key for signature */
							   Computed_Secret, &computed_size,
							   //Computed_Hash, CMOX_SHA224_SIZE,          /* Digest to sign */
                               Computed_Signature, &computed_size_ecdsa);      /* Data buffer to receive signature */

      /* Verify API returned value */
      if (retval != CMOX_ECC_SUCCESS)
      {
        Error_Handler();
      }

      /* Verify generated data size is the expected one */
      if (computed_size_ecdsa != sizeof(Known_Signature))
      {
        Error_Handler();
      }

      /* Verify generated data are the expected ones */
      /*if (memcmp(Computed_Signature, Known_Signature, computed_size_ecdsa) != 0)
      {
        Error_Handler();
      }*/

      /* Cleanup context */
      cmox_ecc_cleanup(&Ecc_Ctx);
      /* Construct a ECC context, specifying mathematics implementation and working buffer for later processing */
      /* Note: CMOX_ECC256_MATH_FUNCS refer to the default mathematics implementation
       * selected in cmox_default_config.h. To use a specific implementation, user can
       * directly choose:
       * - CMOX_MATH_FUNCS_SMALL to select the mathematics small implementation
       * - CMOX_MATH_FUNCS_FAST to select the mathematics fast implementation
       * - CMOX_MATH_FUNCS_SUPERFAST256 to select the mathematics fast implementation optimized for 256 bits computation
       */
      cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

      /* Verify directly the signature passing all the needed parameters */
      /* Note: CMOX_ECC_CURVE_SECP256R1 refer to the default SECP256R1 definition
       * selected in cmox_default_config.h. To use a specific definition, user can
       * directly choose:
       * - CMOX_ECC_SECP256R1_LOWMEM to select the low RAM usage definition (slower computing)
       * - CMOX_ECC_SECP256R1_HIGHMEM to select the high RAM usage definition (faster computing)
       */
      retval = cmox_ecdsa_verify(&Ecc_Ctx,                                  /* ECC context */
                                 CMOX_ECC_CURVE_SECP256R1,                  /* SECP256R1 ECC curve selected */
                                 Remote_Public_Key, sizeof(Remote_Public_Key),            /* Public key for verification */
								 Computed_Secret, &computed_size,
								 //Computed_Hash, CMOX_SHA224_SIZE,           /* Digest to verify */
                                 Known_Signature, sizeof(Known_Signature),  /* Data buffer to receive signature */
                                 &fault_check);                             /* Fault check variable:
                                                                to ensure no fault injection occurs during this API call */

      /* Verify API returned value */
      if (retval != CMOX_ECC_AUTH_SUCCESS)
      {
        Error_Handler();
      }
      /* Verify Fault check variable value */
      if (fault_check != CMOX_ECC_AUTH_SUCCESS)
      {
        Error_Handler();
      }

      /* Cleanup context */
      cmox_ecc_cleanup(&Ecc_Ctx);

      /* --------------------------------------------------------------------------
       * TRUE RANDOM USAGE
       * --------------------------------------------------------------------------
       */

      /* Configure RNG peripheral */
      hrng.Instance = RNG;
      hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
      if (HAL_RNG_Init(&hrng) != HAL_OK)
      {
        Error_Handler();
      }

      /* Construct a ECC context, specifying mathematics implementation and working buffer for later processing */
      /* Note: CMOX_ECC256_MATH_FUNCS refer to the default mathematics implementation
       * selected in cmox_default_config.h. To use a specific implementation, user can
       * directly choose:
       * - CMOX_MATH_FUNCS_SMALL to select the mathematics small implementation
       * - CMOX_MATH_FUNCS_FAST to select the mathematics fast implementation
       * - CMOX_MATH_FUNCS_SUPERFAST256 to select the mathematics fast implementation optimized for 256 bits computation
       */
      cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

      /* Note: The random value must satisfy some arithmetic constraints versus the selected curve and points
       * to minimize the statictical vulnerability.
       * In case this is not satisfied, cmox_ecdsa_sign returns CMOX_ECC_ERR_WRONG_RANDOM: new random has to be
       * generated and API call again.
       */
      do
      {
        /* Generate random numbers */
        for (uint32_t i = 0; i < sizeof(Computed_Random) / sizeof(uint32_t); i++)
        {
          if (HAL_RNG_GenerateRandomNumber(&hrng, &Computed_Random[i]) != HAL_OK)
          {
            /* Random number generation error */
            Error_Handler();
          }
        }

        /* Compute directly the signature passing all the needed parameters */
        /* Note: CMOX_ECC_CURVE_SECP256R1 refer to the default SECP256R1 definition
         * selected in cmox_default_config.h. To use a specific definition, user can
         * directly choose:
         * - CMOX_ECC_SECP256R1_LOWMEM to select the low RAM usage definition (slower computing)
         * - CMOX_ECC_SECP256R1_HIGHMEM to select the high RAM usage definition (faster computing)
         */
        retval = cmox_ecdsa_sign(&Ecc_Ctx,                                            /* ECC context */
                                 CMOX_ECC_CURVE_SECP256R1,                            /* SECP256R1 ECC curve selected */
                                 (uint8_t *)Computed_Random, sizeof(Computed_Random), /* Random data buffer */
                                 Private_Key, sizeof(Private_Key),                    /* Private key for signature */
								 Computed_Secret, &computed_size,
								 //Computed_Hash, CMOX_SHA224_SIZE,                     /* Digest to sign */
                                 Computed_Signature, &computed_size_ecdsa);                 /* Data buffer to receive signature */

      } while (retval == CMOX_ECC_ERR_WRONG_RANDOM);

      /* Verify API returned value */
      if (retval != CMOX_ECC_SUCCESS)
      {
        Error_Handler();
      }

      /* Verify generated data size is the expected one */
      if (computed_size_ecdsa != sizeof(Known_Signature))
      {
        Error_Handler();
      }

      /* Cleanup context */
      cmox_ecc_cleanup(&Ecc_Ctx);

      /* Construct a ECC context, specifying mathematics implementation and working buffer for later processing */
      /* Note: CMOX_ECC256_MATH_FUNCS refer to the default mathematics implementation
       * selected in cmox_default_config.h. To use a specific implementation, user can
       * directly choose:
       * - CMOX_MATH_FUNCS_SMALL to select the mathematics small implementation
       * - CMOX_MATH_FUNCS_FAST to select the mathematics fast implementation
       * - CMOX_MATH_FUNCS_SUPERFAST256 to select the mathematics fast implementation optimized for 256 bits computation
       */
      cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

      /* Verify directly the signature passing all the needed parameters */
      /* Note: CMOX_ECC_CURVE_SECP256R1 refer to the default SECP256R1 definition
       * selected in cmox_default_config.h. To use a specific definition, user can
       * directly choose:
       * - CMOX_ECC_SECP256R1_LOWMEM to select the low RAM usage definition (slower computing)
       * - CMOX_ECC_SECP256R1_HIGHMEM to select the high RAM usage definition (faster computing)
       */
      retval = cmox_ecdsa_verify(&Ecc_Ctx,                                        /* ECC context */
                                 CMOX_ECC_CURVE_SECP256R1,                        /* SECP256R1 ECC curve selected */
                                 Remote_Public_Key, sizeof(Remote_Public_Key),                  /* Public key for verification */
								 Computed_Secret, &computed_size,
								 //Computed_Hash, CMOX_SHA224_SIZE,                 /* Digest to verify */
                                 Computed_Signature, sizeof(Computed_Signature),  /* Data buffer to receive signature */
                                 &fault_check);                                   /* Fault check variable:
                                                                to ensure no fault injection occurs during this API call */

      /* Verify API returned value */
      if (retval != CMOX_ECC_AUTH_SUCCESS)
      {
        Error_Handler();
      }
      /* Verify Fault check variable value */
      if (fault_check != CMOX_ECC_AUTH_SUCCESS)
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

void toHex(uint32_t number, uint8_t* hexData, int size)
{
  for (int i = 0; i < size; i++)
  {
    hexData[i] = (number >> (8 * (size - i - 1))) & 0xFF;
  }
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
