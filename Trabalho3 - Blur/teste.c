#include <stdio.h>
#include <stdlib.h>

int main() {
    int** v = malloc(5*sizeof(int*));
    int a = 4;
    v[0] = &a;
    printf("%d", *v[0]);
    scanf("%d", a);

}