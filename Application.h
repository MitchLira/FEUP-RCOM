#ifndef APPLICATION_H
#define APPLICATION_H

struct Application {
  int filedes;
  char *fileName;
  char *buffer;
  int fileNameLength;
  unsigned int fileSize;
};


int appopen(struct Application *app, const char *path, int oflag, int status, ...
              /*const char *fileName, unsigned int fileNameLength */);
int appwrite(struct Application app);
void appclose(struct Application app);

#endif
