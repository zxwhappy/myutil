#ifndef _BASE64_H
#define _BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

	/*
	 *  * Note: The 'output' variable should be a preallocated buffer that
	 *  * should have enough allocated space for the encoded/decoded
	 *  * result. Failure to do so will result in a buffer overrun.
	 *  */

	int base64_encode(unsigned char *input, unsigned input_length, unsigned char *output, int outputsize);

	int base64_decode(unsigned char *input, unsigned input_length, unsigned char *output, int outputsize);

#ifdef __cplusplus
}
#endif

#endif
