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
#include "interface_mqtt.h"

#define MAX_PRIVATE_KEY_LEN 256

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
HAL_StatusTypeDef hal_status = HAL_OK;

 // ECC context
 cmox_ecc_handle_t ecc_ctx;

 // CBC context handle
 cmox_cbc_handle_t Cbc_Ctx;

 //ECC working buffer
 uint8_t working_buffer[4000];

 // Random data buffer
 uint32_t Computed_Random[32];

 __IO TestStatus glob_status = FAILED;

 /* USER CODE END PTD */

/* Private variables ---------------------------------------------------------*/
 /* USER CODE BEGIN PV */

 UART_HandleTypeDef huart2;

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

const uint8_t Random[] =
 {
   0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda, 0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
   0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e, 0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
 };

/*
 * efc93803bd60c2be2a864409a289373126cc6554fa1f6c01f096dbf98b810abf
 */
  const uint8_t sPrivateKey[]={
  0xef, 0xc9, 0x38, 0x03, 0xbd, 0x60, 0xc2, 0xbe, 0x2a, 0x86, 0x44, 0x09, 0xa2, 0x89, 0x37, 0x31,
  0x26, 0xcc, 0x65, 0x54, 0xfa, 0x1f, 0x6c, 0x01, 0xf0, 0x96, 0xdb, 0xf9, 0x8b, 0x81, 0x0a, 0xbf

  };
/*
 * 04
 * 2a b3 a7 6c 0a 40 8d ab 50 31 44 d7 6f 40 24 ed ee 9a bc
 * aa 54 7a 81 c7 48 e6 d7 64 a3 e4 6f 39 5c f4 84 44 d9 b3
 * 97 1d ae ac e9 74 22 55 80 cf 12 4f 2b eb 71 20 da f2 3d
 * 8e 16 6e 20 5d 93 c9
 */
  const uint8_t sPublicKey[]={
  0x2a, 0xb3, 0xa7, 0x6c,0x0a,0x40,0x8d, 0xab,0x50,0x31,0x44,0xd7,0x6f,0x40,0x24,0xed,0xee,0x9a,0xbc,
  0xaa,0x54,0x7a,0x81,0xc7,0x48,0xe6,0xd7,0x64,0xa3,0xe4,0x6f,
  0x39,0x5c,0xf4,0x84,0x44,0xd9,0xb3,0x97,0x1d,0xae,0xac,0xe9,0x74,0x22,0x55,0x80,0xcf,0x12,0x4f,0x2b,
  0xeb,0x71,0x20,0xda,0xf2,0x3d,0x8e,0x16,0x6e,0x20,0x5d,0x93,0xc9
};
/*
--------------MCU public key------------
1ebde72699eb046423784bd9f329a59ab485555dd83d45c7dc0c51928d159ac7f21e21d6b598e1e1e1719943b793f7f76886fb78f87879c32b2bce64309237ff
*/
uint8_t mcuPublicKey[]={
	 0x1e , 0xbd , 0xe7 , 0x26 ,0x99 ,0xeb ,0x04 ,0x64 ,0x23 ,0x78 ,0x4b,0xd9 ,
	 0xf3 , 0x29 , 0xa5 ,0x9a , 0xb4 ,0x85 , 0x55 ,0x5d , 0xd8 ,0x3d ,
	 0x45 ,0xc7 , 0xdc ,0x0c , 0x51 ,0x92 , 0x8d ,0x15 , 0x9a ,0xc7 ,
	 0xf2 ,0x1e , 0x21 ,0xd6 , 0xb5 ,0x98 , 0xe1 ,0xe1 , 0xe1 ,0x71 , 0x99 ,0x43 ,
	 0xb7 ,0x93 ,0xf7 ,0xf7 , 0x68,0x86 , 0xfb ,0x78 , 0xf8 ,0x78 , 0x79 ,0xc3 ,
	 0x2b ,0x2b , 0xce ,0x64, 0x30 ,0x92 , 0x37 ,0xff
};
/*
----------MCU private key------------
7d7dc5f71eb29ddaf80d6214632eeae03d9058af1fb6d22ed80b
*/
const uint8_t mcuPrivateKey[]={
	 0x7d , 0x7d , 0xc5 , 0xf7 , 0x1e ,	0xb2 ,0x9d, 0xda ,0xf8 ,0x0d ,0x62 ,0x14 ,0x63 ,0x2e , 0xea , 0xe0 ,
	 0x3d ,	0x90 , 0x58 ,	0xaf , 0x1f ,0xb6 ,0xd2 ,0x2e , 0xd8 , 0x0b ,0xad ,	0xb6 ,0x2b ,0xc1 ,0xa5,0x35
};
/*
 // SECP256R1 curve
 //	uint8_t Computed_Secret[CMOX_ECC_SECP256R1_SECRET_LEN];
 //	uint8_t pubKey[CMOX_ECC_SECP256R1_PUBKEY_LEN];
 //	uint8_t privateKey[CMOX_ECC_SECP256R1_PRIVKEY_LEN];
*/
 /*
 //	BPP512T1 CURVE
 //	uint8_t Computed_Secret[CMOX_ECC_BPP512T1_SECRET_LEN];
 //	uint8_t Computed_Signature[CMOX_ECC_BPP512T1_SIG_LEN];
 //	uint8_t pubKey[CMOX_ECC_BPP512T1_PUBKEY_LEN];
 //	uint8_t privateKey[CMOX_ECC_BPP512T1_PRIVKEY_LEN];
*/

 //	SECP256K1 CURVE
 uint8_t sharedSecret[CMOX_ECC_SECP256K1_SECRET_LEN];
 uint8_t pubKey[CMOX_ECC_SECP256K1_PUBKEY_LEN];
 uint8_t privateKey[CMOX_ECC_SECP256K1_PRIVKEY_LEN];

 //uint8_t Computed_Ciphertext[CMOX_CIPHER_BLOCK_SIZE];
 //uint8_t Computed_Plaintext[CMOX_CIPHER_BLOCK_SIZE];

 uint8_t Computed_Ciphertext[64];
 uint8_t Computed_Plaintext[64];
