#ifndef LINKLAYER_H
#define LINKLAYER_H

#define TRANSMITTER     0
#define RECEIVER        1
#define INPUT_MAX_SIZE  40

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } ConnectionState;


int llopen(const char* path, int oflag, int status);
int llwrite(int fd, char* buffer, int length);

#endif
