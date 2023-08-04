#include "main.h"
#include "driver_uart.h"
#include "driver_timer.h"
#include "cmox_crypto.h"
#include <string.h>
#include "generate_keys.h"
#include "encryption.h"

 const uint8_t IV[] =
 {
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
 };



cmox_cipher_retval_t retval_cipher;	//return value for cmox_cipher_encrypt
//uint8_t cipherText[CMOX_CIPHER_BLOCK_SIZE];



void encryption(uint8_t *sharedSecret, uint8_t *plainText, uint8_t *cipherText){
	 size_t sharedSecretLen=CMOX_CIPHER_256_BIT_KEY;
	 size_t plainTextLen;
	 size_t cipherTextLen;
	 //uint8_t cipherText[128];

	   //AES CBC ENCRYPTION for 256-bit key length
	    retval_cipher = cmox_cipher_encrypt(CMOX_AESFAST_CBC_ENC_ALGO,                  // Use AES CBC algorithm
	    									plainText, plainTextLen,           // Plaintext to encrypt
											sharedSecret, CMOX_CIPHER_256_BIT_KEY, 	// AES key to use
											IV, sizeof(IV),                         	// Initialization vector
											(uint8_t *)cipherText, cipherTextLen); // Data buffer to receive generated ciphertext
	    //Verify API returned value
	    if (retval_cipher != CMOX_CIPHER_SUCCESS)
	    {
	    	Error_Handler();
	    }
}
