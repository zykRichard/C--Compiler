#include "intercode.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void ICListInit(){
    InterCodes = NewInterCodeList();
}

void InterCodesGenerator(Node *AST, char *filename){
    ICListInit();
    InterCodesTranslate(AST);
    //TraverseICList(InterCodes);
    FILE* fp = fopen(filename, "w+");
    InterCodesPrint(fp, InterCodes);
    fclose(fp);
}

void InterCodesTranslate(Node *root){
    Node *start = root -> childs;
    assert(strcmp(start -> token, "ExtDefList") == 0);
    TranslateExtDefList(start);
}


// Debug
void TraverseICList(InterCodeList *iclist){
    assert(iclist);
    Intercode *ic = iclist -> head;

    while(ic){
        TraverseInterCode(ic);
        ic = ic -> next;
    }
}

void TraverseInterCode(Intercode *ic){
    TraverseIR(ic -> IRcode);
}

void TraverseIR(IR *ir){
    IrKind kind = ir -> kind;
    switch (kind)
    {
        case LABEL_IR:
        case FUNCTION_IR:
        case GOTO_IR:
        case RETURN_IR:
        case ARG_IR:
        case PARAM_IR:
        case READ_IR:
        case WRITE_IR:
            TraverseOP(ir -> IR_Type.Single.op);
            break;

        case ASSIGN_IR:
        case GADDR_IR:
        case RADDR_IR:
        case WADDR_IR:
        case CALL_IR:
            TraverseOP(ir -> IR_Type.Assign.left);
            TraverseOP(ir -> IR_Type.Assign.right);
            break;
        
        case ADD_IR:
        case SUB_IR:
        case MUL_IR:
        case DIV_IR:
            TraverseOP(ir -> IR_Type.Binary.result);
            TraverseOP(ir -> IR_Type.Binary.op1);
            TraverseOP(ir -> IR_Type.Binary.op2);
            break;
        
        case IF_IR:
            TraverseOP(ir -> IR_Type.IfGoto.x);
            TraverseOP(ir -> IR_Type.IfGoto.relop);
            TraverseOP(ir -> IR_Type.IfGoto.y);
            TraverseOP(ir -> IR_Type.IfGoto.z);
            break;
        case DEC_IR:
            TraverseOP(ir -> IR_Type.Dec.x);
            int k = ir -> IR_Type.Dec.sz;
            break;
        
    }
}

void TraverseOP(Operand *op){
    assert(op);
}
// operand
Operand *NewOperand(OpKind kind, ...){
    va_list vaList;
    va_start(vaList, kind);

    Operand *NewOP = (Operand*)malloc(sizeof(Operand));
    NewOP -> kind = kind;
    switch (kind)
    {
    case VARIABLE_OP:
    case ADDRESS_OP:
    case LABEL_OP:
    case FUNCTION_OP:
    case RELOP_OP:
        NewOP -> id.name = va_arg(vaList, char *);
        break;
    case CONSTANT_OP:
        NewOP -> id.value = va_arg(vaList, int);
        break;
     
    default:
        assert(0);
        break;
    }
    return NewOP;
}

void OperandChange(Operand *op, OpKind ChangeKind, char *ChangeName){
    op -> kind = ChangeKind;
    if(ChangeKind == CONSTANT_OP){
        int val = (int)(ChangeName);
        op -> id.value = val;
    }
    else {
        op -> id.name = ChangeName;
    }
}


void printOP(FILE* fp, Operand *op){
    assert(op);
    switch(op -> kind){
        case VARIABLE_OP:
        case ADDRESS_OP:
        case LABEL_OP:
        case FUNCTION_OP:
        case RELOP_OP:
            fprintf(fp, "%s", op -> id.name);
            break;
        
        case CONSTANT_OP:
            fprintf(fp, "#%d", op -> id.value);
            break;

        default: 
            assert(0);
            break;
    }
}


Operand *FindSetNewOpID(sym* s, OpKind kind){
    Operand *ret = (Operand *)malloc(sizeof(Operand));
    if(s -> TempVar == 0){
        ret = NewTempt(InterCodes);
        s -> TempVar = InterCodes -> TempVarNum_t;
        ret -> kind = kind;
        if(kind == ADDRESS_OP) s -> isAddr = 1;
        else s -> isAddr = -1;
    }

    else {
        char *sname = (char *)malloc(16);
        sprintf(sname, "t%d", s -> TempVar);
        if(s -> isAddr == 1)
            ret = NewOperand(ADDRESS_OP, sname);
        else if(s -> isAddr == -1)
            ret = NewOperand(VARIABLE_OP, sname);
    }

    type *tp = s -> tp;
    if(tp -> kind == ARRAY || tp -> kind == STRUCTURE)
        ret -> tp = tp;
    return ret;
}

