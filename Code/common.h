#ifndef COMMON_H
#define COMMON_H

// Lab 1 : AST Node 
typedef enum nodetype{
    TOKEN_ID,          // ID
    TOKEN_TYPE,        // INT|FLOAT
    TOKEN_INT,         // int value
    TOKEN_FLOAT,       // float value
    TOKEN_OTHER,       // OTHER TOKENS
    LANG               // NOT A TOKEN
} NodeType;

typedef enum nodekind{
    BASIC,      // 0 for int ; 1 for float
    ARRAY,      // array
    STRUCTURE,  // structure
    FUNCTION
} NodeKind;

typedef enum _opkind {
        VARIABLE_OP,
        CONSTANT_OP,
        ADDRESS_OP,
        LABEL_OP,
        FUNCTION_OP,
        RELOP_OP,
} OpKind;

typedef enum _irkind {
     LABEL_IR,         // single OP
     FUNCTION_IR,      // single OP
     ASSIGN_IR,        // assign OP
     ADD_IR,           // binary OP
     SUB_IR,           // binary OP
     MUL_IR,           // binary OP
     DIV_IR,           // binary OP
     GADDR_IR,         // assign OP
     RADDR_IR,         // assign OP
     WADDR_IR,         // assign OP
     GOTO_IR,          // single OP
     IF_IR,            // if-goto OP
     RETURN_IR,        // single OP
     DEC_IR,           // dec OP
     ARG_IR,           // single OP
     CALL_IR,          // assign OP
     PARAM_IR,         // single OP
     READ_IR,          // single OP
     WRITE_IR,         // single OP
} IrKind;


typedef struct Node {
    union {
        int int_val;
        float float_val;
    };
    int lineno;
    char *content;
    char *token;
    NodeType type;
    /*  From left to right */
    struct Node *bros;
    struct Node *childs; 
} Node;


// Lab 2 : Semantic Analysis
typedef struct FieldList_t {
    char *name;
    struct type_t *tp;
    struct FieldList_t *nxt;
}FieldList;

typedef struct array_t {
    struct type_t *entry;
    int sz;
} array;


typedef struct structure_t
{
    char *sname;
    struct FieldList_t *field;
    int isAnony;
} structure;

typedef struct fun_t {
    int argc;
    struct FieldList_t *argv;
    struct type_t *ret;
    int lineno;
    int isDef;
} fun;

typedef struct type_t {
    NodeKind kind;
    union {
        int basic; // 0 for int && 1 for float

        struct array_t *arr;
        /*
        type *entry;
        int dim;
        */

        struct structure_t *structure;

        struct fun_t *function;
    } u;
} type;


/*
stack : field1 -> field2 -> ...  controlled by nxt_field
hash : sym1 -> sym2 -> ... controlled by nxt_sym
*/
typedef struct sym_t {
    char *name;
    struct type_t *tp;

    struct sym_t *nxt_field;
    struct sym_t *nxt_sym;

    unsigned int StackDepth;
    int TempVar;
    int isAddr;
} sym;




// global function
void TreeTraverse(Node *root);
void LexicalAnalysis();


// Lab 3 : Intercode 
typedef struct _operand {
    OpKind kind;

    union {
        int value;
        char *name;
    } id;

    type *tp;
    int no;
    int temp; // 1 for t variable; 0 for v variable
} Operand;

typedef struct _ir {
    IrKind kind;

    union 
    {
        struct {Operand *left; Operand *right; } Assign;
        struct {Operand *op;} Single;
        struct {Operand *result; Operand *op1; Operand *op2;} Binary;
        struct {Operand *x; Operand *relop; Operand *y; Operand *z;} IfGoto;
        struct {Operand *x; int sz;} Dec;
        
    } IR_Type;
    
    
} IR;

// Linklist Intercode Representation
typedef struct _intercode {
    IR *IRcode;
    struct _intercode *prev, *next;
} Intercode;


// InterCodes LinkList
typedef struct _intercodelist {
    Intercode *head;
    Intercode *tail;

    /*
    PARAM ARG 用 v
    其余用t
    */
   
    int TempVarNum_t; // t variables
    int TempLabelNum; // label 
} InterCodeList;

// Args
typedef struct arg{
    Operand *op;
    struct arg *next;
} Arg;

typedef struct arglist{
    Arg *head;
    Arg *tail;
} Arglist;

void InterCodesGenerator(Node *AST, char *filename);
void ASMGeneratorMIPS(char *filename);

// lab4: MIPS Generator
typedef struct VarInfo{
    int offset;
    int instack;
} VarInfo;

// lab5 : optimize
typedef struct DAGnode{
    struct DAGnode* lchild;
    struct DAGnode* rchild;
    int cnt;
} DAGnode;

typedef struct BB{
    Intercode *start;
    Intercode *end;
    struct BB *prev;
    struct BB *nxt;
}BB;

DAGnode* NodeHash;
void optimize(char *filename);
#endif /* C6A9B744_6E0F_4B2E_AE52_EAD5BB167546 */
