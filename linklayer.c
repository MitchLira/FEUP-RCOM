#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "linklayer.h"


/* Definitions */
#define FLAG   0x7E
#define A      0x03
#define C_SET  0x03
#define C_DISC 0x0B
#define C_UA   0x07
#define C_RR_SUFFIX  0x05
#define C_REJ_SUFFIX 0x01

#define CONNECT_NR_TRIES  3
#define UA_WAIT_TIME      3
#define BAUDRATE B38400
#define FRAME_SIZE 47


/* Global/Const variables */
const unsigned char SET[] = {FLAG, A, C_SET, A^C_SET, FLAG};
const unsigned char UA[] = {FLAG, A, C_UA, A^C_UA, FLAG};
int filedes, numberTries = 0;
struct termios oldtio,newtio;


/* Function headers */
void updateState(ConnectionState* state, unsigned char byte, int status);
void reconnect();



int llopen(const char* path, int oflag, int status) {
  ConnectionState state;
  unsigned char receivedByte;
  int i;

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    filedes = open(path, oflag);
    if (filedes <0) {perror(path); exit(-1); }

    if ( tcgetattr(filedes,&oldtio) == -1) { /* save current port settings */
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



    tcflush(filedes, TCIOFLUSH);

    if ( tcsetattr(filedes,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n\n");


  state = START_RCV;
  if (status == TRANSMITTER) {
    (void) signal(SIGALRM, reconnect);   // Install routine to re-send SET if receiver doesn't respond

    write(filedes, SET, sizeof(SET));  // Send SET to receiver
    alarm(UA_WAIT_TIME);

    for (i = 0; i < sizeof(UA); i++) {
      read(filedes, &receivedByte, 1);
      updateState(&state, receivedByte, status);
    }

    if (state != STOP_RCV) {
      exit(-1);
    }

    alarm(0);

  }
  else if (status == RECEIVER) {

    for (i = 0; i < sizeof(SET); i++) {
      read(filedes, &receivedByte, 1);
      updateState(&state, receivedByte, status);
    }

    if (state != STOP_RCV) {
      exit(-1);
    }

    write(filedes, UA, sizeof(UA));
  }


  return filedes;
}



int llwrite(int fd, char* buffer, int length) {
  static unsigned char C = 0x00;
  unsigned char Bcc2 = 0x00;
  char frame[FRAME_SIZE];
  int i, start;

  frame[0] = FLAG;
  frame[1] = A;
  frame[2] = C;
  frame[3] = A^C;

  start = 4;
  for (i = 0; i < length; i++) {
    frame[start + i] = buffer[i];
    Bcc2 ^= buffer[i];
  }

  frame[start + i] = Bcc2;
  i++;
  frame[start + i] = FLAG;

  write(filedes, frame, start+i);

  C ^= 0x40;
  return (start + i);
}


void updateState(ConnectionState* state, unsigned char byte, int status) {

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
    if (status == TRANSMITTER && byte == C_UA)
      *state = C_RCV;
    else if (status == RECEIVER && byte == C_SET)
      *state = C_RCV;
    else if (byte == FLAG)
      *state = FLAG_RCV;
    else
      *state = START_RCV;
    break;

  case C_RCV:
    if (status == TRANSMITTER && byte ==  (A^C_UA))
      *state = BCC_OK;
    else if (status == RECEIVER && byte ==  (A^C_SET))
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


void reconnect() {

  if (numberTries < CONNECT_NR_TRIES) {
    printf("No answer was given by the receiver. Retrying...\n");
    write(filedes, SET, sizeof(SET));
    numberTries++;
    alarm(UA_WAIT_TIME);
  }
  else {
    printf("Receiver is not replying. Shutting down...\n");
    alarm(0);
    close(filedes);
    exit(-1);
  }
}
