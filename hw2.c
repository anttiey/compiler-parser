#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "header.h"

extern int yylex();
extern char* yytext;
extern int yyleng;

int main(int argc, char *argv[]) {
    int tok;

    while ((tok=yylex()) != 0)
    {
        printf("token: %d, value:%s\n", tok, yytext);
    }
    
}