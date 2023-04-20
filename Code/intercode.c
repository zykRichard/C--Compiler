#include "intercode.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// operand
Operand *NewOperand(OpKind kind, ...){
    va_list vaList;
    va_start(vaList, kind);

    Operand *NewOP = (Operand*)malloc(sizeof(Operand));
    NewOP -> kind = kind;
    switch (kind)
    {
    case VARIABLE_OP:
    case ADDRESS_OP:
    case LABEL_OP:
    case FUNCTION_OP:
    case RELOP_OP:
        NewOP -> id.name = va_arg(vaList, char *);
        break;
    case CONSTANT_OP:
        NewOP -> id.value = va_arg(vaList, int);
        break;
     
    default:
        assert(0);
        break;
    }
    return NewOP;
}


void printOP(Operand *op){
    assert(op);
    switch(op -> kind){
        case VARIABLE_OP:
        case ADDRESS_OP:
        case LABEL_OP:
        case FUNCTION_OP:
        case RELOP_OP:
            printf("%s", op -> id.name);
            break;
        
        case CONSTANT_OP:
            printf("#%d", op -> id.value);
            break;

        default: 
            assert(0);
            break;
    }
}

// Intermediate Representation
IR *NewIR(IrKind kind, ...){
    va_list VaList;
    va_start(VaList, kind);

    IR *newIR = (IR*)malloc(sizeof(IR));
    switch (kind)
    {
    case LABEL_IR:
    case FUNCTION_IR:
    case GOTO_IR:
    case RETURN_IR:
    case ARG_IR:
    case PARAM_IR:
    case READ_IR:
    case WRITE_IR:
        // single OP
        newIR -> IR_Type.Single.op = va_arg(VaList, Operand*);
        break;
    
    case ASSIGN_IR:
    case GADDR_IR:
    case RADDR_IR:
    case WADDR_IR:
    case CALL_IR:
        // assign OP
        newIR -> IR_Type.Assign.left = va_arg(VaList, Operand*);
        newIR -> IR_Type.Assign.right = va_arg(VaList, Operand*);
        break;

    case ADD_IR:
    case SUB_IR:
    case MUL_IR:
    case DIV_IR:
        // binary OP
        newIR -> IR_Type.Binary.result = va_arg(VaList, Operand*);
        newIR -> IR_Type.Binary.op1 = va_arg(VaList, Operand*);
        newIR -> IR_Type.Binary.op2 = va_arg(VaList, Operand*);
        break;

    case IF_IR:
        // if-goto OP
        newIR -> IR_Type.IfGoto.x = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.relop = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.y = va_arg(VaList, Operand*);
        newIR -> IR_Type.IfGoto.z = va_arg(VaList, Operand*);
        break;
    
    case DEC_IR:
        // dec OP
        newIR -> IR_Type.Dec.x = va_arg(VaList, Operand*);
        newIR -> IR_Type.Dec.sz = va_arg(VaList, int);
        break;

    default:
        assert(0);
        break;
    }
    return NewIR;
}


