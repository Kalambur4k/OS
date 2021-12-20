#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAXLEN 200
#define SEMA_NAME "/semaphore_for_pipe1_1d5LFo4"
#define SEMB_NAME "/semaphore_for_pipe2_1d5LFo4"
#define SHARED_MEMORY_OBJECT_NAME "shared_memory_1d5LFo4"
#define SHARED_MEMORY_SIZE 2

sem_t *sema, *semb;
int shm;
char *addr;
FILE* fp;

int sema_ok = 0, semb_ok = 0, shm_ok = 0, mmap_ok = 0, file_ok = 0;

void clean_all() {
    if ( file_ok ) {
        fclose( fp );
    }
    if ( mmap_ok ) {
        munmap( addr, SHARED_MEMORY_SIZE );        
    }
    if ( shm_ok ) {
        close( shm );
        shm_unlink(SHARED_MEMORY_OBJECT_NAME);
    }
    if ( semb_ok ) {
        sem_close(semb);
        sem_unlink(SEMB_NAME);        
    }
    if ( sema_ok ) {
        sem_close(sema);
        sem_unlink(SEMA_NAME);        
    }
}

int main(void)
{
    
    sema = sem_open(SEMA_NAME, O_CREAT, 0777, 0);
    if ( sema == SEM_FAILED ) {
        perror("Cannot create semaphore A");
        return EXIT_FAILURE;
    }
    sema_ok = 1;
    
    semb = sem_open(SEMB_NAME, O_CREAT, 0777, 0);
    if ( semb == SEM_FAILED ) {
        perror("Cannot create semaphore B");
        clean_all();
        return EXIT_FAILURE;
    }
    semb_ok = 1;

    shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT|O_RDWR, 0777);
    if ( shm < 0 ) {
        perror("Cannot create shared memory object");
        clean_all();
        return EXIT_FAILURE;
    }
    shm_ok = 1;
    
    if ( ftruncate(shm, SHARED_MEMORY_SIZE) == -1 ) {
        perror("Cannot set shared memory object size");
        clean_all();
        return EXIT_FAILURE;
    }    
    
    addr = mmap(0, SHARED_MEMORY_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
    if ( addr == (char*)-1 ) {
        perror("Cannot do mmap");
        clean_all();
        return EXIT_FAILURE;
    }    
    mmap_ok = 1;
        
    char fname[MAXLEN];
    //input filename
    printf("Input file name: ");
    scanf("%[^\n]s", fname);
    getc(stdin); // óáðàòü ïåðåâîä ñòðîêè
    fp = fopen(fname, "w");
    if (!fp) {
        perror("Cannot create file");
        clean_all();
        return EXIT_FAILURE;
    }
    file_ok = 1;
    
    // FORK
    int id = fork();
    //error FORK
    if (id == -1)
    {
        perror("Fork error");
        clean_all();
        return EXIT_FAILURE;
    }
    //CHILD WORK
    else if (id == 0)
    {
        execlp("./check_mm", NULL);
    }
    //PARENT WORK
    else
    {
        
        char *test = addr + 1;
        
        
        char buffer[MAXLEN];
        char *buf = buffer;
        int buf_size = MAXLEN;
        
        int len = getline(&buf,&buf_size,stdin); 
        while ( len != EOF ) {

            
            *test = buffer[0];

            if ( sem_post( sema ) < 0 ) {
                perror("Cannot post semaphore a");    
                clean_all();
                return EXIT_FAILURE;
            }
            
            if (sem_wait(semb) < 0 ) {
                perror("Cannot wait semaphore b");    
                clean_all();
                return EXIT_FAILURE;
            }
            
            if ( *addr ) {
                printf( "Error: %s", buf );
            } else {
                fprintf( fp, buf );
            }
            len = getline(&buf,&buf_size,stdin); 
        }
        
        *test = 0;
        if ( sem_post( sema ) < 0 ) {
            perror("Cannot post semaphore a");    
            clean_all();
            return EXIT_FAILURE;
        }

        wait(NULL);
        clean_all();
        
    }
    return 0;
}

