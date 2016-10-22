#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
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
int appopenWriter(struct Application *app, const char *path, int oflag,
                      const char *fileName, unsigned int fileNameLength);
int appopenReader(struct Application *app, const char *path, int oflag);



int appopen(struct Application *app, const char *path, int oflag, int status, ...
            /*const char *fileName, unsigned int fileNameLength */) {
  va_list ap;
  int r;
  unsigned int fileNameLength;
  const char *fileName;

  va_start(ap, status);
  r = 0;

  app->status = status;
  if (status == TRANSMITTER) {
    fileName = va_arg(ap, char *);
    fileNameLength = va_arg(ap, unsigned int);
    r = appopenWriter(app, path, oflag, fileName, fileNameLength);
  }
  else {
    printf("hello1\n");
    r = appopenReader(app, path, oflag);
    printf("hello2\n");
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
  }

  if (createControlPacket(app, &packet, END) != 0) {
    exit(-1);
  }

  llwrite(app.filedes, packet.frame, packet.size);

  free(packet.frame);
  free(app.fileName);
  free(app.buffer);

  return 0;
}


int appread(struct Application app){
  FILE *file;
  char buf[LL_INPUT_MAX_SIZE];
  char name[256];
  unsigned int fileLength;
  int i;

  int s = llread(app.filedes, buf);

  memcpy(name, &buf[9], buf[8]);
  name[buf[8]] = '\0';

  printf("%s\n", name);

  file = fopen(name, "w");
  if (file == NULL) {
    fprintf(stderr, "Can't open file to write\n");
    return -1;
  }

  memcpy(&fileLength, &buf[3], sizeof(fileLength));

  int nrPackets = ceil((float) fileLength / LL_INPUT_MAX_SIZE);

  for(i = 0; i < nrPackets; i++) {
    int length = llread(app.filedes,buf);
    int j;
    for (j = 0; j < length; j++) {
      fprintf(file, "%c", buf[j]);
    }
  }

  s = llread(app.filedes, buf);

  return 0;
}

int appclose(struct Application app) {
  return llclose(app.filedes, app.status);
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




int appopenWriter(struct Application *app, const char *path, int oflag,
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


  app->filedes = llopen(path, oflag, TRANSMITTER);
  if (app->filedes == -1) {
    exit(-1);
  }

  return 0;
}

int appopenReader(struct Application *app, const char *path, int oflag) {
  app->filedes = llopen(path, oflag, RECEIVER);
  if (app->filedes == -1) {
    exit(-1);
  }

  return 0;
}
