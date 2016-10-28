/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "DataLink.h"
#include "Application.h"
#include "Settings.h"
#include "Statistics.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



volatile int STOP=FALSE;

int main(int argc, char** argv)
{
        struct Application app;
        struct StatisticsPacket counters;

        if ( (argc < 2) ||
             ((strcmp("/dev/ttyS0", argv[1])!=0) &&
              (strcmp("/dev/ttyS1", argv[1])!=0) )) {
                printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
                exit(1);
        }

        loadTransmitterSettings();

        appopen(&app, argv[1], O_RDWR | O_NOCTTY, TRANSMITTER);
        appwrite(app);
        appclose(app);

        counters = getStatisticsPacket();

        printf("Frames Trasmitted: %d\n", counters.framesTransmitted);
        printf("Frames Rejected: %d\n", counters.framesRejected);
        printf("Frames Repeated: %d\n", counters.framesRepeated);
        printf("Number of time outs: %d\n", counters.timeOutCounter);

        return 0;
}
