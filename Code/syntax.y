%{
    #include "intercode.h"
    #include "lex.yy.c"
    int yyerror(char *msg);
    void ResetOP(Operand *op, int istemp);
    void CmpOP(Operand *op);
%}

%union
{
    Operand* node;
}

%locations
%token <node> END COLON LABEL FUNC GOTO IF RETURN DEC ARG CALL PARAM READ WRITE
%token <node> GOPERAND ROPERAND
%token <node> INT STAR ASSIGN PLUS MINUS DIV
%token <node> CONSTANT RELOP LABELOP TOPERAND VOPERAND ID

%type <node> IRLIST IR OPERAND

%%
IRLIST:         /* empty */
|               IR END IRLIST
|               IR

OPERAND:        CONSTANT                    { $$ = $1; }
|               TOPERAND                    { ResetOP($1, 1); $$ = $1; }
|               VOPERAND                    { ResetOP($1, 0); $$ = $1; }
|               GOPERAND                    { CmpOP($1); $$ = $1; }
|               ROPERAND                    { CmpOP($1); $$ = $1; }

IR:             LABEL LABELOP COLON         { InterCodeInsert(NewInterCode(NewIR(LABEL_IR, $2)), InterCodes);}
|               FUNC ID COLON           { InterCodeInsert(NewInterCode(NewIR(FUNCTION_IR, $2)), InterCodes);}
|               OPERAND ASSIGN OPERAND      { InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, $1, $3)), InterCodes);}
|               OPERAND ASSIGN OPERAND PLUS OPERAND        { InterCodeInsert(NewInterCode(NewIR(ADD_IR, $1, $3, $5)), InterCodes);}
|               OPERAND ASSIGN OPERAND MINUS OPERAND       { InterCodeInsert(NewInterCode(NewIR(SUB_IR, $1, $3, $5)), InterCodes);}
|               OPERAND ASSIGN OPERAND STAR OPERAND        { InterCodeInsert(NewInterCode(NewIR(MUL_IR, $1, $3, $5)), InterCodes);}
|               OPERAND ASSIGN OPERAND DIV OPERAND        { InterCodeInsert(NewInterCode(NewIR(DIV_IR, $1, $3, $5)), InterCodes);}
//|               OPERAND ASSIGN AND OPERAND                 { InterCodeInsert(NewInterCode(NewIR(GADDR_IR, $1, $4)), InterCodes);}
//|               OPERAND ASSIGN STAR OPERAND                 { InterCodeInsert(NewInterCode(NewIR(RADDR_IR, $1, $4)), InterCodes);}
//|               STAR OPERAND ASSIGN OPERAND                 { InterCodeInsert(NewInterCode(NewIR(WADDR_IR, $2, $4)), InterCodes);}
|               GOTO LABELOP                                { InterCodeInsert(NewInterCode(NewIR(GOTO_IR, $2)), InterCodes);}
|               IF OPERAND RELOP OPERAND GOTO LABELOP       { InterCodeInsert(NewInterCode(NewIR(IF_IR, $2, $3, $4, $6)), InterCodes);}
|               RETURN OPERAND                              { InterCodeInsert(NewInterCode(NewIR(RETURN_IR, $2)), InterCodes);}
|               DEC OPERAND INT                             { InterCodeInsert(NewInterCode(NewIR(DEC_IR, $2, $3 -> id.value)), InterCodes);}
|               ARG OPERAND                                 { InterCodeInsert(NewInterCode(NewIR(ARG_IR, $2)), InterCodes);}
|               OPERAND ASSIGN CALL ID                      { InterCodeInsert(NewInterCode(NewIR(CALL_IR, $1, $4)), InterCodes);}
|               PARAM OPERAND                               { InterCodeInsert(NewInterCode(NewIR(PARAM_IR, $2)), InterCodes);}
|               READ OPERAND                                { InterCodeInsert(NewInterCode(NewIR(READ_IR, $2)), InterCodes); }
|               WRITE OPERAND                               { InterCodeInsert(NewInterCode(NewIR(WRITE_IR, $2)), InterCodes); }
%%


void ResetOP(Operand *op, int istemp){
    op -> temp = istemp;
    int id = atoi(op -> id.name + 1);
    op -> no = id;
}

void CmpOP(Operand *op){
    if(strncmp(op -> id.name, "v", 1) == 0)
        op -> temp = 0;
    else if(strncmp(op -> id.name, "t", 1) == 0)
        op -> temp = 1;
    else printf("Error in comparing\n");
    int id = atoi(op -> id.name + 1);
    op -> no = id;
}
