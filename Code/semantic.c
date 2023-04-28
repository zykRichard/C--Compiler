#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"
extern unsigned int stack_top;
int AnonyStruct = 0;
int FunDefNum = 0;
sym *funcs[999];
int isFunDef[999];
char *err_msg[20] = {
    " reserved ",
    "Undefined variables.",
    "Undefined function." ,
    "Redefined variable." ,
    "Redefined function." ,
    "Type mismatch for assignment." ,
    "The left-hand side of an assignment must be a variable." ,
    "Type mismatch for operands." ,
    "Type mismatch for return." ,
    "Function is not applicable for arguments." ,
    "Variable is not an array." ,
    "Variable is not a function." ,
    "Index of an array must be an integer." ,
    "Illegal use of \".\"." ,
    "Non-existent field." ,
    "Redefined member inside structure, or make initialization of field variables." ,
    "Duplicated name." ,
    "Undefined structure." ,
    "Undefined function." ,
    "Inconsistence of function declaration."
};

// Global Function
void LexicalAnalysis(Node *AST){
    TableInit();
    memset(isFunDef, 0, sizeof(isFunDef));
    TreeTraverse(AST);
    FunNotDefCheck();
    stack_top = (unsigned int)(-1); // for Lab 3;
}


void TreeTraverse(Node *root){
    if(root == NULL) return ;
    if(strcmp(root -> token, "ExtDef") == 0) ExtDef(root);

    TreeTraverse(root -> childs);
    TreeTraverse(root -> bros);
}



static inline void ErrorReport(int code, int lineno){
    printf("Error type %d at Line %d: %s\n", code, lineno, err_msg[code]);
}


void FunNotDefCheck(){
    for(int i = 0; i < FunDefNum; i++){
        if(isFunDef[i]) continue;
        int flag = 0;
        sym *hash_head = hash_search(funcs[i] -> name);
        while(hash_head && hash_head -> StackDepth == (unsigned int)(-1))
            hash_head = hash_head -> nxt_sym;
        if(hash_head == NULL)
            ErrorReport(18, funcs[i] -> tp -> u.function -> lineno);
        else {
            
            while(hash_head){
                if(strcmp(hash_head -> name, funcs[i] -> name) == 0){
                    assert(hash_head -> tp -> kind == FUNCTION);
                    if(hash_head -> tp -> u.function -> isDef == 1){
                        // match successfully
                        flag = 1;
                        break;
                    }
                }
                hash_head = hash_head -> nxt_sym;
                while(hash_head && hash_head -> StackDepth == (unsigned int)(-1))
                    hash_head = hash_head -> nxt_sym;
                }
            }
            if(!flag) ErrorReport(18, funcs[i] -> tp -> u.function -> lineno);
        }
    }


// Symbol Table Generation
// directly check every ExtDef


void ExtDef(Node *n){
    /*
    ExtDef     ->    Specifier ExtDecList SEMI
                 |   Specifier SEMI
                 |   Specifier FunDec CompSt
                 |   Specifier FunDec SEMI
    */

    Node *sp = n -> childs;
    type *specifier = Specifier(sp);
    // situation 1:
    // Get Specifier, Give to ExtDecList 

    if(strcmp(sp -> bros -> token, "ExtDecList") == 0){
        ExtDecList(sp -> bros, specifier); // add new symbol
    }

    // situation 2:
    // Nothing to do

    // situation 3:
    // Get Specifier, Give to FunDec and CompSt
    // FunDec add function symbol, CompSt process function body and check return type
    else if(strcmp(sp -> bros -> token, "FunDec") == 0){
        // Function definition
        if (strcmp(sp -> bros -> bros -> token, "CompSt") == 0){
        FunDec(sp -> bros, specifier, 1);
        CompSt(sp -> bros -> bros, specifier);
    }   
        // Function declaration
        else {
            FunDec(sp -> bros, specifier, 0);
                // TODO
        }
    }

    // TODO : clean process 

}


