#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAXLEN 200
#define SEMA_NAME "/semaphore_for_pipe1_1d5LFo4"
#define SEMB_NAME "/semaphore_for_pipe2_1d5LFo4"
#define SHARED_MEMORY_OBJECT_NAME "shared_memory_1d5LFo4"

int main()
{
    
    sem_t *sema, *semb;
    
    sema = sem_open(SEMA_NAME, 0);
    if ( sema == SEM_FAILED ) {
        perror("Cannot open semaphore A");
        return EXIT_FAILURE;
    }
    
    semb = sem_open(SEMB_NAME, 0);
    if ( semb == SEM_FAILED ) {
        perror("Cannot open semaphore B");
        return EXIT_FAILURE;
    }
    
    if (sem_wait(sema) < 0 ) {
        perror("Cannot wait semaphore a");    
        return EXIT_FAILURE;
    }
    
    int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDWR, 0777);
    if ( shm < 0 ) {
        perror("Cannot open shared memory object");
        return EXIT_FAILURE;
    }
    
    char *addr = mmap(0, MAXLEN+1, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("Cannot do mmap");
        return EXIT_FAILURE;
    }
    
    FILE* fp = fopen(addr, "w");
    if (!fp) {
        perror("File create failed");
        return EXIT_FAILURE;
    }
    
    if ( sem_post( semb ) < 0 ) {
        perror("Cannot post semaphore b");    
        return EXIT_FAILURE;
    }
        
    if (sem_wait(sema) < 0 ) {
        perror("Cannot wait semaphore a");    
        return EXIT_FAILURE;
    }
    
    while ( *addr ) {
        
        if ( isupper( *addr ) ) {
            fprintf( fp, "%s\n", addr );
            *addr = 0;
        } else {
            sprintf( addr, "Error: %s", addr );            
        }

        if ( sem_post( semb ) < 0 ) {
            perror("Cannot post semaphore b");    
            return EXIT_FAILURE;
        }
            
        if (sem_wait(sema) < 0 ) {
            perror("Cannot wait semaphore a");    
            return EXIT_FAILURE;
        }
        
    }
    
    fclose( fp );
    
    munmap( addr, MAXLEN );
    close( shm );

}    
    
