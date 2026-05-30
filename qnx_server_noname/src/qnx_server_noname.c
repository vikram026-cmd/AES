#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <process.h>

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
    int chid, rcvid;
    sensor_data_t msg;
    server_resp_t resp;

    chid = ChannelCreate(0);
    printf("Server Active. PID: %d, CHID: %d\n", getpid(), chid);

    while (1)
    {
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

        printf("\n[Server] Received Update:\n");
        printf("Temp: %d°C | Hum: %d%% | Soil: %d cb\n",
                msg.temperature, msg.humidity, msg.soil_moisture);

        sprintf(resp.acknowledgment, "Server ACK: Values logged successfully.");

        MsgReply(rcvid, 0, &resp, sizeof(resp));
    }
    return 0;
}
