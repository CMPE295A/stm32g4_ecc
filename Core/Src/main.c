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
#include "interface_at_command.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/
 UART_HandleTypeDef huart2;
/*
server side
curve:	secp256k1
server's public key: 0486690d59b5dcd031dd9bdc7addd9cc2de0ace5970c37bd751394bd0c73972a
					a4f05c3c0f513e31796a4ee559d8ef518103f25565a1e1a6a307ddfd2e0cc06220
*/

 /* for HKDF:

 const uint8_t IKM[] =
 {
		  0xff, 0x62, 0x4d, 0x0b, 0xa0, 0x2c, 0x7b, 0x63, 0x70, 0xc1, 0x62, 0x2e, 0xec, 0x3f, 0xa2, 0x18,
		  0x6e, 0xa6, 0x81, 0xd1, 0x65, 0x9e, 0x0a, 0x84, 0x54, 0x48, 0xe7, 0x77, 0xb7, 0x5a, 0x8e, 0x77,
		  0xa7, 0x7b, 0xb2, 0x6e, 0x57, 0x33, 0x17, 0x9d, 0x58, 0xef, 0x9b, 0xc8, 0xa4, 0xe8, 0xb6, 0x97,
		  0x1a, 0xef, 0x25, 0x39, 0xf7, 0x7a, 0xb0, 0x96, 0x3a, 0x34, 0x15, 0xbb, 0xd6, 0x25, 0x83, 0x39,
		  0xbd, 0x1b, 0xf5, 0x5d, 0xe6, 0x5d, 0xb5, 0x20, 0xc6, 0x3f, 0x5b, 0x8e, 0xab, 0x3d, 0x55, 0xde,
		  0xbd, 0x05, 0xe9, 0x49, 0x42, 0x12, 0x17, 0x0f, 0x5d, 0x65, 0xb3, 0x28, 0x6b, 0x8b, 0x66, 0x87,
		  0x05, 0xb1, 0xe2, 0xb2, 0xb5, 0x56, 0x86, 0x10, 0x61, 0x7a, 0xbb, 0x51, 0xd2, 0xdd, 0x0c, 0xb4,
		  0x50, 0xef, 0x59, 0xdf, 0x4b, 0x90, 0x7d, 0xa9, 0x0c, 0xfa, 0x7b, 0x26, 0x8d, 0xe8, 0xc4, 0xc2

 };
 const uint8_t info[]=
 {
		 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x63, 0x6d,
		 	0x70, 0x65, 0x32, 0x39, 0x35
 };

 const uint8_t salt[] =
 {
   0x58, 0xf7, 0x41, 0x77, 0x16, 0x20, 0xbd, 0xc4, 0x28, 0xe9, 0x1a, 0x32, 0xd8, 0x6d, 0x23, 0x08,
   0x73, 0xe9, 0x14, 0x03, 0x36, 0xfc, 0xfb, 0x1e, 0x12, 0x28, 0x92, 0xee, 0x1d, 0x50, 0x1b, 0xdb
 };
*/

 const uint8_t IV[] =
 {
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

uint8_t Plaintext[] =
 {
   0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
   0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
   0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
   0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
 };

 //to
 const uint8_t Random[] =
 {
   0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda, 0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
   0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e, 0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
 };
 const uint8_t serverPublicKey[]=
 {
	0x86, 0x69, 0x0d, 0x59, 0xb5, 0xdc, 0xd0, 0x31, 0xdd, 0x9b, 0xdc, 0x7a, 0xdd, 0xd9, 0xcc, 0x2d,
	0xe0, 0xac, 0xe5, 0x97, 0x0c, 0x37, 0xbd, 0x75, 0x13, 0x94, 0xbd, 0x0c, 0x73, 0x97, 0x2a, 0xa4,
	0xf0, 0x5c, 0x3c, 0x0f, 0x51, 0x3e, 0x31, 0x79, 0x6a, 0x4e, 0xe5, 0x59, 0xd8, 0xef, 0x51, 0x81,
	0x03, 0xf2, 0x55, 0x65, 0xa1, 0xe1, 0xa6, 0xa3, 0x07, 0xdd, 0xfd, 0x2e, 0x0c, 0xc0, 0x62, 0x20
 };

 // SECP256R1 curve
 //	uint8_t Computed_Secret[CMOX_ECC_SECP256R1_SECRET_LEN];
 //	uint8_t pubKey[CMOX_ECC_SECP256R1_PUBKEY_LEN];
 //	uint8_t privateKey[CMOX_ECC_SECP256R1_PRIVKEY_LEN];

 //	BPP512T1 CURVE
 //	uint8_t Computed_Secret[CMOX_ECC_BPP512T1_SECRET_LEN];
 //	uint8_t Computed_Signature[CMOX_ECC_BPP512T1_SIG_LEN];
 //	uint8_t pubKey[CMOX_ECC_BPP512T1_PUBKEY_LEN];
 //	uint8_t privateKey[CMOX_ECC_BPP512T1_PRIVKEY_LEN];

 //	SECP256K1 CURVE
	 uint8_t Computed_Secret[CMOX_ECC_SECP256K1_SECRET_LEN];
	 uint8_t pubKey[CMOX_ECC_SECP256K1_PUBKEY_LEN];
	 uint8_t privateKey[CMOX_ECC_SECP256K1_PRIVKEY_LEN];

 //	HASH SIZE
 //	uint8_t Computed_Hash[CMOX_SHA224_SIZE];

 //uint8_t Computed_Ciphertext[CMOX_CIPHER_BLOCK_SIZE];

 uint8_t Computed_Ciphertext[sizeof(Plaintext)];
 uint8_t Computed_Plaintext[sizeof(Plaintext)];


 /* ECC context */
 cmox_ecc_handle_t Ecc_Ctx;

 /* CBC context handle */
 cmox_cbc_handle_t Cbc_Ctx;

 /* ECC working buffer */
 uint8_t Working_Buffer[4000];

 /* Random data buffer */
 uint32_t Computed_Random[32];

 /* RNG peripheral handle */
 RNG_HandleTypeDef hrng;

 __IO TestStatus glob_status = FAILED;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void process_cli(void);
static void MX_RNG_Init(void);


/**
  * @brief  The application entry point.
  * @retval int
  */

void wait_ms(int ms_wait_threshold)
{
	  uint16_t modem_reset_count = 0;
	  while(modem_reset_count < ms_wait_threshold)
	  {
		  if(timer__ms_elapsed(DRIVER_TIMER2))
		  {
			  modem_reset_count++;
		  }
	  }
}

int main(void)
{
	  cmox_ecc_retval_t retval_ecc; //return value for cmox_ecdh
	  cmox_ecc_retval_t retval;
	  cmox_mac_retval_t retval_hash; //return value for cmox_mac_compute (HKDF)
	  size_t computed_size;
	  size_t computed_size_ecdsa;
	  cmox_ecc_retval_t retval_keys;	//return value for cmox_ecdsa_keyGen

	  cmox_cipher_retval_t retval_cipher;	//return value for cmox_cipher_encrypt

	  size_t computed_size_encdec;	//Computed_Ciphertex and Computed_Plaintext

	  uint32_t index;

	  uint32_t startTick = 0;
	  uint32_t endTick = 0;
	  uint32_t elapsedTime = 0;
	  /* Fault check verification variable */
	  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  //SysTick_Config(SystemCoreClock / 1000);
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  timer__initialize(DRIVER_TIMER2);
  //uint16_t count = 0;

  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }

  // Initialize cryptographic library
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }
  wait_ms(1000);
  HAL_GPIO_WritePin(WIFI_MODEM_RESET_PORT, WIFI_MODEM_RESET_PIN, GPIO_PIN_SET);
  wait_ms(1000);
  at_interface__initialize();
