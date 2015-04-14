
/*
 * $Id: v_string.c,V1.0 $
 */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "likely.h"
#include "v_string.h"

char *v_strnstr(char *dst, int32_t dstlen, char *src, int32_t slen)
{
    if (NULL == src || NULL == dst) {
        return NULL;

    }

    if (slen > dstlen) {
        return NULL;

    }

    return strstr(dst, src);

}

int v_noCRLFncpy(char *dst, int size, const char *src, int len)
{
    int l;
    char *dp = NULL;
    const char *sp = NULL;
    if (unlikely(size < 1 || len < 0)) {
        return 0;

    }
    /* new add by wuyao 2009-03-22 */
    if (unlikely(NULL == dst || NULL == src)) {
        return 0;

    }

    dp = dst;
    sp = src;
    l = MIN(size - 1, len);
    while( sp != src + l ){
        if( '\r' == *sp || '\n' == *sp  ) {
            *dp = ' ';
            dp++;
            sp++;

        } else {
            *dp = *sp;
            dp++;
            sp++;

        }

    }
    dst[l] = 0;

    return l;

}

void noCRLFinstead(char *dst)
{
    char *ptr = NULL;

    if (unlikely(NULL == dst)) {
        return; 
    }

    ptr = dst;
    while (*ptr != '\0') {
        if ('\r' == *ptr || '\n' == *ptr) {
            *ptr = ' ';
        }
		ptr++;

    }

    return;
}

static inline int _v_strncpy(char *dst, int size, const char *src, int len)
{
	int l;
	
	l = MIN(size - 1, len);
	memcpy(dst, src, l);
	dst[l] = 0;

	return l;
}


int v_strncpy(char *dst, int size, const char *src, int len)
{
	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	/* new add by wuyao 2009-03-22 */
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	return _v_strncpy(dst, size, src, len);
}

int v_real_strncpy(char *dst, int size, const char *src, int len)
{
	int l = 0;
	int i = 0;

	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	/* new add by wuyao 2009-03-22 */
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}
	
	l = MIN(size - 1, len);

	for (i = 0; i < l; i++) {
		if (0 == src[i]) {
			break;
		}
		dst[i] = src[i];
	}

	dst[i] = 0;

	return i;
}


int v_wstrncpy(char *dst, int size, const char *src, int len)
{
	int l = 0;
	int i = 0;

	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	/* new add by wuyao 2009-03-22 */
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}
	
	l = MIN(size - 2, len);

	for (i = 0; i + 1 < l; i += 2) {
		if (0 == src[i] && 0 == src[i + 1]) {
			break;
		}
		dst[i] = src[i];
		dst[i + 1] = src[i + 1];
	}

	dst[i] = 0;
	dst[i + 1] = 0;

	return i;
}

int v_strcpy(char *dst, int size, const char *src)
{
	int len = 0;
	
	
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	len = strlen(src);
	
	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	

	return _v_strncpy(dst, size, src, len);
}

static int table[256];//one byte data can represent 256 char

void ShiftTable(const char *pattern, int p_len){
	int i = 0;
	for(i = 0; i < 256; i++) { 
		table[i] = p_len;
	}
	for(i = 0; i < p_len - 1; i++) {
		table[ (uint8_t)pattern[i] ] = p_len - 1 - i;
	}

}

char *v_memstr(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	if (1 == needle_len) {
		return memchr(haystack, needle[0], range);
	}

	ShiftTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && (needle[m - 1 - k] == haystack[i - k])) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + table[(uint8_t)haystack[i]]; 
		}
	}

	return NULL;
}

char *v_memrstr(const char *haystack, int range, const char *needle, int needle_len)
{
	char *p = (char *) haystack + range;
	
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	p -= needle_len;

	while(p >= haystack) {
		if(!memcmp(p, needle, needle_len))
			return p;
		p--;
	}

	return NULL;
}

static int nocasetable[256];//one byte data can represent 256 char

