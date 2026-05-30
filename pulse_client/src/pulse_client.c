#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <sched.h>

int main()
{
    int coid, server_pid, server_chid;
    int event_counter = 100; // Sample system event state payload
    int pulse_code = 55;     // Custom pulse identifier (Must be between _PULSE_CODE_MINAVAIL and _PULSE_CODE_MAXAVAIL)
    struct sched_param param;

    // Enforce Round Robin Scheduling Policy
    param.sched_priority = 10;
    sched_setscheduler(0, SCHED_RR, &param);

    printf("Enter Target Server PID and CHID: ");
    if (scanf("%d %d", &server_pid, &server_chid) != 2) {
        return EXIT_FAILURE;
    }

    // Attach path to target server
    coid = ConnectAttach(0, server_pid, server_chid, 0, 0);
    if (coid == -1) {
        perror("ConnectAttach failed");
        return EXIT_FAILURE;
    }

    printf("Sender: Commencing asynchronous notifications every 2 seconds...\n");

    while (1)
    {
        printf("\nSender: Firing non-blocking pulse notification...\n");

        // MsgSendPulse is strictly non-blocking. It sends and immediately continues.
        if (MsgSendPulse(coid, 10, pulse_code, event_counter) == -1)
        {
            perror("MsgSendPulse failed");
            break;
        }

        printf("Sender: Pulse issued successfully. Continuing execution instantly.\n");

        event_counter += 5; // Mutate event code data state
        sleep(2);           // Stagger pulses every 2 seconds
    }

    ConnectDetach(coid);
    return 0;
}
