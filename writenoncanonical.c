/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG   0x7E
#define A      0x03
#define C_SET  0x03
#define C_UA   0x07

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } RcvUAState;

void updateState(RcvUAState* state, unsigned char byte);

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    char resp[255];
    int i, sum = 0, speed = 0;
    unsigned char SET[5];
    
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
    leitura do(s) próximo(s) caracter(es)
  */

    
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C_SET;
    SET[3] = A^C_SET;
    SET[4] = FLAG;

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    for (i = 0; i < 5; i++) {
      write(fd, SET + i, sizeof(char));
    }

    RcvUAState state = START_RCV;
    for(i = 0; i < 5; i++) {
      unsigned char rcv_byte;
      read(fd, &rcv_byte, 1);
      updateState(&state, rcv_byte);
    }

    if(state == STOP_RCV)
      printf("STOP\n");

    printf("Write your message: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
      fprintf(stderr, "Error reading message!\n");
      exit(-1);
    }

    int n = strcspn(buf, "\n");
    buf[n] = '\0';

    
    res = write(fd,buf,n+1);

    i = 0;
    while (STOP == FALSE) {
      res = read(fd, resp + i, 1);
      if (resp[i] == '\0') STOP = TRUE;
      i++;
    }

    printf("Response: %s\n", resp);
    
    sleep(2);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}

void updateState(RcvUAState* state, unsigned char byte) {

  switch(*state) {
  case START_RCV:
    if (byte == FLAG)
      *state = FLAG_RCV;
    break;

  case FLAG_RCV:
    if (byte == A)
      *state = A_RCV;
    else if (byte == FLAG) ;
    else
      *state = START_RCV;
    break;

  case A_RCV:
    if (byte == C_UA)
      *state = C_RCV;
    else if (byte == FLAG)
      *state = FLAG_RCV;
    else
      *state = START_RCV;
    break;

  case C_RCV:
    if (byte ==  (A^C_UA))
      *state = BCC_OK;
    else if (byte == FLAG)
      *state = FLAG_RCV;
    else
      *state = START_RCV;
    break;

  case BCC_OK:
    if (byte == FLAG)
      *state = STOP_RCV;
    else
      *state = START_RCV;
    break;

  default:
    break;
  }
}
