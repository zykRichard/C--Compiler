#ifndef E6897323_E43B_448B_9109_5F5444F8B384
#define E6897323_E43B_448B_9109_5F5444F8B384

#include "mips.h"

void optimize_BB();
BB *NewBB();
BB *BB_Partition(InterCodeList *iclist);
void printBB(FILE* fp, BB *block);
#endif /* E6897323_E43B_448B_9109_5F5444F8B384 */
