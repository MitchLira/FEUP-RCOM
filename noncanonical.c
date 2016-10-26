/*Non-Canonical Input Processing*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DataLink.h"
#include "Application.h"
#include "Settings.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
        struct SettingsReceiver settings;
        struct Application app;

        if ( (argc < 2) ||
             ((strcmp("/dev/ttyS0", argv[1])!=0) &&
              (strcmp("/dev/ttyS1", argv[1])!=0) )) {
                printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
                exit(1);
        }

        settings = loadReceiverSettings();

        /*
        appopen(&app, argv[1], O_RDWR | O_NOCTTY, RECEIVER);
        appread(app);
        appclose(app);
        */

        return 0;
}
