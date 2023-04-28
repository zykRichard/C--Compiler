#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "semantic.h"


sym *hash_table[0x3fff] = {0};
sym *stack[0x3fff] = {0};
unsigned int stack_top = 0;


// member function for sym structure
// hash table
void TableInit(){
    memset(hash_table, 0, sizeof(hash_table));
    memset(stack, 0, sizeof(stack));

    // add read function :
    // int read ();
    
    type *rd = NewType(FUNCTION, 0, NULL, 
                    NewType(BASIC, 0), -1, 1);
    
    sym *read = NewSym("read", rd, 0);
    hash_insert(read, 0);
    // add write function :
    // int write (int arg);

    FieldList *argv = (FieldList *)malloc(sizeof(FieldList));
    argv -> name = "c";
    argv -> tp = NewType(BASIC, 0);
    argv -> nxt = NULL;

    type *wt = NewType(FUNCTION, 1, argv, 
                    NewType(BASIC, 0), -1, 1);
    
    sym *write = NewSym("write", wt, 0);
    hash_insert(write, 0);
    
}

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
            // 越是内层的符号越是排在hash表的前面，因此第一次返回的一定是最靠内层定义的符号
             return hash_head;
        hash_head = hash_head -> nxt_sym;
    }
    return NULL;
}


unsigned int GetTypeSz(type *tp){
    assert(tp -> kind != FUNCTION);
    switch(tp -> kind){
        case BASIC: 
            return 4;
        case ARRAY:
            return (tp -> u.arr -> sz) * GetTypeSz(tp -> u.arr -> entry);
        case STRUCTURE:{
            FieldList *members = tp -> u.structure -> field;
            unsigned int sz = 0;
            while(members){
                sz = sz + GetTypeSz(members -> tp); // 一定是4的倍数
                members = members -> nxt;
            }
            return sz;
        } 
    }
}

void StackPush(){

    stack_top ++;
    //stack[stack_top] = NULL;
}

void StackPop(int DoNotTearDown){
    if(!DoNotTearDown){
    sym *ToBeNull = stack[stack_top];
    while(ToBeNull && ToBeNull -> StackDepth != (unsigned int) (-1)){
        ToBeNull -> StackDepth = (unsigned int) (-1);
        ToBeNull = ToBeNull -> nxt_field;
    } 
    }
    stack_top --;
    
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
        f -> lineno = va_arg(vaList, int);
        f -> isDef = va_arg(vaList, int);
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
    s -> TempVar = 0;
    return s;
}

// Assistance
int CheckForConflict(sym *s){
    sym *ref = hash_search(s -> name); 
    if(ref == NULL) return 0;
    while(ref){
        while(ref && ref -> StackDepth == (unsigned int)(-1)) ref = ref -> nxt_sym;
        if(ref == NULL) return 0;
        if(strcmp(ref -> name, s -> name) == 0){
           // 一定冲突的三种特殊情况特判
           // 1. ref 或 s 其中一个是结构体名
           if((ref -> tp -> kind == STRUCTURE && strcmp(ref -> name, ref -> tp -> u.structure -> sname) == 0)
            || (s -> tp -> kind == STRUCTURE && strcmp(ref -> name, ref -> tp -> u.structure -> sname) == 0))
                return 1;
           // 2. ref 和 s 都是 function 且都是isdef
           if(ref -> tp -> kind == FUNCTION && s -> tp -> kind == FUNCTION
           && ref -> tp -> u.function -> isDef && ref -> tp -> u.function -> isDef)
                return 1;

            // 3. ref 和 s 中有一个是function 另一个不是
            if((ref -> tp -> kind == FUNCTION && s -> tp -> kind != FUNCTION)
            || (ref -> tp -> kind != FUNCTION && s -> tp -> kind == FUNCTION))
                return 1;

            // 其他情况:
            // 直接比较作用域
            assert(ref -> StackDepth != (unsigned int)(-1));
            // 函数体除外
            if(s -> tp -> kind != FUNCTION)
                if(ref -> StackDepth >= s -> StackDepth) return 1;
        }

        ref = ref -> nxt_sym;
        
    }
    return 0;
}


int FuncConflictCheck(sym *func){
    sym *ref = hash_search(func -> name);
    if(ref == NULL) return 0;
    while(ref && ref -> StackDepth == (unsigned int)(-1)) ref = ref -> nxt_sym;
    if(ref == NULL) return 0;
    if(func -> tp -> u.function -> isDef == 1){
        // 只关心定义声明不匹配
        assert(ref -> tp -> kind == FUNCTION);
        assert(ref -> tp -> u.function -> isDef == 0);
        
        fun *f = func -> tp -> u.function;
        fun *f_ref = ref -> tp -> u.function;
        if(f -> argc != f_ref -> argc) return 1;
        if(!TypeCheck(f -> ret, f_ref -> ret)) return 1;
        FieldList *argv1 = f -> argv;
        FieldList *argv2 = f_ref -> argv;
        while(argv1)
        {
            if(!TypeCheck(argv1 -> tp, argv2 -> tp))
                return 1;
            argv1 = argv1 -> nxt;
            argv2 = argv2 -> nxt;
        }
        assert(argv1 == NULL);
        assert(argv2 == NULL);

        return 0;
    }

    else if(func -> tp -> u.function -> isDef == 0){
        // 只关心多次声明冲突或声明定义不匹配
        assert(ref -> tp -> kind == FUNCTION);

        fun *f = func -> tp -> u.function;
        fun *f_ref = ref -> tp -> u.function;
        if(f -> argc != f_ref -> argc) return 1;
        if(!TypeCheck(f -> ret, f_ref -> ret)) return 1;
        FieldList *argv1 = f -> argv;
        FieldList *argv2 = f_ref -> argv;
        while(argv1)
        {
            if(!TypeCheck(argv1 -> tp, argv2 -> tp))
                return 1;
            argv1 = argv1 -> nxt;
            argv2 = argv2 -> nxt;
        }
        assert(argv1 == NULL);
        assert(argv2 == NULL);

        return 0;

    }
    return 1;
}

int StructTypeCheck(type *t1, type *t2){
    assert(t1 -> kind == STRUCTURE && t2 -> kind == STRUCTURE);
    structure *s1 = t1 -> u.structure;
    structure *s2 = t2 -> u.structure;
    FieldList *cur1 = s1 -> field;
    FieldList *cur2 = s2 -> field;
    while(cur1 && cur2){
        if(!TypeCheck(cur1 -> tp, cur2 -> tp))
            return 0;
        cur1 = cur1 -> nxt;
        cur2 = cur2 -> nxt;
    }
    if(cur1 == NULL && cur2 == NULL)
        return 1;
    else return 0;
}




int TypeCheck(type *t1, type *t2){
    if(t1 == NULL || t2 == NULL) return 1;
    if(t1 -> kind != t2 -> kind) return 0;
    else {
        switch(t1 -> kind){
            case BASIC:
                return t1 -> u.basic == t2 -> u.basic;
            case ARRAY:
                return TypeCheck(t1 -> u.arr -> entry, t2 -> u.arr -> entry);
            case STRUCTURE:
                return StructTypeCheck(t1, t2);
            case FUNCTION:
                return 0;

        }
    }
}