#ifndef ZS_STRING_H
#define ZS_STRING_H

#include "zs_type.h"

#include <string.h>

/* constructors */

zstr_t z_strc(const char* c_str);
zstr_t z_strn(const char* buf, const size_t n);

/* accessors/modifiers */

// void    *z_memccpy(void *, const void *, int, size_t);
// void    *z_memchr(const void *, int, size_t);
// int      z_memcmp(const void *, const void *, size_t);
// void    *z_memcpy(void *, const void *, size_t);
// void    *z_memmove(void *, const void *, size_t);
// void    *z_memset(void *, int, size_t);
// char    *z_strcat(char *, const char *);
char *z_strchr(const zstr_t s, int c);
int z_strcmp(const zstr_t s1, const zstr_t s2);
int z_strcmpc(const char *s1, const zstr_t s2);
// int      z_strcoll(const char *, const char *);
// char    *z_strcpy(char *, const char *);
// size_t   z_strcspn(const char *, const char *);
// char    *z_strdup(const char *);
// char    *z_strerror(int);
size_t   z_strlen(const zstr_t s);
// char    *z_strncat(char *, const char *, size_t);
int z_strncmp(const zstr_t s1, const zstr_t s2, size_t n);
int z_strncmpc(const char *s1, const zstr_t s2, size_t n);
zstr_t z_strncpy(zstr_t dst, const zstr_t src, size_t n);
// char    *z_strpbrk(const char *, const char *);
char *z_strrchr(const zstr_t s, int c);
// size_t   z_strspn(const char *, const char *);
// char    *z_strstr(const char *, const char *);
// char    *z_strtok(char *, const char *);
// char    *z_strtok_r(char *, const char *, char **);
// size_t   z_strxfrm(char *, const char *, size_t);

/* conversion */

char* z_cstr(const zstr_t str);

/* utility */

void z_free(zstr_t *s);
void z_freec(char* c_str);


#endif