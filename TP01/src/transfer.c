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

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define MODEMDEVICE "/dev/ttyS1"


int main(int argc, char** argv)
{
        struct Application app;
        struct StatisticsPacket counters;
        int status;

        if ( (argc < 3) ||
             ( (strcmp("/dev/ttyS0", argv[1])!=0) &&
              (strcmp("/dev/ttyS1", argv[1])!=0) ) ||
              ((strcmp("transmitter", argv[2])!=0) &&
                (strcmp("receiver", argv[2])!=0))) {
                printf("Usage:\tnserial SerialPort status\n\tex: nserial /dev/ttyS1 [receiver || transmitter]\n");
                exit(1);
        }

        if (strcmp("receiver", argv[2]) == 0) {
          status = RECEIVER;
          loadReceiverSettings();
          appopen(&app, argv[1], O_RDWR | O_NOCTTY, RECEIVER);
          appread(app);
        }
        else {
          status = TRANSMITTER;
          loadTransmitterSettings();
          appopen(&app, argv[1], O_RDWR | O_NOCTTY, TRANSMITTER);
          appwrite(app);
        }

        appclose(app);

        counters = getStatisticsPacket();

        if(status == RECEIVER) {

          printf("Frames Received: %d\n", counters.framesReceived);
          printf("Frames Rejected: %d\n", counters.framesRejected);
          printf("Frames Repeated: %d\n", counters.framesRepeated);
        }
        else {

          printf("Frames Trasmitted: %d\n", counters.framesTransmitted);
          printf("Number of time outs: %d\n", counters.timeOutCounter);
        }
        
        return 0;
}
