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
#include "linklayer.h"

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
    char buf[255];
    char resp[255];
    struct termios oldtio;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    fd = llopen(argv[1], O_RDWR | O_NOCTTY, TRANSMITTER);

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }


    printf("Write your message: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      fprintf(stderr, "Error reading message!\n");
      exit(-1);
    }

    n = strcspn(buf, "\n");
    buf[n] = '\0';

    llwrite(fd, buf, n);

    receiveMessage(fd, resp);
    printf("Receiver's response: %s\n", resp);

    sleep(2);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
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
