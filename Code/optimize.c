#include "optimize.h"


void optimize(char *filename){
    FILE *fp = fopen(filename, "w+");
    BB *bbs = BB_Partition(InterCodes);
    printBB(fp, bbs);
    fclose(fp);
}


void LocalConstFold(BB *BBList){
    BB *curBB = BBList;
    while(curBB != NULL){
        memset(NodeHash, 0, sizeof(NodeHash));
        Intercode *ic = curBB -> start;
        while(1){
            switch(ic -> IRcode -> kind){
                case ASSIGN_IR:{
                    Operand *right = ic -> IRcode -> IR_Type.Assign.right;
                    Operand *left = ic -> IRcode -> IR_Type.Assign.left;
                    if(left -> kind != RADDR_OP){
                        if(right -> kind == CONSTANT_OP){
                            // 常量赋值，加进表，对中间代码不做处理
                            NodeHash[left -> no + left -> temp * 1024].value = right -> id.value;
                            NodeHash[left -> no + left -> temp * 1024].valid = 1;
                        }

                        else if(right -> kind == VARIABLE_OP){
                            // 查表赋值
                            int id = right -> no + right -> temp * 1024;
                            if(NodeHash[id].valid){
                                // 表中更新：
                                NodeHash[left -> no + left -> temp * 1024].value = NodeHash[id].value;
                                NodeHash[left -> no + left -> temp * 1024].valid = 1;
                                // 更改当前中间代码
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                ic -> IRcode -> IR_Type.Assign.right = newconst;
                            }
                        }

                        else if(right -> kind == GADDR_OP){
                            // 此时left一定是指针
                            // 出现这种情况意味着right的值即有可能因为left的更改而更改，因此哈希表直接作废
                            int id = right -> no + right -> temp * 1024;
                            NodeHash[id].valid = 0;
                        }

                        // TODO : WADDR_OP
                    }
                }

                break;

                case ADD_IR:{
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    if(res -> kind != RADDR_OP){

                    }
                }
                break;
            }
        }
    }
}