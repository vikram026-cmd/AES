#include <stdio.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

typedef struct {
    int temperature, humidity, soil_moisture;
} sensor_data_t;

int main() {
    name_attach_t *attach;
    sensor_data_t msg;
    int rcvid;

    // Register the name "Sensor_Server" in the QNX namespace
    if ((attach = name_attach(NULL, "Sensor_Server", 0)) == NULL) {
        return EXIT_FAILURE;
    }

    printf("Server 'Sensor_Server' is registered and listening...\n");

    while (1) {
        // Receive message on the channel managed by name_attach
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

        if (rcvid == -1) break;

        // Verify if it's a standard pulse or a data message
        printf("\n[Server] Automatic Lookup Success. Data Received:\n");
        printf("Temp: %d°C | Hum: %d%% | Soil: %d cb\n",
                msg.temperature, msg.humidity, msg.soil_moisture);

        MsgReply(rcvid, 0, "ACK", 4);
    }

    name_detach(attach, 0);
    return 0;
}
