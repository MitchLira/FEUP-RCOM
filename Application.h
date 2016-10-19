#ifndef APPLICATION_H
#define APPLICATION_H

struct Application {
  int filedes;
  char *fileName;
  char *buffer;
  int fileNameLength;
  unsigned int fileSize;
};


int appopen(struct Application *app, const char *fileName, int length, const char *path, int oflag, int status);
int appwrite(struct Application app);
void appclose(struct Application app);

#endif
