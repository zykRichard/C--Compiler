#include "optimize.h"


void optimize(char *filename){
    FILE *fp = fopen(filename, "w+");
    BB *bbs = BB_Partition(InterCodes);
    LocalConstFold(bbs);
    LocalCommonExpDel(bbs);
    InterCodesPrint(fp, InterCodes);
    fclose(fp);
}


void CheckAndResetBB(Intercode* oldic, Intercode* newic, BB* bb){
    if(bb -> start == oldic)
        bb -> start = newic;
    if(bb -> end == oldic)
        bb -> end = newic;
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
                            else NodeHash[left -> no + left -> temp * 1024].valid = 0;
                        }

                        else if(right -> kind == GADDR_OP){
                            // 此时left一定是指针
                            // 出现这种情况意味着right的值即有可能因为left的更改而更改，因此哈希表直接作废
                            int id = right -> no + right -> temp * 1024;
                            NodeHash[id].valid = 0;
                            NodeHash[left -> no + left -> temp * 1024].valid = 0;
                        }

                        // TODO : WADDR_OP
                    }
                }

                break;

                case ADD_IR:{
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    int id = res -> no + res -> temp * 1024;
                    if(res -> kind != RADDR_OP){
                        if(op1 -> kind == CONSTANT_OP && op2 -> kind == CONSTANT_OP){
                            NodeHash[id].value = op1->id.value + op2 ->id.value;
                            NodeHash[id].valid = 1;
                            Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                            Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                            ic -> prev -> next = newic;
                            ic -> next -> prev = newic;
                            newic -> prev = ic -> prev;
                            newic -> next = ic -> next;
                            CheckAndResetBB(ic, newic, curBB);
                            ic = newic;
                        }
                        else if(op1 -> kind == CONSTANT_OP && op2 -> kind == VARIABLE_OP){
                            if(NodeHash[op2 -> no + 1024 * (op2 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = op1 -> id.value + NodeHash[op2 -> no + 1024 * (op2 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else NodeHash[res -> no + res -> temp * 1024].valid = 0;
                        }
                        else if(op2 -> kind == CONSTANT_OP && op1 -> kind == VARIABLE_OP){
                            if(NodeHash[op1 -> no + 1024 * (op1 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = op2 -> id.value + NodeHash[op1 -> no + 1024 * (op1 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;

                    }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP && op2 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id1].valid && NodeHash[id2].valid){
                                // 更改ic
                                NodeHash[id].value = NodeHash[id1].value + NodeHash[id2].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else if(op1 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else if(op2 -> kind == VARIABLE_OP){
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                }
                break;
            }
                case SUB_IR:{
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    int id = res -> no + res -> temp * 1024;
                    if(res -> kind != RADDR_OP){
                        if(op1 -> kind == CONSTANT_OP && op2 -> kind == CONSTANT_OP){
                            NodeHash[id].value = op1->id.value - op2 ->id.value;
                            NodeHash[id].valid = 1;
                            Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                            Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                            ic -> prev -> next = newic;
                            ic -> next -> prev = newic;
                            newic -> prev = ic -> prev;
                            newic -> next = ic -> next;
                            CheckAndResetBB(ic, newic, curBB);
                            ic = newic;
                        }
                        else if(op1 -> kind == CONSTANT_OP && op2 -> kind == VARIABLE_OP){
                            if(NodeHash[op2 -> no + 1024 * (op2 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = op1 -> id.value - NodeHash[op2 -> no + 1024 * (op2 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op2 -> kind == CONSTANT_OP && op1 -> kind == VARIABLE_OP){
                            if(NodeHash[op1 -> no + 1024 * (op1 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = -(op2 -> id.value) + NodeHash[op1 -> no + 1024 * (op1 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;

                    }
                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP && op2 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id1].valid && NodeHash[id2].valid){
                                // 更改ic
                                NodeHash[id].value = NodeHash[id1].value - NodeHash[id2].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                                
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else if(op2 -> kind == VARIABLE_OP){
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                }
                break;
            }
                case MUL_IR:{
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    int id = res -> no + res -> temp * 1024;
                    if(res -> kind != RADDR_OP){
                        if(op1 -> kind == CONSTANT_OP && op2 -> kind == CONSTANT_OP){
                            NodeHash[id].value = op1->id.value * op2 ->id.value;
                            NodeHash[id].valid = 1;
                            Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                            Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                            ic -> prev -> next = newic;
                            ic -> next -> prev = newic;
                            newic -> prev = ic -> prev;
                            newic -> next = ic -> next;
                            CheckAndResetBB(ic, newic, curBB);
                            ic = newic;
                        }
                        else if(op1 -> kind == CONSTANT_OP && op2 -> kind == VARIABLE_OP){
                            if(NodeHash[op2 -> no + 1024 * (op2 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = op1 -> id.value * NodeHash[op2 -> no + 1024 * (op2 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                           else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op2 -> kind == CONSTANT_OP && op1 -> kind == VARIABLE_OP){
                            if(NodeHash[op1 -> no + 1024 * (op1 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = (op2 -> id.value) * NodeHash[op1 -> no + 1024 * (op1 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;

                    }
                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP && op2 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id1].valid && NodeHash[id2].valid){
                                // 更改ic
                                NodeHash[id].value = NodeHash[id1].value * NodeHash[id2].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }

                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;

                        }
                        else if(op1 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else if(op2 -> kind == VARIABLE_OP){
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                }
                break;
            }
                case DIV_IR:{
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    int id = res -> no + res -> temp * 1024;
                    if(res -> kind != RADDR_OP){
                        if(op1 -> kind == CONSTANT_OP && op2 -> kind == CONSTANT_OP){
                            NodeHash[id].value = op1->id.value / op2 ->id.value;
                            NodeHash[id].valid = 1;
                            Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                            Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                            ic -> prev -> next = newic;
                            ic -> next -> prev = newic;
                            newic -> prev = ic -> prev;
                            newic -> next = ic -> next;
                            CheckAndResetBB(ic, newic, curBB);
                            ic = newic;
                        }
                        else if(op1 -> kind == CONSTANT_OP && op2 -> kind == VARIABLE_OP){
                            if(NodeHash[op2 -> no + 1024 * (op2 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = op1 -> id.value / NodeHash[op2 -> no + 1024 * (op2 -> temp)].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op2 -> kind == CONSTANT_OP && op1 -> kind == VARIABLE_OP){
                            if(NodeHash[op1 -> no + 1024 * (op1 -> temp)].valid){
                                // 更改ic
                                NodeHash[id].value = NodeHash[op1 -> no + 1024 * (op1 -> temp)].value / (op2 -> id.value);
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;

                    }
                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP && op2 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id1].valid && NodeHash[id2].valid){
                                // 更改ic
                                NodeHash[id].value = NodeHash[id1].value / NodeHash[id2].value;
                                NodeHash[id].valid = 1;
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id].value);
                                Intercode *newic = NewInterCode(NewIR(ASSIGN_IR, res, newconst));
                                ic -> prev -> next = newic;
                                ic -> next -> prev = newic;
                                newic -> prev = ic -> prev;
                                newic -> next = ic -> next;
                                CheckAndResetBB(ic, newic, curBB);
                                ic = newic;
                            }
                            else if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                                NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                            }
                            else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else if(op1 -> kind == VARIABLE_OP){
                            int id1 = op1 -> no + 1024 * (op1 -> temp);
                            if(NodeHash[id1].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id1].value);
                                ic -> IRcode->IR_Type.Binary.op1 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }

                        else if(op2 -> kind == VARIABLE_OP){
                            int id2 = op2 -> no + 1024 * (op2 -> temp);
                            if(NodeHash[id2].valid){
                                Operand *newconst = NewOperand(CONSTANT_OP, NodeHash[id2].value);
                                ic -> IRcode->IR_Type.Binary.op2 = newconst;
                            }
                            NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                        }
                        else NodeHash[res -> no + 1024 * (res -> temp)].valid = 0;
                }
                break;
            }
                case CALL_IR:{
                    Operand *res = ic -> IRcode->IR_Type.Assign.left;
                    int id = res -> no + 1024 * (res -> temp);
                    NodeHash[id].valid = 0;
                    break;
                }
                case READ_IR:{
                    Operand *op = ic -> IRcode->IR_Type.Single.op;
                    int id = op -> no + 1024 * (op -> temp);
                    NodeHash[id].valid = 0;
                    break;
                }
        }
        if(ic == curBB -> end) break;
        ic = ic -> next;
    }
    curBB = curBB -> nxt;
}
}


void IRundefine(Operand *op){
    // irlist potential types:
    // ADD_IR
    // SUB_IR
    // MUL_IR
    // DIV_IR
    // ASSIGN_IR
    IRentry *cur = irlist -> head;
    while(cur != NULL){
        if(cur -> ir -> kind == ADD_IR || cur -> ir -> kind == SUB_IR || cur -> ir -> kind == MUL_IR || cur -> ir -> kind == DIV_IR){
        IRentry *saver = cur -> nxt;
        Operand *res = cur -> ir->IR_Type.Binary.result;
        Operand *op1 = cur -> ir->IR_Type.Binary.op1;
        Operand *op2 = cur -> ir -> IR_Type.Binary.op2;
        if(op -> kind != CONSTANT_OP)
        if(CheckIndentity(op, res) || CheckIndentity(op, op1) || CheckIndentity(op, op2)){
            //delete
            if(cur == (irlist -> tail))
                irlist -> tail = cur -> prev;
            if(cur -> prev != NULL)
                cur -> prev -> nxt = cur -> nxt;
            if(cur -> nxt != NULL)
                cur -> nxt -> prev = cur -> prev;
            if(cur == (irlist -> head))
                irlist -> head = saver;
            cur = saver;
            continue;
        }
        }

        else if(cur -> ir -> kind == ASSIGN_IR){
            IRentry *saver = cur -> nxt;
            Operand *left = cur -> ir -> IR_Type.Assign.left;
            Operand *right = cur -> ir->IR_Type.Assign.right;
            if(op -> kind != CONSTANT_OP)
            if(CheckIndentity(op, left) || CheckIndentity(op, right)){
                if(cur == (irlist -> tail))
                    irlist -> tail = cur -> prev;
                if(cur -> prev != NULL)
                    cur -> prev -> nxt = cur -> nxt;
                if(cur -> nxt != NULL)
                    cur -> nxt -> prev = cur -> prev;
                if(cur == (irlist -> head))
                    irlist -> head = saver;
                cur = saver;
                continue;
            }
        }
        cur = cur -> nxt;
    }
}

Operand *CheckConsistency(IR *ir){
    IRentry *cur = irlist -> head;
    while(cur != NULL){
        if(ir -> kind == ASSIGN_IR){
            Operand *cmp = ir -> IR_Type.Assign.right;
            if(cur -> ir->kind == ASSIGN_IR){
                Operand *tobecmp = cur -> ir-> IR_Type.Assign.right;
                if(CheckIndentity(cmp, tobecmp))
                    return (cur -> ir -> IR_Type.Assign.left);
            }
        }

        else if(ir -> kind == ADD_IR ||
                ir -> kind == SUB_IR ||
                ir -> kind == MUL_IR ||
                ir -> kind == DIV_IR){
                    Operand *op1 = ir -> IR_Type.Binary.op1;
                    Operand *op2 = ir -> IR_Type.Binary.op2;
                    if(cur -> ir -> kind == ir -> kind){
                        Operand *cmpop1 = cur -> ir->IR_Type.Binary.op1;
                        Operand *cmpop2 = cur -> ir -> IR_Type.Binary.op2;
                        if(CheckIndentity(op1, cmpop1) && CheckIndentity(op2, cmpop2))
                            return (cur -> ir -> IR_Type.Binary.result);
                    }
                }
        cur = cur -> nxt;
    }
    return NULL;
}

void LocalCommonExpDel(BB *BBList){
    BB *curBB = BBList;
    while(curBB != NULL){
        irlist = NewIRentryList();
        Intercode *ic = curBB -> start;
        while(1){
            switch(ic -> IRcode -> kind){
                case ASSIGN_IR:{
                    Operand *left = ic -> IRcode->IR_Type.Assign.left;
                    Operand *right = ic -> IRcode->IR_Type.Assign.right;
                    IRundefine(left);
                    if(right -> kind == CONSTANT_OP) goto END;
                    Operand *reduce = CheckConsistency(ic -> IRcode);
                    IR *tobeinsert = NewIR(ASSIGN_IR, left, right);
                    IRentryInsert(NewIRentry(tobeinsert), irlist);
                    if(reduce != NULL){
                        // 修改当前中间代码
                        ic -> IRcode->IR_Type.Assign.right = reduce;
                    }
                }
                break;

                case ADD_IR:
                case SUB_IR:
                case MUL_IR:
                case DIV_IR:
                {
                    Operand *res = ic -> IRcode -> IR_Type.Binary.result;
                    Operand *op1 = ic -> IRcode -> IR_Type.Binary.op1;
                    Operand *op2 = ic -> IRcode -> IR_Type.Binary.op2;
                    IRundefine(res);
                    Operand *reduce = CheckConsistency(ic -> IRcode);
                    IR *tobeinsert = NewIR(ic -> IRcode -> kind, res, op1, op2);
                    IRentryInsert(NewIRentry(tobeinsert), irlist);
                    if(reduce != NULL){
                        ic -> IRcode ->kind = ASSIGN_IR;
                        ic -> IRcode->IR_Type.Assign.left = res;
                        ic -> IRcode->IR_Type.Assign.right = reduce;
                    } 
                }
                break;

                case CALL_IR:
                    IRundefine(ic -> IRcode -> IR_Type.Assign.left);
                    break;
                
                case READ_IR:
                    IRundefine(ic -> IRcode -> IR_Type.Single.op);
                    break;

                default:
                    break;
            }
            END: 
            if(ic == curBB->end) 
                break;
            ic = ic -> next;
        }
        curBB = curBB -> nxt;
    }

}