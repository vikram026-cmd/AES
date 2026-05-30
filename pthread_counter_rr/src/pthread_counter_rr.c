#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>

int global_rr_count = 0;

void* rr_worker(void* arg)
{
    int id = *((int*)arg);
    while (global_rr_count < 10000)
    {
        global_rr_count++;
        printf("RR Thread %d incremented count to: %d\n", id, global_rr_count);
        for (volatile int j = 0; j < 100000; j++); // Busy loop to observe switching
    }
    return NULL;
}

int main()
{
    pthread_t threads[4];
    int ids[4] = {1, 2, 3, 4};
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    param.sched_priority = 10;
    pthread_attr_setschedparam(&attr, &param);

    for (int i = 0; i < 4; i++)
    {
        pthread_create(&threads[i], &attr, rr_worker, &ids[i]);
    }

    for (int i = 0; i < 4; i++)
    {
    	pthread_join(threads[i], NULL);
    }
    printf("Final Round Robin Counter: %d\n", global_rr_count);
    return EXIT_SUCCESS;
}
