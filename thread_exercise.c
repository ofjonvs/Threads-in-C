#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

void *max_value(void *args);
void *array_sum(void *args);
struct timeval tv_delta(struct timeval start, struct timeval end) {
   struct timeval delta = end;

   delta.tv_sec -= start.tv_sec;
   delta.tv_usec -= start.tv_usec;
   if (delta.tv_usec < 0) {
      delta.tv_usec += 1000000;
      delta.tv_sec--;
   }

   return delta;
}

int main(int argc, char *argv[]){   
    int *int_array, *split_array, i, j, x= 0, *temp, elements = atoi(argv[1]), 
    threads = atoi(argv[2]), seed = atoi(argv[3]), task = atoi(argv[4]);
    char print_results = argv[5][0];
    pthread_t *tids;
    void *result_ptr = NULL;
    struct rusage start_ru, end_ru;
    struct timeval start_wall, end_wall;
    struct timeval diff_ru_utime, diff_wall, diff_ru_stime;

    if(argc != 6){
        fprintf(stderr, "Invalid number of arguments\n");
        return EX_USAGE;
    }

    if(print_results == 'N' || print_results == 'n'){
        print_results = 0;
    }
    else if(print_results != 'Y' && print_results != 'y'){
        fprintf(stderr, "Invalid print_results entry\n");
        return EX_USAGE;
    }

    tids = malloc(sizeof(pthread_t) * threads);
    int_array = malloc(elements * sizeof(int) + 1);
    split_array = malloc(((elements * sizeof(int))/threads) + 2);
    
    srand(seed);
    for (i = 0; i < elements ; i++) {
        int_array[i] = rand() % 100;
    }
    int_array[i] = -1;

    getrusage(RUSAGE_SELF, &start_ru);
    gettimeofday(&start_wall, NULL);
    if(task == 1){
        int k;
        x = 0;
        temp = &x;
        for(i = 0, j = 0; i < threads ; i++){
            for (k = 0; k < (elements/threads) + 1; j++, k++) {
                if(int_array[j] == -1){
                    break;
                }
                split_array[k] = int_array[j];
                printf("g: %d\n", split_array[k]);
            }
            split_array[k] = -1;
            pthread_create(&tids[i], NULL, max_value, split_array);
            pthread_join(tids[i], &result_ptr);
            if(*(int*)result_ptr > *temp){
                *temp = *(int*)result_ptr;
            } 
        }
    
        if(print_results){
            printf("max value: %d\n", *temp);
        }
    }
    else if(task == 2){
        int k;
        x = 0;
        temp = &x;
        for(i = 0, j = 0; i < threads ; i++){
            for (k = 0; k < (elements/threads) + 1; j++, k++) {
                split_array[k] = int_array[j];
                if(int_array[j] == -1){
                    break;
                }
                printf("g: %d\n", split_array[k]);
            }
            split_array[k] = -1;
            pthread_create(&tids[i], NULL, array_sum, split_array);
            pthread_join(tids[i], &result_ptr);
            *temp += *(int *) result_ptr;
        }
    
        if(print_results){
            printf("sum: %d\n", *temp);
        }
    }
    else{
        fprintf(stderr, "Invalid task\n");
        return EX_USAGE;
    }

    gettimeofday(&end_wall, NULL);
    getrusage(RUSAGE_SELF, &end_ru);

    /* Computing difference */
    diff_ru_utime = tv_delta(start_ru.ru_utime, end_ru.ru_utime);
    diff_ru_stime = tv_delta(start_ru.ru_stime, end_ru.ru_stime);
    diff_wall = tv_delta(start_wall, end_wall);

    printf("User time: %ld.%06ld\n", diff_ru_utime.tv_sec,
          diff_ru_utime.tv_usec);
    printf("System time: %ld.%06ld\n", diff_ru_stime.tv_sec,
          diff_ru_stime.tv_usec);
    printf("Wall time: %ld.%06ld\n", diff_wall.tv_sec, diff_wall.tv_usec);

    free(result_ptr); 
    free(tids);
    free(int_array);
    return 0;
}

void *max_value(void *args) {
    int *argument = (int *) args, i;
    int *answer_ptr = malloc(sizeof(int));
    *answer_ptr = argument[0];

    for(i = 1; argument[i] != -1; i++){
        if(*answer_ptr < argument[i]){
            *answer_ptr = argument[i];
        }
    }

   return answer_ptr;
}

void *array_sum(void *args) {
    int *argument = (int *) args, i;
    int *answer_ptr = malloc(sizeof(int));
    *answer_ptr = 0;

    for(i = 0; argument[i] != -1; i++){
        *answer_ptr = (*answer_ptr + argument[i]) % 1000000;
    }
   return answer_ptr;
}





