#include <stdio.h>
#include "common.h"

extern FILE* yyin;
extern int yylex();
extern int yyparse();
extern int yydebug;
extern Node* root;
extern int lexerror;
extern int synerror;
extern int yydebug;
extern void TreePrint(Node* root, int depth);

int main(int argc, char **argv){
    if(argc > 1){
        if(!(yyin = fopen(argv[1], "r"))){
            perror(argv[1]);
            return 1;
        }
    }
    
    yyparse();
    if(lexerror == 0 && synerror == 0)
        {   //TreePrint(root, 0);
            LexicalAnalysis(root);
            }
    //printf("parser over file: %s finished", argv[1]);
    return 0;
}