type *Specifier(Node *n){
    /*
    Specifier     ->    TYPE
                     |  StructSpecifier
    */

    // situation 1:
    // Directly get specifier type
    if(strcmp(n -> childs -> token, "TYPE") == 0){
        if(strcmp(n -> childs -> content, "int") == 0)
            return NewType(BASIC, 0);
        else 
            return NewType(BASIC, 1);
    }

    // situation 2:
    // Get StructSpecifier
    else if(strcmp(n -> childs -> token, "StructSpecifier") == 0)
        return StructSpecifier(n -> childs);

    // TODO : clean / error process 
}


type *StructSpecifier(Node *n){
    /*
    StructSpecifier    ->     STRUCT OptTag LC DefList RC
                           |  STRUCT Tag
    */

    Node *st = n -> childs -> bros;
    char *sname;
    
    int isAnony;

    // situation 1:
    // definition of structure
    if(strcmp(st -> token, "OptTag") == 0){
        // OptTag -> e
        if(st -> childs == NULL){
            sname = (char *)malloc(2);
            memset(sname, AnonyStruct, sizeof(sname));
            isAnony = 1;        
            AnonyStruct ++;
        }
        // OptTag -> ID
        else {
            char *IDname = st -> childs -> content;
            sname = (char *)malloc(strlen(IDname) + 1);
            strcpy(sname, IDname);
            isAnony = 0;
        }

        // Conflict Detection
        // sym *StructSym = hash_search(sname);
        // if(StructSym != NULL){
        //     // Redefined Structure
        //     ErrorReport(16, n -> lineno);
        //     return NULL;
        // }

        Node *Dlist = st -> bros -> bros;
        type *StructSpecifier = NewType(STRUCTURE, sname, NULL, isAnony);

        // LC 栈Push
        StackPush();
        DefList(Dlist, StructSpecifier); // add FieldList to StructSpecifier

        // Struct Definition , insert at stack[0]
        // hash_insert(NewSym(sname, StructSpecifier, 0), 0);
        // TODO : Conflict detection and Symbol insertion
        // RC 栈Pop
        StackPop(0);
        sym *StructID = NewSym(sname, StructSpecifier, 0);
        if(CheckForConflict(StructID)){
            // Redefined Structure
            ErrorReport(16, n -> lineno);
            return NULL;
        }
        hash_insert(StructID, 0);
        return StructSpecifier;
    }


    // situation 2:
    // declaration of structure
    else if(strcmp(st -> token, "Tag") == 0){
        sym *hash_head = hash_search(st -> childs -> content);
        if(hash_head == NULL ||
            hash_head -> tp -> kind != STRUCTURE){
                // undefined structure
                ErrorReport(17, st -> lineno);
                return NULL;
        }

        else return hash_head -> tp;
    }
}



void ExtDecList(Node *n, type* specifier){
    /*
    ExtDecList    ->     VarDec 
                     |   VarDec COMMA ExtDecList
    */

   Node *var = n -> childs;
   if(var -> bros == NULL){
    sym *newsym = VarDec(var, specifier);
    
    if(CheckForConflict(newsym))
        // Redefine variable
        ErrorReport(3, var -> lineno);

    else 
        hash_insert(newsym, stack_top);

    return;
   }

   else {
    sym *newsym = VarDec(var, specifier);

    if(CheckForConflict(newsym))
        ErrorReport(3, var -> lineno);
    else 
        hash_insert(newsym, stack_top);
    
    Node *NxtDecList = var -> bros -> bros;
    ExtDecList(NxtDecList, specifier);
   }
   
}


