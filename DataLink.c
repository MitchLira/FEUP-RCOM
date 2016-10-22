#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "DataLink.h"
#include "Alarm.h"


/* Definitions */
#define FLAG          0x7E
#define A_SENDER      0x03
#define A_RECEIVER    0x01
#define C_SET         0x03
#define C_DISC        0x0B
#define C_UA          0x07
#define C_RR_SUFFIX   0x05
#define C_REJ_SUFFIX  0x01
#define S             0x40
#define R             0x80
#define ESCAPE        0x7D
#define STUFF_BYTE    0x20


#define CONNECT_NR_TRIES  3
#define UA_WAIT_TIME      3
#define BAUDRATE          B38400

#define FRAME_DELIMITERS_SIZE       6
#define FRAME_SIZE                  LL_INPUT_MAX_SIZE * 2
#define START_FLAG_INDEX            0
#define A_INDEX                     1
#define C_INDEX                     2
#define BCC1_INDEX                  3


/* Enums / structs */
typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } CommandState;
typedef enum { SET, UA_SENDER, UA_RECEIVER, DISC_SENDER, DISC_RECEIVER } CommandType;

/* Global / const variables */
const unsigned int  COMMAND_LENGTH = 5;
const unsigned char SET_PACKET[] = {FLAG, A_SENDER, C_SET, A_SENDER^C_SET, FLAG};
const unsigned char UA_SENDER_PACKET[] = {FLAG, A_SENDER, C_UA, A_SENDER^C_UA, FLAG};
const unsigned char UA_RECEIVER_PACKET[] = {FLAG, A_RECEIVER, C_UA, A_RECEIVER^C_UA, FLAG};
const unsigned char DISC_SENDER_PACKET[] = {FLAG, A_SENDER, C_DISC, A_SENDER^C_DISC, FLAG};
const unsigned char DISC_RECEIVER_PACKET[] = {FLAG, A_RECEIVER, C_DISC, A_RECEIVER^C_DISC, FLAG};

int fd, numberTries = 0;
struct termios oldtio,newtio;


/* Function headers */
void updateCommandState(CommandState* state, CommandType type, unsigned char byte);
int needsStuffing(char byte);
void stuff(char *frame, int index, char byte);
int destuff(char *dst, char *src, int length);
int buildPacket(char *dst, char *src, int length, char controlByte);
int receivePacket(int fd, char *frame);
int stripAndValidate(char *dst, char *src, int length, char controlByte);
char *receiveSU();
int receiveCommand(CommandType type);
void reconnect();



int llopen(const char *path, int oflag, int status) {
  int i, bytesRead, r;
  unsigned char receivedByte;
  CommandType type;
  CommandState state;

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(path, oflag);
    if (fd <0) {perror(path); exit(-1); }

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

    if (status == TRANSMITTER) {
      /* If MIN = 0 and TIME > 0, the read will be satisfied if a single
      character is read, or TIME is exceeded (t = TIME * 0.1 s) */
      newtio.c_cc[VTIME]    = 0;
      newtio.c_cc[VMIN]     = 0;
    }
    else if (status == RECEIVER) {
      newtio.c_cc[VTIME]    = 0;
      newtio.c_cc[VMIN]     = 1;
    }



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

  configAlarm(CONNECT_NR_TRIES, UA_WAIT_TIME);


  state = START_RCV;
  if (status == TRANSMITTER) {
    write(fd, SET_PACKET, COMMAND_LENGTH);  // Send SET to receiver
    setAlarm(reconnect, (char *) SET_PACKET, COMMAND_LENGTH);

    type = UA_SENDER;
    if (receiveCommand(type) != 0) {
      fprintf(stderr, "Can't connect to the receiver. Please try again later.\n");
    }

    disableAlarm();

  }
  else if (status == RECEIVER) {

    type = SET;
    if (receiveCommand(type) != 0) {
      fprintf(stderr, "Can't connect to the sender. Please try again later.\n");
    }

    write(fd, UA_SENDER_PACKET, COMMAND_LENGTH);
  }


  return fd;
}



int llwrite(int fd, char *buffer, int length) {
  static unsigned char C = 0x00;
  char frame[FRAME_SIZE];
  char SU[COMMAND_LENGTH];
  int size, i;

  size = buildPacket(frame, buffer, length, C);
  write(fd, frame, size);

  for (i = 0; i < COMMAND_LENGTH; i++) {
    read(fd, SU + i, 1);
  }

  C ^= S;

  return size;
}



int llread(int fd, char *buffer) {
  static unsigned char transmitterControl = 0x00;
  static unsigned char receiverControl = R;
  int i, stuffedSize, frameSize;
  char frame[FRAME_SIZE];
  unsigned char SU[COMMAND_LENGTH];

  stuffedSize = receivePacket(fd, frame);
  frameSize = destuff(frame, frame, stuffedSize);

  if (stripAndValidate(buffer, frame, frameSize, transmitterControl) == 0) {
    receiverControl ^= C_RR_SUFFIX;     // TODO falta remover sufixo
    transmitterControl ^= S;
    receiverControl ^= R;
  }
  else {
    receiverControl ^= C_REJ_SUFFIX;
  }

  SU[START_FLAG_INDEX] = FLAG;
  SU[A_INDEX] = A_SENDER;
  SU[C_INDEX] = receiverControl;
  SU[BCC1_INDEX] = SU[A_INDEX] ^ SU[C_INDEX];
  SU[COMMAND_LENGTH-1] = FLAG;


  write(fd, SU, COMMAND_LENGTH);

  return frameSize - FRAME_DELIMITERS_SIZE;
}


int llclose(int fd) {
  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}