void printIR(IR *ir){
    switch(ir -> kind){
        case LABEL_IR :
            // LABEL x :
            printf("LABEL ");
            printOP(ir -> IR_Type.Single.op);
            printf(" :");
            break;
        case FUNCTION_IR:
            // FUNCTION f :
            printf("FUNCTION ");
            printOP(ir -> IR_Type.Single.op);
            printf(" :");
            break;
        case ASSIGN_IR:
            // x := y
            printOP(ir -> IR_Type.Assign.left);
            printf(" := ");
            printOP(ir -> IR_Type.Assign.right);
            break;
        case ADD_IR:
            // x := y + z
            printOP(ir -> IR_Type.Binary.result);
            printf(" := ");
            printOP(ir -> IR_Type.Binary.op1);
            printf(" + ");
            printOP(ir -> IR_Type.Binary.op2);
            break;
        case SUB_IR:
            // x := y - z
            printOP(ir -> IR_Type.Binary.result);
            printf(" := ");
            printOP(ir -> IR_Type.Binary.op1);
            printf(" - ");
            printOP(ir -> IR_Type.Binary.op2);
            break;
        case MUL_IR:
            // x := y * z
            printOP(ir -> IR_Type.Binary.result);
            printf(" := ");
            printOP(ir -> IR_Type.Binary.op1);
            printf(" * ");
            printOP(ir -> IR_Type.Binary.op2);
            break;
        case DIV_IR:
            // x := y / z
            printOP(ir -> IR_Type.Binary.result);
            printf(" := ");
            printOP(ir -> IR_Type.Binary.op1);
            printf(" / ");
            printOP(ir -> IR_Type.Binary.op2);
            break;
        case GADDR_IR:
            // x := &y 
            printOP(ir -> IR_Type.Assign.left);
            printf(" := &");
            printOP(ir -> IR_Type.Assign.right);
            break;
        case RADDR_IR:
            // x := *y
            printOP(ir -> IR_Type.Assign.left);
            printf(" := *");
            printOP(ir -> IR_Type.Assign.right);
            break;
        case WADDR_IR:
            // \*x := y
            printf("*");
            printOP(ir -> IR_Type.Assign.left);
            printf(" := ");
            printOP(ir -> IR_Type.Assign.right);
            break;
        case GOTO_IR:
            // GOTO x
            printf("GOTO ");
            printOP(ir -> IR_Type.Single.op);
            break;
        case IF_IR:
            // IF x [relop] y GOTO z
            printf("IF ");
            printOP(ir -> IR_Type.IfGoto.x);
            printf(" ");
            printOP(ir -> IR_Type.IfGoto.relop);
            printf(" ");
            printOP(ir -> IR_Type.IfGoto.y);
            printf(" GOTO ");
            printOP(ir -> IR_Type.IfGoto.z);
            break;
        case RETURN_IR: 
            // RETURN x
            printf("RETURN ");
            printOP(ir -> IR_Type.Single.op);
            break;
        case DEC_IR:
            // DEC x [size]
            printf("DEC ");
            printOP(ir -> IR_Type.Dec.x);
            printf(" %d", ir -> IR_Type.Dec.sz);
            break;
        case ARG_IR:
            // ARG x
            printf("ARG ");
            printOP(ir -> IR_Type.Single.op);
            break;
        case CALL_IR:
            // x := CALL f
            printOP(ir -> IR_Type.Assign.left);
            printf(" := CALL ");
            printOP(ir -> IR_Type.Assign.right);
            break;
        case PARAM_IR:
            // PARAM x
            printf("PARAM ");
            printOP(ir -> IR_Type.Single.op);
            break;
        case READ_IR:
            // READ x
            printf("READ ");
            printOP(ir -> IR_Type.Single.op);
            break;
        case WRITE_IR:
            // WRITE x
            printf("WRITE ");
            printOP(ir -> IR_Type.Single.op);
            break; 

        default : assert(0); break;
    }
}

// intermediate codes
Intercode *NewInterCode(IR *ir){
    Intercode *newic = (Intercode*)malloc(sizeof(Intercode));
    newic -> IRcode = ir;
    newic -> next = NULL;
    newic -> prev = NULL;
    return newic;
}

void InterCodeInsert(Intercode *ic, InterCodeList *iclist){
    // 尾插法
    if(iclist -> head == NULL){
        iclist -> head = ic;
        iclist -> tail = ic;
    }
    else {
        iclist -> tail -> next = ic;
        ic -> prev = iclist -> tail;
        iclist -> tail = ic;
    }

    assert(iclist -> head);
    assert(iclist -> tail);
}

void printInterCode(Intercode *ic){
    assert(ic);
    assert(ic -> IRcode);
    printIR(ic -> IRcode);
}

// intercodes list
InterCodeList *NewInterCodeList(){
    InterCodeList *NewICL = (InterCodeList *)malloc(sizeof(InterCodeList));
    NewICL -> head = NULL;
    NewICL -> tail = NULL;
    NewICL -> TempLabelNum = 0;
    NewICL -> TempVarNum = 0;
    return NewICL;
}

void InterCodesPrint(InterCodeList *iclist){
    assert(iclist);
    Intercode *ic = iclist -> head;

    while(ic){
        printInterCode(ic);
        ic = ic -> next;
    }

}