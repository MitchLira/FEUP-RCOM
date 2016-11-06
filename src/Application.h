#ifndef APPLICATION_H
#define APPLICATION_H

struct Application {
        int status;
        int filedes;
        char *fileName;
        char *buffer;
        int fileNameLength;
        unsigned int fileSize;
};


int appopen(struct Application *app, const char *path, int oflag, int status);
int appwrite(struct Application app);
int appread(struct Application app);
int appclose(struct Application app);

#endif
