#ifndef LINKLAYER_H
#define LINKLAYER_H

#define TRANSMITTER        0
#define RECEIVER           1
#define LL_INPUT_MAX_SIZE  256


int llopen(const char *path, int oflag, int status);
int llwrite(int fd, char *buffer, int length);
int llread(int fd, char *buffer);
int llclose(int fd, int status);

#endif
