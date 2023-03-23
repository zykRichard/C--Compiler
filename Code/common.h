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


#endif /* C6A9B744_6E0F_4B2E_AE52_EAD5BB167546 */
