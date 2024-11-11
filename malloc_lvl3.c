#include "types.h"
// #include "stat.h"
#include "user.h"
// #include "param.h"

int
main(void){
    int i = 12;
    int total = 0;
    // int count = 0;
    while(i+4 < 4096){
        printf(1, "%s %d\n", "REQUESTED SIZE: ", i);
        p2malloc(i);
        total = total + i;
        i = i + 7;
    }
    int allocate = p2allocated();
    printf(1, "%d\n", allocate);

    int totmem = p2totmem();
    printf(1, "%d\n", totmem);
    printf(1, "%d\n", total);


    exit();
}