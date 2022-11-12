%{
    #include "header.h"
%}

blank       [ \t\n]+
integer     [-+]?[1-9][0-9]*
real        [-+]?((0|[1-9][0-9]*)\.[0-9]+)
string      \"([^"\\\n]|\\(.|\n))*\"
variable    [a-zA-Z][a-zA-Z0-9]*

%%

{integer}    { return (INTEGER); }
{real}       { return (REAL); }
{variable}   { return (VARIABLE); }
{blank}      ;

%%