%{
    
    #include "intercode.h"
    #include "syntax.tab.h"
    int lexerror = 0;
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    
%}

%option yylineno

newline         (\n|\r)
delim           ([ \t])
ws              ({delim}+)
digit           ([0-9])
digits          ({digit}+)
Label           (label)
INT             (([1-9]{digit}*)|(0))
PLUS            (\+)
MINUS           (-)
DIV             (\/)
LABELOP         ({Label}{digits})
letter          ([_a-zA-Z])
CONSTANT        ((\#){digits})
AND             (\&)
STAR            (\*)
ASSIGN          (:=)
RELOP           (>|<|>=|<=|==|!=)
TOPERAND        ([t]{digits})
VOPERAND        ([v]{digits})
ID              ({letter}({letter}|{digit})*)

%% 

{newline}   { return END; }
{ws}        {}
":"         { return COLON; }
"LABEL"     { return LABEL; }
"FUNCTION"  { printf("It's function\n"); return FUNC; }
"GOTO"      { return GOTO; }
"IF"        { return IF; }
"RETURN"    { return RETURN; }
"DEC"       { return DEC; }
"ARG"       { return ARG; }
"CALL"      { return CALL; }
"PARAM"     { return PARAM; }
"READ"      { return READ; }
"WRITE"     { return WRITE; }
{INT}       { yylval.node = NewOperand(CONSTANT_OP, atoi(yytext)); return INT; }
{PLUS}      { return PLUS; }
{MINUS}     { return MINUS; }
{DIV}       { return DIV; }
{CONSTANT}  { yylval.node = NewOperand(CONSTANT_OP, atoi(yytext + 1)); return CONSTANT;}
{AND}       { return AND; }
{STAR}      { return STAR; }
{ASSIGN}    { return ASSIGN; }
{RELOP}     { yylval.node = NewOperand(RELOP_OP, yytext); return RELOP; }  
{LABELOP}   { yylval.node = NewOperand(LABEL_OP, yytext); return LABELOP; }
{TOPERAND}  { yylval.node = NewOperand(VARIABLE_OP, yytext); return TOPERAND; }
{VOPERAND}  { yylval.node = NewOperand(VARIABLE_OP, yytext); return VOPERAND; }
{ID}        { yylval.node = NewOperand(FUNCTION_OP, yytext); return ID; }


%%