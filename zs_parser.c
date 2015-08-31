#include <stdlib.h>

#include "zs_type.h"
#include "zs_string.h"
#include "zs_io.h"
#include "zs_err.h"
#include "zs_lex.h"
#include "zs_parser.h"

#define F()//(printf("in: %s\n",__FUNCTION__))

void printAST(const ast_t* t, const int lev) {
	for(int i = 0; i < lev; ++i)
		printf("  ");

	if(t->type != AST_GENERIC) {
		if(t->type == AST_OP)
			printf("%s ", 
			(t->op == OP_PREFIX ? "prefix" : 
				(t->op == OP_INFIX ? "infix" : "postfix")));
		printf("%s:%li:%li: '", t->tag, t->line, t->col);
		for(int i = 0; i < t->len; ++i)
			printf("%c", *(t->pbuf + i));
		puts("'");
	}
	else
		printf("<%s>\n", t->tag);

	if(t->nleaf > 0) {
		for(int i = 0; i < t->nleaf; ++i)
			printAST(&t->leaf[i], lev + 1);
	}
}

/*------------------------------------------------------------------------*/

ast_t* pushTree(ast_t *t)
{
	if(t->nleaf > 0) {
		++t->nleaf;
		t->leaf = (ast_t*)realloc(t->leaf, sizeof(ast_t) * t->nleaf);
	}
	else {
		t->leaf = (ast_t*)malloc(sizeof(ast_t));
		t->nleaf = 1;
	}
	ast_t* s = &t->leaf[t->nleaf - 1];
	s->nleaf = 0;
	return s;
}

ast_t* pushTreeNoLeaf(ast_t *t, const char* tag)
{
	ast_t* new = pushTree(t);
	new->tag = tag;
	new->type = AST_GENERIC;
	return new;
}

ast_t* pushTreeLeaf(ast_t *t, z_token tk, const char* tag, enum asttype type)
{
	ast_t* new = pushTree(t);
	new->tag = tag;
	new->type = type;
	new->tk = tk.type;
	new->line = tk.line;
	new->col = tk.col;
	new->pbuf = tk.pc;
	new->len = tk.len;
	return new;
}

ast_t* pushTreeOp(ast_t *t, z_token tk, enum optype type) {
	ast_t *op = pushTreeLeaf(t, tk, "op", AST_OP);
	op->op = type;
	return op;
}

ast_t* pushTreeSymbol(ast_t *t, z_token tk) {
	return pushTreeLeaf(t, tk, "symbol", AST_SYMBOL);
}

ast_t* pushTreeTerminal(ast_t *t, z_token tk) {
	return pushTreeLeaf(t, tk, "terminal", AST_TERMINAL);
}

/*------------------------------------------------------------------------*/

#define next(ls)lex_nexttoken(ls)

static void error_unexpected(z_lexstate *ls) {
	syntaxError(ls, "unexpected '%s'", tk_str(ls->token.type));
}

static void error_expected(z_lexstate *ls, int token) {
	syntaxError(ls, "expected '%s' found '%s'", tk_str(token), tk_str(ls->token.type));
}

static int test(z_lexstate *ls, int token) {
	return ls->token.type == token;
}

static int testnext(z_lexstate *ls, int token) {
	int t = test(ls, token);
	next(ls);
	return t;
}

static void expect(z_lexstate *ls, int token) {
	if(!test(ls, token))
		error_expected(ls, token);
}

static void expectnext(z_lexstate *ls, int token) {
	if(!test(ls, token))
		error_expected(ls, token);
	next(ls);
}

