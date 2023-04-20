#ifndef A7B2DBAC_66BE_45D4_8034_17B1F54DC173
#define A7B2DBAC_66BE_45D4_8034_17B1F54DC173

#include "common.h"

InterCodeList InterCodes;

// operand
Operand *NewOperand(OpKind kind, ...);
void printOP(Operand *op);

// IR
IR *NewIR(IrKind kind, ...);
void printIR(IR *ir);

// intercode
Intercode *NewInterCode(IR *ir);
void InterCodeInsert(Intercode *ic, InterCodeList *iclist);
void printInterCode(Intercode *ic);

// intercodes list
InterCodeList *NewInterCodeList(); // init InterCodes
void InterCodesPrint(InterCodeList *iclist);

#endif /* A7B2DBAC_66BE_45D4_8034_17B1F54DC173 */