sym *VarDec(Node *n, type* specifier){
    /*
    VarDec    ->     ID
                 |   VarDec LB INT RB
    */
    // NodeKind nkind = specifier -> kind;

    if(strcmp(n -> childs -> token, "ID") == 0)
        return NewSym(n -> childs -> content, specifier, stack_top);
    
    // array 
    else if(strcmp(n -> childs -> token, "VarDec") == 0){
        sym *subsym = VarDec(n -> childs, specifier);
        int sz = n -> childs -> bros -> bros -> int_val;
        type *newtp = NewType(ARRAY, specifier, sz);
        type *curtp = subsym -> tp;
        while(curtp != specifier){
            curtp = curtp -> u.arr->entry;
        }
        curtp -> kind = ARRAY;
        curtp -> u.arr = newtp -> u.arr;
        return subsym;

    }
        
}

void DefList(Node *n, type* ScopeSpecifier){
    /*
    DefList     ->     Def DefList
                    |  e
    */

   if(n -> childs == NULL) return ;
   else {
     Def(n -> childs, ScopeSpecifier);
     DefList(n -> childs -> bros, ScopeSpecifier);
   }
   
}


void Def(Node *n, type *ScopeSpecifier){
    /*
    Def       ->       Specifier DecList SEMI
    */
   type *specifier = Specifier(n -> childs);

   DecList(n -> childs -> bros, specifier, ScopeSpecifier);

   // TODO : clean process
}


void DecList(Node *n, type *specifier, type *ScopeSpecifier){
    /*
    DecList    ->      Dec
                    |  Dec COMMA DecList
    */
   Dec(n -> childs, specifier, ScopeSpecifier);
   if(n -> childs -> bros != NULL)
        DecList(n -> childs -> bros -> bros, specifier, ScopeSpecifier);
}



void Dec(Node *n, type *specifier, type *ScopeSpecifier){
    /*
    Dec        ->      VarDec
                    |  VarDec ASSIGNOP Exp
    */
   if(ScopeSpecifier != NULL && ScopeSpecifier -> kind == STRUCTURE){
    // 结构体内各成员定义：
    // 不允许出现ASSIGNOP，且在该函数体内更新FieldList
    // 现在在结构体内！！！
    sym *member = VarDec(n -> childs, specifier);
    if(CheckForConflict(member)){
        // redefine of variable in structure field
        ErrorReport(15, n -> lineno);
        return ;
    }

    if(n -> childs -> bros && strcmp(n -> childs -> bros -> token, "ASSIGNOP") == 0){
        // 虽然报错,但该符号依然需要加入struct field里面
        FieldList *NewField = (FieldList *)malloc(sizeof(FieldList));
        char *newname = (char *)malloc(strlen(member -> name) + 1);
        strcpy(newname, member -> name);
        NewField -> name = newname;
        NewField -> tp = member -> tp;
        NewField -> nxt = ScopeSpecifier -> u.structure -> field;
        ScopeSpecifier -> u.structure -> field = NewField;
        // initialized variable in structure field
        ErrorReport(15, n -> childs -> bros -> lineno);
        return ;
    }
    
    // 加入符号表
    hash_insert(member, stack_top);
    // 创建Fieldlist
    FieldList *NewField = (FieldList *)malloc(sizeof(FieldList));
    char *newname = (char *)malloc(strlen(member -> name) + 1);
    strcpy(newname, member -> name);
    NewField -> name = newname;
    NewField -> tp = member -> tp;
    NewField -> nxt = ScopeSpecifier -> u.structure -> field;
    ScopeSpecifier -> u.structure -> field = NewField;
   }

   else {
    sym *member = VarDec(n -> childs, specifier);
    if(CheckForConflict(member)){
        // redefine variable in funtion 
        ErrorReport(3, n -> lineno);
        return;
    }
    
    if((n -> childs -> bros != NULL) && 
            (strcmp(n -> childs -> bros -> token, "ASSIGNOP") == 0)){
        Node *exp = n -> childs -> bros -> bros;
        type *to_copy = Exp(exp);
        if(!TypeCheck(member -> tp, to_copy)){
            // illegal assignment
            ErrorReport(5, n -> lineno);
            return ;
        }
    }

    hash_insert(member, stack_top);
   }

   return ;
}



