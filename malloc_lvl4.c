#include "types.h"
#include "user.h"

int 
main(void){
    while (1) {
    p2free(p2malloc(12));
    int allocate = p2totmem();
    printf(1, "%d\n", allocate);
    }
    exit();

}