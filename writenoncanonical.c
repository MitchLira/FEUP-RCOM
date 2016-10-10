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

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG   0x7E
#define A      0x03
#define C_SET  0x03
#define C_UA   0x07



void unanswered();
int receiveMessage(int fd, char* buf);


volatile int STOP=FALSE;
int fd;

int main(int argc, char** argv)
{
    int i, sum = 0, speed = 0, n;
    struct termios oldtio,newtio;
    char buf[255];
    char resp[255];
    unsigned char rcv_byte;
    //RcvUAState state;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n\n");

    llopen(fd, TRANSMITTER);


    printf("Write your message: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      fprintf(stderr, "Error reading message!\n");
      exit(-1);
    }

    n = strcspn(buf, "\n");
    buf[n] = '\0';


    write(fd, buf, n + 1);

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