void FunDec(Node *n, type* ReturnSpecifier, int isDef){
    /*
    FunDec       ->     ID LP VarList RP
                    |   ID LP RP
    */
   type *newfun = NewType(FUNCTION, 0, NULL, ReturnSpecifier, n -> lineno, isDef);
   Node *IDnode = n -> childs;
   

   Node *VarNode = n -> childs -> bros -> bros;
   if(strcmp(VarNode -> token, "VarList") == 0){
        VarList(VarNode, newfun); // update argc and argv
   }

  
   // 函数定义都在最外部，即depth = 0;
   sym *f = NewSym(IDnode -> content, newfun, 0);

   if(CheckForConflict(f)){
            // redefine of function
            ErrorReport(4, n -> lineno);
            return ;
    }

    if(FuncConflictCheck(f)){
            // Inconsistence
            ErrorReport(19, n -> lineno);
            // 为避免多次报错,需要用isfundef记下已经定义过的函数
            if(isDef){
                
                for(int i = 0; i < FunDefNum; i++){
                    if (strcmp(funcs[i] -> name, f -> name) == 0)
                    {
                        isFunDef[i] = 1;
                        break;
                    }
                }
            }
            return ;
        }
    hash_insert(f, 0);
    if(isDef){
        for(int i = 0; i < FunDefNum; i++){
                    if (strcmp(funcs[i] -> name, f -> name) == 0)
                    {
                        isFunDef[i] = 1;
                        break;
                    }
                }
    }

    if(!isDef) {
        funcs[FunDefNum] = f;
        FunDefNum ++;
    }
   
}



void CompSt(Node *n, type *ReturnSpecifier){
    /*
    CompSt     ->      LC DefList StmtList RC
    */

   // 碰见LC，作用域改变，栈push
   StackPush();
   
   Node *Dlist = n -> childs -> bros;
   DefList(Dlist, NULL); // 此时是非structure作用域，ScopeSpecifier设为NULL
   Node *slist = Dlist -> bros;
   StmtList(slist, ReturnSpecifier);

   // 碰见RC，作用域改变，栈pop
   StackPop(0);
}


void VarList(Node *n, type *ScopeSpecifier){
    /*
    VarList    ->    ParamDec COMMA VarList
                   | ParamDec
    */

   // VarList 定义了函数体的形参，栈需要push
    StackPush();

    int argc = 0;

    Node *tmp = n -> childs;
    FieldList *cur = NULL;

    // VarList -> ParamDec
    FieldList *param = ParamDec(tmp);
    if(param){
    param -> nxt = ScopeSpecifier -> u.function -> argv;
    ScopeSpecifier -> u.function -> argv = param;
    argc ++;
    }

    // VarList -> ParamDec COMMA VarList
    while(tmp -> bros){
        tmp = tmp -> bros -> bros -> childs;
        param = ParamDec(tmp);
        if(param){
            param -> nxt = ScopeSpecifier -> u.function -> argv;
            ScopeSpecifier -> u.function -> argv = param;
            argc++;
        }
    }
    int isDef = ScopeSpecifier -> u.function -> isDef;
    ScopeSpecifier -> u.function -> argc = argc;
    if(isDef)
        // 函数形参需要进入下一层，因此不销毁
        StackPop(1);

    else StackPop(0); // 函数声明，形参在右括号结束后销毁
}


FieldList *ParamDec(Node *n){
    /*
    ParamDec    ->    Specifier VarDec
    */
    type *specifier = Specifier(n -> childs);
    sym *var = VarDec(n -> childs -> bros, specifier);
    if(CheckForConflict(var)){
        // Redefined Variable
        ErrorReport(3, n -> lineno);
        return NULL;
    }
    hash_insert(var, stack_top);
    FieldList *NewField = (FieldList *)malloc(sizeof(FieldList));
    char *newname = (char *)malloc(strlen(var -> name) + 1);
    strcpy(newname, var -> name);
    NewField -> name = newname;
    NewField -> tp = var -> tp;
    NewField -> nxt = NULL;
    return NewField;
}

