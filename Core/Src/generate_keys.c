#include "main.h"
#include "driver_uart.h"
#include "driver_timer.h"
#include "cmox_crypto.h"
#include <string.h>
#include "generate_keys.h"

 const uint8_t Random[] =
 {
   0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda, 0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
   0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e, 0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
 };
 const uint8_t IV[] =
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
  };
 /* ECC context */
 cmox_ecc_handle_t Ecc_Ctx;

 cmox_ecc_retval_t retval_keys;	//return value for cmox_ecdsa_keyGen

 cmox_ecc_retval_t retval_sharedSecret; //return value for cmox_ecdh

 cmox_cipher_retval_t retval_ciphertext;	//return value for cmox_cipher_encrypt
 cmox_cipher_retval_t retval_plaintext;

 uint8_t Working_Buffer[4000];	//ecc working buffer

 RNG_HandleTypeDef hrng;

 uint32_t computedRandom[32];

 size_t mcuPrivateKeyLen=CMOX_ECC_SECP256K1_PRIVKEY_LEN;
 size_t mcuPublicKeyLen=CMOX_ECC_SECP256K1_PUBKEY_LEN;

 void generate_keypair(uint8_t *mcuPrivateKey, uint8_t *mcuPublicKey) {

	 if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
	  {
	    Error_Handler();
	  }

	 /*
	   * 	for CMOX_ECC_BPP512T1_LOWMEM curve use the following cmox_ecc_construct
	   *
	   *    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC128MULT_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
	   */

	  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

	  	hrng.Instance = RNG;
	  	hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
	  	if (HAL_RNG_Init(&hrng) != HAL_OK)
	  	  {
	  	    Error_Handler();
	  	  }
	  	  //Generate random number to use computedRandom in cmox_ecdsa_keyGen function

	  	   for (uint32_t i = 0; i < sizeof(computedRandom) / sizeof(uint32_t); i++)
	  			  {
	  				if (HAL_RNG_GenerateRandomNumber(&hrng, &computedRandom[i]) != HAL_OK)
	  				{
	  				  // Random number generation error
	  				Error_Handler();
	  				}
	  			  }
	  //generate_random((uint32_t *)computedRandom);
	  // generate mcuPublicKey and mcuPrivateKey

	  retval_keys = cmox_ecdsa_keyGen(&Ecc_Ctx,							//ECC context
			  	  	  	  	 //CMOX_ECC_BPP512T1_LOWMEM,
			  	  	  	  	  CMOX_ECC_SECP256K1_LOWMEM,				//SECP256K1 curve
							  //Random, sizeof(Random),
							  (uint8_t *)computedRandom, sizeof(computedRandom), //random
							  mcuPrivateKey, &mcuPrivateKeyLen,			//mcuPrivateKey
							  mcuPublicKey, &mcuPublicKeyLen);			//mcuPublicKey

  if (retval_keys != CMOX_ECC_SUCCESS) {
    printf("Error generating keys\n");
    return;
  }

  cmox_ecc_cleanup(&Ecc_Ctx);
}

void generate_sharedsecret(uint8_t *mcuPrivateKey, size_t mcuPrivateKeyLen, uint8_t *serverPublicKey, size_t serverPublicKeyLen, uint8_t *sharedSecret){

		/*
		 * 	for CMOX_ECC_BPP512T1_LOWMEM curve use the following cmox_ecc_construct
		 *
		 *    cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC128MULT_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));
		 */
    if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS) {
        Error_Handler();
    }

    size_t sharedSecretLen;

    // Create ECC context and working buffer
    cmox_ecc_handle_t Ecc_Ctx;
    uint8_t Working_Buffer[4000]; // Make sure this buffer is large enough

	  cmox_ecc_construct(&Ecc_Ctx, CMOX_ECC256_MATH_FUNCS, Working_Buffer, sizeof(Working_Buffer));

	  retval_sharedSecret = cmox_ecdh(&Ecc_Ctx,          			// ECC context
							  //CMOX_ECC_BPP512T1_LOWMEM,
			  	  	  	  	  CMOX_ECC_SECP256K1_LOWMEM,      		// SECP256K1 ECC curve
							  mcuPrivateKey, mcuPrivateKeyLen,		//mcuPrivateKey
							  serverPublicKey, serverPublicKeyLen,	//serverPublicKey
							  sharedSecret, &sharedSecretLen);		//sharedSecret

	  if (retval_sharedSecret != CMOX_ECC_SUCCESS) {
	     printf("Error generating shared secret\n");
	     return;
	   }

	   cmox_ecc_cleanup(&Ecc_Ctx);

}

void encrypt_data(uint8_t *sharedSecret, uint8_t *plainText, size_t plainTextLen, uint8_t *cipherText){

	size_t computed_cipher;
	  //startTick = HAL_GetTick();
	  //AES CBC ENCRYPTION for 256-bit key length
	retval_ciphertext = cmox_cipher_encrypt(CMOX_AESFAST_CBC_ENC_ALGO,                  	// [in] AES CBC algorithm
			  	  	  	  	  	  	  	  plainText, plainTextLen,           		// [in] plaintext to encrypt
											sharedSecret, CMOX_CIPHER_256_BIT_KEY, 			// [in] AES key to use
											IV, sizeof(IV),                         		// [in] initialization vector
											(uint8_t *)cipherText, &computed_cipher); // [out] data buffer to receive generated ciphertext
	  //Verify API returned value
	  if (retval_ciphertext != CMOX_CIPHER_SUCCESS)
	  {
		 printf("Error generating ciphertext \n");
		 return;
		//Error_Handler();
	  }
	  //endTick = HAL_GetTick();
	  //elapsedTime = endTick - startTick;

	  //No more need of cryptographic services, finalize cryptographic library
	 if (cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
	 {
		 Error_Handler();
	 }
}

/*
void decrypt_data(uint8_t *sharedSecret, uint8_t *plainText, size_t plainTextLen, uint8_t *cipherText){

	size_t computed_cipher;

		   //AES CBC DECRYPTION for 256-bit key length
		   retval_plaintext = cmox_cipher_decrypt(CMOX_AESFAST_CBC_DEC_ALGO,                  				// [in] AES CBC algorithm
												cipherText, cipherTextLen,       // [in] ciphertext to decrypt
												sharedSecret, CMOX_CIPHER_256_BIT_KEY, 					// [in] AES key to use
												IV, sizeof(IV),                         				// [in] initialization vector
												(uint8_t *)plainComputed, &computed_cipher); 		// [out] data buffer to receive decrypted plaintext
	  //Verify API returned value
	  if (retval_ciphertext != CMOX_CIPHER_SUCCESS)
	  {

		//Error_Handler();
	  }
}
*/
