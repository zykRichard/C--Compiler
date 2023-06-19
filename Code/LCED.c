#include "optimize.h"

IRentry *NewIRentry(IR *ir){
    IRentry *entry = (IRentry *)malloc(sizeof(IRentry));
    entry -> ir = ir;
    entry -> nxt = NULL;
    entry -> prev = NULL;
}

IRentryList *NewIRentryList(){
    IRentryList *irlist = (IRentryList *)malloc(sizeof(IRentryList));
    irlist -> head = NULL;
    irlist -> tail = NULL;
}

void IRentryInsert(IRentry *entry, IRentryList *irlist){
    // 尾插法
    if(irlist -> head == NULL){
        irlist -> head = entry;
        irlist -> tail = entry;
    }
    else {
        irlist -> tail -> nxt = entry;
        entry -> prev = irlist -> tail;
        irlist -> tail = entry;
    }
}

int CheckIndentity(Operand *op1, Operand *op2){
    if((op1 -> kind == op2 -> kind) && (op1 -> kind != CONSTANT_OP)){
        if(op1 -> no == op2 -> no && op1->temp == op2->temp)
            return 1;
        else return 0;
    }
    else if((op1 -> kind == op2 -> kind) && (op1 -> kind == CONSTANT_OP))
        return (op1 -> id.value == op2 -> id.value);
    
    else if(op1 -> kind != op2 -> kind)
        return 0;
}