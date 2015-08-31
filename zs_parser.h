#ifndef ZS_PARSER_H
#define ZS_PARSER_H

#include "zs_type.h"
#include "zs_lex.h"

/* abstract syntax tree node */

enum asttype {
	AST_GENERIC,
	AST_SYMBOL,
	AST_TERMINAL,
	AST_OP
};

enum optype {
	OP_PREFIX,
	OP_INFIX,
	OP_POSTFIX
};

typedef struct __ast_t {
	const char* tag;
	enum asttype type;
	enum optype op;
	int tk;
	char *pbuf;
	long len;
	long line;
	long col;
	int nleaf;
	struct __ast_t *leaf;
} ast_t;

enum pr { ptop, pblock, pstat, pexpr };

ast_t parse(const char* file, const char *src, const size_t len, enum pr);

#endif