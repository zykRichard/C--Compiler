#ifndef A7B2DBAC_66BE_45D4_8034_17B1F54DC173
#define A7B2DBAC_66BE_45D4_8034_17B1F54DC173

#include "semantic.h"
#include <stdio.h>


InterCodeList *InterCodes;

void InterCodesTranslate(Node *root);
//initialization
void ICListInit();
// operand
Operand *NewOperand(OpKind kind, ...);
void OperandChange(Operand *op, OpKind ChangeKind, char* ChangeName);
void printOP(FILE* fp, Operand *op);
Operand *NewTempt(InterCodeList *iclist);
Operand *NewLabel(InterCodeList *iclist);
Operand *FindSetNewOpID(sym *s, OpKind kind);
// IR
IR *NewIR(IrKind kind, ...);
void printIR(FILE* fp, IR *ir);

// intercode
Intercode *NewInterCode(IR *ir);
void InterCodeInsert(Intercode *ic, InterCodeList *iclist);
void printInterCode(FILE* fp, Intercode *ic);

// intercodes list
InterCodeList *NewInterCodeList(); // init InterCodes
void InterCodesPrint(FILE *fp, InterCodeList *iclist);

// Arg List
Arg *NewArg(Operand *op);
void ArgListInsert(Arg *a, Arglist *aglist);


// SDT Translation
void TranslateCond(Node *n, Operand *labelTrue, Operand *labelFalse);
void TranslateExtDefList(Node *n);
void TranslateExtDef(Node *n);
void TranslateFunDec(Node *n);
void TranslateCompSt(Node *n);
void TranslateDefList(Node *n);
void TranslateStmtList(Node *n);
void TranslateStmt(Node *n);
void TranslateDef(Node *n);
void TranslateDecList(Node *n);
void TranslateDec(Node *n);
void TranslateVarDec(Node *n, Operand* place);
void TranslateExp(Node *n, Operand* place, int LorR);
void TranslateArgs(Node *n, Arglist *aglist);

// debug
void TraverseICList(InterCodeList *iclist);
void TraverseInterCode(Intercode *ic);
void TraverseIR(IR *ir);
void TraverseOP(Operand *op);
#endif /* A7B2DBAC_66BE_45D4_8034_17B1F54DC173 */
