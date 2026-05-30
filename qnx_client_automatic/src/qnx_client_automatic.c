#include <stdio.h>
#include <sys/dispatch.h>
#include <unistd.h>

typedef struct {
    int temperature, humidity, soil_moisture;
} sensor_data_t;

int main() {
    int coid;
    sensor_data_t s_data = {24, 55, 40};
    char ack[10];

    printf("Client: Looking for 'Sensor_Server'...\n");

    // Automatically find the server by name
    while ((coid = name_open("Sensor_Server", 0)) == -1) {
        sleep(1); // Wait if server isn't started yet
    }

    printf("Client: Connected to Server. Starting 2s loop...\n");

    while (1) {
        MsgSend(coid, &s_data, sizeof(s_data), ack, sizeof(ack));
        printf("Client: Data Sent. Server says: %s\n", ack);

        s_data.temperature++; // Simulate variations
        sleep(2);
    }

    name_close(coid);
    return 0;
}
