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

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



int receiveMessage(int fd, char* buf);


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int i, r;
    char buf[LL_INPUT_MAX_SIZE];
    char resp[255];
    struct Application app;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    r = appopen(&app, "res/pinguim.gif", strlen("res/pinguim.gif"),
      argv[1], O_RDWR | O_NOCTTY, TRANSMITTER);

    appwrite(app);

    /*
    for (i = 0; i < 1; i++) {
      memcpy(buf, &app.buffer[i * LL_INPUT_MAX_SIZE], LL_INPUT_MAX_SIZE);
      llwrite(fd, buf, LL_INPUT_MAX_SIZE);
    }
    */

    //receiveMessage(fd, resp);
    //printf("Receiver's response: %s\n", resp);

    sleep(2);
    //llclose(fd);
    appclose(app);
    return 0;
}





int receiveMessage(int fd, char* buf) {
  int i = 0, res;

  while (STOP==FALSE) {
    res = read(fd, buf + i, 1);
    if (buf[i] == '\0') {
      STOP = TRUE;
    }

    if (res == 1) {
      i++;
    }
  }

  return i;
}
