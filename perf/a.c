#include <stdio.h>
#include <stdlib.h>
int main()
{
    int x=1;
    int y=2;
    int *p=(int*)malloc(sizeof(int)*10);
    for(int i=0;i<10;++i)
        p[i]=i;
    printf("%d %d", x+y, x-y);
    free(p);
    p=(int*)malloc(sizeof(int)*100);
    p[80]=10;
    free(p);
    return 0;
}
