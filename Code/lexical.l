%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "common.h"
    #include "syntax.tab.h"
    int lexerror = 0;
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

    #define RESPONSE(token_id, token_name, type) \
    do \
    {\
        yylval.node = NewNode(yytext, token_name, yylineno, type); return token_id; \
    }while(0)


    #define Error_Handler \
            fprintf(stdout, "Error type B at Line %d: Syntax Error.\n", yylineno)
    
    Node *NewNode(char *content, char *token, int lineno, NodeType type);
%}

%option yylineno


newline    (\n|\r)
delim      ([ \t])
ws         ({delim}+)
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
    lexerror = 1;
}

{newline}  {yycolumn = 1;}
{ws}  {}
{DEC} { RESPONSE(INT, "DEC", TOKEN_INT); }
{OCT} { RESPONSE(INT, "OCT", TOKEN_INT); }
{HEX} { RESPONSE(INT, "HEX", TOKEN_INT); }
{SEMI} { RESPONSE(SEMI, "SEMI", TOKEN_OTHER); }
{COMMA} { RESPONSE(COMMA, "COMMA", TOKEN_OTHER); }
{ASSIGNOP} { RESPONSE(ASSIGNOP, "ASSIGNOP", TOKEN_OTHER); }
{RELOP} { RESPONSE(RELOP, "RELOP", TOKEN_OTHER); }
{PLUS} { RESPONSE(PLUS, "PLUS", TOKEN_OTHER); }
{MINUS} { RESPONSE(MINUS, "MINUS", TOKEN_OTHER); }
{STAR} { RESPONSE(STAR, "STAR", TOKEN_OTHER); }
{DIV} { RESPONSE(DIV, "DIV", TOKEN_OTHER); }
{AND} { RESPONSE(AND, "AND", TOKEN_OTHER); }
{OR} { RESPONSE(OR, "OR", TOKEN_OTHER); }
{DOT} { RESPONSE(DOT, "DOT", TOKEN_OTHER); }
{NOT} { RESPONSE(NOT, "NOT", TOKEN_OTHER); }
{TYPE} { RESPONSE(TYPE, "TYPE", TOKEN_TYPE); }
{LP} { RESPONSE(LP, "LP", TOKEN_OTHER); }
{RP} { RESPONSE(RP, "RP", TOKEN_OTHER); }
{LB} { RESPONSE(LB, "LB", TOKEN_OTHER); }
{RB} { RESPONSE(RB, "RB", TOKEN_OTHER); }
{LC} { RESPONSE(LC, "LC", TOKEN_OTHER); }
{RC} { RESPONSE(RC, "RC", TOKEN_OTHER); }
{STRUCT} { RESPONSE(STRUCT, "STRUCT", TOKEN_OTHER); }
{RETURN} { RESPONSE(RETURN, "RETURN", TOKEN_OTHER); }
{IF} { RESPONSE(IF, "IF", TOKEN_OTHER); }
{ELSE} { RESPONSE(ELSE, "ELSE", TOKEN_OTHER); }
{WHILE} { RESPONSE(WHILE, "WHILE", TOKEN_OTHER); }
{FLOAT} { RESPONSE(FLOAT, "FLOAT", TOKEN_FLOAT); }
{ID} { RESPONSE(ID, "ID", TOKEN_ID); }
. { lexerror = 1; fprintf(stdout, "Error type A at Line %d: Unknown Token \"%s\".\n", yylineno, yytext);}
%%

Node *NewNode(char *content, char *token, int lineno, NodeType type){
    //printf("Making Node %s\n", token);
    Node *n = malloc(sizeof(Node));
    n->type = type;
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