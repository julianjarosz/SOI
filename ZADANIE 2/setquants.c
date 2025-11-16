#include <lib.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    message m;
    m.m1_i1 = atoi(argv[1]); /*category*/
    m.m1_i2 = atoi(argv[2]); /*quant value*/
    if(_syscall(MM, SETQUANTS, &m) == -1){
        printf("ERROR\n");
        return 1;
    }
    return 0;
}