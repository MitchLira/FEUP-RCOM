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
int fd;

int main(int argc, char** argv)
{
    int i, sum = 0, speed = 0, n;
    char buf[LL_INPUT_MAX_SIZE];
    char resp[255];
    struct Application app;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    app = appopen("res/pinguim.gif");

    printf("%lu - %s\n", app.fileLength, app.fileName);
    for (i = 0; i < LL_INPUT_MAX_SIZE; i++) {
      printf("%02X\n", app.buffer[i]);
    }
    printf("\n\n");

    fd = llopen(argv[1], O_RDWR | O_NOCTTY, TRANSMITTER);


    int loops = app.fileLength / LL_INPUT_MAX_SIZE;
    int bytesRemaining = app.fileLength;
    int s = 0;
    for (i = 0; i < 1; i++) {
      if (bytesRemaining < LL_INPUT_MAX_SIZE)
        s = bytesRemaining;
      else {
        s = LL_INPUT_MAX_SIZE;
        bytesRemaining -= LL_INPUT_MAX_SIZE;
      }

      memcpy(buf, &app.buffer[i * LL_INPUT_MAX_SIZE], s);

      llwrite(fd, buf, s);
    }

    receiveMessage(fd, resp);
    printf("Receiver's response: %s\n", resp);

    sleep(2);
    llclose(fd);
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
