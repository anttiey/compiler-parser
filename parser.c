#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "header.h"

extern int yylex();
extern FILE* yyin;
extern char* yytext;
extern void yyrestart(FILE* input);

int main(int argc, char *argv[]) {

    char line[MAX_LINE];
    char *str;

    while ((str = fgets(line, 1024, stdin)) != NULL)
    {
        yyin = stdin;
        yylex();
        yyrestart(yyin);

    }
    
}