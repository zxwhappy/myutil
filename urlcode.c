#include <stdio.h>
#include <ctype.h>
#include "urlcode.h"

static char x2c(char *what) {
	int digit;
	int c;

	c = ((unsigned char *)what)[0];
	if (islower(c)) {
		c = toupper(c);
	}
	digit = (c <= 'Z' && c >= 'A' ? c - 'A' + 10 : c - '0') * 16;

	c = ((unsigned char *)what)[1];
	if (islower(c)) {
		c = toupper(c);
	}
	digit += (c <= 'Z' && c >= 'A' ? c - 'A' + 10 : c - '0');

	return digit;
}

int32_t url_decode(char *decode_buf, char *src, int32_t srclen)
{
	int n = 0;
	char *end = src+srclen;
	char ch;

	while (src < end) {
		if (*src == '+') {
			decode_buf[n++] = ' ';
		} else if (*src == '%') {
			//%u0061, etc.
			if (*(src+1) == 'u' || *(src+1) == 'U') {// && src+5 < end) {
				if (src+5 < end) {
					if (isxdigit(*(src+4)) &&  isxdigit(*(src+5))) {

						ch = (char)x2c(src+4);
						if (ch == '\r' || ch == '\n') {
							decode_buf[n++] = ' ';
						} else {
							decode_buf[n++] = ch;
						}
						src += 5;
					}
				} else {
					//ignore the %
				}
			} else if (src+2 < end) {
				if (isxdigit(*(src+1)) &&  isxdigit(*(src+2))) {
					ch = (char)x2c(src+2);
					if (ch == '\r' || ch == '\n') {
						decode_buf[n++] = ' ';
					} else {
						decode_buf[n++] = ch;
					}
					src += 2;
				} else {
					//ignore the %
				}
			} else {
					if (*src == '\r' || *src == '\n') {
						decode_buf[n++] = ' ';
					} else {
						decode_buf[n++] = *src;
					}
			}

		} else {
			if (*src == '\r' || *src == '\n') {
				decode_buf[n++] = ' ';
			} else {
				decode_buf[n++] = *src;
			}
		}
		src++;
	}

	return n;
}
