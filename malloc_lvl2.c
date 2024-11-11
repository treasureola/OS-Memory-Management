#include "types.h"
#include "user.h"

int 
main(void){
    void * one = p2malloc(12);
    printf(1, "%s %p\n", "ONE", one);
    void * two = p2malloc(12);
    printf(1, "%s %p\n", "TWO", two);
    void * three = p2malloc(12);
    printf(1, "%s %p\n", "THREE", three);
    void * four = p2malloc(12);
    printf(1, "%s %p\n", "FOUR", four);
    void * five = p2malloc(12);
    printf(1, "%s %p\n", "FIVE", five);

    p2free(three);
    printf(1, "%s %p\n", "FREE THREE", three);

    void * six = p2malloc(12);
    printf(1, "%s %p\n", "SIX", six);
    void * seven = p2malloc(12);
    printf(1, "%s %p\n", "SEVEN", seven);

    p2free(one);
    printf(1, "%s %p\n", "FREE ONE", one);
    
    void * eight = p2malloc(12);
    printf(1, "%s %p\n", "EIGHT", eight);
    
    int allocated = p2allocated();
    printf(1, "%d\n", allocated);

    // p2free(one);
    p2free(eight);
    printf(1, "%s %p\n", "FREE EIGHT", eight);
    p2free(six);
    printf(1, "%s %p\n", "FREE SIX", six);
    p2free(four);
    printf(1, "%s %p\n", "FREE FOUR", four);
    p2free(two);
    printf(1, "%s %p\n", "FREE TWO", two);
    p2free(seven);
    printf(1, "%s %p\n", "FREE SEVEN", seven);
    p2free(five);
    printf(1, "%s %p\n", "FREE FIVE", five);
    // p2free(three);
    // printf(1, "%s %p\n", "FREE THREE", three);
    // p2free(one);
    // printf(1, "%s %p\n", "FREE ONE", one);

    int totem = p2totmem();
    printf(1, "%d\n", totem);

    int allocated1 = p2allocated();
    printf(1, "%d\n", allocated1);

    exit();

}