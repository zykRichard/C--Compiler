#ifndef COMMON_H
#define COMMON_H


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

    int StackDepth;
} sym;




// global function
void TreeTraverse(Node *root);

#endif /* C6A9B744_6E0F_4B2E_AE52_EAD5BB167546 */