void StmtList(Node *n, type *ReturnSpecifier){
    /*
    StmtList     ->      Stmt StmtList
                    |    e
    */

   Node * stmt = n -> childs;
   if(stmt){
    Stmt(stmt, ReturnSpecifier);
    StmtList(stmt -> bros, ReturnSpecifier);
   }

   return ;
}


void Stmt(Node *n, type *ReturnSpecifier){
    /*
    Stmt        ->        Exp SEMI
                    |     CompSt
                    |     RETURN Exp SEMI
                    |     IF LP Exp RP Stmt 
                    |     IF LP Exp RP Stmt ELSE Stmt
                    |     WHILE LP Exp RP Stmt
    */

    // Stmt  ->   Exp SEMI 
    // 调用Exp处理一下子结点即可
    if(strcmp(n -> childs -> token, "Exp") == 0) {
            type *exp = Exp(n -> childs);
            return ;
            }
    
    // Stmt   ->  CompSt
    else if(strcmp(n -> childs -> token, "CompSt") == 0)
            return CompSt(n -> childs, ReturnSpecifier);
    

    // Stmt   ->   RETURN Exp SEMI
    // 处理return报错
    else if(strcmp(n -> childs -> token, "RETURN") == 0){
            type *exp = Exp(n -> childs -> bros);
            if(!TypeCheck(ReturnSpecifier, exp))
                // return type dismatch
                ErrorReport(8, n -> lineno);
    }

    // Stmt  ->  IF LP Exp RP Stmt
    else if(strcmp(n -> childs -> token, "IF") == 0){
            Node *stmt = n -> childs -> bros -> bros -> bros -> bros;
            Node *exp = n -> childs -> bros -> bros;
            type *k = Exp(exp);
            if(k && k -> kind != BASIC){
                // if mismatch
                ErrorReport(7, n -> lineno);
            }
            else if(k && k -> kind == BASIC && k -> u.basic != 0)
                ErrorReport(7, n -> lineno);
            Stmt(stmt, ReturnSpecifier);
            // Stmt   ->   IF LP Exp RP Stmt ELSE Stmt
            if(stmt -> bros != NULL) Stmt(stmt -> bros -> bros, ReturnSpecifier);
    }
    // Stmt   ->   WHILE LP Exp RP Stmt
    else if(strcmp(n -> childs -> token, "WHILE") == 0){
            Node *stmt = n -> childs -> bros -> bros -> bros -> bros;
            Node *exp = n -> childs -> bros -> bros;
            type *k = Exp(exp);
            if(k && k -> kind != BASIC){
                // while mismatch
                ErrorReport(7, n -> lineno);
            }
            else if(k && k -> kind == BASIC && k -> u.basic != 0)
                ErrorReport(7, n -> lineno);
            Stmt(stmt, ReturnSpecifier);
    }

}

