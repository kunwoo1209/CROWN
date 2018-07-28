#include <crown.h>
#include <stdio.h>

int main(){
    int input;
    SYM_int(input);

    printf("input = %d\n", input);
    if (input > 0){
        printf("input > 0\n");
    }else{
        printf("input <= 0\n");
    }
    return 0;
}
