%{
#include <stdio.h>
#include <stdlib.h>
%}

digit    ([0-9])
ADD      (\+)
SUB      (-)
MUL      (\*)
DIV      (\/)

%%
{digit}* {
    yylval = atoi(yytext);
    return INT;
}

{ADD}   {return ADD;}
{SUB}   {return SUB;}
{MUL}   {return MUL;}
{DIV}   {return DIV;}
#		return 0;
.		ECHO;

%%



int yywrap(){
    return 1;
}