// Intermediate Representation
IR *NewIR(IrKind kind, ...){
    va_list VaList;
    va_start(VaList, kind);

    IR *newIR = (IR*)malloc(sizeof(IR));
    switch (kind)
    {
    case LABEL_IR:
    case FUNCTION_IR:
    case GOTO_IR:
    case RETURN_IR:
    case ARG_IR:
    case PARAM_IR:
    case READ_IR:
    case WRITE_IR:
        // single OP
        newIR -> IR_Type.Single.op = va_arg(VaList, Operand*);
        break;
    
    case ASSIGN_IR:
    case GADDR_IR:
    case RADDR_IR:
    case WADDR_IR:
    case CALL_IR:
        // assign OP
        newIR -> IR_Type.Assign.left = va_arg(VaList, Operand*);
        newIR -> IR_Type.Assign.right = va_arg(VaList, Operand*);
        break;

    case ADD_IR:
    case SUB_IR:
    case MUL_IR:
    case DIV_IR:
        // binary OP
        newIR -> IR_Type.Binary.result = va_arg(VaList, Operand*);
        newIR -> IR_Type.Binary.op1 = va_arg(VaList, Operand*);
        newIR -> IR_Type.Binary.op2 = va_arg(VaList, Operand*);
        break;

    case IF_IR:
        // if-goto OP
        newIR -> IR_Type.IfGoto.x = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.relop = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.y = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.z = va_arg(VaList, Operand*);
        break;
    
    case DEC_IR:
        // dec OP
        newIR -> IR_Type.Dec.x = va_arg(VaList, Operand*);
        newIR -> IR_Type.Dec.sz = va_arg(VaList, int);
        break;

    default:
        assert(0);
        break;
    }
    newIR -> kind = kind;
    return newIR;
}


void printIR(FILE* fp, IR *ir){
    switch(ir -> kind){
        case LABEL_IR :
            // LABEL x :
            fprintf(fp, "LABEL ");
            printOP(fp, ir -> IR_Type.Single.op);
            fprintf(fp, " :");
            break;
        case FUNCTION_IR:
            // FUNCTION f :
            fprintf(fp, "FUNCTION ");
            printOP(fp, ir -> IR_Type.Single.op);
            fprintf(fp, " :");
            break;
        case ASSIGN_IR:
            // x := y
            printOP(fp, ir -> IR_Type.Assign.left);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Assign.right);
            break;
        case ADD_IR:
            // x := y + z
            printOP(fp, ir -> IR_Type.Binary.result);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Binary.op1);
            fprintf(fp, " + ");
            printOP(fp, ir -> IR_Type.Binary.op2);
            break;
        case SUB_IR:
            // x := y - z
            printOP(fp, ir -> IR_Type.Binary.result);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Binary.op1);
            fprintf(fp, " - ");
            printOP(fp, ir -> IR_Type.Binary.op2);
            break;
        case MUL_IR:
            // x := y * z
            printOP(fp, ir -> IR_Type.Binary.result);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Binary.op1);
            fprintf(fp, " * ");
            printOP(fp, ir -> IR_Type.Binary.op2);
            break;
        case DIV_IR:
            // x := y / z
            printOP(fp, ir -> IR_Type.Binary.result);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Binary.op1);
            fprintf(fp, " / ");
            printOP(fp, ir -> IR_Type.Binary.op2);
            break;
        case GADDR_IR:
            // x := &y 
            printOP(fp, ir -> IR_Type.Assign.left);
            fprintf(fp, " := &");
            printOP(fp, ir -> IR_Type.Assign.right);
            break;
        case RADDR_IR:
            // x := *y
            printOP(fp, ir -> IR_Type.Assign.left);
            fprintf(fp, " := *");
            printOP(fp, ir -> IR_Type.Assign.right);
            break;
        case WADDR_IR:
            // \*x := y
            fprintf(fp, "*");
            printOP(fp, ir -> IR_Type.Assign.left);
            fprintf(fp, " := ");
            printOP(fp, ir -> IR_Type.Assign.right);
            break;
        case GOTO_IR:
            // GOTO x
            fprintf(fp, "GOTO ");
            printOP(fp, ir -> IR_Type.Single.op);
            break;
        case IF_IR:
            // IF x [relop] y GOTO z
            fprintf(fp, "IF ");
            printOP(fp, ir -> IR_Type.IfGoto.x);
            fprintf(fp, " ");
            printOP(fp, ir -> IR_Type.IfGoto.relop);
            fprintf(fp, " ");
            printOP(fp, ir -> IR_Type.IfGoto.y);
            fprintf(fp, " GOTO ");
            printOP(fp, ir -> IR_Type.IfGoto.z);
            break;
        case RETURN_IR: 
            // RETURN x
            fprintf(fp, "RETURN ");
            printOP(fp, ir -> IR_Type.Single.op);
            break;
        case DEC_IR:
            // DEC x [size]
            fprintf(fp, "DEC ");
            printOP(fp, ir -> IR_Type.Dec.x);
            fprintf(fp, " %d", ir -> IR_Type.Dec.sz);
            break;
        case ARG_IR:
            // ARG x
            fprintf(fp, "ARG ");
            if(ir -> IR_Type.Single.op -> kind == VARIABLE_OP)
                fprintf(fp, "&");
            printOP(fp, ir -> IR_Type.Single.op);
            break;
        case CALL_IR:
            // x := CALL f
            printOP(fp, ir -> IR_Type.Assign.left);
            fprintf(fp, " := CALL ");
            printOP(fp, ir -> IR_Type.Assign.right);
            break;
        case PARAM_IR:
            // PARAM x
            fprintf(fp, "PARAM ");
            printOP(fp, ir -> IR_Type.Single.op);
            break;
        case READ_IR:
            // READ x
            fprintf(fp, "READ ");
            printOP(fp, ir -> IR_Type.Single.op);
            break;
        case WRITE_IR:
            // WRITE x
            fprintf(fp, "WRITE ");
            printOP(fp, ir -> IR_Type.Single.op);
            break; 

        default : assert(0); break;
    }
}

