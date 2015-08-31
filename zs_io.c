#include "zs_io.h"

void buf_init(z_iobuf *b, const char *file, const char *buf, const size_t size)
{
	b->file = file;
	b->pbuf = buf;
	b->pend = (char*)buf + size;
	b->size = size;
}