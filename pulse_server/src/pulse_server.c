#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <process.h>
#include <sched.h>

int main()
{
    int chid, rcvid;
    struct _pulse pulse_msg;
    struct sched_param param;

    // Enforce Round Robin Scheduling Policy
    param.sched_priority = 10;
    if (sched_setscheduler(0, SCHED_RR, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }

    // Create channel to receive messages and pulses
    chid = ChannelCreate(0);
    printf("Pulse Server Active. PID: %d, CHID: %d\n", getpid(), chid);
    printf("Awaiting asynchronous pulses...\n");

    while (1)
    {
        // MsgReceive extracts both standard messages and tiny kernel pulses
        rcvid = MsgReceive(chid, &pulse_msg, sizeof(pulse_msg), NULL);

        if (rcvid == -1) {
            perror("MsgReceive failed");
            break;
        }

        // Critical Check: rcvid == 0 indicates a QNX Kernel Pulse (Asynchronous Notification)
        if (rcvid == 0)
        {
            printf("\n[Receiver] Asynchronous Pulse Event Captured!\n");
            printf("Pulse Type/Code: %d\n", pulse_msg.code);
            printf("Pulse Associated Value: %d\n", pulse_msg.value.sival_int);
        }
        else
        {
            // Fallback case if a standard synchronous message hits this channel
            printf("[Receiver] Received standard message from rcvid: %d\n", rcvid);
            MsgReply(rcvid, 0, NULL, 0);
        }
    }
    return 0;
}
