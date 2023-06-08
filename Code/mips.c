#include "mips.h"

extern InterCodeList* InterCodes;
VarInfo *VarInfoRec;
FILE *fp;

int FunctionNum = 1024;
// 0 for main; 1 for read; 2 for write

int CurFunNum = 3;
int *FunStackSZ;
int NArgs = 0;
int Param = 0;
char *FuncTable[1024];
int *DecSize;
int *vis;
int StackOffset = 0;

void ASMGeneratorMIPS(char *filename){
    fp = fopen(filename, "w+");
    MIPSGeneratorInit();
    MIPSGenerator();
    fclose(fp);
}

void before_func(int funcid){
    fprintf(fp, "move $t3, $sp\n"); // not used
    fprintf(fp, "addi $sp, $sp, -8\n");
    fprintf(fp, "sw $fp, 4($sp)\n");
    fprintf(fp, "sw $ra, 0($sp)\n");
    fprintf(fp, "move $fp, $sp\n");
    fprintf(fp, "addi $sp, $sp, -%d\n", FunStackSZ[funcid] );

}

void after_func(int ifmain, int ArgsNum){
    fprintf(fp, "move $sp, $fp\n");
    fprintf(fp, "lw $fp, 4($sp)\n");
    fprintf(fp, "lw $ra, 0($sp)\n");
    fprintf(fp, "addi $sp, $sp, -8\n");
    fprintf(fp, "addi $sp, $sp, %d\n", 4 * ArgsNum);
}

void TransFunctions(){
    FunStackSZ[1] = 64;
    FunStackSZ[2] = 64;
    Intercode *cur = InterCodes -> head;
    int funcid = -1;
    int varcnt = 0;
    int decsz = 0;
    while(cur != NULL){
        
        IR *ir = cur -> IRcode;
        switch(ir -> kind){
        case FUNCTION_IR: {
            // load function:
            if(funcid != -1){
                FunStackSZ[funcid] = varcnt * 4 + decsz;
            }
            varcnt = 0;
            decsz = 0;
            if(strcmp(ir -> IR_Type.Single.op->id.name, "main") == 0){
                funcid = 0;
                FuncTable[0] = "main";
            }
            else {
                FunctionNum++;
                funcid = FunctionNum;
                FuncTable[funcid] = ir -> IR_Type.Single.op->id.name;
            }
            memset(vis, 0, (InterCodes -> TempVarNum_t + 1)* sizeof(int));
        }
            break;

        case DEC_IR:
            decsz += ir -> IR_Type.Dec.sz;
            break;
        
        // case ARG_IR: 放在函数调用时处理
        case PARAM_IR:
        case READ_IR:
            // single OP
            {
                int id = Find_Op_ID(ir -> IR_Type.Single.op);
                if(id == -1) break;
                if(!vis[id]){
                    varcnt++;
                    vis[id] = 1;
                }
            }
            break;
        
        case ASSIGN_IR:
        case GADDR_IR:
        case RADDR_IR:
        case WADDR_IR:
        case CALL_IR:
            // assign OP
            {
                int id1 = Find_Op_ID(ir -> IR_Type.Assign.left);
                int id2 = Find_Op_ID(ir -> IR_Type.Assign.right);
                if(id1 != -1){
                    if(!vis[id1]){
                        varcnt++;
                        vis[id1] = 1;
                    }
                }
                
                if(id2 != -1){
                    if(!vis[id2]){
                        varcnt++;
                        vis[id2] = 1;
                    }
                }
            }
            break;
        
        case ADD_IR:
        case SUB_IR:
        case MUL_IR:
        case DIV_IR:
            // binary OP
            {
                int id1 = Find_Op_ID(ir -> IR_Type.Binary.result);
                int id2 = Find_Op_ID(ir -> IR_Type.Binary.op1);
                int id3 = Find_Op_ID(ir -> IR_Type.Binary.op2);
                if(id1 != -1){
                    if(!vis[id1]){
                        varcnt++;
                        vis[id1] = 1;
                    }
                }
                
                if(id2 != -1){
                    if(!vis[id2]){
                        varcnt++;
                        vis[id2] = 1;
                    }
                }

                if(id3 != -1){
                    if(!vis[id3]){
                        varcnt++;
                        vis[id3] = 1;
                    }
                }
            }
            break;

        default: break;
    }
    cur = cur -> next;
    }
    FunStackSZ[funcid] = varcnt * 4 + decsz;
}

