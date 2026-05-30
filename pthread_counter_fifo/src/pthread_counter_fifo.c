#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

int global_counter = 0;

typedef struct {
    int start;
    int end;
    int thread_id;
} thread_data_t;

void* count_function(void* arg)
{
    thread_data_t* data = (thread_data_t*)arg;
    for (int i = data->start; i <= data->end; i++) {
        global_counter = i;
    }
    printf("Thread %d finished. Current global_counter: %d\n", data->thread_id, global_counter);
    return NULL;
}

int main() {
    pthread_t threads[4];
    thread_data_t t_data[4];
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 10;
    pthread_attr_setschedparam(&attr, &param);

    int ranges[4][3] = {{0, 250, 1}, {251, 500, 2}, {501, 750, 3}, {751, 1000, 4}};

    for (int i = 0; i < 4; i++) {
        t_data[i].start = ranges[i][0];
        t_data[i].end = ranges[i][1];
        t_data[i].thread_id = ranges[i][2];
        pthread_create(&threads[i], &attr, count_function, &t_data[i]);
    }

    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);
    printf("Final Global Counter Value: %d\n", global_counter);
    return EXIT_SUCCESS;
}
