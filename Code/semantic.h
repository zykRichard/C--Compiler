#ifndef FCC3C8F1_8301_441D_AEEA_3DF97988851B
#define FCC3C8F1_8301_441D_AEEA_3DF97988851B

#include "common.h"

// Data Structure
unsigned int hash_pjw(char *name);
void hash_insert(sym *s, int depth);
sym* hash_search(char *name);

// Assistance
type *NewType(NodeKind kind, ...);
sym *NewSym(char *name, type *tp, int depth);
int CheckForConflict(sym *s);
int StructTypeCheck(type *t1, type *t2);
int FuncConflictCheck(sym *func);
void FunNotDefCheck();
int TypeCheck(type *t1, type *t2);
void StackPush();
void StackPop();

// Symbol Table
void ExtDef(Node *n);
void ExtDecList(Node *n, type* specifier);
type *Specifier(Node *n);
type *StructSpecifier(Node *n); // 统一处理opttag 和 tag
sym *VarDec(Node *n, type* specifier);
void FunDec(Node *n, type* ReturnSpecifier, int isDef);
void VarList(Node *n, type *ScopeSpecifier);
FieldList *ParamDec(Node *n);
void CompSt(Node *n, type* ReturnSpecifier);
void DefList(Node *n, type* ScopeSpecifier);
void Def(Node *n, type* ScopeSpecifier);
void DecList(Node *n, type* specifier, type* ScopeSpecifier);
void Dec(Node *n, type *specifier, type *ScopeSpecifier);
void VarList(Node *n, type *ScopeSpecifier);
void StmtList(Node *n, type *ReturnSpecifier);
void Stmt(Node *n, type *ReturnSpecifier);
type *Exp(Node *n);
void Args(Node *n, type *ScopeSpecifier);
#endif /* FCC3C8F1_8301_441D_AEEA_3DF97988851B */