void ShiftNocaseTable(const char *pattern, int p_len){
	int i = 0;
	int m = p_len;
	for(i = 0; i < 256; i++) { 
		nocasetable[i] = p_len;
	}
	for(i = 0; i < p_len - 1; i++) {
		nocasetable[ (uint8_t)pattern[i] ] = p_len - 1 - i;
		if ('a' < (uint8_t)pattern[i] && (uint8_t)pattern[i] < 'z') {
			nocasetable[ (uint8_t)pattern[i] - 32 ] = m - 1 - i;
		} else if ('A' < (uint8_t)pattern[i] && (uint8_t)pattern[i] < 'Z') {
			nocasetable[ (uint8_t)pattern[i] + 32 ] = m - 1 - i;
		}
	}

}

int nocaseequal(char a, char b) 
{
	int ret = 0;

	if (a == b) {
		ret = 1;
	} else if ('a' < a && a < 'z') {
		if (a - 32 == b) {
			ret = 1;
		}
	} else if ('A' < a && a < 'Z') {
		if (a + 32 == b) {
			ret = 1;
		}
	}

	return ret;
}

char *v_nocasememstr(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	if (1 == needle_len) {
		return memchr(haystack, needle[0], range);
	}

	ShiftNocaseTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && nocaseequal(needle[m - 1 - k], haystack[i - k])) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + nocasetable[(uint8_t)haystack[i]]; 
		}
	}

	return NULL;
}

static int casetable[256];//one byte data can represent 256 char
void ShiftcaseTable(const char *pattern, int p_len){
	int i = 0;
	int j = 0;
	int m = p_len;
	for(i = 0; i < 256; i++) { 
		casetable[i] = m;
	}
	for(j = 0; j < m - 1; j++) {
		casetable[ (uint8_t)pattern[j] ] = m - 1 - j;
		if ('a' < (uint8_t)pattern[j] && (uint8_t)pattern[j] < 'z') {
			casetable[ (uint8_t)pattern[j] - 32 ] = m - 1 - j;
		}
	}

}

char *v_memcasestr_lowerneedle(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	ShiftcaseTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && (needle[m - 1 - k] == haystack[i - k] || needle[m - 1 - k] == (haystack[i - k] + 32))  ) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + casetable[(uint8_t)haystack[i]]; 
		}
	}

	return NULL;
}

int v_memcpy(void *dst, int size, const void *src, int n)
{
	int l;

	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	if (unlikely(size < 1 || n < 1)) {
		return 0;
	}

	l = MIN(size, n);
	memcpy(dst, src, l);

	return l;
}

int v_memcat(char *dst, int size, int max_size, const char *src, int len)
{
	int32_t min = 0;

	if (unlikely(!dst || !src || size < 1 || len < 1 || max_size < 1)) {
		goto err;
	}

	min = (max_size - size) < len ? (max_size - size) : len;

	memcpy(dst + size, src, min);
	return min + size;

err:
	return -1;
}

unsigned int s_str_append(char *dst, int dst_size, const char *src, int src_len)
{
    int remain_size = 0;
    int dst_len = 0;
    unsigned int min = 0;

    if (NULL == dst || 0 >= dst_size || NULL == src || 0 >= src_len) {
        goto out;

    }

    //printf("dst_len = %d; dst_size=%d\n", dst_len, dst_size);
    dst_len = strlen(dst);
	//printf("dst_len = %d; dst_size=%d\n", dst_len, dst_size);
    remain_size = dst_size - dst_len;
    if (remain_size <= 0) {
        goto out;

    }

    min = MIN(remain_size - 1, src_len);
	//printf("min = %d; src_len=%d\n", min, src_len);
	//printf("src = %s\n", src);
    strncpy(dst+dst_len, src, min);
    dst[dst_len+min] = '\0';
	//printf("dst_string(%d) = %s\n", strlen(dst), dst);
out:
    return min;
}

int32_t v_strncat(int8_t *dst, int32_t max_dst_size, int8_t *src)
{
	int32_t src_size = strlen(src);
	int32_t dst_size = strlen(dst);
	int32_t cp_len = 0;

	if (dst_size >= max_dst_size - 1) {
		goto err;
	}

	cp_len = src_size > max_dst_size - dst_size - 1 ? max_dst_size - dst_size - 1 : src_size;
	memcpy(dst + dst_size, src, cp_len);
	dst[cp_len + dst_size] = '\0';

err:
	return cp_len + dst_size;

}

