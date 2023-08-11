#ifndef GENERATE_KEYPAIR_H
#define GENERATE_KEYPAIR_H



void generate_keypair(uint8_t *mcuPrivateKey,
						uint8_t *mcuPublicKey);

void generate_sharedsecret(uint8_t *mcuPrivateKey, size_t mcuPrivateKeyLen,
							uint8_t *serverPublicKey, size_t serverPublicKeyLen,
							uint8_t *sharedSecret);

void encrypt_data(uint8_t *sharedSecret,
					uint8_t *plainText, size_t plainTextLen,
					uint8_t *cipherText);
/*
void decrypt_data(uint8_t *sharedSecret,
					uint8_t *cipherText, size_t cipherTextLen,
					uint8_t *plainComputed){
*/

/*
void generate_sharedsecret( uint8_t *mcuPrivateKey, size_t mcuPrivateKeyLen,
							uint8_t *serverPublicKey, size_t serverPublicKeyLen,
							uint8_t *sharedSecret, size_t *sharedSecretLen);

*/
#endif // GENERATE_KEYPAIR_H
