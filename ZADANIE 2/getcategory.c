#include <lib.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[]){
    message m;
    int pid = m.m1_i1 = atoi(argv[1]);
    if(_syscall(MM, GETCATEGORY, &m) == -1){
        printf("ERROR\n");
        return 1;
    }
    printf("Category for pid %d is %d\n", pid, m.m1_i1);
}