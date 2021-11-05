#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXLEN 200

int main()
{
    char buf[MAXLEN];
    int buf_size = MAXLEN;
    char *bufp = buf; // чтобы компилятор не ругался на несоответствие типов
    
    while ( getline(&bufp,&buf_size,stdin) != EOF ) {
        if ( isupper( buf[0] ) ) {
            printf( "%s", buf );
        } else {
            fprintf( stderr, "Error: %s", buf );            
        }        
    }

}    
    
