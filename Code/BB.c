#include "optimize.h"


BB *NewBB(){
    BB *ret = (BB *)malloc(sizeof(BB));
    ret -> start = NULL;
    ret -> end = NULL;
    ret -> nxt = NULL;
    ret -> prev = NULL;
    return ret;
}

BB *BB_Partition(InterCodeList *iclist){
    int is_leader = 1;
    int is_head = 1;
    Intercode *ic = iclist -> head;
    BB *ret = NewBB();
    BB *cur = ret;
    while(ic != NULL){
        
        switch (ic -> IRcode -> kind)
        {
        case LABEL_IR:
        case FUNCTION_IR:
        case GOTO_IR:
        case IF_IR:
            if(!is_leader){
                cur -> end = ic;
                is_leader = 1;
            }
            break;
        
        default:
            {
                if(is_leader){
                    if(is_head){
                        cur -> start = ic;
                        is_head = 0;
                    }
                    else {
                        BB *newbb = NewBB();
                        cur -> nxt = newbb;
                        newbb -> prev = cur;
                        newbb -> start = ic;
                        cur = newbb;
                        
                    }
                    is_leader = 0;
                }
            }
        }
        ic = ic -> next;
    }
    if(cur -> end == NULL)
        cur -> end = iclist -> tail;
    return ret;
}

void printBB(FILE *fp, BB *block){
    BB *cur = block;
    int cnt = 0;
    while(cur != NULL){
        fprintf(fp, "BLOCK %d\n", cnt);
        printInterCode(fp, cur -> start);
        printInterCode(fp, cur ->end);
        cur = cur -> nxt;
    }
    return ;
}