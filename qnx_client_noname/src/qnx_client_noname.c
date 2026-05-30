#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <unistd.h>

typedef struct
{
    int temperature;    // Unit: °C
    int humidity;       // Unit: %
    int soil_moisture;  // Unit: cb
} sensor_data_t;

typedef struct
{
  char acknowledgment[50];
} server_resp_t;

int main()
{
    int coid, server_pid, server_chid;
    sensor_data_t s_data = {25, 60, 30};
    server_resp_t resp;

    printf("Enter Server PID and CHID: ");
    scanf("%d %d", &server_pid, &server_chid);

    coid = ConnectAttach(0, server_pid, server_chid, 0, 0);

    printf("Client: Starting repeated transmission every 2 seconds...\n");

    while (1)
    {
        if (MsgSend(coid, &s_data, sizeof(s_data), &resp, sizeof(resp)) == -1)
        {
            break;
        }

        printf("Client: Server Response -> %s\n", resp.acknowledgment);

        sleep(2);
    }

    ConnectDetach(coid);
    return 0;
}
