#include "zs_err.h"
#include "zs_type.h"
#include "zs_string.h"
#include "zs_lex.h"

#include <string.h>

/* Keywords */
const int kw_count = 11;
const struct {
	const char* str;
	int len;
	int tk;
} kw[kw_count] = {
	{	"true", 	4,	T_TRUE		},
	{	"false", 	5,	T_FALSE		},
	{	"def",  	3,	T_FUNC		},
	{	"return", 	6,	T_RET		},
	{	"end", 		3, 	T_END		},
	{	"if", 		2, 	T_IF		},
	{	"else", 	4, 	T_ELSE		},
	{	"print", 	5, 	T_PRINT		},
	{	"pre", 		3, 	T_PRE		},
	{	"in", 		2, 	T_IN		},
	{	"pos", 		3, 	T_POS		},
};

static void lex_matchKeyword(z_lexstate *ls, z_token *tk)
{
	zstr_t ident = z_strn((char*)tk->pc, tk->len);
	int i;
	for(i = 0; i < kw_count; ++i) {
		if(ident.len == kw[i].len) {
			if(strncmp((const char*)ident.s, kw[i].str, ident.len) == 0) {
				tk->type = kw[i].tk;
				tk->semtype = tk_keyword;
				return;
			}
		}
	}
}

//---------------------------------------------------------
// Lexer methods
//---------------------------------------------------------

void lex_init(z_lexstate *ls, z_iobuf b)
{
	ls->cur.b = b;
	ls->cur.pc = (char*)b.pbuf;
	ls->cur.pline = ls->cur.pc;
	ls->cur.plast = ls->cur.pc;
	ls->cur.line = 1;
	ls->cur.col = 1;
	ls->cur.eof = false;
	ls->mk = ls->cur;

	if(ls->cur.pc >= ls->cur.b.pend)
		ls->cur.eof = true;
}

void lex_save(z_lexstate *ls)
{
	ls->mk = ls->cur;
}

void lex_restore(z_lexstate *ls)
{
	ls->cur = ls->mk;
}

void lex_nextchar(z_lexstate *ls)
{
	if(ls->cur.pc + 1 < ls->cur.b.pend) {
		/* keep track of last non-white char
			for debugging and error messages */
		if(!isWhite(*ls->cur.pc) && *ls->cur.pc != '\0')
			ls->cur.plast = ls->cur.pc;

		++ls->cur.pc;
		++ls->cur.col;
	}
	else {
		*(++ls->cur.pc) = '\0';	// insert a null char at the last placeholder
		ls->cur.eof = true;
	}
}

static z_token lex_scan(z_lexstate *ls);
void lex_nexttoken(z_lexstate *ls)
{
	ls->token = lex_scan(ls);
}

static void lex_incline(z_lexstate *ls)
{
	ls->cur.pline = ls->cur.pc;
	++ls->cur.line;
	ls->cur.col = 1;
}

//---------------------------------------------------------
// Utility
//---------------------------------------------------------

static z_token lex_newToken(z_lexstate *ls, int type, enum tk_sem semtype)
{
	z_token t;
	t.type = type;
	t.semtype = semtype;
	t.pc = ls->mk.pc;
	t.pline = ls->mk.pline;
	t.len = ls->cur.pc - ls->mk.pc;
	t.line = ls->mk.line;
	t.col = ls->mk.col;
	return t;
}

//---------------------------------------------------------
// Lexical scanners
//---------------------------------------------------------
#define nc ((char)*ls->cur.pc)
#define nnc ((ls->cur.pc + 1 < ls->cur.b.pend)?(char)*(ls->cur.pc + 1):'\0')

static void lex_scanNewLine(z_lexstate *ls)
{
	if(isNewLine(nc)) {
		char nlc = nc;
		lex_nextchar(ls);
		if(isNewLine(nc) && nc != nlc)	// treat LF + CR newlines
			lex_nextchar(ls);
		lex_incline(ls);
	}
}

