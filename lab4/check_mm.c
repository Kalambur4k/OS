#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SEMA_NAME "/semaphore_for_pipe1_1d5LFo4"
#define SEMB_NAME "/semaphore_for_pipe2_1d5LFo4"
#define SHARED_MEMORY_OBJECT_NAME "shared_memory_1d5LFo4"
#define SHARED_MEMORY_SIZE 2

sem_t *sema, *semb;
int shm;
char *addr;

int sema_ok = 0, semb_ok = 0, shm_ok = 0, mmap_ok = 0, file_ok = 0;

void clean_all() {
    if ( mmap_ok ) {
        munmap( addr, SHARED_MEMORY_SIZE );        
    }
    if ( shm_ok ) {
        close( shm );
    }
    if ( semb_ok ) {
        sem_close(semb);
    }
    if ( sema_ok ) {
        sem_close(sema);
    }
}

int main()
{
    
    sem_t *sema, *semb;
    
    sema = sem_open(SEMA_NAME, 0);
    if ( sema == SEM_FAILED ) {
        perror("Cannot open semaphore A");
        return EXIT_FAILURE;
    }
    sema_ok = 1;
    
    semb = sem_open(SEMB_NAME, 0);
    if ( semb == SEM_FAILED ) {
        perror("Cannot open semaphore B");
        clean_all();
        return EXIT_FAILURE;
    }
    semb_ok = 1;
    
    int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDWR, 0777);
    if ( shm < 0 ) {
        perror("Cannot open shared memory object");
        clean_all();
        return EXIT_FAILURE;
    }
    shm_ok = 1;
    
    char *addr = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("Cannot do mmap");
        clean_all();
        return EXIT_FAILURE;
    }
    mmap_ok = 1;
    
    char *test = addr + 1;
    
    if ( sem_wait(sema) < 0 ) {
        perror("Cannot wait semaphore a");
        clean_all();
        return EXIT_FAILURE;
    }
    
    while ( *test ) {
        
        //printf( "Child check: test is %c\n", *test );
        if ( isupper( *test ) ) {
            *addr = 0;
        } else {
            *addr = 1;
        }
        //printf( "Child check result: %d\n", (int)*addr );

        if ( sem_post( semb ) < 0 ) {
            perror("Cannot post semaphore b");
            clean_all();
            return EXIT_FAILURE;
        }
            
        if (sem_wait(sema) < 0 ) {
            perror("Cannot wait semaphore a");
            clean_all();
            return EXIT_FAILURE;
        }
        
    }
    
    clean_all();
}    
    