int32_t v_strncat_comma(int8_t *dst, int32_t max_dst_size, int8_t *src, int32_t src_size)
{
	int32_t dst_size = 0;
	int32_t cp_len = 0;
	
	if (0 == src_size) {
		goto err;
	}

	dst_size = strlen(dst);
	if (0 != dst_size) {
		if (dst_size >= max_dst_size - 2) {
			goto err;
		}
		dst[dst_size] = ',';
		dst_size++;
	} else {
		if (dst_size >= max_dst_size - 1) {
			goto err;
		}
	}

	cp_len = src_size > max_dst_size - dst_size - 1 ? max_dst_size - dst_size - 1 : src_size;
	memcpy(dst + dst_size, src, cp_len);
	dst[cp_len + dst_size] = '\0';
err:
	return cp_len + dst_size;
}

int32_t trim_str(int8_t *str)
{
	int32_t  i = 0;
	int32_t  j = 0;
	int32_t len = 0;

	len = strlen(str);
	if (len <= 0) {
		return 0;
	}

	for (i = 0; i < len; i++) {
		if ((*(str+i) == 9 || *(str+i) == 32) == 0) {
			break;
		}
	}
	for (; i < len; i++, j++) {
		*(str+j) = *(str+i);
	}
	*(str+j) = '\0';

	len = strlen(str);
	for (i = len - 1; i >= 0; i--) {
		if ((*(str+i) == 9 || *(str+i) == 32)) {
			*(str+i) = '\0';
		}else{
			break;
		}
	}
	return strlen(str);
}

int32_t v_strspn(int8_t* string, int32_t string_len, int8_t* control)
{
	if (NULL == string || NULL == control || string_len <= 0) {
		return -1;
	}
	uint8_t *str = (uint8_t*)string;
	uint8_t *ctrl = (uint8_t *)control;
	uint8_t  map[32] = {0};
	int32_t count = 0;

	/* Clear out bit map */
	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}

	/* Set bits in control map */
	while (*ctrl) {//register each char in control string set  
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++;
	}

	/* 1st char NOT in control map stops search */
	if (*str) {//check each char in string, comparing the each char in the register table 
		count = 0;
		while (count < string_len && (map[*str >> 3] & (1 << (*str & 7)))) {
			count++;
			str++;
		}
		return(count);
	}
	return(0);
}


int32_t v_strcspn(int8_t* string, int32_t string_len, int8_t* control)
{
	if (NULL == string || NULL == control || string_len <= 0) {
		return -1;
	}
	uint8_t *str = (uint8_t*)string;
	uint8_t *ctrl = (uint8_t*)control;
	uint8_t  map[32] = {0};
	int32_t count = 0;

	/* Clear out bit map */
	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}

	/* Set bits in control map */
	while (*ctrl) {//register each char in control string set  
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++;
	}

	map[0] |= 1;    /* null chars not considered */
	/* 1st char NOT in control map stops search */
	if (*str) {//check each char in string, comparing the each char in the register table 
		count = 0;
		while (count < string_len && (!(map[*str >> 3] & (1 << (*str & 7))))) {
			count++;
			str++;
		}
		return(count);
	}

	return(0);
}

int32_t v_is_digit_string(int8_t *str, int32_t str_len)
{
	int32_t i = 0;

	if (NULL == str || 0 >= str_len) {
		goto err;
	}

	for (i = 0; i < str_len; i++) {
		if (str[i] < '0' || str[i] > '9') {
			goto err;
		}
	}
	
	return 0;
err:
	return -1;
}

