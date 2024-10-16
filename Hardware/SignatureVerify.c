#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define uECC_CURVE uECC_secp256k1
#include "uecc.h"

extern char Address[50];
extern char Message[20];
extern char SignatureHash[129];
extern char time[20];
uint8_t publicKey[64];
uint8_t signature[64];
uint8_t messageHash[32];


void hexStringToBytes(const char* hexString, uint8_t* byteArray, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sscanf(hexString + 2 * i, "%2hhx", &byteArray[i]);
    }
}

void Signature_get(void){
	char* hexPtr = SignatureHash;
	
	if(SignatureHash[1] == '0' && SignatureHash[2] == 'x'){
		hexPtr = SignatureHash + 2;
	}
	
	hexStringToBytes(hexPtr, signature, 64);
}

void MesseageHash_Get(const char* message, uint8_t* hash) {
	keccak_hash(message, strlen(message), hash);
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	