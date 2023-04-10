#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"

char *err_msg[20] = {
    " reserved ",
    "Undefined variables",
    "Undefined function" ,
    "Redefined variable" ,
    "Redefined function" ,
    "Type mismatch for assignment" ,
    "The left-hand side of an assignment must be a variable" ,
    "Type mismatch for operands" ,
    "Type mismatch for return" ,
    "Function is not applicable for arguments" ,
    "Variable is not an array" ,
    "Variable is not a function" ,
    "Index of an array must be an integer" ,
    "Illegal use of \".\"" ,
    "Non-existent field" ,
    "Redefined member inside structure" ,
    "Duplicated name" ,
    "Undefined structure" ,
    "Undefined function" ,
    "Inconsistence of function declaration"
};



static inline void ErrorReport(int code, int lineno){
    printf("Error type %d at Line %d: %s", code, lineno, err_msg[code]);
}


// Symbol Table Generation
// directly check every ExtDef


void ExtDef(Node *n){
    /*
    ExtDef     ->    Specifier ExtDecList SEMI
                 |   Specifier SEMI
                 |   Specifier FunDec CompSt
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
        FunDec(sp -> bros, specifier);
        CompSt(sp -> bros -> bros, specifier);
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
            memset(sname, 0, sizeof(sname));
            isAnony = 1;        
        }
        // OptTag -> ID
        else {
            char *IDname = st -> childs -> content;
            sname = (char *)malloc(strlen(IDname) + 1);
            strcpy(sname, IDname);
            isAnony = 0;
        }

        Node *Dlist = st -> bros -> bros;
        type *StructSpecifier = NewType(STRUCTURE, sname, NULL, isAnony);
        DefList(Dlist, StructSpecifier); // add FieldList to StructSpecifier

        // TODO : Conflict detection and Symbol insertion

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
        hash_insert(newsym);

    return;
   }

   else {
    sym *newsym = VarDec(var, specifier);

    if(CheckForConflict(newsym))
        ErrorReport(3, var -> lineno);
    else 
        hash_insert(newsym);
    
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
        return NewSym(subsym -> name, 
            NewType(ARRAY, subsym -> tp, sz), stack_top);

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

    if(strcmp(n -> childs -> bros -> token, "ASSIGNOP") == 0){
        // initialized variable in structure field
        ErrorReport(15, n -> childs -> bros -> lineno);
        return ;
    }

    // 创建Fieldlist
    FieldList *NewField = (FieldList *)malloc(sizeof(FieldList));
    char *newname = (char *)malloc(strlen(member -> name) + 1);
    strcpy(newname, member -> name);
    NewField -> name = newname;
    NewField -> tp = member -> tp;
    NewField -> nxt = ScopeSpecifier -> u.structure -> field;
   }

   else {
    // TODO : function scope
    sym *member = VarDec(n -> childs, specifier);
    if(CheckForConflict(member)){
        // redefine variable in funtion 
        ErrorReport(3, n -> lineno);
        return;
    }

    if(strcmp(n -> childs -> bros -> token, "ASSIGNOP") == 0){
        Node *exp = n -> childs -> bros -> bros;
        type *to_copy = Exp(exp);
        member -> tp = to_copy;  // assignment
    }

    hash_insert(member, stack_top);
   }

   return ;
}



void FunDec(Node *n, type* ReturnSpecifier){
    /*
    FunDec       ->     ID LP VarList RP
                    |   ID LP RP
    */
   type *newfun = NewType(FUNCTION, 0, NULL, ReturnSpecifier);
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

   hash_insert(f, 0);
}



void CompSt(Node *n, type *ReturnSpecifier){
    /*
    CompSt     ->      LC DefList StmtList RC
    */

   // 碰见LC，作用域改变，栈push
   stack_top ++;
   

   // 碰见RC，作用域改变，栈pop
   stack_top --;
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
                    |   LP Exp Rp
                    |   MINUS Exp %prec UMINUS
                    |   NOT Exp
                    |   ID LP Args RP
                    |   ID LP RP
                    |   Exp LB Exp RB
                    |   Exp DOT ID
                    |   ID
                    |   INT
                    |   FLOAT
    */

   Node *c = n -> childs;
   if(strcmp(c -> token, "Exp") == 0){
        if(strcmp(c -> bros -> token, "LB") && strcmp(c -> bros -> token, "DOT")){
            // 二元运算关系

            // type *p1 = Exp(c);
            // type *p2 = Exp(c -> bros -> bros);

            Node *op = c -> bros;
            if(strcmp(op -> token, "ASSIGNOP") == 0){
                // Assignment
                if(strcmp(c -> childs -> token, "INT") == 0 ||
                    strcmp(c -> childs -> token, "FLOAT")){
                    // 赋值号左值报错
                    ErrorReport(6, c -> lineno);
                    return ;
                    }
                
                else if(1){
                    // TODO : other operator
                }
            }
        }

   }


   /*
   Exp     ->     ID
   */

    else if(strcmp(c -> token, "ID") == 0){
        sym *IDsym = hash_search(c -> content);
        if(IDsym == NULL){
            // Undefined Variable
            ErrorReport(1, c -> lineno);
            return ;
        }
        else return (IDsym -> tp);
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