// intermediate codes
Intercode *NewInterCode(IR *ir){
    Intercode *newic = (Intercode*)malloc(sizeof(Intercode));
    newic -> IRcode = ir;
    newic -> next = NULL;
    newic -> prev = NULL;
    return newic;
}

void InterCodeInsert(Intercode *ic, InterCodeList *iclist){
    // 尾插法
    if(iclist -> head == NULL){
        iclist -> head = ic;
        iclist -> tail = ic;
    }
    else {
        iclist -> tail -> next = ic;
        ic -> prev = iclist -> tail;
        iclist -> tail = ic;
    }

    assert(iclist -> head);
    assert(iclist -> tail);
}

void printInterCode(FILE* fp, Intercode *ic){
    assert(ic);
    assert(ic -> IRcode);
    printIR(fp, ic -> IRcode);
    fprintf(fp, "\n");
}

// intercodes list
InterCodeList *NewInterCodeList(){
    InterCodeList *NewICL = (InterCodeList *)malloc(sizeof(InterCodeList));
    NewICL -> head = NULL;
    NewICL -> tail = NULL;
    NewICL -> TempLabelNum = 0;
    NewICL -> TempVarNum_t = 0;
    return NewICL;
}

void InterCodesPrint(FILE* fp, InterCodeList *iclist){
    assert(iclist);
    Intercode *ic = iclist -> head;

    while(ic){
        printInterCode(fp, ic);
        ic = ic -> next;
    }

}

// generate a new operand


Operand *NewTempt(InterCodeList *iclist){
    // generate a new operand with the formation like tx
    char *TempName = (char *)malloc(16);
    iclist -> TempVarNum_t ++;
    sprintf(TempName, "t%d", iclist -> TempVarNum_t);
    
    Operand *t = NewOperand(VARIABLE_OP, TempName);
    return t;
}

Operand *NewLabel(InterCodeList *iclist){
    // generate a new operand with the formation like labelx
    char *TempLabel = (char *)malloc(16);
    sprintf(TempLabel, "label%d", iclist -> TempLabelNum);
    iclist -> TempLabelNum ++;
    Operand *labels = NewOperand(LABEL_OP, TempLabel);
    return labels;
}


// Args
Arg *NewArg(Operand *op){
    Arg *a = (Arg*)malloc(sizeof(Arg));
    a -> op = op;
    a -> next = NULL;
}

void ArgListInsert(Arg *a, Arglist *aglist){
    // 头插法
    a -> next = aglist -> head;
    aglist -> head = a;    
}

