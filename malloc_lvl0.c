#include "types.h"
#include "user.h"

int main(void){
    int size = 12;
    void * ptr = &size;

    p2malloc(12);
    p2free(ptr + 4);
    int allocate = p2allocated();
    int totmem = p2totmem();
    printf(1, "%d\n", allocate);
    printf(1, "%d\n", totmem);
    exit();
}
