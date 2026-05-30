#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

int global_fifo_count = 0;
int global_rr_count = 0;

void* fifo_worker(void* arg)
{
    int* range = (int*)arg;
    int start = range[0];
    int end = range[1];
    int id = range[2];

    for (int i = start; i <= end; i++)
    {
        global_fifo_count = i;
        // No printf here to ensure no voluntary yielding
    }
    printf("FIFO Thread %d finished. Global Count: %d\n", id, global_fifo_count);
    return NULL;
}

void* rr_worker(void* arg)
{
    int id = *((int*)arg);
    while (global_rr_count < 10)
    {
        global_rr_count++;
        // Printf here helps visualize the context switching (RR time-slicing)
        printf("RR Thread %d incremented count to: %d\n", id, global_rr_count);
        for (volatile int j = 0; j < 100000; j++); // Small busy loop
    }
    return NULL;
}

int main() {
    pthread_t threads[4];
    pthread_attr_t attr;
    struct sched_param param;
    int fifo_ranges[4][3] = {{0, 250, 1}, {251, 500, 2}, {501, 750, 3}, {751, 1000, 4}};
    int ids[4] = {1, 2, 3, 4};

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    param.sched_priority = 10;
    pthread_attr_setschedparam(&attr, &param);

    // --- TEST 1: FIFO ---
    printf("Starting FIFO Execution...\n");
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], &attr, fifo_worker, &fifo_ranges[i]);
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);

    // --- TEST 2: Round Robin ---
    printf("\nStarting RR Execution...\n");
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    for (int i = 0; i < 4; i++)
        pthread_create(&threads[i], &attr, rr_worker, &ids[i]);
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);

    return EXIT_SUCCESS;
}