// SDT Translate
void TranslateCond(Node *n, Operand *labelTrue, Operand *labelFalse){
    /*
    Exp     ->      Exp RELOP Exp
            ->      NOT Exp
            ->      Exp AND Exp
            ->      Exp OR Exp
    */

    if(strcmp(n -> childs -> token, "Exp") == 0){
        Node *cmp = n -> childs -> bros;
        if(strcmp(cmp -> token, "RELOP") == 0){
            Operand *relop = NewOperand(RELOP_OP, cmp -> content);
            Operand *t1 = NewTempt(InterCodes);
            Operand *t2 = NewTempt(InterCodes);
            TranslateExp(n -> childs, t1, 0);
            TranslateExp(cmp -> bros, t2, 0);
            InterCodeInsert(NewInterCode(
                NewIR(IF_IR, t1, relop, t2, labelTrue)), InterCodes);
            InterCodeInsert(NewInterCode(
                NewIR(GOTO_IR, labelFalse)), InterCodes);
        }

        else if(strcmp(cmp -> token, "AND") == 0){
            Operand *newlabel = NewLabel(InterCodes);
            TranslateCond(n -> childs, newlabel, labelFalse);
            InterCodeInsert(NewInterCode(
                NewIR(LABEL_IR, newlabel)), InterCodes);
            TranslateCond(cmp -> bros, labelTrue, labelFalse);
        }

        else if(strcmp(cmp -> token, "OR") == 0){
            Operand *newlabel = NewLabel(InterCodes);
            TranslateCond(n -> childs, labelTrue, newlabel);
            InterCodeInsert(NewInterCode(
                NewIR(LABEL_IR, newlabel)), InterCodes);
            TranslateCond(cmp -> bros, labelTrue, labelFalse);
        }
    }

    else if(strcmp(n -> childs -> token, "NOT") == 0)
        return TranslateCond(n -> childs -> bros, labelFalse, labelTrue);
}


void TranslateExtDefList(Node *n){
    /*
    ExtDefList   ->    ExtDef ExtDefList
                 ->    e
    */
   if(n -> childs) {
        TranslateExtDef(n -> childs);
        TranslateExtDefList(n -> childs -> bros);
   }
}


void TranslateExtDef(Node *n){
   /*
   ExtDef       ->      Specifier ExtDecList SEMI
                ->      Specifier SEMI
                ->      Specifier FunDec CompSt
                ->      Specifier FunDec SEMI
   */
   // 不考虑全局变量，不考虑函数声明，只需要处理情况3
   if(strcmp(n -> childs -> bros -> token, "FunDec") == 0 &&
        strcmp(n -> childs -> bros -> bros -> token, "CompSt") == 0){
            TranslateFunDec(n -> childs -> bros);
            TranslateCompSt(n -> childs -> bros -> bros);
        }
    return ;

}


void TranslateFunDec(Node *n){
    /*
    FunDec      ->      ID LP VarList RP
                ->      ID LP RP
    */
   // FUNCTION ID :
    Intercode *FunIC = 
        NewInterCode(NewIR(FUNCTION_IR, 
                        NewOperand(FUNCTION_OP, n -> childs -> content)));
    InterCodeInsert(FunIC, InterCodes);
    
    Node *args = n -> childs -> bros -> bros;
    if(strcmp(args -> token, "VarList") == 0){
        // PARAM vx
        // // TODO : array arg, struct arg
        sym *s = hash_search(n -> childs -> content);
        fun *f = s -> tp ->u.function;
        for(int i = 0; i < f -> argc; i++){
            FieldList *argv = f -> argv;
            Intercode *ParamIC;
            if(argv -> tp -> kind == BASIC){
                Operand *arg = FindSetNewOpID(hash_search(argv -> name), VARIABLE_OP);
                ParamIC = NewInterCode(NewIR(PARAM_IR, arg));
            }
            else if(argv -> tp -> kind == STRUCTURE || argv -> tp -> kind == ARRAY){
                Operand *arg = FindSetNewOpID(hash_search(argv -> name), ADDRESS_OP);
                ParamIC = NewInterCode(NewIR(PARAM_IR, arg));
            }
            InterCodeInsert(ParamIC, InterCodes);
            argv = argv -> nxt;
        }
    }


}


void TranslateCompSt(Node *n){
    /*
    CompSt      ->      LC DefList StmtList RC
    */
    Node *deflist = n -> childs -> bros;
    Node *stmtlist = deflist -> bros;
    TranslateDefList(deflist);
    TranslateStmtList(stmtlist);
}


void TranslateDefList(Node *n){
    /*
    DefList     ->      Def DefList
                ->      e
    */
    if(n -> childs){
        TranslateDef(n -> childs);
        TranslateDefList(n -> childs -> bros);
    }
}


void TranslateDef(Node *n){
    /*
    Def         ->      Specifier DecList SEMI
    */
    Node *declist = n -> childs -> bros;
    TranslateDecList(declist);
}


void TranslateDecList(Node *n){
    /*
    DecList     ->      Dec
                ->      Dec COMMA DecList
    */
    TranslateDec(n -> childs);
    if(n -> childs -> bros)
        TranslateDecList(n -> childs -> bros -> bros);
}


void TranslateDec(Node *n){
    /*
    Dec         ->      VarDec
                ->      VarDec ASSIGNOP Exp
    */

    if(n -> childs -> bros == NULL)
        TranslateVarDec(n -> childs, NULL);
    
    else {
        // 生成中间变量tx = VarDec;
        // 生成中间变量vx = Exp;
        // tx = vx;
        Operand *op1 = NewTempt(InterCodes);
        Operand *op2 = NewTempt(InterCodes);
        TranslateVarDec(n -> childs, op1);
        TranslateExp(n -> childs -> bros -> bros, op2, 0);
        InterCodeInsert(NewInterCode(
            NewIR(ASSIGN_IR, op1, op2)), InterCodes);
    }
}


