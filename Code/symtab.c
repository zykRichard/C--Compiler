#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"


sym *hash_table[0x3fff] = {0};
sym *stack[0x3fff] = {0};
int stack_top = 0;


// member function for sym structure
// hash table
unsigned int hash_pjw(char *name){
    unsigned int val = 0, i;
    for (; *name; ++name){
        val = (val << 2) + *name;
        if(i = val & ~0x3fff)
            val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

void hash_insert(sym *s, int depth){
    assert(s);
    unsigned int pos = hash_pjw(s -> name);
    s -> nxt_sym = hash_table[pos];
    hash_table[pos] = s;
    s -> nxt_field = stack[depth];
    stack[depth] = s;
}

sym* hash_search(char *name){
    int pos = hash_pjw(name);
    sym *hash_head = hash_table[pos];
    while(hash_head){
        if((strcmp(hash_head -> name, name) == 0) && (hash_head -> StackDepth <= stack_top))
             return hash_head;
        hash_head = hash_head -> nxt_sym;
    }
    return NULL;
}


// Generation new node
type *NewType(NodeKind kind, ...){
    type *tp = (type *)malloc(sizeof(type));
    assert(tp);
    tp -> kind = kind;
    va_list vaList;

    switch (kind)
    {
    case BASIC:
        va_start(vaList, kind);
        tp -> u.basic = va_arg(vaList, int); 
        break;
    
    case ARRAY:
        va_start(vaList, kind);
        array *newarr = (array *)malloc(sizeof(array));
        newarr -> entry = va_arg(vaList, type*);
        newarr -> sz = va_arg(vaList, int);
        tp -> u.arr = newarr;
        break;

    case STRUCTURE:
        va_start(vaList, kind);
        structure *st = (structure *)malloc(sizeof(structure));
        char *sname = va_arg(vaList, char*);
        st -> sname = (char *)malloc(strlen(sname) + 1);
        strcpy(st -> sname, sname);
        st -> field = va_arg(vaList, FieldList*);
        st -> isAnony = va_arg(vaList, int);
        tp -> u.structure = st;
        break;

    case FUNCTION:
        va_start(vaList, kind);
        fun *f = (fun*)malloc(sizeof(fun));
        f -> argc = va_arg(vaList, int);
        f -> argv = va_arg(vaList, FieldList*);
        f -> ret = va_arg(vaList, type*);
        tp -> u.function = f;
        break;

    default:
        assert(0);
        break;
    }
    va_end(vaList);
    return tp;
}


sym *NewSym(char *name, type* tp, int depth){
    sym *s = (sym*)malloc(sizeof(sym));
    assert(s);
    s -> nxt_field = NULL;
    s -> nxt_sym = NULL;
    s -> name = (char *)malloc(strlen(name) + 1);
    strcpy(s -> name, name);
    s -> tp = tp;
    s -> StackDepth = depth;
    return s;
}

// Assistance
int CheckForConflict(sym *s){
    sym *ref = hash_search(s -> name); 
    if(ref == NULL) return 0;
    while(ref){
        if(strcmp(ref -> name, s -> name) == 0){
            if(ref -> tp -> kind == STRUCTURE || 
                s -> tp -> kind == STRUCTURE)
                return 1;

            else if(ref -> tp -> kind == FUNCTION ||
                    s -> tp -> kind == FUNCTION)
                return 1;

            else if(ref -> StackDepth <= stack_top)
                return 1;
        }

        ref = ref -> nxt_sym;
    }
    return 0;
}

int TypeCheck(type *t1, type *t2){
    if(t1 == NULL || t2 == NULL) return 1;
    if(t1 -> kind == FUNCTION || 
            t2 -> kind == FUNCTION) return 0;
    if(t1 -> kind != t2 -> kind) return 0;
    else {
        switch(t1 -> kind){
            case BASIC:
                return t1 -> u.basic == t2 -> u.basic;
            case ARRAY:
                return TypeCheck(t1 -> u.arr -> entry, t2 -> u.arr -> entry);
            case STRUCTURE:
                return !strcmp(t1 -> u.structure -> sname, 
                                    t2 -> u.structure -> sname);

        }
    }
}