type *Exp(Node *n){
    /*
    Exp       ->        Exp ASSIGNOP Exp
                    |   Exp AND Exp
                    |   Exp OR Exp
                    |   Exp RELOP Exp
                    |   Exp PLUS Exp
                    |   Exp MINUS Exp
                    |   Exp STAR Exp
                    |   Exp DIV Exp
                    |   LP Exp RP
                    |   MINUS Exp %prec UMINUS
                    |   NOT Exp
                    |   ID LP Args RP
                    |   ID LP RP
                    |   Exp LB Exp RB    数组赋值可行
                    |   Exp DOT ID       结构体成员赋值可行
                    |   ID               变量赋值可行
                    |   INT
                    |   FLOAT
    */

   Node *c = n -> childs;
   if(strcmp(c -> token, "Exp") == 0){
        if(strcmp(c -> bros -> token, "LB") && strcmp(c -> bros -> token, "DOT")){
            // 二元运算关系

            type *p1 = Exp(c);
            type *p2 = Exp(c -> bros -> bros);

            Node *op = c -> bros;
            if(strcmp(op -> token, "ASSIGNOP") == 0){
                // Assignment
                if(strcmp(c -> childs -> token, "INT") == 0 ||
                         strcmp(c -> childs -> token, "FLOAT") == 0){
                    
                    // 赋值号左值报错
                    ErrorReport(6, c -> lineno);
                    return NULL;
                    }

                
                else {
                    Node *cc = c -> childs;
                    if((strcmp(cc -> token, "ID") == 0 && cc -> bros == NULL) ||
                       (strcmp(cc -> token, "Exp") == 0 && cc -> bros != NULL && strcmp(cc -> bros -> token, "DOT") == 0) ||
                       (strcmp(cc -> token, "Exp") == 0 && cc -> bros != NULL && strcmp(cc -> bros -> token, "LB") == 0))
                        { 
                            if(!TypeCheck(p1, p2)){
                                // 类型不匹配
                                ErrorReport(5, c -> lineno);
                                return NULL;
                            }
                            else
                                return p1;
                        }
                    
                    else {
                        // 左值报错
                        ErrorReport(6, c -> lineno);
                        return NULL;
                    }
                    
                }
            }
            /*
            Exp     ->     Exp AND Exp
                    ->     Exp OR Exp
                    ->     Exp RELOP Exp
                    ->     Exp PLUS Exp
                    ->     Exp MINUS Exp
                    ->     Exp STAR Exp
                    ->     Exp DIV Exp
            */
            else {
                // 考虑到每一行只能报一个错，若某个Exp返回NULL，意味着这行已经报错，不需要再报错
                // 非BASIC类型运算
                if(p1 && p2 && ((p1 -> kind != BASIC) || (p2 -> kind != BASIC))){
                    ErrorReport(7, c -> lineno);
                    return NULL;
                }
                else if(!TypeCheck(p1, p2)) {
                    // BASIC类型不匹配
                    ErrorReport(7, c -> lineno);
                    return NULL;
                }
                return p1;
            }
        }

    // Exp  ->  Exp LB Exp RB
    else if(strcmp(c -> bros -> token, "LB") == 0){
        // array
        type *p1 = Exp(c);
        type *p2 = Exp(c -> bros -> bros);
        if(p1 && p1 -> kind != ARRAY){
            // 非法使用[]
            ErrorReport(10, n -> lineno);
            return NULL;
        }
        else if(p2 && (p2 -> kind != BASIC || p2 -> u.basic != 0)){
            // 索引出现浮点数
            ErrorReport(12, n -> lineno);
            return NULL;
        }

        else return p1 -> u.arr -> entry;
    }

    // Exp   ->   Exp DOT ID
    else if(strcmp(c -> bros -> token, "DOT") == 0){
        type *p1 = Exp(c);
        if(p1 && p1 -> kind != STRUCTURE){
            // 非结构体类型使用DOT
            ErrorReport(13, n -> lineno);
            return NULL;
        }
        else if(p1 && p1 -> kind == STRUCTURE){
            Node *id = c -> bros -> bros;
            FieldList *members = p1 -> u.structure -> field;

            while(members){
                if(strcmp(members -> name, id -> content) == 0)
                    return members -> tp;

                members = members -> nxt;
            }

            assert(members == NULL);
            // 访问不存在的域
            ErrorReport(14, n -> lineno);
            return NULL;
        }
    }
   }

    

   


   /*
   Exp     ->     ID
   */

    else if(strcmp(c -> token, "ID") == 0){
        if(c -> bros == NULL) {
            sym *IDsym = hash_search(c -> content);
            if(IDsym == NULL){
                // Undefined Variable
                ErrorReport(1, c -> lineno);
                return NULL;
        }
        // 结构体名也不是变量
            else if(IDsym -> tp -> kind == STRUCTURE && 
                strcmp(IDsym -> name, IDsym -> tp -> u.structure -> sname) == 0){
                    ErrorReport(1, c -> lineno);
                    return NULL;
                }
            else return (IDsym -> tp);
    }
        else {
            sym *Funsym = hash_search(c -> content);

            if(Funsym == NULL){
                // Undefined Function
                ErrorReport(2, c -> lineno);
                return NULL;
            }

            else if(Funsym -> tp -> kind != FUNCTION){
                // Not a Function
                ErrorReport(11, c -> lineno);
                return NULL;
            }
            /*
            Exp   ->   ID LP Args RP
            */
            if(strcmp(c -> bros -> bros -> token , "Args") == 0){
                Node *args = c -> bros -> bros;
                Args(args, Funsym -> tp);
                return Funsym -> tp -> u.function -> ret;
            }
            /*
            Exp   ->   ID LP RP
            */
            else {
                if(Funsym -> tp -> u.function -> argc != 0){
                    // too few arguments
                    ErrorReport(9, c -> lineno);
                    return NULL;
                }
                else return Funsym -> tp -> u.function -> ret;
            }
        }

    }

    /*
    Exp      ->    LP Exp RP   
    */

    else if(strcmp(c -> token, "LP") == 0)
        return Exp(c -> bros);

    /*
    Exp      ->    MINUS Exp %prec UMINUS
    */

    else if(strcmp(c -> token, "MINUS") == 0)
    {
        type *exp = Exp(c -> bros);
        if(exp && exp -> kind != BASIC){
            // MINUS mismatch
            ErrorReport(7, c -> lineno);
            return NULL;
        }
        return exp;
    }

    /*
    Exp      ->   NOT Exp
    */

    else if(strcmp(c -> token, "NOT") == 0){
        type *exp = Exp(c -> bros);
        if(exp && (exp -> kind != BASIC || exp -> u.basic != 0)){
            // NOT mismatch
            ErrorReport(7, c -> lineno);
            return NULL;
        }
        return exp;
    }

    /*
    Exp      ->    INT
    */

    else if(strcmp(c -> token, "INT") == 0){
        return NewType(BASIC, 0);
    }

    /*
    Exp     ->    FLOAT
    */

    else if(strcmp(c -> token, "FLOAT") == 0){
        return NewType(BASIC, 1);
    }
}



