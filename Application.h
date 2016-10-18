#ifndef APPLICATION_H
#define APPLICATION_H

struct Application {
  int valid;
  unsigned long fileLength;
  unsigned char* fileName;
  unsigned char* buffer;
};


struct Application appopen(const char *fileName);
void appclose(struct Application app);

#endif
