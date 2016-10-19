#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int createStartPacket(struct Application app, struct Packet *packet);



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
  int r, i;

  r = createStartPacket(app, &packet);
  if (r != 0) {
    exit(-1);
  }

  llwrite(app.filedes, packet.frame, packet.size);

  free(packet.frame);
  return 0;
}



void appclose(struct Application app) {
  printf("Fecha\n");
  free(app.fileName);
  free(app.buffer);
  llclose(app.filedes);
}



int createStartPacket(struct Application app, struct Packet *packet) {

  packet->size = 9 + app.fileNameLength;
  packet->frame = (char *) malloc(packet->size);
  if (packet->frame == NULL) {
    return -1;
  }

  packet->frame[0] = START;
  packet->frame[1] = T_FILE_SIZE;
  packet->frame[2] = 4;
  packet->frame[3] = app.fileSize % 0x100;
  packet->frame[4] = (app.fileSize % 0x1000) % 0x100;
  packet->frame[5] = (app.fileSize % 0x10000) % 0x100;
  packet->frame[6] = app.fileSize % 0x1000000;
  packet->frame[7] = T_FILE_NAME;
  packet->frame[8] = app.fileNameLength;
  memcpy(&packet->frame[9], app.fileName, app.fileNameLength);

  return 0;
}
