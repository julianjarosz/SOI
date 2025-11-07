#include <lib.h>
#include <unistd.h>
#include <stdio.h>
#include <minix/callnr.h>
#include <stdlib.h>

int getprocnr(int pid)
{
    message m;
    m.m1_i1 = pid;
    
    return _syscall(MM, GETPROCNR, &m);
}

int main(int argc, char *argv[])
{


    int pidstart = atoi(argv[1]);
    int pid;
    int result;


for(pid = pidstart; pid <= pidstart + 10; pid++){
        errno = 0;
        result = getprocnr(pid);

        if (result < 0){
            printf("not found: %d , %d\n", pid, errno);
        } else {
            printf("found  %d , %d\n", pid, result);
        }
        }
        return 0;
}
