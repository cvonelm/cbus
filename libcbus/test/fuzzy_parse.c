#include "../libcbus.c"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
int main()
{
    /*Kind of hacky fuzzy test to check wether the parser is failsafe*/
    unsigned char *str = malloc(255);
    srand(time(NULL));
    int x;
    for(x = 0;x < 1000;++x)
    {
        int len = rand() % 252 + 4;
        int i;
        for(i = 0;i < len;i++)
            str[i] = rand() % 256;
        
        *(int *)str = len;
        printf("%d\n", *(int *)str);
        CBUS_msg *msg = cbus_parse_msg(str);

        if(msg != NULL)
            cbus_print_msg(msg);
        else
            printf("NULL\n");
    }
    free(str);
}
