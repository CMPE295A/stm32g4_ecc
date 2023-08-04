#ifndef GENERATE_KEYPAIR_H
#define GENERATE_KEYPAIR_H

uint8_t generateKeys();
/*
void generate_keypair(uint8_t *mcuPrivateKey, size_t *mcuPrivateKeyLen,
						uint8_t *mcuPublicKey, size_t *mcuPublicKeyLen);

void generate_sharedsecret(uint8_t *mcuPrivateKey, size_t *mcuPrivateKeyLen,
							uint8_t *serverPublicKey, size_t *serverPublicKeyLen,
							uint8_t *sharedSecret, size_t *sharedSecretLen);
*/

#endif // GENERATE_KEYPAIR_H
