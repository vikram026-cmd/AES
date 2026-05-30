#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <hw/inout.h> // Required for in8() to read hardware ports
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define HW_INTERRUPT_VECTOR 1  // Mapped to x86 Hardware Keyboard Controller (IRQ 1)
#define KEYBOARD_DATA_PORT  0x60 // Standard x86 Keyboard Data Register Port

int global_event_count = 0;
int simulated_temperature = 25; // Unit: °C

int main() {
    int chid, rcvid;
    int interrupt_id;
    struct sigevent event;
    struct _pulse pulse_msg;
    struct sched_param param;

    uint64_t last_interrupt_time = 0;
    uint64_t current_time;

    param.sched_priority = 15;
    if (sched_setscheduler(0, SCHED_RR, &param) == -1)
    {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }

    if (ThreadCtl(_NTO_TCTL_IO_PRIV, 0) == -1)
    {
        perror("ThreadCtl I/O privileges failed. Run as root/superuser.");
        exit(EXIT_FAILURE);
    }

    chid = ChannelCreate(0);
    if (chid == -1)
    {
        perror("ChannelCreate failed");
        exit(EXIT_FAILURE);
    }

    int coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
    SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, _PULSE_CODE_MINAVAIL, 0);

    interrupt_id = InterruptAttachEvent(HW_INTERRUPT_VECTOR, &event, 0);
    if (interrupt_id == -1)
    {
        perror("InterruptAttachEvent failed");
        exit(EXIT_FAILURE);
    }

    printf("Interrupt Service Thread (IST) with Software Filtering Active.\n");
    printf("Monitoring IRQ Vector %d (Keyboard) for clean key-press events...\n", HW_INTERRUPT_VECTOR);

    while (1)
    {
        rcvid = MsgReceive(chid, &pulse_msg, sizeof(pulse_msg), NULL);

        if (rcvid == -1)
        {
            perror("MsgReceive failed");
            break;
        }

        if (rcvid == 0 && pulse_msg.code == _PULSE_CODE_MINAVAIL)
        {
            uint8_t scancode = in8(KEYBOARD_DATA_PORT);
            ClockTime(CLOCK_REALTIME, NULL, &current_time);

            if (scancode & 0x80)
            {
                InterruptUnmask(HW_INTERRUPT_VECTOR, interrupt_id);
                continue;
            }

            if ((current_time - last_interrupt_time) < 50000000)
            {
                InterruptUnmask(HW_INTERRUPT_VECTOR, interrupt_id);
                continue;
            }

            last_interrupt_time = current_time;

            global_event_count++;
            simulated_temperature += 2;

            printf("\n[IST] Valid External Keyboard Press Captured (Filter Passed)!\n");
            printf("Raw Scan Code Read: 0x%02X\n", scancode);
            printf("Interrupt Event Sequence: %d\n", global_event_count);
            printf("Updated Peripheral Temperature Status: %d°C\n", simulated_temperature);

            InterruptUnmask(HW_INTERRUPT_VECTOR, interrupt_id);
        }
    }
    InterruptDetach(interrupt_id);
    return 0;
}
