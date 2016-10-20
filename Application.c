#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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



int appopen(struct Application *app, const char *fileName, int length, const char *path, int oflag, int status) {
  FILE *file;

  app->fileNameLength = length;
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
  printf("Fecha\n");
  free(app.fileName);
  free(app.buffer);
  llclose(app.filedes);
}



int createControlPacket(struct Application app, struct Packet *packet, char C_FLAG) {

  packet->size = 9 + app.fileNameLength;
  packet->frame = (char *) malloc(packet->size);
  if (packet->frame == NULL) {
    return -1;
  }

  packet->frame[0] = C_FLAG;
  packet->frame[1] = T_FILE_SIZE;
  packet->frame[2] = 4;
  packet->frame[3] = app.fileSize & 0xFF;
  packet->frame[4] = (app.fileSize >> 8) & 0xFF;
  packet->frame[5] = (app.fileSize >> 16) & 0xFF;
  packet->frame[6] = (app.fileSize >> 24) & 0xFF;
  packet->frame[7] = T_FILE_NAME;
  packet->frame[8] = app.fileNameLength;
  memcpy(&packet->frame[9], app.fileName, app.fileNameLength);

  return 0;
}
