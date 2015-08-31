/*
** Zombie Script Interpreter
** - Command line utility
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zs_parser.h"

#define CHUNK_SIZE 8
#define MAX_BUF_SIZE 1024
void _getline(char **buf, long *len)
{
char *s, *c, *p;
long mlen = CHUNK_SIZE;

	c = (s = malloc(mlen));

	*c = fgetc(stdin);

	while(mlen < MAX_BUF_SIZE) {
		p = s + mlen - 1;
		while(c < p) {
			*(++c) = getchar();
			if( *c == '\n' || 
				*c == '\0' || 
				*c == EOF)
			goto done;
		}
		s = realloc(s, mlen += CHUNK_SIZE);
		c = s + mlen - CHUNK_SIZE - 1;
	}

done:
	*c = '\0';
	*buf = s;
	*len = (long)(c - s);
}

static void eval(ast_t *t, env_t *e) {
	switch(t->type) {
		case AST_GENERIC:
			if(strstr("expr", t->tag)) {

			}
			break;

		case AST_SYMBOL:

			break;

		case AST_TERMINAL:
			switch(t->tk) {
				case T_NUMBER:
				case T_STRING:
					break;
			}
			break;
		case AST_OP:
			switch(t->tk) {
				case '+':
				case '-':
				case '*':
				case '/':
				case '^':
					break;
			}
			break;
	}
}

int main(const int argc, const char **argv)
{
	const char br[] = "--------------------------------------\n";
	printf("%s** Zombie Script Interpreter v1.3.1 **\n%s\033[2J", br, br);

	// system ("/bin/stty raw");

	long len;
	char* buf;

	env_t env;
	ast_t ast;

	while(1) {
		printf("> ");
		_getline(&buf, &len);

		if(len == 1 && buf[0] == 'q')
			break;

		ast = parse("<stdin>", buf, len, pexpr);

		eval(ast.leaf[0], env);		// ast.leaf[0] is the top <expr>
	}

	free(buf);

	// system ("/bin/stty cooked");
}