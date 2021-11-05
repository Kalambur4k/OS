#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NCARDS 52
// кодируем колоду от 0 до 51; сначала пики, потом кести, потом бубы, потом червы, в каждой масти 13 карт; значение карты = номер карты % 13
#define SUIT_SIZE 13

void *calc_probability(void *arg) {
    long rounds = *(long*)arg;
    long *success;
    success = (long*) malloc(sizeof(long));
    *success = 0;
    
    int cards[NCARDS];
    int i;
    for ( i = 0; i < NCARDS - 1; i++ ) {
        cards[i] = i;
    }
    unsigned int seed = time(NULL) + (unsigned int) pthread_self(); 
    long j;
    for ( j = 0; j < rounds; j++ ) {
        int card1 = rand_r(&seed) % NCARDS;
        cards[card1] = NCARDS - 1; 
        int card2 = rand_r(&seed) % ( NCARDS - 1 ); 
        if ( cards[card2] % SUIT_SIZE == card1 % SUIT_SIZE ) { 
            (*success)++;
        }
        cards[card1] = card1; 
    }
    
    pthread_exit(success);
}

int main(int argc, char* argv[] ) {
    
    if ( argc < 2 ) {
        printf( "Usage: %s n_threads n_rounds_per_thread\n", argv[0] );
        return 0;
    }
    
    int n_threads = atoi(argv[1]);
    long n_rounds = atol(argv[2]);
    
    printf( "%d threads and %ld rounds, PID = %d\n", n_threads, n_rounds, (int)getpid());
    
    pthread_t *threads = (pthread_t*) malloc(n_threads*sizeof(pthread_t));
    int *thread_ok = (int*) malloc(n_threads*sizeof(int));
    
    int i;
    for ( i = 0; i < n_threads; i++ ) {
        thread_ok[i] = pthread_create( &(threads[i]), NULL, calc_probability, (void*) &n_rounds );
    }
    
    long long total_rounds = 0, total_success = 0;
    for ( i = 0; i < n_threads; i++ ) {
        if ( !thread_ok[i] ) {
            void *res;
            pthread_join( threads[i], &res );
            total_rounds += n_rounds;
            total_success += *(long*)res;
            free( res );
        } else {
            printf( "thread %d error\n", i );
        }
    }
    
    long double result = 1.0L*total_success/total_rounds;
    
    printf( "%lld total rounds %lld total success\ncalculated result = %.15Lf\n", total_rounds, total_success, result );
    printf( "ideal result      = %.15Lf\n", 3.0L/51 );
    
    free(threads);
    free(thread_ok);
}
