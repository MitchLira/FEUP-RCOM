/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG  0x7E
#define A     0x03
#define C_SET 0x03
#define C_UA  0x07

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } RcvSetState;


void updateState(RcvSetState* state, unsigned char byte);



volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, i;
    struct termios oldtio,newtio;
    char buf[255];
    unsigned char UA[5];

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



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    UA[0] = FLAG;
    UA[1] = A;
    UA[2] = C_UA;
    UA[3] = A ^ C_UA;
    UA[4] = FLAG;

    printf("New termios structure set\n");

    

    RcvSetState rcv_state = START_RCV;
    for (i = 0; i < 5; i++) {
      unsigned char rcv_byte;
      read(fd, &rcv_byte, 1);
      updateState(&rcv_state, rcv_byte);	  
    }

    if (rcv_state == STOP_RCV)
      printf("ola\n");

    write(fd, UA, sizeof(UA));


    i = 0;
    while (STOP==FALSE) {       /* loop for input */
      res = read(fd, buf + i, 1);   /* returns after 1 chars have been input */
      if (buf[i] == '\0')
	STOP=TRUE;
      i++;
    }

    write(fd, buf, i),

  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}



void updateState(RcvSetState* state, unsigned char byte) {

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
    if (byte == C_SET)
      *state = C_RCV;
    else if (byte == FLAG)
      *state = FLAG_RCV;
    else
      *state = START_RCV;
    break;

  case C_RCV:
    if (byte ==  (A^C_SET))
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
