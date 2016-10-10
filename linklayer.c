#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "linklayer.h"


/* Definitions */
#define FLAG   0x7E
#define A      0x03
#define C_SET  0x03
#define C_UA   0x07
#define CONNECT_NR_TRIES  3
#define UA_WAIT_TIME      3


/* Global/Const variables */
const unsigned char SET[] = {FLAG, A, C_SET, A^C_SET, FLAG};
const unsigned char UA[] = {FLAG, A, C_UA, A^C_UA, FLAG};
int filedes, numberTries = 0;


/* Function headers */
void updateState(ConnectionState* state, unsigned char byte, int status);
void reconnect();



int llopen(int fd, int status) {
  ConnectionState state;
  unsigned char receivedByte;
  int i;

  filedes = fd;
  state = START_RCV;
  if (status == TRANSMITTER) {
    (void) signal(SIGALRM, reconnect);   // Install routine to re-send SET if receiver doesn't respond

    write(filedes, SET, sizeof(SET));  // Send SET to receiver
    alarm(UA_WAIT_TIME);

    for (i = 0; i < sizeof(UA); i++) {
      read(filedes, &receivedByte, 1);
      updateState(&state, receivedByte, status);
    }

    if (state == STOP_RCV) {
      printf("Great success\n");
      alarm(0);
      return 0;
    }

  }
  else if (status == RECEIVER) {

    for (i = 0; i < sizeof(SET); i++) {
      read(filedes, &receivedByte, 1);
      updateState(&state, receivedByte, status);
    }

    if (state == STOP_RCV) {
      printf("SET received successfully\n");
      printf("Waiting for message...\n\n");
    }

    write(filedes, UA, sizeof(UA));

  }

  return -1;
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
