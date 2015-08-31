#ifndef ZS_ERR_H
#define ZS_ERR_H

#include "zs_lex.h"

#define MAX_ERRORS		8

enum errtype {
	err_warn,
	err_error,
	err_syntax,
	err_fatal
};

void z_error(int errcode, int errtype, const char* format, ...);
void syntaxError(z_lexstate *ls, const char* format, ...);

#endif