int32_t v_strcasecmp(int8_t *text, int32_t text_len, int8_t *pattern, int32_t pattern_len) 
{
	int32_t	ret = 0;
	int32_t	i = 0;

	if (NULL == text || NULL == pattern || pattern_len <= 0 || pattern_len != text_len) {
		ret = 1;
		goto err;
	}

	for (i = 0; i < pattern_len; i++) {
		if (text[i] == pattern[i]) {
			continue;
		} else if (('a' < text[i] && text[i] < 'z') || ('A' < text[i] && text[i] < 'Z')) {
			if (('a' < pattern[i] && pattern[i] < 'z') || ('A' < pattern[i] && pattern[i] < 'Z')) {
				if (((text[i] - 32) == pattern[i]) || ((text[i] + 32) == pattern[i])) {
					continue;
				} else {
					ret = 1;
					break;
				}
			} else {
				ret = 1;
				break;
			}
		} else {
			ret = 1;
			break;
		}
	}

err:
	return ret;
}

int32_t v_strncmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len)
{
	int32_t len = 0;

	if (NULL == dst || NULL == src || dst_len < 0 || src_len < 0) {
		return -1;
	}

	len = MIN(dst_len, src_len);

	return strncmp(dst, src, len);
}

int32_t v_strncasecmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len)
{
	int32_t len = 0;

	if (NULL == dst || NULL == src || dst_len < 0 || src_len < 0) {
		return -1;
	}

	len = MIN(dst_len, src_len);

	return strncasecmp(dst, src, len);
}

int32_t v_atoi(int8_t *begin, int32_t data_len)
{
	int32_t size = 0;
	int8_t *data = begin;
	int8_t *end = data + data_len;
	int32_t sign = 1;

	if (data < end && '-' == *data) {
		sign = -1;
		data++;
	}

	for (; data < end; data++) {
		if (*data >= '0' && *data <= '9') {
			size = size * 10 + *data - '0';
		} else {
			break;
		}
	}

	return size * sign;

}

int32_t v_safe_atoi(int8_t *begin, int32_t data_len, int32_t *value)
{
	int32_t size = 0;
	int8_t *data = begin;
	int8_t *end = data + data_len;
	int32_t sign = 1;

	if (data < end && '-' == *data) {
		sign = -1;
		data++;
	}

	for (; data < end; data++) {
		if (*data >= '0' && *data <= '9') {
			size = size * 10 + *data - '0';
		} else {
			goto err;
		}
	}

	*value = size * sign;
	return 0;
err:
	return -1;

}

//clear first byte of each row of a string array
int32_t	v_strarray_mem_clear(int8_t *strarray, int32_t row_num, int32_t column_num)
{
	int32_t i = 0;
	if (NULL == strarray || 0 >= row_num || 0 >= column_num) {
		goto err;
	}

	for (i = 0; i < row_num; i++) {
		strarray[i * column_num] = '\0';
	}

	return 0;
err:
	return -1;
}

int32_t v_strlen(int8_t *begin, int32_t data_len)
{
	if (NULL == begin || data_len <= 0) {
		return 0;
	}

	return MIN(strlen(begin), data_len);
}

int8_t *v_memchr_escape(int8_t *s, int32_t c, int32_t n)
{
	int32_t i = 0;
	for (i = 0; i < n; i++) {
		if ((int32_t)s[i] == '\\' && i < n - 1 && (int32_t)s[i + 1] == c) {
			i++;
		} else if (s[i] == c) {
			break;
		} 
	}	
	if (i == n) {
		return NULL;
	} else {
		return s + i;
	}
}

void get_time_str(char *time_str, int time_str_size)
{
	time_t t;
	struct tm *tt = NULL;

	if (NULL == time_str || time_str_size <= 0) {
		return;
	}

	time(&t);
	tt = localtime(&t);
	if (NULL == tt) {
		return;
	}
	snprintf(time_str, time_str_size - 1, "%u-%02u-%02u %02u:%02u:%02u", tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);

	return;
}

void get_timestr(char *timestr, int32_t timestr_size, time_t *cur)
{
	struct tm *tt = NULL;

	tt = localtime(cur);
	if (NULL == tt) {
		return;
	}

	snprintf(timestr, timestr_size - 1, "%u-%02u-%02u %02u:%02u:%02u", tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
}
