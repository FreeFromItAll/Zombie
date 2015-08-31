#include <string.h>
#include <stdlib.h>

#include "zs_string.h"

/* constructors */

zstr_t z_strc(const char* c_str) {
	zstr_t zstr;
	zstr.len = strlen(c_str);
	zstr.s = (char*)c_str;
	return zstr;
}

zstr_t z_strn(const char* buf, const size_t n) {
	zstr_t zstr;
	zstr.len = n;
	zstr.s = (char*)buf;
	return zstr;
}

/* accessing/manipulation */

// void    *z_memccpy(void *, const void *, int, size_t);
// void    *z_memchr(const void *, int, size_t);
// int      z_memcmp(const void *, const void *, size_t);
// void    *z_memcpy(void *, const void *, size_t);
// void    *z_memmove(void *, const void *, size_t);
// void    *z_memset(void *, int, size_t);
// char    *z_strcat(char *, const char *);
char *z_strchr(const zstr_t s, int c) {
	char *p = s.s;
	char *end = s.s + s.len;
	while(p++ <= end){
		if(c == (int)*p)
			return p;
	}
	return NULL;
}

int z_strcmp(const zstr_t s1, const zstr_t s2)
{
	return strncmp(s1.s, s2.s, (s1.len <= s2.len ? s1.len : s2.len));
}

int z_strcmpc(const char *s1, const zstr_t s2)
{
	return strncmp(s1, s2.s, s2.len);
}

// int      z_strcoll(const char *, const char *);
// char    *z_strcpy(char *, const char *);
// size_t   z_strcspn(const char *, const char *);
// char    *z_strdup(const char *);
// char    *z_strerror(int);
size_t   z_strlen(const zstr_t s) {
	return s.len;
}
// char    *z_strncat(char *, const char *, size_t);

int z_strncmp(const zstr_t s1, const zstr_t s2, size_t n)
{
	if(n <= s1.len && n <= s2.len)
		return strncmp(s1.s, s2.s, n);
	return z_strcmp(s1, s2);
}

int z_strncmpc(const char *s1, const zstr_t s2, size_t n)
{
	return strncmp(s1, s2.s, (n <= s2.len ? n : s2.len));
}

zstr_t z_strncpy(zstr_t dst, const zstr_t src, size_t n)
{
	size_t nn = n > src.len ? src.len : n;
	zstr_t s;
	s.s = strncpy(dst.s, src.s, (nn > dst.len ? dst.len : n));
	s.len = n;
	return s;
}

// char    *z_strpbrk(const char *, const char *);

char *z_strrchr(const zstr_t s, int c)
{
	char *p = s.s + s.len;
	while(p--) {
		if(c == (int)*p)
			return p;
	}
	return NULL;
}
// size_t   z_strspn(const char *, const char *);
// char    *z_strstr(const char *, const char *);
// char    *z_strtok(char *, const char *);
// char    *z_strtok_r(char *, const char *, char **);
// size_t   z_strxfrm(char *, const char *, size_t);

/* conversion */

char* z_cstr(const zstr_t str) {
	char* c_str = (char*)malloc(sizeof(char) * (str.len + 1));
	if(c_str == NULL)
		return NULL;
	memcpy(c_str, str.s, str.len);
	c_str[str.len] = '\0';
	return c_str;
}

/* utility */

void z_free(zstr_t *s) {
	free(s->s);
	s->s = NULL;
	s->len = 0;
}

void z_freec(char* c_str) {
	free(c_str);
	c_str = NULL;
}