void TranslateVarDec(Node *n, Operand* place){
    /*
    VarDec      ->      ID
                ->      VarDec LB INT RB
    */
   // TODO : VarDec in FUNCTION DEF
    if(strcmp(n -> childs -> token, "ID") == 0){
        sym* id = hash_search(n -> childs -> content);
        assert(id);
        type* tp = id -> tp;
        // 不考虑数组和结构体的赋值
        // 如果place不为NULL，只有可能是BASIC赋值
        if(tp -> kind == BASIC){
            if(place){
                // 将place修正为n.content.TempVar
                // // TODO : temperate variable numbers -- 
                *place = *FindSetNewOpID(id, VARIABLE_OP);

            }
            else return;
        }

        // 数组
        // place 一定为NULL
        else if(tp -> kind == ARRAY){
            assert(place == NULL);
            unsigned int sz = GetTypeSz(tp);
            // 声明数组 --- DEC arr_variable
            InterCodeInsert(NewInterCode(
                    NewIR(DEC_IR, FindSetNewOpID(id, VARIABLE_OP), sz)), InterCodes);
            
        }

        // 结构体
        // place 一定为NULL
        else if(tp -> kind == STRUCTURE){
            assert(place == NULL);
            unsigned int sz = GetTypeSz(tp);
            // 声明结构体 --- DEC STRUCT_NAME
            InterCodeInsert(NewInterCode(
                    NewIR(DEC_IR, FindSetNewOpID(id, VARIABLE_OP), sz)), InterCodes);
        }

    }
    
    else if(strcmp(n -> childs -> token, "VarDec") == 0){
        if(place == NULL){
            // 说明是数组声明，直接跳到ID部分判断出来是数组即可
            TranslateVarDec(n -> childs, place);
        }
        else {
            // 说明出现类似于 int a = op[1][2]的情况
            // TODO ： 赋值为高维数组

        }
    }
}


void TranslateStmtList(Node *n){
    /*
    StmtList        ->      Stmt StmtList
                    ->      e
    */
    if(n -> childs){
        TranslateStmt(n -> childs);
        TranslateStmtList(n -> childs -> bros);
    }
}


void TranslateStmt(Node *n){
    /*
    Stmt        ->      Exp SEMI
                ->      CompSt
                ->      RETURN Exp SEMI
                ->      IF LP Exp RP Stmt
                ->      IF LP Exp RP Stmt ELSE Stmt
                ->      WHILE LP Exp RP Stmt
    */
    if(strcmp(n -> childs -> token, "Exp") == 0)
        return TranslateExp(n -> childs, NULL, 0);
    
    else if(strcmp(n -> childs -> token, "CompSt") == 0)
        return TranslateCompSt(n -> childs);
    
    else if(strcmp(n -> childs -> token, "RETURN") == 0){
        // 创建一个新的临时变量表达Exp
        // Return Exp
        Operand *t = NewTempt(InterCodes);
        TranslateExp(n -> childs -> bros, t, 0);
        InterCodeInsert(NewInterCode(
                    NewIR(RETURN_IR, t)), InterCodes);
    }

    else if(strcmp(n -> childs -> token, "IF") == 0){
        Operand *label1 = NewLabel(InterCodes);
        Operand *label2 = NewLabel(InterCodes);
        TranslateCond(n -> childs -> bros -> bros, label1, label2);
        InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label1)), InterCodes);
        Node *stmt1 = n -> childs -> bros -> bros -> bros -> bros;
        TranslateStmt(stmt1);
        if(stmt1 -> bros == NULL){
            InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label2)), InterCodes);
        }
        // IF stmt ELSE stmt
        else {
            Operand *label3 = NewLabel(InterCodes);
            Node *stmt2 = stmt1 -> bros -> bros;
            InterCodeInsert(NewInterCode(NewIR(GOTO_IR, label3)), InterCodes);
            InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label2)), InterCodes);
            TranslateStmt(stmt2);
            InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label3)), InterCodes);
        }
    }

    else if(strcmp(n -> childs -> token, "WHILE") == 0){
        Operand *label1 = NewLabel(InterCodes);
        Operand *label2 = NewLabel(InterCodes);
        Operand *label3 = NewLabel(InterCodes);
        Node *stmt = n -> childs -> bros -> bros -> bros -> bros;
        InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label1)), InterCodes);
        TranslateCond(n -> childs -> bros -> bros, label2, label3);
        InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label2)), InterCodes);
        TranslateStmt(stmt);
        InterCodeInsert(NewInterCode(NewIR(GOTO_IR, label1)), InterCodes);
        InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label3)), InterCodes);
    }
}