static void matchnext(z_lexstate *ls, int tk, int other, int other_line) {
	if(!test(ls, tk)){
		if(other_line != ls->cur.line) {
			syntaxError(ls, "expecting '%s' to match opening '%s' on line %d",
				tk_str(tk), tk_str(other), other_line);
		}
		else
			error_expected(ls, tk);
	}
	next(ls);
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

/*
	op			: '!'|'@'|'#'|'$'|'%'|'^'|'&'|'*'|'-'|'+'|'_'|'~'|'.'|':'|','
	preop		: op
	posop		: op
	args		: expr [',' args]
	symbol		: IDENT ['.' symbol]
	atom		: NUMBER  
					| [INC|DEC] symbol [INC|DEC] 
					| symbol ['(' args ')']
	factor		: [preop] ( atom | '(' expr ')' )
	term		: factor ['*'|'/'|'^' factor]+
	mexpr 		: ['+'|'-'] term ['+'|'-' term]+ [posop]
	sexpr		: STRING ['+' STRING]+

fix syntaxes:	
	bterm		: TRUE | FALSE | mexpr
	bexpr		: [NOT] bterm [AND|OR|XOR bterm]+
end fix;

	expr		: sexpr | mexpr
	list		: %nothing% | IDENT [',' IDENT]+
	opdef		: PRE|IN|POS '(' list ')' '{' block '}'
	assign		: IDENT '='|T_AA|T_SA|T_MA|T_DA|T_DE expr
	retstat		: RETURN | RETURN expr
	stat		: PRINT expr | retstat | assign [;|NL]
	instr		: '{' block '}' | stat
	block		: instr+
	top 		: instr+ EOF
*/

static void expr(z_lexstate *ls, ast_t *t);
static void block(z_lexstate *ls, ast_t *t);

#define _tk ls->token.type

#define isOp(t)(strchr("!@#$%^&*-+_~.,", (int)(t)) != NULL || (t) == T_INC || (t) == T_DEC)
static void op(z_lexstate *ls, ast_t *t)
{F();
	switch(_tk) {
		case '!':
		case '@':
		case '#':
		case '$':
		case '%':
		case '^':
		case '&':
		case '*':
		case '-':
		case '+':
		case '_':
		case '~':
		case '.':
		case ':':
		case ',':
		case T_INC:
		case T_DEC:
			next(ls);
			break;
		default:
			syntaxError(ls, "expecting operator");
	}
}

static z_token preop(z_lexstate *ls, ast_t *t)
{F();
	z_token _op = ls->token;
	op(ls, t);
	return _op;
}

static z_token inop(z_lexstate *ls, ast_t *t)
{F();
	z_token _op = ls->token;
	op(ls, t);
	return _op;
}

static z_token posop(z_lexstate *ls, ast_t *t)
{F();
	z_token _op = ls->token;
	op(ls, t);
	return _op;
}

static void args(z_lexstate *ls, ast_t *t)
{F();
	//t = pushTreeNoLeaf(t, "args");

	if(_tk == ')')
		return;

	expr(ls, t);
	if(_tk == ',') {
		next(ls);
		args(ls, t);
	}
}

static int symbolpart(z_lexstate *ls, ast_t *t)
{F(); int len;
	//t = pushTreeNoLeaf(t, "symbol");

	len = ls->token.len;

	if(_tk == T_IDENT) {
		next(ls);

		if(_tk == '.') {
			len++;
			next(ls);
			len += symbolpart(ls, t);
		}
	}
	else
		error_expected(ls, T_IDENT);

	return len;
}

static void symbol(z_lexstate *ls, ast_t *t)
{F(); z_token tk;
	tk = ls->token;
	tk.len = symbolpart(ls, t);
	pushTreeSymbol(t, tk);
}


#define isIncOp(t)((t).type == T_INC || (t).type == T_DEC)
static void atom(z_lexstate *ls, ast_t *t)
{F(); z_token op;
	t = pushTreeNoLeaf(t, "atom");

	if(isIncOp(op = ls->token)) {
		next(ls);
		if(_tk == T_IDENT) {
			symbol(ls, t);
			next(ls);
		}
		else
			error_expected(ls, T_IDENT);

		pushTreeOp(t, op, OP_PREFIX);

		if(isIncOp(op = ls->token)) {
			next(ls);
			pushTreeOp(t, op, OP_POSTFIX);
		}
	}
	else if(_tk == T_IDENT) {
		symbol(ls, t);
		if(_tk == '(') {
			next(ls);
				args(ls, t);
			expectnext(ls, ')');
		}
	}
	else if(_tk == T_NUMBER) {
		pushTreeTerminal(t, ls->token);
		next(ls);
	}
	else
		error_unexpected(ls);
}

static void factor(z_lexstate *ls, ast_t *t)
{F(); z_token op; int pre = 0;

	if(isOp(_tk)) {
		op = preop(ls, t);
		pre = 1;
	}

	if(_tk == '(') {
		next(ls);
			expr(ls, t);
		expectnext(ls, ')');
	}
	else
		atom(ls, t);

	if(pre) pushTreeOp(t, op, OP_PREFIX);
}

#define isMulOp(t)((t).type == '*' || (t).type == '/' || (t).type == '^')
static void term(z_lexstate *ls, ast_t *t)
{F();
	factor(ls, t);
	z_token op;
	while(isMulOp(op = ls->token)) {
		next(ls);
		factor(ls, t);
		pushTreeOp(t, op, OP_INFIX);
	}
}

#define isAddOp(t)((t).type == '+' || (t).type == '-')
static void mexpr(z_lexstate *ls, ast_t *t)
{F(); z_token op; int pre = 0;

	if(isAddOp(op = ls->token)) {
		next(ls);
		pre = 1;
	}

	term(ls, t);
	
	if(pre) pushTreeOp(t, op, OP_PREFIX);
	
	while(isAddOp(op = ls->token)) {
		next(ls);
		term(ls, t);
		pushTreeOp(t, op, OP_INFIX);
	}
}

static void sexpr(z_lexstate *ls, ast_t *t)
{F(); z_token op; int pre = 0;

	if(_tk == T_STRING) {
		pushTreeTerminal(t, ls->token);
		next(ls);

		if(isOp((op = ls->token).type)) {
			next(ls);
			sexpr(ls, t);
			pushTreeOp(t, op, OP_INFIX);
		}
	}
	else
		error_expected(ls, T_STRING);
}

static void expr(z_lexstate *ls, ast_t *t)
{F(); z_token op;
	t = pushTreeNoLeaf(t, "expr");

	if(_tk == T_STRING) {
		sexpr(ls, t);
	}
	else {
		mexpr(ls, t);

		if(isOp(_tk)) {
			op = posop(ls, t);
			pushTreeOp(t, op, OP_POSTFIX);
		}
	}
}

static void list(z_lexstate* ls, ast_t* t)
{F();
	//t = pushTreeNoLeaf(t, "args");

	if(_tk == ')')
		return;

	if(_tk == T_IDENT) {
		pushTreeSymbol(t, ls->token);
		next(ls);
	}
	else
		error_unexpected(ls);

	while(_tk == ',') {
		next(ls);
		if(_tk == T_IDENT) {
			pushTreeSymbol(t, ls->token);
			next(ls);
		}
		else
			error_unexpected(ls);
	}
}

#define isPosKw(t)((t).type == T_PRE || (t).type == T_IN || (t).type == T_POS)
static void opdef(z_lexstate* ls, ast_t* t)
{F(); z_token op; 
	t = pushTreeNoLeaf(t, "opdef");

	if(isPosKw(op = ls->token)) {
		pushTreeTerminal(t, op);
		next(ls);
		expectnext(ls, '(');
			list(ls, t);
		expectnext(ls, ')');
		expectnext(ls, '{');
			t = pushTreeNoLeaf(t, "body");
			block(ls, t);
		expectnext(ls, '}');
	}
}

#define isAssOp(t)((t) == '=' || (t) == T_AA || (t) == T_SA || (t) == T_MA || (t) == T_DA || (t) == T_DE)
#define isAnyOp(t)(isOp(t) || isAssOp(t))
static void assign(z_lexstate* ls, ast_t* t)
{F(); z_token op; 
	t = pushTreeNoLeaf(t, "assign");

	if(_tk == T_IDENT) {
		pushTreeTerminal(t, ls->token);
		next(ls);
	}
	else if(isAnyOp(_tk)) {
		pushTreeTerminal(t, ls->token);
		next(ls);
	}
	else
		error_unexpected(ls);


	if((op = ls->token).type == T_DE) {
		next(ls);
		opdef(ls, t);
		pushTreeOp(t, op, OP_INFIX);
	}
	else if(isAssOp((op = ls->token).type)) {
		next(ls);
		expr(ls, t);
		pushTreeOp(t, op, OP_INFIX);
	}
	else
		syntaxError(ls, "expected assignment operator for assignment statement");

}

static void retstat(z_lexstate* ls, ast_t* t)
{F();
	t = pushTreeNoLeaf(t, "retstat");

	expectnext(ls, T_RET);

	if(_tk != T_NL || _tk != ';')
		expr(ls, t);
}

static void stat(z_lexstate* ls, ast_t* t)
{F(); z_token op;

	if(_tk == T_NL || _tk == ';') {
		next(ls);
		return;
	}

	if((op = ls->token).type == T_PRINT) {
		next(ls);
		expr(ls, t);
		pushTreeOp(t, op, OP_PREFIX);
	}
	else if(_tk == T_RET) {
		retstat(ls, t);
	}
	else {
		assign(ls, t);
	}

	if(_tk == T_NL || _tk == ';')
		next(ls);
	// else
	// 	syntaxError(ls, "expecting end of statement");
}

static void instr(z_lexstate* ls, ast_t* t)
{F();
	if(_tk == '{') {
		next(ls);
			block(ls, t);
		expectnext(ls, '}');
	}
	else
		stat(ls, t);
}

static void block(z_lexstate *ls, ast_t *t)
{F();
	while(_tk != '}' && _tk != T_EOF) {
		stat(ls, t);
	}
}

static void top(z_lexstate *ls, ast_t *t)
{F();

}

ast_t parse(const char* file, const char *src, const size_t len, enum pr t)
{
	z_iobuf in;
	buf_init(&in, file, src, len);

	z_lexstate ls;
	lex_init(&ls, in);

	lex_nexttoken(&ls);

	ast_t tree;
	tree.tag = "root";
	tree.type = AST_GENERIC;
	tree.nleaf = 0;

	switch(t) {
		case ptop: top(&ls, &tree); break;
		case pblock: block(&ls, &tree); break;
		case pstat: stat(&ls, &tree); break;
		case pexpr: expr(&ls, &tree); break;
	}

	printAST(&tree, 0);

	return tree;
}