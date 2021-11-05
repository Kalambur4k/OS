#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAXLEN 200

// check вернет нам результат не на каждую нашу запись, поэтому пусть его вывод читает и дублирует на stdout отдельный тред
void *read_check_errors(void *arg) {
    int fd = *(int*)arg;
    char buf[MAXLEN];
    int readen = read(fd,buf, MAXLEN);
    while (readen > 0) {
        write(STDOUT_FILENO,buf,readen);
        readen = read(fd,buf, MAXLEN);
    }
    close(fd);
    pthread_exit(NULL);
}

int main(void)
{
    int fda[2],fdb[2]; 
    if ( pipe(fda) < 0 || pipe(fdb) < 0 ) {
        perror("Cannot create pipe");
        return EXIT_FAILURE;
    }
    char fname[MAXLEN];
    //input filename
    printf("Input file name: ");
    scanf("%[^\n]s", fname);
    getc(stdin); // убрать перевод строки
    //open file
    FILE* fp = fopen(fname, "w");
    if (!fp) {
        perror("Cannot create file");
        return EXIT_FAILURE;
    }
    // FORK
    int id = fork();
    //error FORK
    if (id == -1)
    {
        perror("Fork error");
        return EXIT_FAILURE;
    }
    //CHILD WORK
    else if (id == 0)
    {
        close(STDIN_FILENO);
        dup(fda[0]);
        close(fda[0]);
        
        close(STDOUT_FILENO);
        dup(fileno(fp));
        close(fileno(fp));
        
        close(STDERR_FILENO);
        dup(fdb[1]);
        close(fdb[1]);
        
        close(fda[1]);
        close(fdb[0]);
        execlp("./check", NULL);
    }
    //PARENT WORK
    else
    {
        close(fda[0]);
        close(fdb[1]);
        close(fileno(fp));
        
        pthread_t err_thread;
        if ( pthread_create( &(err_thread), NULL, read_check_errors, (void*) &(fdb[0]) ) != 0 ) {
            perror("Cannot create thread for errors");
            return EXIT_FAILURE;
        }
        
        char buffer[MAXLEN];
        char *buf = buffer;
        int buf_size = MAXLEN;
        
        int len = getline(&buf,&buf_size,stdin); 
        while ( len != EOF ) {
            write( fda[1], buffer, len );
            len = getline(&buf,&buf_size,stdin); 
        }
        close(fda[1]);
        
        pthread_join( err_thread, NULL );        
        wait(NULL);
    }
    return 0;
}

