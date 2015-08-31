/*
** Zombie Script Interpreter
** - Command line utility
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



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

int main(const int argc, const char **argv)
{
	const char br[] = "--------------------------------------\n";
	printf("%s** Zombie Script Interpreter v1.3.1 **\n%s\033[2J", br, br);

	// system ("/bin/stty raw");

	long len;
	char* buf;

	printf("> ");
	_getline(&buf, &len);

	puts(buf);

	free(buf);

	// system ("/bin/stty cooked");
}