// RNG peripheral handle
RNG_HandleTypeDef hrng;
 /* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void process_cli(void);
static void MX_RNG_Init(void);


typedef struct{
	  float lat;
	  float lon;
	  float battery;
	  float temperature;
}drone_status_t;

static const drone_status_t test_data = {.lat=37.3387, .lon=-121.8853, .battery=45.0, .temperature=21.1};


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

void get_drone_status(drone_status_t *status)
{
	*status = test_data;
}

int get_drone_status_string(drone_status_t *status, char *str)
{
	//return sprintf(str, "\"{\"GPS\":[%.4f, %.4f],\"Battery\":%.1f%,\"Temperature\":%.1fC}\"", status->lat, status->lon, status->battery, status->temperature);
}
void convert_to_hex_string(const uint8_t *inputArray, size_t inputLength, char *outputString) {
    for (size_t i = 0; i < inputLength; i++) {
        snprintf(&outputString[i * 2], 3, "%02x", inputArray[i]);
    }
    outputString[inputLength * 2] = '\0'; // null-terminate the string
}
void convert_to_byte_array(const char *hexString, uint8_t *outputArray, size_t *outputLength) {
    size_t inputLength = strlen(hexString);
    if (inputLength % 2 != 0) {
        // handle odd-length strings
        return;
    }
    size_t arrayLength = inputLength / 2;
    *outputLength = arrayLength;
    for (size_t i = 0; i < arrayLength; i++) {
        sscanf(&hexString[i * 2], "%2hhx", &outputArray[i]);
    }
}

int main(void)
{
	 /* USER CODE BEGIN 1 */
	  cmox_ecc_retval_t retval_sharedSecret; //return value for cmox_ecdh
	  size_t computed_size;
	  cmox_ecc_retval_t retval_mcuKeys;	//return value for cmox_ecdsa_keyGen
	  cmox_cipher_retval_t retval_cipher;	//return value for cmox_cipher_encrypt
	  size_t computed_cipher;	//Computed_Ciphertex and Computed_Plaintext
	  //uint32_t startTick = 0;
	  //uint32_t endTick = 0;
	  //uint32_t elapsedTime = 0;
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
  //SysTick_Config(SystemCoreClock / 1000);
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  timer__initialize(DRIVER_TIMER2, TIMER_MS_PER_SECOND);

  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
