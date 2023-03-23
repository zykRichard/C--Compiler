#ifndef COMMON_H
#define COMMON_H

typedef struct Node {
    union {
        int int_val;
        float float_val;
    };
    int lineno;
    char *content;
    char *token;
    /*  From left to right */
    struct Node *bros;
    struct Node *childs; 
} Node;


#endif /* C6A9B744_6E0F_4B2E_AE52_EAD5BB167546 */