void Args(Node *n, type *ScopeSpecifier){
    /*
    Args    ->    Exp COMMA Args
                | Exp
    */
   // WARNING : 
   // 在VarList的处理中，靠前的参数存在链表的后面，这里需要构建一个对Args的反向链表
   Node *tmp = n;
   FieldList *args = NULL;
   int arg_c = 0;

   while(tmp -> childs -> bros != NULL){
        Node *exp = tmp -> childs;
        type *r_exp = Exp(exp);
        FieldList *newentry = (FieldList *)malloc(sizeof(FieldList));
        newentry -> tp = r_exp;
        newentry -> nxt = args;
        args = newentry;

        arg_c ++;
        tmp = tmp -> childs -> bros -> bros;
   }
   // 少了一个参数，需要补上
   FieldList *newentry = (FieldList *)malloc(sizeof(FieldList));
   newentry -> tp = Exp(tmp -> childs);
   newentry -> nxt = args;
   args = newentry;
   arg_c ++;

   FieldList *FunField = ScopeSpecifier -> u.function -> argv;
   int argc = ScopeSpecifier -> u.function -> argc;

   if(arg_c != argc){
    // arguments number unmatched
        ErrorReport(9, n -> lineno);
        return ;
   }

   // 对比两个fieldlist
   else {
        while(args){
            if(!TypeCheck(args -> tp, FunField -> tp)){
                // arguments type unmatched
                ErrorReport(9, n -> lineno);
                return ;
            }
            args = args -> nxt;
            FunField = FunField -> nxt;
        }
        assert(args == NULL);
        assert(FunField == NULL);
   }
   return ;
}