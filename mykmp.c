#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main()
{
    char str[200];
    scanf("%s", str);
    uint64_t size=strlen(str);
    int64_t back[size+1];
    back[0] = -1;
    {
        uint64_t i = 1;
        int64_t k = 0;
        while (1)
        {
            if ( str[i] == str[k] )
            {
                back[i]=back[k];
            }
            else
            {
                back[i]=k;
                do
                {
                    k=back[k];
                }
                while ( k!=-1 && str[i]!=str[k] );
            }
            if ( str[i] == '\0' )
            {
                break;
            }
            ++i;
            ++k;
        }
    }
    for (uint64_t i=0; i<=size; ++i)
    {
        printf("%lld ", back[i]);
    }
    putchar('\n');
    return 0;
}
