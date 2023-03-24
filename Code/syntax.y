%{
    #include "common.h"
    #include <stdio.h>
    #include <stdarg.h>
    #include "lex.yy.c"
    
    int synerror = 0;
    int yyerror(char *msg);
    void NodeGen(Node *pre, int argc, ...);
    void TreePrint(Node *root, int depth);
    Node* root = NULL;
%}

%union
{
    Node* node;    
}

%locations
%token <node> SEMI COMMA
%token <node> ASSIGNOP RELOP PLUS MINUS STAR DIV
%token <node> AND OR DOT NOT 
%token <node> TYPE
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE
%token <node> INT FLOAT
%token <node> ID

%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt 
%type <node> DefList Def DecList Dec
%type <node> Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS 
%left STAR DIV  
%right UMINUS NOT
%left LP RP LB RB LC RC DOT
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%define parse.error verbose

%%
Program:            ExtDefList                              { $$ = NewNode(NULL, "Program", @$.first_line, LANG); NodeGen($$, 1, $1); root = $$; }
;

ExtDefList:         /* empty */                             { $$ = NewNode(NULL, "ExtDefList", @$.first_line, LANG); }
|                   ExtDef ExtDefList                       { $$ = NewNode(NULL, "ExtDefList", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
;

ExtDef:             Specifier ExtDecList SEMI               { $$ = NewNode(NULL, "ExtDef", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Specifier SEMI                          { $$ = NewNode(NULL, "ExtDef", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
|                   Specifier FunDec CompSt                 { $$ = NewNode(NULL, "ExtDef", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   error SEMI                              { synerror = 1;}
;

ExtDecList:         VarDec                                  { $$ = NewNode(NULL, "ExtDecList", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   VarDec COMMA ExtDecList                 { $$ = NewNode(NULL, "ExtDecList", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
;

Specifier:          TYPE                                    { $$ = NewNode(NULL, "Specifier", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   StructSpecifier                         { $$ = NewNode(NULL, "Specifier", @$.first_line, LANG); NodeGen($$, 1, $1); }
;

StructSpecifier:    STRUCT OptTag LC DefList RC             { $$ = NewNode(NULL, "StructSpecifier", @$.first_line, LANG); NodeGen($$, 5, $1, $2, $3, $4, $5); }
|                   STRUCT Tag                              { $$ = NewNode(NULL, "StructSpecifier", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
|                   error RC                                { synerror = 1; }
;

OptTag:             /* empty */                             { $$ = NewNode(NULL, "OptTag", @$.first_line, LANG); }
|                   ID                                      { $$ = NewNode(NULL, "OptTag", @$.first_line, LANG); NodeGen($$, 1, $1); }
;

Tag:                ID                                      { $$ = NewNode(NULL, "Tag", @$.first_line, LANG); NodeGen($$, 1, $1); }
;

VarDec:             ID                                      { $$ = NewNode(NULL, "VarDec", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   VarDec LB INT RB                        { $$ = NewNode(NULL, "VarDec", @$.first_line, LANG); NodeGen($$, 4, $1, $2, $3, $4); }                    
|                   VarDec LB error RB                      { synerror = 1; } 
;

FunDec:             ID LP VarList RP                        { $$ = NewNode(NULL, "FunDec", @$.first_line, LANG); NodeGen($$, 4, $1, $2, $3, $4); }
|                   ID LP RP                                { $$ = NewNode(NULL, "FunDec", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   error RP                                { synerror = 1; }
;

VarList:            ParamDec COMMA VarList                  { $$ = NewNode(NULL, "VarList", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   ParamDec                                { $$ = NewNode(NULL, "VarList", @$.first_line, LANG); NodeGen($$, 1, $1); }
;

ParamDec:           Specifier VarDec                        { $$ = NewNode(NULL, "ParamDec", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
;

CompSt:             LC DefList StmtList RC                  { $$ = NewNode(NULL, "CompSt", @$.first_line, LANG); NodeGen($$, 4, $1, $2, $3, $4); }
|                   error RC                                { synerror = 1; }
;

StmtList:           /* empty */                             { $$ = NewNode(NULL, "StmtList", @$.first_line, LANG); }
|                   Stmt StmtList                           { $$ = NewNode(NULL, "StmtList", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
;

Stmt:               Exp SEMI                                { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
|                   CompSt                                  { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   RETURN Exp SEMI                         { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 5, $1, $2, $3, $4, $5); }
|                   IF LP Exp RP Stmt ELSE Stmt             { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 7, $1, $2, $3, $4, $5, $6, $7); }
|                   WHILE LP Exp RP Stmt                    { $$ = NewNode(NULL, "Stmt", @$.first_line, LANG); NodeGen($$, 5, $1, $2, $3, $4, $5); }
|                   error SEMI                              { synerror = 1; }
;

DefList:            /* empty */                             { $$ = NewNode(NULL, "DefList", @$.first_line, LANG); }
|                   Def DefList                             { $$ = NewNode(NULL, "DefList", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }
;

Def:                Specifier DecList SEMI                  { $$ = NewNode(NULL, "Def", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   error SEMI                              { synerror = 1; }
;

DecList:            Dec                                     { $$ = NewNode(NULL, "DecList", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   Dec COMMA DecList                       { $$ = NewNode(NULL, "DecList", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
;

Dec:                VarDec                                  { $$ = NewNode(NULL, "Dec", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   VarDec ASSIGNOP Exp                     { $$ = NewNode(NULL, "Dec", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
;

Exp:                Exp ASSIGNOP Exp                        { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp AND Exp                             { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp OR Exp                              { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp RELOP Exp                           { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp PLUS Exp                            { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }             
|                   Exp MINUS Exp                           { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp STAR Exp                            { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp DIV Exp                             { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   LP Exp RP                               { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   MINUS Exp %prec UMINUS                  { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }                
|                   NOT Exp                                 { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 2, $1, $2); }                                   
|                   ID LP Args RP                           { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 4, $1, $2, $3, $4); }                
|                   ID LP RP                                { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp LB Exp RB                           { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 4, $1, $2, $3, $4); }
|                   Exp DOT ID                              { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   ID                                      { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   INT                                     { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   FLOAT                                   { $$ = NewNode(NULL, "Exp", @$.first_line, LANG); NodeGen($$, 1, $1); }
|                   LP error RP                             { synerror = 1; }
|                   ID LP error RP                          { synerror = 1; }
|                   Exp LB error RB                         { synerror = 1; }
// |                   Exp ASSIGNOP error                      { synerror = 1; }
// |                   Exp AND error                           { synerror = 1; }
// |                   Exp OR error                            { synerror = 1; } 
// |                   Exp RELOP error                         { synerror = 1; }
// |                   Exp PLUS error                          { synerror = 1; }
// |                   Exp MINUS error                         { synerror = 1; }
// |                   Exp STAR error                          { synerror = 1; }
// |                   Exp DIV error                           { synerror = 1; }
;

Args:               Exp COMMA Args                          { $$ = NewNode(NULL, "Args", @$.first_line, LANG); NodeGen($$, 3, $1, $2, $3); }
|                   Exp                                     { $$ = NewNode(NULL, "Args", @$.first_line, LANG); NodeGen($$, 1, $1); }
;

%%

void NodeGen(Node *pre, int argc, ...){
    Node *curchild = pre -> childs;
    va_list vaList;
    va_start(vaList, argc);

    Node *childnode = va_arg(vaList, Node*);
    //printf("Add node %s, father is %s\n", childnode -> token, pre -> token);
    if(curchild == NULL) {
        pre -> childs = childnode;
        curchild = childnode;
    }
    else {
        while(curchild -> bros != NULL)
            curchild = curchild -> bros;
        curchild -> bros = childnode;
        curchild = childnode;
    }

    for(int i = 1; i < argc; i++){
        childnode = va_arg(vaList, Node*);
        //printf("Add node %s, father is %s\n", childnode -> token, pre -> token);
        curchild -> bros = childnode;
        curchild = childnode;
    }   
}

void TreePrint(Node *root, int depth){
    int flag = 1;
    if(root == NULL) return ;
    //printf("Now token is %s\n", root -> token);
    if(root -> type == LANG && root -> childs == NULL)
        flag = 0;
    if(flag)
    for(int i = 0; i < depth; i++)
        printf("  ");

    if(root -> type == LANG){
        if(root -> childs != NULL) 
            printf("%s (%d)\n", root -> token, root -> lineno);
    }

    else if(root -> type == TOKEN_ID)
        printf("%s: %s\n", root -> token, root -> content);
    else if(root -> type == TOKEN_TYPE)
        printf("%s: %s\n", root -> token, root -> content);
    else if(root -> type == TOKEN_INT)
        printf("%s: %d\n", root -> token, root -> int_val);
    else if(root -> type == TOKEN_FLOAT)
        printf("%s: %lf\n", root -> token, root -> float_val);
    else if(root -> type == TOKEN_OTHER)
        printf("%s\n", root -> token);
    
    TreePrint(root -> childs, depth + 1);
    TreePrint(root -> bros, depth);
}

int yyerror(char *msg){
    synerror = 1;
    fprintf(stdout, "Error type B at Line %d: ", yylineno);
    fprintf(stdout, "%s\n", msg);
    
}