#include "optimize.h"


void optimize(char *filename){
    FILE *fp = fopen(filename, "w+");
    BB *bbs = BB_Partition(InterCodes);
    printBB(fp, bbs);
    fclose(fp);
}
