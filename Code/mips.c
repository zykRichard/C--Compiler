#include "mips.h"

extern InterCodeList* InterCodes;
VarInfo *VarInfoRec;
int StackOffset = 0;

void MIPSGeneratorInit(){
    VarInfoRec = (VarInfo *)malloc(InterCodes -> TempVarNum_t * sizeof(VarInfo));
}

int Find_Op_ID(Operand *op){
    if(op -> kind == CONSTANT_OP) return -1;
    else return atoi((op -> id.name) + 1);
}

void VarLoad(int reg_no, int var_no){

}

void VarStore(int reg_no, int var_no){

}

void MIPSGenerator(FILE *fp){
    // result : t0 
    // op1 : t1
    // op2 : t2

    Intercode *curic = InterCodes -> head;
    while(curic -> next != NULL){
        IR *IRcode = curic -> IRcode;

        switch (IRcode -> kind)
        {
        case LABEL_IR:
            fprintf(fp, "L%s:", IRcode -> IR_Type.Single.op -> id.name);
            break;

        case ASSIGN_IR:
            if(IRcode -> IR_Type.Assign.right -> kind == CONSTANT_OP){
                fprintf(fp, "li $t0, %d\n", IRcode -> IR_Type.Assign.right -> id.value);
            }
            else {
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Assign.right));
                fprintf(fp, "move $t0, $t1\n");
            }
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
            break;
        
        case ADD_IR:
            if(IRcode -> IR_Type.Binary.op1 -> kind == VARIABLE_OP)
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Binary.op1));
            else fprintf(fp, "li $t1, %d\n", IRcode -> IR_Type.Binary.op1->id.value);
            if(IRcode -> IR_Type.Binary.op2 -> kind == VARIABLE_OP)
                VarLoad(2, Find_Op_ID(IRcode -> IR_Type.Binary.op2));
            else fprintf(fp, "li $t2, %d\n", IRcode -> IR_Type.Binary.op2->id.value);
        
        default:
            break;
        }
    }
}