void TranslateExp(Node *n, Operand *place, int LorR){
    // Exp  ->  ID
    // // TODO : 结构体
    if(strcmp(n -> childs -> token, "ID") == 0 && (n -> childs -> bros == NULL)){
        sym *id = hash_search(n -> childs -> content);
        if(id -> tp -> kind == BASIC)
            *place = *FindSetNewOpID(id, VARIABLE_OP);
        else if(id -> tp -> kind == STRUCTURE || id -> tp -> kind == ARRAY)
            *place = *FindSetNewOpID(id, ADDRESS_OP);
    }
    // Exp  ->  INT
    else if(strcmp(n -> childs -> token, "INT") == 0){
        Operand *num = NewOperand(CONSTANT_OP, n -> childs -> int_val);
        InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, place, num)), InterCodes);
    }
    // Exp  ->  LP Exp RP
    else if(strcmp(n -> childs -> token, "LP") == 0)
        TranslateExp(n -> childs -> bros, place, 0);
    
    // 条件表达式 运算表达式
    else if(strcmp(n -> childs -> token, "Exp") == 0 ||
                strcmp(n -> childs -> token, "NOT") == 0){
        /*
        Exp     ->      Exp AND Exp
                ->      Exp OR Exp
                ->      Exp RELOP Exp
                ->      NOT Exp
        */

        if(strcmp(n -> childs -> token, "NOT") == 0 ||
            strcmp(n -> childs -> bros -> token, "AND") == 0 ||
            strcmp(n -> childs -> bros -> token, "OR") == 0 ||
            strcmp(n -> childs -> bros -> token, "RELOP") == 0){

                Operand *label1 = NewLabel(InterCodes);
                Operand *label2 = NewLabel(InterCodes);
                Operand *trueop = NewOperand(CONSTANT_OP, 1);
                Operand *falseop = NewOperand(CONSTANT_OP, 0);
                InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, place, falseop)), InterCodes);
                TranslateCond(n, label1, label2);
                InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label1)), InterCodes);
                InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, place, trueop)), InterCodes);
                InterCodeInsert(NewInterCode(NewIR(LABEL_IR, label2)), InterCodes);
            }
        // Exp  ->  Exp ASSIGN Exp
        else if(strcmp(n -> childs -> bros -> token, "ASSIGNOP") == 0){
            Operand *t1 = NewTempt(InterCodes);
            Operand *t2 = NewTempt(InterCodes);
            TranslateExp(n -> childs -> bros -> bros, t2, 1);
            TranslateExp(n -> childs, t1, -1);
            if((t1 -> kind == ADDRESS_OP) && (t2 -> kind != ADDRESS_OP))
                // 左值是地址
                InterCodeInsert(NewInterCode(NewIR(WADDR_IR, t1, t2)), InterCodes);
            else 
                InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, t1, t2)), InterCodes);
            if(place != NULL)
                InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, place, t1)), InterCodes);
        }
        
       // Exp   ->  Exp LB Exp RB 
       else if(strcmp(n -> childs -> bros -> token, "LB") == 0){
            // Exp1   ->   ID
            //        ->   LP Exp RP
             if(strcmp(n -> childs -> childs -> token, "ID") == 0){
                Operand *t = NewTempt(InterCodes);
                TranslateExp(n -> childs, t, 0);
                // t一定是数组
                assert(t -> tp -> kind == ARRAY);
                //assert(t -> kind == ADDRESS_OP);
                Operand *id = NewTempt(InterCodes); // id = Exp (CONSTANT)
                TranslateExp(n -> childs -> bros -> bros, id, 0);
                Operand *offset = NewTempt(InterCodes);
                InterCodeInsert(NewInterCode(NewIR(MUL_IR, offset, id, 
                                    NewOperand(CONSTANT_OP, GetTypeSz(t -> tp -> u.arr -> entry)))), InterCodes); 
                Operand *trueaddr = NewTempt(InterCodes);
                trueaddr -> kind = ADDRESS_OP;
                if(LorR == -1){
                    // 左边被赋值
                    if(t -> kind == VARIABLE_OP){
                        Operand *addr = NewTempt(InterCodes);
                        addr -> kind = ADDRESS_OP;
                        InterCodeInsert(NewInterCode(NewIR(GADDR_IR, addr, t)), InterCodes);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, addr, offset)), InterCodes);
                    }
                    else if(t -> kind == ADDRESS_OP){
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, t, offset)), InterCodes);
                    }
                    *place = *trueaddr;
                    place -> tp = t -> tp -> u.arr -> entry;
                }

                else {
                    // 右边赋值
                    if(t -> kind == VARIABLE_OP){
                        Operand *addr = NewTempt(InterCodes);
                        addr -> kind = ADDRESS_OP;
                        InterCodeInsert(NewInterCode(NewIR(GADDR_IR, addr, t)), InterCodes);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, addr, offset)), InterCodes);
                    }
                    else if(t -> kind == ADDRESS_OP){
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, t, offset)), InterCodes);
                    }
                    Operand *CopyPlace = NewOperand(VARIABLE_OP, " ");
                    *CopyPlace = *place;
                    InterCodeInsert(NewInterCode(NewIR(RADDR_IR, CopyPlace, trueaddr)), InterCodes);
                    place -> tp = t -> tp -> u.arr -> entry;
                }
        }
            //Exp1  -> Exp LB Exp RB
            else if(strcmp(n -> childs -> childs -> token, "Exp") == 0 && 
                            strcmp(n -> childs -> childs -> bros -> token, "LB") == 0)
                {
                    Node *exp1 = n -> childs;
                    Node *exp2 = n -> childs -> bros -> bros;
                    Operand *id = NewTempt(InterCodes);
                    TranslateExp(exp2, id, 0);
                    
                    Operand *offset = NewTempt(InterCodes);
                    offset -> kind = ADDRESS_OP;
                    Operand *trueaddr = NewTempt(InterCodes);
                    trueaddr -> kind = ADDRESS_OP;
                    if(LorR == -1){
                        TranslateExp(exp1, place, LorR);
                        assert(place -> kind == ADDRESS_OP);
                        type *entry = place -> tp -> u.arr -> entry; 
                        InterCodeInsert(NewInterCode(NewIR(MUL_IR, offset, id, 
                                    NewOperand(CONSTANT_OP, GetTypeSz(entry)))), InterCodes);
                        Operand *CopyPlace = NewOperand(VARIABLE_OP, " ");
                        *CopyPlace = *place;
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, CopyPlace, offset)), InterCodes);
                        *place = *trueaddr;
                        place -> tp = entry;
                        }
                    else {
                        // 还是需要利用左值查询出地址
                        Operand *expaddr = NewTempt(InterCodes);
                        expaddr -> kind = ADDRESS_OP;
                        TranslateExp(exp1, expaddr, -1);
                        assert(expaddr -> kind == ADDRESS_OP);
                        
                        InterCodeInsert(NewInterCode(NewIR(MUL_IR, offset, id,
                                    NewOperand(CONSTANT_OP, GetTypeSz(expaddr -> tp -> u.arr -> entry)))), InterCodes);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, trueaddr, expaddr, offset)), InterCodes);
                        InterCodeInsert(NewInterCode(NewIR(RADDR_IR, place, trueaddr)), InterCodes);
                    }
                    }
                

       }
       // Exp   ->  Exp DOT ID
       else if(strcmp(n -> childs -> bros -> token, "DOT") == 0){
            Operand *t = NewTempt(InterCodes);
            TranslateExp(n -> childs, t, 0);
            // t 一定是结构体
            
            assert(t -> tp -> kind == STRUCTURE);
            Node *id = n -> childs -> bros -> bros;
            // get id offset
            FieldList *MemberList = t -> tp -> u.structure -> field;
            while(strcmp(MemberList -> name, id -> content)){
                MemberList = MemberList -> nxt;
            }
            int offset = 0;
            while(MemberList -> nxt != NULL){
                MemberList = MemberList -> nxt;
                offset += GetTypeSz(MemberList -> tp);
            }
            Operand *opaddr = FindSetNewOpID(hash_search(id -> content), ADDRESS_OP);
            if(LorR == -1){
                // 左值赋值
                if(t -> kind == VARIABLE_OP){
                    // 先算地址
                    Operand *addr = NewTempt(InterCodes);
                    addr -> kind = ADDRESS_OP;
                    InterCodeInsert(NewInterCode(NewIR(GADDR_IR, addr, t)), InterCodes);
                    if(offset == 0)
                        InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, opaddr, addr)), InterCodes);
                    else {
                        Operand *off = NewOperand(CONSTANT_OP, offset);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, opaddr, addr, off)), InterCodes);
                    }
                    *place = *opaddr;
                }

                else if(t -> kind == ADDRESS_OP){
                    if(offset == 0)
                        InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, opaddr, t)), InterCodes);
                    else {
                        Operand *off = NewOperand(CONSTANT_OP, offset);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, opaddr, t, off)), InterCodes);
                    }
                    *place = *opaddr;
                }
            }

            else {
                // 其余情况一律按取地址操作处理
                if(t -> kind == VARIABLE_OP){
                    Operand *addr = NewTempt(InterCodes);
                    addr -> kind = ADDRESS_OP;
                    InterCodeInsert(NewInterCode(NewIR(GADDR_IR, addr, t)), InterCodes);
                    if(offset == 0)
                        InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, opaddr, addr)), InterCodes);
                    else {
                        Operand *off = NewOperand(CONSTANT_OP, offset);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, opaddr, addr, off)), InterCodes);
                    }
                    InterCodeInsert(NewInterCode(NewIR(RADDR_IR, place, opaddr)), InterCodes);
                }

                else if(t -> kind == ADDRESS_OP){
                    if(offset == 0)
                        InterCodeInsert(NewInterCode(NewIR(ASSIGN_IR, opaddr, t)), InterCodes);
                    else {
                        Operand *off = NewOperand(CONSTANT_OP, offset);
                        InterCodeInsert(NewInterCode(NewIR(ADD_IR, opaddr, t, off)), InterCodes);
                    }
                    InterCodeInsert(NewInterCode(NewIR(RADDR_IR, place, opaddr)), InterCodes);
                }
            }
       }
        /*
        Exp     ->      Exp PLUS Exp
                ->      Exp MINUS Exp
                ->      Exp STAR Exp
                ->      Exp DIV Exp
        */        
       else {
            Operand *t1 = NewTempt(InterCodes);
            Operand *t2 = NewTempt(InterCodes);
            TranslateExp(n -> childs, t1, 0);
            TranslateExp(n -> childs -> bros -> bros, t2, 0);
            // Exp -> Exp PLUS Exp
            if(strcmp(n -> childs -> bros -> token, "PLUS") == 0)
                InterCodeInsert(NewInterCode(NewIR(ADD_IR, place, t1, t2)), InterCodes);
            // Exp -> Exp MINUS Exp
            else if(strcmp(n -> childs -> bros -> token, "MINUS") == 0)
                InterCodeInsert(NewInterCode(NewIR(SUB_IR, place, t1, t2)), InterCodes);
            // Exp -> Exp STAR Exp
            else if(strcmp(n -> childs -> bros -> token, "STAR") == 0)
                InterCodeInsert(NewInterCode(NewIR(MUL_IR, place, t1, t2)), InterCodes);
            // Exp -> Exp DIV Exp
            else if(strcmp(n -> childs -> bros -> token, "DIV") == 0)
                InterCodeInsert(NewInterCode(NewIR(DIV_IR, place, t1, t2)), InterCodes);
       }
    }

    // Exp  ->  MINUS Exp
    else if(strcmp(n -> childs -> token, "MINUS") == 0){
        Operand *t1 = NewTempt(InterCodes);
        TranslateExp(n -> childs -> bros, t1, 0);
        Operand *zero = NewOperand(CONSTANT_OP, 0);
        InterCodeInsert(NewInterCode(NewIR(SUB_IR, place, zero, t1)), InterCodes);
    }

    // Exp -> ID LP Args RP
    // Exp -> ID LP RP
    else if(strcmp(n -> childs -> token, "ID") == 0 && (n -> childs -> bros != NULL)){
        Node *isArgs = n -> childs -> bros -> bros;
        sym *f = hash_search(n -> childs -> content);
        assert(f -> tp -> kind == FUNCTION);
        if(strcmp(isArgs -> token, "RP") == 0){
            
            if(strcmp(f -> name, "read") == 0)
                InterCodeInsert(NewInterCode(NewIR(READ_IR, place)), InterCodes);
            else{
                Operand *fc = NewOperand(FUNCTION_OP, f -> name);
                InterCodeInsert(NewInterCode(NewIR(CALL_IR, place, fc)), InterCodes);
            }
        }

        else if(strcmp(isArgs -> token, "Args") == 0){
            // TODO : 结构体和一维数组传参
            Arglist *newlist = (Arglist*)malloc(sizeof(Arglist));
            TranslateArgs(isArgs, newlist);
            if(strcmp(f -> name, "write") == 0)
                InterCodeInsert(NewInterCode(NewIR(WRITE_IR, newlist -> head -> op)), InterCodes);
            else {
                Arg *cur = newlist -> head;
                while(cur){
                    InterCodeInsert(NewInterCode(NewIR(ARG_IR, cur -> op)), InterCodes);
                    cur = cur -> next;
                }
                Operand *fc = NewOperand(FUNCTION_OP, f -> name);
                InterCodeInsert(NewInterCode(NewIR(CALL_IR, place, fc)), InterCodes);
            }
        }
    }
}


void TranslateArgs(Node *n, Arglist *aglist){
    /*
    Args    ->      Exp
            ->      Exp COMMA Args
    */
    Node *exp = n -> childs;
    Operand *t1 = NewTempt(InterCodes);
    TranslateExp(exp, t1, 0);
    Arg *newarg = NewArg(t1);
    ArgListInsert(newarg, aglist);
    if(exp -> bros != NULL)
        TranslateArgs(exp -> bros -> bros, aglist);
}