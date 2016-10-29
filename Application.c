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
#include "Settings.h"

#define START   2
#define END     3
#define T_FILE_SIZE 0
#define T_FILE_NAME 1


/* Enums / structs */
struct Packet {
        char *frame;
        int size;
};


/* Function headers */
int createControlPacket(struct Application app, struct Packet *packet, char C_FLAG);
int appopenWriter(struct Application *app, const char *path, int oflag, const char *fileName, unsigned int fileNameLength);
int appopenReader(struct Application *app, const char *path, int oflag);
void writeToFile(FILE *file, char *buf, int length);




int appopen(struct Application *app, const char *path, int oflag, int status) {
        int r;
        struct SettingsTransmitter settingsT;

        r = 0;
        app->status = status;
        if (status == TRANSMITTER) {
                settingsT = getSettingsTransmitter();

                fprintf(stdout, "Connecting to the receiver...\n");
                r = appopenWriter(app, path, oflag, settingsT.fileName, strlen(settingsT.fileName));
                fprintf(stdout, "Connected successfully!\n\n");
        }
        else {
                fprintf(stdout, "Connecting to the sender...\n");
                r = appopenReader(app, path, oflag);
                fprintf(stdout, "Connected successfully!\n\n");
        }

        return r;
}


int appwrite(struct Application app) {
        struct Packet packet;
        int i, nrPackets, bytesRemaining;


        if (createControlPacket(app, &packet, START) != 0) {
                fprintf(stderr, "Unable to create end control packet.\n");
                exit(-1);
        }

        fprintf(stdout, "Sending START control packet...\n");
        if (llwrite(app.filedes, packet.frame, packet.size) == -1) {
          fprintf(stderr, "Unable to send START packet.\n");
          exit(-1);
        }
        fprintf(stdout, "Sent successfully.\n");

        free(packet.frame);

        nrPackets = ceil((float) app.fileSize / LL_INPUT_MAX_SIZE);
        bytesRemaining = app.fileSize;

        for (i = 0; i < nrPackets; i++) {
                fprintf(stdout, "\n\nSending packet #%d...\n", i+1);

                int size;
                if (bytesRemaining < LL_INPUT_MAX_SIZE)
                        size = bytesRemaining;
                else
                        size = LL_INPUT_MAX_SIZE;

                if (llwrite(app.filedes, &app.buffer[i * LL_INPUT_MAX_SIZE], size) == -1) {
                  fprintf(stderr, "Something occured. Unable to send file's info!\n");
                  exit(-1);
                }

                bytesRemaining -= size;

                fprintf(stdout, "Packet sent successfully!\n");
        }

        if (createControlPacket(app, &packet, END) != 0) {
                fprintf(stderr, "Unable to create end control packet.\n");
                exit(-1);
        }


        fprintf(stdout, "\n\nSending END control packet...\n");
        if (llwrite(app.filedes, packet.frame, packet.size) == -1) {
          fprintf(stderr, "Unable to send END packet.\n");
          exit(-1);
        }
        fprintf(stdout, "Sent successfully.\n");

        free(packet.frame);
        free(app.fileName);
        free(app.buffer);
        return 0;
}


int appread(struct Application app){
        FILE *file;
        char buf[LL_INPUT_MAX_SIZE];
        unsigned int fileLength;
        int i;
        struct SettingsReceiver settingsR;


        settingsR = getSettingsReceiver();

        fprintf(stdout, "Receiving START control packet...\n");
        if (llread(app.filedes, buf) == -1) {
          exit(-1);
        }
        fprintf(stdout, "Received.\n");

        if (strcmp(settingsR.fileName, "source") == 0) {
          memcpy(settingsR.fileName, &buf[9], buf[8]);
        }

        file = fopen(settingsR.fileName, "w");
        if (file == NULL) {
                fprintf(stdout, "Can't open file to write.\n");
                return -1;
        }

        memcpy(&fileLength, &buf[3], buf[2]);

        int nrPackets = ceil((float) fileLength / LL_INPUT_MAX_SIZE);

        for(i = 0; i < nrPackets; i++) {
                fprintf(stdout, "\n\nReceiving packet #%d...\n", i+1);
                int length = llread(app.filedes, buf);
                if (length == -1) {
                  exit(-1);
                }
                writeToFile(file, buf, length);
                printf("Received packet successfully!\n");
        }

        fprintf(stdout, "\n\nReceiving END control packet...\n");
        if (llread(app.filedes, buf) == -1) {
          exit(-1);
        }
        fprintf(stdout, "Received.\n");

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
                fprintf(stderr, "File \"%s\" doesn't exist!\n", fileName);
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


        app->filedes = llopen(path,oflag, TRANSMITTER);
        if (app->filedes == -1) {
                exit(-1);
        }

        return 0;
}


int appopenReader(struct Application *app, const char *path, int oflag) {
        app->filedes = llopen(path,oflag, RECEIVER);
        if (app->filedes == -1) {
                exit(-1);
        }

        return 0;
}

void writeToFile(FILE *file, char *buf, int length) {
        int i;

        for (i = 0; i < length; i++) {
                fprintf(file, "%c", buf[i]);
        }
}
