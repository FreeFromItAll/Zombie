#ifndef ZS_TYPE_H
#define ZS_TYPE_H

#include <stdint.h>

typedef const char* c_str;		// '\0' terminated char array
typedef char char_t;

typedef uint8_t bool;
#define false 0
#define true ~false

typedef long zint_t;
typedef double zfloat_t;

struct __zstr {
	char* s;
	size_t len;
};

typedef struct __zstr zstr_t;

#endif