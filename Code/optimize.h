#ifndef E6897323_E43B_448B_9109_5F5444F8B384
#define E6897323_E43B_448B_9109_5F5444F8B384

#include "mips.h"

void optimize_BB();
BB *NewBB();
BB *BB_Partition(InterCodeList *iclist);
void printBB(FILE* fp, BB *block);
void CheckAndResetBB(Intercode* oldic, Intercode* newic, BB* bb);
void LocalConstFold(BB *BBList);


void LocalCommonExpDel(BB *BBList);
IRentry *NewIRentry(IR *ir);
IRentryList *NewIRentryList();
void IRentryInsert(IRentry *entry, IRentryList *irlist);
void IRundefine(Operand *op);
int CheckIndentity(Operand *op1, Operand *op2);
Operand *CheckConsistency(IR *ir);
#endif /* E6897323_E43B_448B_9109_5F5444F8B384 */
