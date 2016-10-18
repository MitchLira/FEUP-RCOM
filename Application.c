#include <stdio.h>
#include <stdlib.h>
#include "Application.h"


struct Application appopen(const char *fileName) {
  FILE *file;
  struct Application app;

  app.fileName = fileName;

  file = fopen(fileName, "rb");
  if (file == NULL) {
    app.valid = -1;
    return app;
  }

  fseek(file, 0, SEEK_END);
  app.fileLength = ftell(file);
  fseek(file, 0, SEEK_SET);

  app.buffer = (unsigned char *) malloc(app.fileLength);
  if (app.buffer == NULL) {
    app.valid = -2;
    return app;
  }

  fread(app.buffer, app.fileLength, sizeof(unsigned char), file);
  fclose(file);

  return app;
}


void appclose(struct Application app) {
  printf("Fecha\n");
  free(app.buffer);
}