static z_token lex_scan(z_lexstate *ls)
{
	if(ls->cur.eof) {
		lex_save(ls);
		return lex_newToken(ls, T_EOF, 0);
	}

	if(isWhite(nc)) {
		while(isWhite(nc)) {
			if(isNewLine(nc)) {
				lex_scanNewLine(ls);
				return lex_newToken(ls, T_NL, tk_generic);
			}
			else
				lex_nextchar(ls);
		}
		return lex_scan(ls);
	}

	/* line comment */
	if(nc == '/') {
		if(nnc == '/') {
			while(!isNewLine(nc))
				lex_nextchar(ls);
			return lex_scan(ls);
		}
	}

	/* multi line comment */
	if(nc == '/') {
		if(nnc == '*') {
			lex_nextchar(ls);
			lex_nextchar(ls);
			for(;;) {
				if(ls->cur.eof) {
					syntaxError(ls, "unterminated comment reached end of file");
					break;
				}
				else if(nc == '*') {
					lex_nextchar(ls);
					if(nc == '/') {
						lex_nextchar(ls);
						return lex_scan(ls);
					}
				}
				else if(isNewLine(nc)) {
					lex_scanNewLine(ls);
				}
				else
					lex_nextchar(ls);
			}
		}
	}

	lex_save(ls);

	/* numerical constants */
	if(isDigit(nc)) {
parse_number:
		while(isDigit(nc))
			lex_nextchar(ls);
		if(nc == '.') {
			lex_nextchar(ls);
			while(isDigit(nc))
				lex_nextchar(ls);
			if(nc == '.')
				syntaxError(ls, "invalid numerical constant");
		}
		return lex_newToken(ls, T_NUMBER, tk_numeric);
	}

	/* identifiers */
	else if(isAlpha(nc)) {
parse_ident:
		while(isAlNum(nc) || nc == '_')
			lex_nextchar(ls);
		
		/* check if it matches a keyword token */
		z_token tk = lex_newToken(ls, T_IDENT, tk_identifier);
		lex_matchKeyword(ls, &tk);

		return tk;
	}

	/* string literals */
	else if(nc == '"' || nc == '\''){
//parse_string:
		char q = nc;
		lex_nextchar(ls);
		while(nc != q) {
			if(ls->cur.eof) {
				syntaxError(ls, "unterminated string literal reached end of file");
				break;
			}
			/* skip escaped chars */
			if(nc == '\\') {
				lex_nextchar(ls);
				continue;
			}
			if(isNewLine(nc)) {
				lex_scanNewLine(ls);
			}
			lex_nextchar(ls);
		}
		lex_nextchar(ls);	// skip the closing cc
		return lex_newToken(ls, T_STRING, tk_string);
	}

	/* other multi char tokens */
	switch(nc)
	{
		case '.':	// may be numeric?
			lex_nextchar(ls);
			if(isDigit(nc))
				goto parse_number;
			return lex_newToken(ls, '.', 0);
		case '_':	// may be ident?
			lex_nextchar(ls);
			if(isAlNum(nc))
				goto parse_ident;
			return lex_newToken(ls, '_', 0);
		case '+':
			lex_nextchar(ls);
			if(nc == '+') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_INC, tk_op);
			}
			else if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_AA, tk_op);
			}
			return lex_newToken(ls, '+', tk_op);
		case '-':
			lex_nextchar(ls);
			if(nc == '-') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_DEC, tk_op);
			}
			else if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_SA, tk_op);
			}
			return lex_newToken(ls, '-', tk_op);
		case '*':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_MA, tk_op);
			}
			return lex_newToken(ls, '*', tk_op);
		case '/':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_DA, tk_op);
			}
			return lex_newToken(ls, '/', tk_op);
		case '>':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_GTE, tk_op);
			}
			return lex_newToken(ls, '>', tk_op);
		case '<':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_LTE, tk_op);
			}
			else if(nc == '>') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_NE, tk_op);
			}
			return lex_newToken(ls, '<', tk_op);
		case '=':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_EQ, tk_op);
			}
			return lex_newToken(ls, '=', tk_op);
		case '&':
			lex_nextchar(ls);
			if(nc == '&') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_AND, tk_op);
			}
			return lex_newToken(ls, '&', tk_op);
		case '|':
			lex_nextchar(ls);
			if(nc == '|') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_OR, tk_op);
			}
			return lex_newToken(ls, '|', tk_op);
		case '^':
			lex_nextchar(ls);
			if(nc == '^') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_XOR, tk_op);
			}
			return lex_newToken(ls, '^', tk_op);
		case '!':
			lex_nextchar(ls);
			return lex_newToken(ls, T_NOT, tk_op);
		case ':':
			lex_nextchar(ls);
			if(nc == '=') {
				lex_nextchar(ls);
				return lex_newToken(ls, T_DE, tk_op);
			}
			return lex_newToken(ls, ':', 0);
	}

	char c = nc;
	lex_nextchar(ls);

	return lex_newToken(ls, c, 0);
}