void updateCommandState(CommandState* state, CommandType type, unsigned char byte) {

  switch(*state) {
  case START_RCV:
    if (byte == FLAG) {
      *state = FLAG_RCV;
    }
    break;

  case FLAG_RCV:
    if ( ((type == SET) ||
         (type == UA_SENDER) ||
         (type == DISC_SENDER)) &&
         byte == A_SENDER) {
      *state = A_RCV;
    }
    else if ( ((type == UA_RECEIVER) ||
               (type == DISC_RECEIVER)) &&
               byte == A_RECEIVER ) {
      *state = A_RCV;
    }
    else if (byte == FLAG) ;        // stay in the same state
    else {
      *state = START_RCV;
    }
    break;

  case A_RCV:
    if ( ((type == UA_SENDER) ||
          (type == UA_RECEIVER)) &&
          byte == C_UA ) {
      *state = C_RCV;
    }
    else if (type == SET && byte == C_SET) {
      *state = C_RCV;
    }
    else if ( ((type == DISC_SENDER) ||
               (type == DISC_RECEIVER)) &&
               byte == C_DISC ) {
      *state = C_RCV;
    }
    else if (byte == FLAG) {
      *state = FLAG_RCV;
    }
    else {
      *state = START_RCV;
    }
    break;

  case C_RCV:
    if (type == UA_SENDER && byte == (A_SENDER^C_UA)) {
      *state = BCC_OK;
    }
    else if (type == UA_RECEIVER && byte == (A_RECEIVER^C_UA)) {
      *state = BCC_OK;
    }
    else if (type == SET && byte == (A_SENDER^C_SET)) {
      *state = BCC_OK;
    }
    else if (type == DISC_SENDER && byte == (A_SENDER^C_DISC)) {
      *state = BCC_OK;
    }
    else if (type == DISC_RECEIVER && byte == (A_RECEIVER^C_DISC)) {
      *state = BCC_OK;
    }
    else if (byte == FLAG) {
      *state = FLAG_RCV;
    }
    else {
      *state = START_RCV;
    }
    break;

  case BCC_OK:
    if (byte == FLAG) {
      *state = STOP_RCV;
    }
    else {
      *state = START_RCV;
    }
    break;

  default:
    break;
  }
}


int needsStuffing(char byte) {
  return (byte == FLAG || byte == ESCAPE);
}


void stuff(char *frame, int index, char byte) {
  frame[index] = ESCAPE;
  frame[index + 1] = byte ^ STUFF_BYTE;
}


int destuff(char *dst, char *src, int length) {
  int i, size;
  int foundEscape;

  foundEscape = 0;
  size = 0;
  for (i = 0; i < length; i++) {
    if (src[i] == ESCAPE) {
      foundEscape = 1;
      continue;
    }

    if (foundEscape) {
      dst[size++] = src[i] ^ STUFF_BYTE;
      foundEscape = 0;
    }
    else
      dst[size++] = src[i];
  }

  return size;
}


int buildPacket(char *dst, char *src, int length, char controlByte) {
  int i, n;
  unsigned char BCC1;
  unsigned char BCC2 = 0x00;

  n = 0;
  BCC1 = A_SENDER ^ controlByte;

  dst[n++] = FLAG;
  dst[n++] = A_SENDER;
  dst[n++] = controlByte;

  if (needsStuffing(BCC1)) {
    stuff(dst, n, BCC1);
    n += 2;
  }
  else {
    dst[n++] = BCC1;
  }


  for (i = 0; i < length; i++, n++) {
    BCC2 ^= src[i];

    if (needsStuffing(src[i])) {
      stuff(dst, n, src[i]);
      n++;
    }
    else {
      dst[n] = src[i];
    }
  }

  if (needsStuffing(BCC2)) {
    stuff(dst, n, BCC2);
    n += 2;
  }
  else {
    dst[n++] = BCC2;
  }

  dst[n++] = FLAG;

  return n;
}


int receivePacket(int fd, char *frame) {
  int i, flagCount;

  flagCount = 0;  i = 0;
  for (i = 0; flagCount < 2; i++) {
    read(fd, frame + i, 1);

    if (frame[i] == FLAG)
      flagCount++;
  }

  return i;
}


int stripAndValidate(char *dst, char *src, int length, char controlByte) {
  const int D1_INDEX = BCC1_INDEX + 1;
  const int END_FLAG_INDEX = length - 1;
  const int BCC2_INDEX = length - 2;
  unsigned char validateBCC2 = 0x00;
  int i;

  /* if (! (
    src[START_FLAG_INDEX] == FLAG &&
    src[A_INDEX] == A_SENDER &&
    src[C_INDEX] == controlByte &&
    src[BCC1_INDEX] == (src[A_INDEX] ^ src[C_INDEX]) &&
    src[END_FLAG_INDEX] == FLAG )) {

      return -1;
      }*/


  for (i = 0; i < BCC2_INDEX; i++) {
    dst[i] = src[i + D1_INDEX];
    validateBCC2 ^= dst[i];
  }

  if (validateBCC2 != src[BCC2_INDEX])
    return -1;

  return 0;
}

int receiveCommand(CommandType type) {
  int bytesRead, r;
  CommandState state;
  unsigned char receivedByte;

  bytesRead = 0;
  state = START_RCV;

  while (bytesRead != COMMAND_LENGTH) {
    r = read(fd, &receivedByte, 1);

    if (connectionTimedOut()) {
      exit(-1);
    }

    if (r == 1) {
      updateCommandState(&state, type, receivedByte);
      bytesRead++;
    }
  }

  if (state != STOP_RCV) {
    exit(-1);
  }

  return 0;
}


void reconnect(char *buffer, int length) {
  write(fd, buffer, length);
}
