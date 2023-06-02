#ifndef B256509A_A6FE_4A61_B148_485A0AC8EB85
#define B256509A_A6FE_4A61_B148_485A0AC8EB85

#include "intercode.h"

void MIPSGeneratorInit();
int Find_Op_ID(Operand *op);
void VarLoad(int reg_no, int var_no);
void VarStore(int reg_no, int var_no);
void MIPSGenerator(FILE *fp);


#endif /* B256509A_A6FE_4A61_B148_485A0AC8EB85 */
