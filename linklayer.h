#ifndef LINKLAYER_H
#define LINKLAYER_H

#define TRANSMITTER 0
#define RECEIVER    1

typedef enum { START_RCV, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP_RCV } ConnectionState;


int llopen(const char* path, int oflag, int status);

#endif
