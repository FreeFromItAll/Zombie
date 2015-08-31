#ifndef ZS_IO_H
#define ZS_IO_H

#include <stdlib.h>

typedef struct __z_iobuf {
	const char* file;
	const char *pbuf;
	char *pend;
	size_t size;
} z_iobuf;

void buf_init(z_iobuf *b, const char *file, const char *buf, const size_t size);

#endif