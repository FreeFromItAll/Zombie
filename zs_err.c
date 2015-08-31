#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "zs_type.h"
#include "zs_lex.h"
#include "zs_err.h"

void z_error(int errcode, int errtype, const char* format, ...)
{
	static int errCount = 0; ++errCount;

	char errmsg[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(errmsg, sizeof errmsg, format, args);
	va_end(args);

	switch(errtype) {
		case err_warn: fprintf(stderr, "\033[1;34mWarning: \033[0m"); break;
		case err_error: fprintf(stderr, "\033[1;31mError: \033[0m"); break;
		case err_syntax: fprintf(stderr, "\033[1;31mSyntax error: \033[0m"); break;
		case err_fatal: fprintf(stderr, "\033[1;31mFatal error: \033[0m"); break;
	}

	fprintf(stderr, "%s\n", errmsg);

	if(errtype == err_fatal)
		exit(1);

	if(errCount >= MAX_ERRORS)
		z_error(0, err_fatal, "too many errors encountered\n");
}

void syntaxError(z_lexstate *ls, const char* format, ...)
{
	static int errCount = 0; ++errCount;

	char errmsg[1024];

	fprintf(stderr, "%s:%d:%d: ", ls->mk.b.file, ls->mk.line, ls->mk.col);

	va_list args;
	va_start(args, format);
	vsnprintf(errmsg, sizeof errmsg, format, args);
	va_end(args);

	char* s = ls->mk.pline;
	char* p = strchr(s, '\n');
	if(p == NULL) p = ((char*)ls->cur.b.pbuf + ls->cur.b.size);
	int len = p - s;
	if(len < 1) {
		s = ls->mk.plast;
		p = s + 1;
		len = p - s;
	}
	char buf[len + 1];
	buf[len] = '\0';
	strncpy(buf, s, len);
	strcpy(&errmsg[strlen(errmsg)], "\n\n\033[0;37m");
	strcpy(&errmsg[strlen(errmsg)], buf);
	strcpy(&errmsg[strlen(errmsg)], "\033[0m\n");

	for(char* c = ls->token.pline; c < ls->token.pc; ++c)
		strcpy(&errmsg[strlen(errmsg)], (*c == '\t' ? "\t" : " "));
	strcpy(&errmsg[strlen(errmsg)], "^");

	z_error(0, err_syntax, errmsg);

	if(ls->token.type == T_EOF)
		exit(1);

	if(errCount >= 1)
		exit(1);
}

// void runtimeError(zebraState *z, const char* format, ...)
// {
// 	char errmsg[1024];

// 	va_list args;
// 	va_start(args, format);
// 	vsnprintf(errmsg, sizeof errmsg, format, args);
// 	va_end(args);
	
// 	z_error(0, err_error, errmsg);
// }