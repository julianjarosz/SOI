#include <lib.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    message m;
    int category = 0;
    category = m.m1_i1 = atoi(argv[1]);
    if(_syscall(MM, GETQUANTS, &m) == -1){
        printf("ERROR\n");
        return 1;
    }
    printf("Quants for category %d is %d\n", category, m.m1_i1);
}