void MIPSGeneratorInit(){
    VarInfoRec = (VarInfo *)malloc((InterCodes -> TempVarNum_t + 1) * sizeof(VarInfo));
    vis = (int *)malloc(InterCodes -> TempVarNum_t * sizeof(int));
    FunStackSZ = (int *)malloc(FunctionNum * sizeof(int));
    FunctionNum = 2;
    TransFunctions();
    memset(VarInfoRec, 0, (InterCodes -> TempVarNum_t + 1) * sizeof(VarInfo));
    FuncTable[0] = "main";
    FuncTable[1] = "read";
    FuncTable[2] = "write";
    fprintf(fp, ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\nread:\nli $v0, 4\nla $a0, _prompt\nsyscall\nli $v0, 5\nsyscall\njr $ra\n\nwrite:\nli $v0, 1\nsyscall\nli $v0, 4\nla $a0, _ret\nsyscall\nmove $v0, $0\njr $ra\n\n");

}

int Find_Op_ID(Operand *op){
    if(op -> kind == CONSTANT_OP || op -> kind == LABEL_OP || op -> kind == FUNCTION_OP || op -> kind == RELOP_OP) return -1;
    else return atoi((op -> id.name) + 1);
}

int Find_Func_ID(Operand *op){
    char *func = op -> id.name;
    for(int i = 0; i <= FunctionNum; i++){
        if(strcmp(FuncTable[i], func) == 0)
            return i;
    }
    return -1;
}

void VarLoad(int reg_no, int var_no){
    //assert(VarInfoRec[var_no].instack == 1);
    fprintf(fp, "lw $t%d, -%d($fp)\n", reg_no, VarInfoRec[var_no].offset);

}

void VarStore(int reg_no, int var_no){
    if(VarInfoRec[var_no].instack == 0){
        StackOffset = StackOffset + 4;
        VarInfoRec[var_no].instack = 1;
        VarInfoRec[var_no].offset = StackOffset;
    }

    fprintf(fp, "sw $t%d, -%d($fp)\n", reg_no, VarInfoRec[var_no].offset);

}

void MIPSGenerator(){
    // result : t0 
    // op1 : t1
    // op2 : t2

    Intercode *curic = InterCodes -> head;
    while(curic != NULL){
        IR *IRcode = curic -> IRcode;

        switch (IRcode -> kind)
        {
        case LABEL_IR:
            fprintf(fp, "%s:\n", IRcode -> IR_Type.Single.op -> id.name);
            break;

        case FUNCTION_IR:
            {   
                Param = 0; 
                StackOffset = 0; 
                fprintf(fp, "%s:\n", IRcode -> IR_Type.Single.op->id.name);
                if(strcmp(IRcode -> IR_Type.Single.op->id.name, "main") != 0)
                    CurFunNum++;
                else before_func(0); 
                // int ifmain = 0;
                // if(strcmp(IRcode -> IR_Type.Single.op->id.name, "main") == 0)
                //     ifmain = 1;
                // else CurFunNum++;
                // before_func(ifmain);
                // fprintf()
                break;
            }
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
            if(IRcode -> IR_Type.Binary.op1 -> kind != CONSTANT_OP)
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Binary.op1));
            else fprintf(fp, "li $t1, %d\n", IRcode -> IR_Type.Binary.op1->id.value);
            if(IRcode -> IR_Type.Binary.op2 -> kind != CONSTANT_OP)
                VarLoad(2, Find_Op_ID(IRcode -> IR_Type.Binary.op2));
            else fprintf(fp, "li $t2, %d\n", IRcode -> IR_Type.Binary.op2->id.value);

            fprintf(fp, "add $t0, $t1, $t2\n"); 
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Binary.result));
            break;

        case SUB_IR:
            if(IRcode -> IR_Type.Binary.op1 -> kind != CONSTANT_OP)
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Binary.op1));
            else fprintf(fp, "li $t1, %d\n", IRcode -> IR_Type.Binary.op1->id.value);
            if(IRcode -> IR_Type.Binary.op2 -> kind != CONSTANT_OP)
                VarLoad(2, Find_Op_ID(IRcode -> IR_Type.Binary.op2));
            else fprintf(fp, "li $t2, %d\n", IRcode -> IR_Type.Binary.op2->id.value);

            fprintf(fp, "sub $t0, $t1, $t2\n");
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Binary.result));
            break;

        case MUL_IR:
            if(IRcode -> IR_Type.Binary.op1 -> kind != CONSTANT_OP)
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Binary.op1));
            else fprintf(fp, "li $t1, %d\n", IRcode -> IR_Type.Binary.op1->id.value);
            if(IRcode -> IR_Type.Binary.op2 -> kind != CONSTANT_OP)
                VarLoad(2, Find_Op_ID(IRcode -> IR_Type.Binary.op2));
            else fprintf(fp, "li $t2, %d\n", IRcode -> IR_Type.Binary.op2->id.value);

            fprintf(fp, "mul $t0, $t1, $t2\n");
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Binary.result));
            break;

        case DIV_IR:
            if(IRcode -> IR_Type.Binary.op1 -> kind != CONSTANT_OP)
                VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Binary.op1));
            else fprintf(fp, "li $t1, %d\n", IRcode -> IR_Type.Binary.op1->id.value);
            if(IRcode -> IR_Type.Binary.op2 -> kind != CONSTANT_OP)
                VarLoad(2, Find_Op_ID(IRcode -> IR_Type.Binary.op2));
            else fprintf(fp, "li $t2, %d\n", IRcode -> IR_Type.Binary.op2->id.value);

            fprintf(fp, "div $t1, $t2\n");
            fprintf(fp, "mflo $t0\n");
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Binary.result));
            break;

        case GADDR_IR:
            // x = &y
            // get address from stack $fp 
            fprintf(fp, "addi $t1, $fp, -%d\n", VarInfoRec[Find_Op_ID(IRcode -> IR_Type.Assign.right)].offset);
            fprintf(fp, "move $t0, $t1\n");
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
            break;

        case RADDR_IR:
            // x = *y
            VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Assign.right));
            fprintf(fp, "lw $t0, 0($t1)\n");
            VarStore(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
            break;

        case WADDR_IR:
            // /*x = y
            VarLoad(1, Find_Op_ID(IRcode -> IR_Type.Assign.right));
            VarLoad(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
            fprintf(fp, "sw $t1, 0($t0)\n");
            break;
        
        case GOTO_IR:
            // GOTO x
            fprintf(fp, "j %s\n", IRcode -> IR_Type.Single.op->id.name);
            break; 

        case IF_IR:
            // IF GOTO
            // t1 : x
            // t2 : y
            {
                char* relop = IRcode -> IR_Type.IfGoto.relop->id.name;
                Operand *x = IRcode -> IR_Type.IfGoto.x;
                Operand *y = IRcode -> IR_Type.IfGoto.y;
                Operand *z = IRcode -> IR_Type.IfGoto.z;
                VarLoad(1, Find_Op_ID(x));
                VarLoad(2, Find_Op_ID(y));
                if(strcmp(relop, "==") == 0)
                    fprintf(fp, "beq $t1, $t2, %s\n", z->id.name);
                else if(strcmp(relop, "!=") == 0)
                    fprintf(fp, "bne $t1, $t2, %s\n", z -> id.name);
                else if(strcmp(relop, ">") == 0)
                    fprintf(fp, "bgt $t1, $t2, %s\n", z -> id.name);
                else if(strcmp(relop, "<") == 0)
                    fprintf(fp, "blt $t1, $t2, %s\n", z -> id.name);
                else if(strcmp(relop, ">=") == 0)
                    fprintf(fp, "bge $t1, $t2, %s\n", z -> id.name);
                else if(strcmp(relop, "<=") == 0)
                    fprintf(fp, "ble $t1, $t2, %s\n", z -> id.name);
            }
            break;

        case RETURN_IR:
            // RETURN x
            VarLoad(0, Find_Op_ID(IRcode -> IR_Type.Single.op));
            fprintf(fp, "move $v0, $t0\n");
            fprintf(fp, "jr $ra\n");
            break;

        case DEC_IR:
            {
                int id = Find_Op_ID(IRcode -> IR_Type.Dec.x);
                VarInfoRec[id].instack = 1;
                StackOffset += IRcode -> IR_Type.Dec.sz;
                VarInfoRec[id].offset = StackOffset;
            }
            break;

        case ARG_IR:
            // TODO
            {   int id = Find_Op_ID(IRcode -> IR_Type.Single.op);
                fprintf(fp, "addi $sp, $sp, -4\n");
                fprintf(fp, "lw $a0, -%d($fp)\n", VarInfoRec[id].offset);
                fprintf(fp, "sw $a0, 0($sp)\n");
                NArgs ++;
            }
            break;

        case CALL_IR:
            // x = call f
            {   
                int funcid = Find_Func_ID(IRcode -> IR_Type.Assign.right);
                before_func(funcid);
                fprintf(fp, "jal %s\n", IRcode -> IR_Type.Assign.right->id.name);
                fprintf(fp, "move $t0, $v0\n");
                //VarStore(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
                after_func(0, NArgs);
                VarStore(0, Find_Op_ID(IRcode -> IR_Type.Assign.left));
                NArgs = 0;
            }
            break;

        case PARAM_IR:
            // 利用t3寻址
            {
                int id = Find_Op_ID(IRcode -> IR_Type.Single.op);
                fprintf(fp, "lw $v1, %d($t3)\n", 4 * Param);
                Param++;
                StackOffset += 4;
                fprintf(fp, "sw $v1, -%d($fp)\n", StackOffset);
                VarInfoRec[id].instack = 1;
                VarInfoRec[id].offset = StackOffset;
            }
            break;
        case READ_IR:
            // TODO
            {   
                int id = Find_Op_ID(IRcode -> IR_Type.Assign.left); 
                before_func(1);
                fprintf(fp, "jal read\n");
                after_func(1, 0);
                fprintf(fp, "move $t0, $v0\n");
                VarStore(0, id);
            }

            break;

        case WRITE_IR:
            // TODO
            {   //Arg:
                fprintf(fp, "addi $sp, $sp, -4\n");
                int id = Find_Op_ID(IRcode -> IR_Type.Single.op);
                fprintf(fp, "lw $a0, -%d($fp)\n", VarInfoRec[id].offset);
                fprintf(fp, "sw $a0, 0($sp)\n");
                before_func(2);
                fprintf(fp, "lw $a0, 0($t3)\n");
                fprintf(fp, "jal write\n");
                after_func(2, 1);
            }
            break;

        default:
            break;
        }
        curic = curic -> next;
    }
}