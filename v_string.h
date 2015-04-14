#ifndef _V_STRING_H
#define _V_STRING_H
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "common.h"

#define MIN(a,b) (a<b)?a:b

void noCRLFinstead(char *dst);
int v_noCRLFncpy(char *dst, int size, const char *src, int len);
int v_strncpy(char *dst, int size, const char *src, int len);
int v_strcpy(char *dst, int size, const char *src);
int v_real_strncpy(char *dst, int size, const char *src, int len);
int v_wstrncpy(char *dst, int size, const char *src, int len);
/* 
 *	Find the first occurrence of the substring.
 */
char *v_strnstr(char *dst, int32_t dstlen, char *src, int32_t slen);
char *v_memstr(const char *haystack, int range, const char *needle, int needle_len);
char *v_nocasememstr(const char *haystack, int range, const char *needle, int needle_len);
/* 
 *	Find the first occurrence of the substring, no case sense
 */
char *v_memcasestr_lowerneedle(const char *haystack, int range, const char *needle, int needle_len);

/* 
 *	Find the last occurrence of the substring.
 */
char *v_memrstr(const char *haystack, int range, const char *needle, int needle_len);

int v_memcpy(void *dst, int size, const void *src, int n);

int v_memcat(char *dst, int size, int max_size, const char *src, int len);
int32_t v_strncat(int8_t *dst, int32_t max_size, int8_t *src);

int32_t trim_str(int8_t *str);

int32_t v_strspn(int8_t* string, int32_t string_len, int8_t* control);
int32_t v_strcspn(int8_t* string, int32_t string_len, int8_t* control);

int32_t v_is_digit_string(int8_t *str, int32_t str_len);
int32_t v_strcasecmp(int8_t *text, int32_t text_len, int8_t *pattern, int32_t pattern_len); 
int32_t v_atoi(int8_t *data, int32_t data_len);
int32_t v_safe_atoi(int8_t *begin, int32_t data_len, int32_t *value);
int32_t	v_strarray_mem_clear(int8_t *strarray, int32_t row_num, int32_t column_num);
int32_t v_strlen(int8_t *begin, int32_t data_len);
unsigned int s_str_append(char *dst, int dst_size, const char *src, int src_len);
int32_t v_strncat_comma(int8_t *dst, int32_t max_dst_size, int8_t *src, int32_t src_size);
int32_t v_strncmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len);
int32_t v_strncasecmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len);

int8_t *v_memchr_escape(int8_t *s, int32_t c, int32_t n);
int8_t *v_str_replace(int8_t *src, int32_t src_len, int8_t *oldstr, int8_t *newstr);
void get_time_str(char *time_str, int time_str_size);
void get_timestr(char *time_str, int time_str_size, time_t *cur);
#endif
