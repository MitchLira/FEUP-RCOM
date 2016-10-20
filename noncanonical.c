/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "DataLink.h"
#include <math.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


int receiveMessage(int fd, char* buf);


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd;
    char buf[LL_INPUT_MAX_SIZE];
    unsigned int fileLength;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = llopen(argv[1], O_RDWR | O_NOCTTY, RECEIVER);

    int i;
    int s = llread(fd, buf);

    printf("\n LELELELELELELE \n");

    //for (i  = 0; i < s; i++) {
      //printf("%c\n", (unsigned char) buf[i]);
    //}

    memcpy(&fileLength, &buf[3], sizeof(fileLength));

    int nrPackets = ceil((float) fileLength / LL_INPUT_MAX_SIZE);


    for(i=0; i<nrPackets; i++){
      int length = llread(fd,buf);
      /*
      int j;
      for(j=0; j<length; j++)
       printf("%02X - ", (unsigned char)buf[j]);
      */
      printf("\n");

    }
    s = llread(fd, buf);

    printf("\n LELELELELELELE \n");

    for (i  = 0; i < s; i++) {
      printf("%02X\n", (unsigned char) buf[i]);
    }
    return 0;
}


int receiveMessage(int fd, char* buf) {
  int i = 0, res;

  while (STOP==FALSE) {       /* loop for input */
    res = read(fd, buf + i, 1);   /* returns after 1 chars have been input */
    if (buf[i] == '\0') {
       STOP=TRUE;
     }

     if (res == 1) {
       i++;
     }
  }

  return i;
}