/*
 //Uncomment to use random number in cmox_ecdsa_keyGen
/
 * /Generate random number to use Computed_Random in cmox_ecdsa_keyGen function

  for (uint32_t i = 0; i < sizeof(Computed_Random) / sizeof(uint32_t); i++)
         {
           if (HAL_RNG_GenerateRandomNumber(&hrng, &Computed_Random[i]) != HAL_OK)
           {
             // Random number generation error
           Error_Handler();
           }
         }
*/
  /* Initialize cryptographic library */
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }
  startTick = HAL_GetTick();
  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

  /*
   * for CMOX_ECC_BPP512T1_LOWMEM curve use
   *    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC128MULT_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
   */

  //generates public and private key for mcu
  retval_keys=cmox_ecdsa_keyGen(&Ecc_Ctx,
		  CMOX_ECC_SECP256K1_LOWMEM,
		  //CMOX_ECC_BPP512T1_LOWMEM,
		  Random, sizeof(Random),
		  //(uint8_t *)Computed_Random, sizeof(Computed_Random),	//uncomment this line to use random number
		  privateKey, CMOX_ECC_SECP256K1_PRIVKEY_LEN,	//for testing - comment this line when you want to use random number
		  pubKey, CMOX_ECC_SECP256K1_PUBKEY_LEN);
  endTick = HAL_GetTick();
  elapsedTime = endTick - startTick;

  // Verify API returned value
    if (retval_keys != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }
    // Cleanup context
    cmox_ecc_cleanup(&Ecc_Ctx);

    startTick = HAL_GetTick();
    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
    /*
     * for CMOX_ECC_BPP512T1_LOWMEM curve use
     *    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC128MULT_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
     */
    //generate shared secret using mcu's private key and remote public key
    retval_ecc = cmox_ecdh(&Ecc_Ctx,                                         // ECC context
						  //CMOX_ECC_BPP512T1_LOWMEM,
		  	  	  	  	  CMOX_ECC_SECP256K1_LOWMEM,                         // SECP256K1 ECC curve selected
						  (uint8_t *)privateKey, sizeof(privateKey),         // Local Private key
						  serverPublicKey, CMOX_ECC_SECP256K1_PUBKEY_LEN,	//Server's public key
						  Computed_Secret, &computed_size);      // Data buffer to receive shared secret

  endTick = HAL_GetTick();
  elapsedTime = endTick - startTick;
  // Verify API returned value
    if (retval_ecc != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }

    // Verify generated data size is the expected one
    if (computed_size != sizeof(Computed_Secret))
    {
      Error_Handler();
    }


    // Cleanup context
    cmox_ecc_cleanup(&Ecc_Ctx);

    startTick = HAL_GetTick();
    //AES CBC ENCRYPTION for 256-bit key length
    retval_cipher = cmox_cipher_encrypt(CMOX_AESFAST_CBC_ENC_ALGO,                  // Use AES EBC algorithm
    									Plaintext, sizeof(Plaintext),           	// Plaintext to encrypt
    									Computed_Secret, CMOX_CIPHER_256_BIT_KEY, 	// AES key to use
										IV, sizeof(IV),                         	// Initialization vector
        								(uint8_t *)Computed_Ciphertext, &computed_size_encdec); // Data buffer to receive generated ciphertext
    //Verify API returned value
    if (retval_cipher != CMOX_CIPHER_SUCCESS)
    {
    	Error_Handler();
    }

    endTick = HAL_GetTick();
    elapsedTime = endTick - startTick;

    startTick = HAL_GetTick();
    	// AES CBC DECRYPTION - using Computed_Secret
        retval_cipher = cmox_cipher_decrypt(CMOX_AESFAST_CBC_DEC_ALGO,                 // Use AES EBC algorithm
        									Computed_Ciphertext, sizeof(Computed_Ciphertext), // Ciphertext to decrypt
											Computed_Secret, CMOX_CIPHER_256_BIT_KEY,                      // AES key to use
    										IV, sizeof(IV),                        // Initialization vector
    										Computed_Plaintext, &computed_size_encdec);   // Data buffer to receive generated plaintext
        //Verify API returned value
        if (retval_cipher != CMOX_CIPHER_SUCCESS)
        {
        	Error_Handler();
         }
        // Verify generated data are the expected ones
        if (memcmp(Plaintext, Computed_Plaintext, computed_size_encdec) != 0)
        {
        	Error_Handler();
        }
        endTick = HAL_GetTick();
        elapsedTime = endTick - startTick;

        // No more need of cryptographic services, finalize cryptographic library
        if (cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
        {
          Error_Handler();
        }
        glob_status = PASSED;
    /*
    //HKDF Function - hmac key derivation function -
    //computed_PRK - psuedorandom key generated using Computed_Secret
    //computed_OKM - output keying material - can be considered as master key

    //Computed data buffer
     uint8_t computed_hash[CMOX_SHA256_SIZE];
     uint8_t computed_PRK[CMOX_SHA256_SIZE];
     uint8_t computed_OKM[128];  // L    = 128

     // HMAC context handle
     cmox_hmac_handle_t Hmac_Ctx;
     //size_t computed_size1;
     // General mac context
     cmox_mac_handle_t *mac_ctx;
     uint32_t L = 128;
     uint32_t N = 0;


     // Initialize cryptographic library
     if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS) Error_Handler();


      // Compute PRK = HMAC-Hash(salt, IKM)

     retval_hash = cmox_mac_compute(CMOX_HMAC_SHA256_ALGO,
							 	 Computed_Secret, sizeof(Computed_Secret),
								   salt, sizeof(salt),
								   NULL, 0,
								   computed_PRK,
								   CMOX_SHA256_SIZE,
								   &computed_size);

     // Verify API returned value
     if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();

      // Compute N = ceil(L/HashLen)

     N = (uint32_t)(L / CMOX_SHA256_SIZE);
     if ((N * CMOX_SHA256_SIZE) < L) N++;
*/
     /*
      * Compute OKM = first L octets of T
      * where:
      * T(0) = empty string (zero length)
      * T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
      * T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
      * T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
      ...
      */
/*
     mac_ctx = cmox_hmac_construct(&Hmac_Ctx, CMOX_HMAC_SHA256);
     if (mac_ctx == NULL) Error_Handler();

     index = 0;
     for (uint8_t i = 1; i <= N; i++)
     {
    	 retval_hash = cmox_mac_init(mac_ctx);
       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       // For the last iteration, tag length may be reduced to fit requested length L
       if (i == N)
       {
    	   retval_hash = cmox_mac_setTagLen(mac_ctx, L - index);
       }
       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       retval_hash = cmox_mac_setKey(mac_ctx, computed_PRK, sizeof(computed_PRK));

       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       if (i > 1)
       {
    	   retval_hash = cmox_mac_append(mac_ctx, computed_hash, CMOX_SHA256_SIZE);
         if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       }
       retval_hash = cmox_mac_append(mac_ctx, info, sizeof(info));
       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       retval_hash = cmox_mac_append(mac_ctx, &i, 1);
       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       retval_hash = cmox_mac_generateTag(mac_ctx, computed_hash, NULL);
       if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();
       if (i == N)
       {
         memcpy(&computed_OKM[index], computed_hash, L - index);
         index = L;
       }
       else
       {
         memcpy(&computed_OKM[index], computed_hash, CMOX_SHA256_SIZE);
         index += CMOX_SHA256_SIZE;
       }
     }
     retval_hash = cmox_mac_cleanup(mac_ctx);

     if (retval_hash != CMOX_MAC_SUCCESS) Error_Handler();

     	 	 // AES CBC ENCRYPTION - using computed_OKM

             retval_cipher = cmox_cipher_encrypt(CMOX_AESFAST_CBC_ENC_ALGO,                  // Use AES EBC algorithm
                                          	  Plaintext, sizeof(Plaintext),           // Plaintext to encrypt
											  computed_OKM, CMOX_CIPHER_128_BIT_KEY, // AES key to use
											  IV, sizeof(IV),                         // Initialization vector
											  (uint8_t *)Computed_Ciphertext, &computed_size_encdec); // Data buffer to receive generated ciphertext
             //Verify API returned value
             if (retval_cipher != CMOX_CIPHER_SUCCESS)
             {
               Error_Handler();
             }
     	 	 // AES CBC DECRYPTION - using computed_OKM
             retval_cipher = cmox_cipher_decrypt(CMOX_AESFAST_CBC_DEC_ALGO,                 // Use AES EBC algorithm
                							Computed_Ciphertext, sizeof(Computed_Ciphertext), // Ciphertext to decrypt
											computed_OKM, CMOX_CIPHER_128_BIT_KEY,                      // AES key to use
											IV, sizeof(IV),                        // Initialization vector
											Computed_Plaintext, &computed_size_encdec);   // Data buffer to receive generated plaintext
                  //Verify API returned value
                  if (retval_cipher != CMOX_CIPHER_SUCCESS)
                  {
                    Error_Handler();
                  }
                  // Verify generated data are the expected ones
                  if (memcmp(Plaintext, Computed_Plaintext, computed_size_encdec) != 0)
                  {
                    Error_Handler();
                  }
                  */


        /* Infinite loop */
        /* USER CODE BEGIN WHILE */
        int count = 0;
        while (1)
        {
      	  at_interface__process();
      	  if(timer__ms_elapsed(DRIVER_TIMER2))
      	  {
      		  if(++count == 1000)
      		  {
      			  count = 0;
      			  at_interface__send_test_string();
      		  }
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WIFI_MODEM_RESET_PORT, WIFI_MODEM_RESET_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin : A2 Wiznet360 reset */
  GPIO_InitStruct.Pin = WIFI_MODEM_RESET_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WIFI_MODEM_RESET_PORT, &GPIO_InitStruct);

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
