#include <stdio.h>
#include "common.h"

extern FILE* yyin;
extern int yylex();
extern int yyparse();
extern int yydebug;
extern Node* root;

int main(int argc, char **argv){
    if(argc > 1){
        if(!(yyin = fopen(argv[1], "r"))){
            perror(argv[1]);
            return 1;
        }
    }

    yyparse();
    printf("parser over file: %s finished", argv[1]);
    return 0;
}