#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;
    printf("Parent Process with PID %d : Launching Child...\n",getpid());

    // Using spawnl to create a new process in a protected address space
    pid = spawnl(P_NOWAIT, "/tmp/hello_world", "hello_world", NULL);

    if (pid == -1)
    {
        perror("spawnl");
        return EXIT_FAILURE;
    }

    printf("Parent Process: Child launched with PID %d\n", pid);
    sleep(15);
    return EXIT_SUCCESS;
}
