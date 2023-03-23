%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "common.h"
    #include "syntax.tab.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

    #define RESPONSE(token_id, token_name)\
    do \
    {\
        yylval.node = NewNode(yytext, token_name, yylineno); return token_id; \
    }while(0)


    #define Error_Handler \
            fprintf(stdout, "Error type B at Line %d: Syntax Error.\n", yylineno)
    
    Node *NewNode(char *content, char *token, int lineno);
%}

%option yylineno


newline    (\n)
delim      ([ \t])
digit      ([0-9])
letter     ([_a-zA-Z])

ID         ({letter}({letter}|{digit})*)

DEC        (([1-9]{digit}*)|(0))
OCT        (0[1-7][0-7]*)
HEX        ((0x|0X)[1-9a-fA-F][0-9a-fA-F]*)
/* INT        ({DEC}|{OCT}|{HEX}) */

SEMI       (;)
COMMA      (,)
ASSIGNOP   (=)
RELOP      (>|<|>=|<=|==|!=)
PLUS       (\+)
MINUS      (-)
STAR       (\*)
DIV        (\/)
AND        (&&)
OR         (\|\|)
DOT        (\.)
NOT        (!)
TYPE       (int|float)
LP         (\()
RP         (\))
LB         (\[)
RB         (\])
LC         (\{)
RC         (\})
STRUCT     (struct)
RETURN     (return)
IF         (if)
ELSE       (else)
WHILE      (while)

digits     ({digit}+)
EXP        ([eE][+-]?{digits})
FLOAT      ({digits}{DOT}{digits}?{EXP}?|{DOT}{digits}{EXP}?)
%%

"//"  {
    char c = input();
    while (c != '\n') c = input();
}

"/*"  {
    char c = input();
    while(c != EOF){
        if (c == '*'){
            c = input();
            if(c =='/') break;
        }
        else c = input();
    }
    Error_Handler;
}

{newline}  {yycolumn = 1;}

{DEC} { RESPONSE(INT, "DEC"); }
{OCT} { RESPONSE(INT, "OCT"); }
{HEX} { RESPONSE(INT, "HEX"); }
{SEMI} { RESPONSE(SEMI, "SEMI"); }
{COMMA} { RESPONSE(COMMA, "COMMA"); }
{ASSIGNOP} { RESPONSE(ASSIGNOP, "ASSIGNOP"); }
{RELOP} { RESPONSE(RELOP, "RELOP"); }
{PLUS} { RESPONSE(PLUS, "PLUS"); }
{MINUS} { RESPONSE(MINUS, "MINUS"); }
{STAR} { RESPONSE(STAR, "STAR"); }
{DIV} { RESPONSE(DIV, "DIV"); }
{AND} { RESPONSE(AND, "AND"); }
{OR} { RESPONSE(OR, "OR"); }
{DOT} { RESPONSE(DOT, "DOT"); }
{NOT} { RESPONSE(NOT, "NOT"); }
{TYPE} { RESPONSE(TYPE, "TYPE"); }
{LP} { RESPONSE(LP, "LP"); }
{RP} { RESPONSE(RP, "RP"); }
{LB} { RESPONSE(LB, "LB"); }
{RB} { RESPONSE(RB, "RB"); }
{LC} { RESPONSE(LC, "LC"); }
{RC} { RESPONSE(RC, "RC"); }
{STRUCT} { RESPONSE(STRUCT, "STRUCT"); }
{RETURN} { RESPONSE(RETURN, "RETURN"); }
{IF} { RESPONSE(IF, "IF"); }
{ELSE} { RESPONSE(ELSE, "ELSE"); }
{WHILE} { RESPONSE(WHILE, "WHILE"); }
{FLOAT} { RESPONSE(FLOAT, "FLOAT"); }
{ID} { RESPONSE(ID, "ID"); }
%%

Node *NewNode(char *content, char *token, int lineno){
    Node *n = malloc(sizeof(Node));
    n->lineno = lineno;
    if (content == NULL) n->content = NULL;
    else {
    n->content = malloc(strlen(content) + 1);
    strcpy(n->content, content);
    }
    if(token == NULL) n->token = NULL;
    else {
        n->token = malloc(strlen(token) + 1);
        strcpy(n->token, token);
    }

    if(strcmp(token, "DEC") == 0) { n->int_val = strtol(content, NULL, 10); n->token = "INT"; }
    else if(strcmp(token, "OCT") == 0) { n->int_val = strtol(content, NULL, 8); n->token = "INT"; }
    else if(strcmp(token, "HEX") == 0) { n->int_val = strtol(content, NULL, 16); n->token = "INT"; }
    else if(strcmp(token, "FLOAT") == 0) n->float_val = atof(content);

    n->bros = NULL;
    n->childs = NULL;

    return n;
}