/*
 //Uncomment to use random number in cmox_ecdsa_keyGen
 //Generate random number to use Computed_Random in cmox_ecdsa_keyGen function

  for (uint32_t i = 0; i < sizeof(Computed_Random) / sizeof(uint32_t); i++)
         {
           if (HAL_RNG_GenerateRandomNumber(&hrng, &Computed_Random[i]) != HAL_OK)
           {
             // Random number generation error
           Error_Handler();
           }
         }
*/
  // Initialize cryptographic library
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }
  cmox_ecc_construct(&ecc_ctx, CMOX_ECC256_MATH_FUNCS, working_buffer, sizeof(working_buffer));

  /*
   * for CMOX_ECC_BPP512T1_LOWMEM curve use
   *    cmox_ecc_construct(&ecc_ctx, CMOX_ECC128MULT_MATH_FUNCS, working_buffer, sizeof(working_buffer));
   */
  //startTick = HAL_GetTick();

  //generate public and private key for mcu
  retval_mcuKeys=cmox_ecdsa_keyGen(&ecc_ctx,
		  CMOX_ECC_SECP256K1_LOWMEM,								// [in] SECP256K1 curve
		  //CMOX_ECC_BPP512T1_LOWMEM,								// [in] BRAINPOOL T-512 curve
		  Random, sizeof(Random),									// [in] for testing - comment this line when you want to use random number using RNG
		  //(uint8_t *)Computed_Random, sizeof(Computed_Random),	// [in] uncomment this line to use random number
		  (uint8_t*)privateKey, CMOX_ECC_SECP256K1_PRIVKEY_LEN,		// [out] mcu private key
		  (uint8_t*)pubKey, CMOX_ECC_SECP256K1_PUBKEY_LEN);			// [out] mcu public key
  //endTick = HAL_GetTick();
  //elapsedTime = endTick - startTick;

  // Verify API returned value
    if (retval_mcuKeys != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }
    // Cleanup context
    cmox_ecc_cleanup(&ecc_ctx);

    //startTick = HAL_GetTick();
    cmox_ecc_construct(&ecc_ctx, CMOX_ECC256_MATH_FUNCS, working_buffer, sizeof(working_buffer));
    /*
     * for CMOX_ECC_BPP512T1_LOWMEM curve use
     *    cmox_ecc_construct(&ecc_ctx, CMOX_ECC128MULT_MATH_FUNCS, working_buffer, sizeof(working_buffer));
     */
    //generate shared secret using mcu's private key and remote public key
    retval_sharedSecret = cmox_ecdh(&ecc_ctx,                                       // [in] ECC context
						  //CMOX_ECC_BPP512T1_LOWMEM,								// [in] BRAINPOOL T-512 curve
		  	  	  	  	  CMOX_ECC_SECP256K1_LOWMEM,                         		// [in] SECP256K1 ECC curve selected
						  (uint8_t *)privateKey, sizeof(privateKey),         		// [in] mcu private key
						  //mcuPrivateKey, sizeof(mcuPrivateKey),
						  sPublicKey, sizeof(sPublicKey),							// [in] server public key
						  sharedSecret, &computed_size);      						// [out] data buffer to receive shared secret
  //endTick = HAL_GetTick();
  //elapsedTime = endTick - startTick;
  // Verify API returned value
    if (retval_sharedSecret != CMOX_ECC_SUCCESS)
    {
      Error_Handler();
    }

    // Verify generated data size is the expected one
    if (computed_size != sizeof(sharedSecret))
    {
      Error_Handler();
    }
    // cleanup context
    cmox_ecc_cleanup(&ecc_ctx);

    if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
    {
      Error_Handler();
    }
    //startTick = HAL_GetTick();
    //AES CBC ENCRYPTION for 256-bit key length
    retval_cipher = cmox_cipher_encrypt(CMOX_AESFAST_CBC_ENC_ALGO,                  	// [in] AES CBC algorithm
    									Plaintext, sizeof(Plaintext),           		// [in] plaintext to encrypt
										sharedSecret, CMOX_CIPHER_256_BIT_KEY, 			// [in] AES key to use
										IV, sizeof(IV),                         		// [in] initialization vector
        								(uint8_t *)Computed_Ciphertext, &computed_cipher); // [out] data buffer to receive generated ciphertext
    //Verify API returned value
    if (retval_cipher != CMOX_CIPHER_SUCCESS)
    {
    	Error_Handler();
    }
    //endTick = HAL_GetTick();
    //elapsedTime = endTick - startTick;

    //No more need of cryptographic services, finalize cryptographic library
   if (cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
   {
	 Error_Handler();
   }
   glob_status = PASSED;


	 char pubKeyStr[sizeof(pubKey) * 2 + 1]; // double the size for two hexadecimal digits per byte + 1 for null terminator
	 convert_to_hex_string(pubKey, sizeof(pubKey), pubKeyStr);

	 /*
	 char test_str[]="1ebde72699eb046423784bd9f329a59ab485555dd83d45c7dc0c51928d159ac7f21e21d6b598e1e1e1719943b793f7f76886fb78f87879c32b2bce64309237ff";
	 uint8_t test_str_array[sizeof(test_str) / 2];
	 size_t test_str_arr_length = 0;
	 convert_to_byte_array(test_str, test_str_array, &test_str_arr_length);

	 printf("converted array: ");
		for (size_t i = 0; i < test_str_arr_length; i++) {
			printf("0x%02x ", test_str_array[i]);
		}
		printf("\n");
	  */

/*
	   //AES CBC DECRYPTION for 256-bit key length
	   retval_cipher = cmox_cipher_decrypt(CMOX_AESFAST_CBC_DEC_ALGO,                  				// [in] AES CBC algorithm
											Computed_Ciphertext, sizeof(Computed_Ciphertext),       // [in] ciphertext to decrypt
											sharedSecret, CMOX_CIPHER_256_BIT_KEY, 					// [in] AES key to use
											IV, sizeof(IV),                         				// [in] initialization vector
											(uint8_t *)Computed_Plaintext, &computed_cipher); 		// [out] data buffer to receive decrypted plaintext
	   	   //Verify API returned value
		  if (retval_cipher != CMOX_CIPHER_SUCCESS)
		  {
			Error_Handler();
		  }
*/
           wait_ms(1000);
           HAL_GPIO_WritePin(WIFI_MODEM_RESET_PORT, WIFI_MODEM_RESET_PIN, GPIO_PIN_SET);
           wait_ms(1000);
           at_interface__initialize();

           int count = 0;
             static const char test_string_aws[] = "\"{\"status\":\"c0f958353c5af040db056247f4d7dd9f\",\"gps\":{\"latitude\":\"test\",\"longitude\":\"10f4678d15f185427e2911791d751b8f\"},\"batteryLevel\":\"ca74ca2f55cbe7f92789a704ff686c2f\",\"temperature\":\"ca74ca2f55cbe7f92789a704ff686c2f\"}\"";

             char data_string[100];
             char private_key_str[MAX_PRIVATE_KEY_LEN] = {0};
           //  static const char TEST_STRING[] = "\"{\"GPS\":[37.3387,-121.8853],\"Battery\":45%,\"Temperature\": 21.1C}\"";
             bool got_shared_secret = false;
             while (1)
             {
           	  bool ms_elapsed = false;
           	  if(timer__ms_elapsed(DRIVER_TIMER2))
           	  {
           		  ms_elapsed = true;
           		  mqtt__process();
           		  if(++count == 1000)
           		  {

           			  count = 0;
           //			  at_interface__publish_test();
           			  if(got_shared_secret)
           			  {
						  drone_status_t data = {0};
						  get_drone_status(&data);
						  get_drone_status_string(&data, data_string);
						  // encrypt string
						  mqtt__publish(test_string_aws, strlen(test_string_aws));
           			  }
           			  else
           			  {
           				  if(mqtt__get_sub_message(private_key_str, MAX_PRIVATE_KEY_LEN) != 0)
           				  {
           					got_shared_secret = true;
           				  }
           			  }
           		  }
           	  }
           	  at_interface__process(ms_elapsed);
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
