#ifndef ZS_LEX_H
#define ZS_LEX_H

#include <stdlib.h>

#include "zs_io.h"
#include "zs_type.h"

enum tk_sem {
	tk_generic,
	tk_numeric,
	tk_string,
	tk_identifier,
	tk_op,
	tk_keyword
};

enum tk_type {
	T_EOF = -1,

	T_NL = 128,

	T_NUMBER,		// [0-9]*(\.[0-9]*)?
	T_IDENT,		// [a-zA-Z_]+[a-zA-Z_0-9]*
	T_STRING,		// "[^"]*"

	T_TRUE,			// true
	T_FALSE,		// false

	T_EQ,			// ==
	T_NE,			// <>
	T_GTE,			// >=
	T_LTE,			// <=

	T_AND,			// and	| &&
	T_OR,			// or	| ||
	T_XOR,			// xor	| ^^
	T_NOT,			// not	| !

	T_AA,			// +=
	T_SA,			// -=
	T_MA,			// *=
	T_DA,			// /=
	T_DE,			// :=

	T_INC,			// ++
	T_DEC,			// --

	T_PRINT,		// print
	T_FUNC,			// def
	T_RET,			// return
	T_END,			// end
	T_IF,			// if
	T_ELSE,			// else

	T_PRE,			// pre
	T_IN,			// in
	T_POS,			// pos
};

static const char* tk_names[] = {
	"??",	// for unknown chars > 0xff
	"T_EOF",
	"\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07", 
	"\x08", "\x09", "\x0a", "\x0b", "\x0c", "\x0d", "\x0e", "\x0f", 
	"\x10", "\x11", "\x12", "\x13", "\x14", "\x15", "\x16", "\x17", 
	"\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e", "\x1f", 
	"\x20", "\x21", "\x22", "\x23", "\x24", "\x25", "\x26", "\x27", 
	"\x28", "\x29", "\x2a", "\x2b", "\x2c", "\x2d", "\x2e", "\x2f", 
	"\x30", "\x31", "\x32", "\x33", "\x34", "\x35", "\x36", "\x37", 
	"\x38", "\x39", "\x3a", "\x3b", "\x3c", "\x3d", "\x3e", "\x3f", 
	"\x40", "\x41", "\x42", "\x43", "\x44", "\x45", "\x46", "\x47", 
	"\x48", "\x49", "\x4a", "\x4b", "\x4c", "\x4d", "\x4e", "\x4f", 
	"\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56", "\x57", 
	"\x58", "\x59", "\x5a", "\x5b", "\x5c", "\x5d", "\x5e", "\x5f", 
	"\x60", "\x61", "\x62", "\x63", "\x64", "\x65", "\x66", "\x67", 
	"\x68", "\x69", "\x6a", "\x6b", "\x6c", "\x6d", "\x6e", "\x6f", 
	"\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77", 
	"\x78", "\x79", "\x7a", "\x7b", "\x7c", "\x7d", "\x7e", "\x7f",
	"T_NL", "T_NUMBER", "T_IDENT", "T_STRING", "T_TRUE", "T_FALSE", "T_EQ", 
	"T_NE", "T_GTE", "T_LTE", "T_AND", "T_OR", "T_XOR", 
	"T_NOT", "T_AA", "T_SA", "T_MA", "T_DA", "T_DE", 
	"T_INC", "T_DEC", "T_PRINT", "T_DEF", "T_RET", 
	"T_END", "T_IF", "T_ELSE", "T_PRE", "T_IN", "T_POS"
};
#define tk_str(t)((t) > 255 ? tk_names[0] : tk_names[(t) + 2])

/* token struct holds meta data about token
	characters found in code space */
typedef struct __z_token {
	int type;		// tk_type
	enum tk_sem semtype;	// semantic type
	char *pc;
	char *pline;
	int len;
	int line;
	int col;
} z_token;

typedef struct __z_bufstate {
	z_iobuf b;
	char *pline;
	char *plast;	// last non white char encountered
	char *pc;	// current char pos in buf
	int line;
	int col;
	int eof;
} z_bufstate;

/* lex state is the general purpose struct
	holding everything lexer */
typedef struct __z_lexstate {
	z_bufstate cur;	// current parsing state
	z_bufstate mk;		// checkpoint state
	z_token token;
} z_lexstate;

#include <ctype.h>

/* recognizer macros */
#define isDigit(c)(isdigit(c))
#define isAlpha(c)(isalpha(c))
#define isAlNum(c)(isalnum(c))
#define isNewLine(c)((c) == '\n' || (c) == '\r')
#define isWhite(c)(isspace(c))

void lex_init(z_lexstate *ls, z_iobuf b);
void lex_save(z_lexstate *ls);
void lex_restore(z_lexstate *ls);
void lex_nextchar(z_lexstate *ls);
void lex_nexttoken(z_lexstate *ls);

#endif