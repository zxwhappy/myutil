#ifndef _MD5_H_
#define _MD5_H_

#define R_memset(x, y, z) memset(x, y, z)
#define R_memcpy(x, y, z) memcpy(x, y, z)
#define R_memcmp(x, y, z) memcmp(x, y, z)

//typedef unsigned long UINT4;
typedef unsigned int UINT4;
typedef unsigned char *POINTER;

/* MD5 context. */
typedef struct {
	/* state (ABCD) */   
	UINT4 state[4];  

	/* number of bits, modulo 2^64 (lsb first) */    
	UINT4 count[2];

	/* input buffer */ 
	unsigned char buffer[64];  
} MD5_CTX;

void MD5Init(MD5_CTX *ctx);
void MD5Update(MD5_CTX *ctx, unsigned char *input, unsigned int input_len);
void MD5Final(unsigned char [16], MD5_CTX *);
void encrypt_md5_data(unsigned char *output, int32_t output_len, unsigned char *buffer, int32_t size);

#endif /* _MD5_H_ */
