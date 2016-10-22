#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "Application.h"
#include "DataLink.h"


#define START   2
#define END     3
#define T_FILE_SIZE 0
#define T_FILE_NAME 1


struct Packet {
  char *frame;
  int size;
};



/* Function headers */
int createControlPacket(struct Application app, struct Packet *packet, char C_FLAG);
int appopenWriter(struct Application *app, const char *path, int oflag, int status,
                      const char *fileName, unsigned int fileNameLength);



int appopen(struct Application *app, const char *path, int oflag, int status, ...
            /*const char *fileName, unsigned int fileNameLength */) {
  va_list ap;
  int r;
  unsigned int fileNameLength;
  const char *fileName;

  va_start(ap, status);
  r = 0;

  if (status == TRANSMITTER) {
    fileName = va_arg(ap, char *);
    fileNameLength = va_arg(ap, unsigned int);
    r = appopenWriter(app, path, oflag, status, fileName, fileNameLength);
  }
  else {

  }

  va_end(ap);
  return r;
}


int appwrite(struct Application app) {
  struct Packet packet;
  int r, i, nrPackets, bytesRemaining;

  if (createControlPacket(app, &packet, START) != 0) {
    exit(-1);
  }

  llwrite(app.filedes, packet.frame, packet.size);
  free(packet.frame);

  nrPackets = ceil((float) app.fileSize / LL_INPUT_MAX_SIZE);
  bytesRemaining = app.fileSize;


  for (i = 0; i < nrPackets; i++) {
    int size;

    if (bytesRemaining < LL_INPUT_MAX_SIZE)
      size = bytesRemaining;
    else
      size = LL_INPUT_MAX_SIZE;\

    llwrite(app.filedes, &app.buffer[i * LL_INPUT_MAX_SIZE], size);
    bytesRemaining -= size;
    /*
    int j;
    for (j = 0; j < size; j++) {
      printf("%02X - ", (unsigned char) app.buffer[i*LL_INPUT_MAX_SIZE + j]);
    }
    printf("\n");
    */
  }

  if (createControlPacket(app, &packet, END) != 0) {
    exit(-1);
  }

  for (i = 0; i < packet.size; i++) {
    printf("%02X\n", (unsigned char) packet.frame[i]);
  }

  llwrite(app.filedes, packet.frame, packet.size);
  free(packet.frame);

  printf("%d\n", app.fileSize);
  return 0;
}



void appclose(struct Application app) {
  llclose(app.filedes, TRANSMITTER);
  free(app.fileName);
  free(app.buffer);
}



int createControlPacket(struct Application app, struct Packet *packet, char C_FLAG) {

  packet->size = 9 + app.fileNameLength;
  packet->frame = (char *) malloc(packet->size);
  if (packet->frame == NULL) {
    return -1;
  }

  packet->frame[0] = C_FLAG;
  packet->frame[1] = T_FILE_SIZE;
  packet->frame[2] = sizeof(app.fileSize);
  memcpy(&packet->frame[3], &app.fileSize, sizeof(app.fileSize));

  packet->frame[7] = T_FILE_NAME;
  packet->frame[8] = app.fileNameLength;
  memcpy(&packet->frame[9], app.fileName, app.fileNameLength);

  return 0;
}




int appopenWriter(struct Application *app, const char *path, int oflag, int status,
                      const char *fileName, unsigned int fileNameLength) {
  FILE *file;

  app->fileNameLength = fileNameLength;
  app->fileName = (char *) malloc(sizeof(app->fileNameLength));
  if (app->fileName == NULL) {
    exit(-1);
  }

  strcpy(app->fileName, fileName);

  file = fopen(fileName, "rb");
  if (file == NULL) {
    exit(-1);
  }

  fseek(file, 0, SEEK_END);
  app->fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  app->buffer = (char *) malloc(app->fileSize);
  if (app->buffer == NULL) {
    exit(-1);
  }

  fread(app->buffer, app->fileSize, sizeof(char), file);
  fclose(file);


  app->filedes = llopen(path, oflag, status);
  if (app->filedes == -1) {
    exit(-1);
  }

  return 0;
}
