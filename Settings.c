#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "Settings.h"

#define BUFFER_SIZE       256
#define DEFAULT_TIMEOUT   3
#define DEFAULT_RETRIES   3
#define BAUDRATE1200      B1200
#define BAUDRATE1800      B1800
#define BAUDRATE2400      B2400
#define BAUDRATE4800      B4800
#define BAUDRATE9600      B9600
#define BAUDRATE38400     B38400
#define BAUDRATE57600     B57600
#define BAUDRATE115200    B115200


/* Function headers */
void clean();
int getTimeout();
int getRetries();
int getBaudRate();
void getFileName(char* fileName);

struct SettingsReceiver settingsR;
struct SettingsTransmitter settingsT;

void loadReceiverSettings() {

  getFileName(settingsR.fileName);

  settingsR.timeout = getTimeout();

  settingsR.baudrate = getBaudRate();

}

void loadTransmitterSettings() {

  getFileName(settingsT.fileName);

  settingsT.timeout = getTimeout();

  settingsT.retries = getRetries();

  settingsT.baudrate = getBaudRate();

}

void getFileName(char* fileName) {

  printf("\nEnter the path: ");
  fgets(fileName, BUFFER_LENGTH, stdin);
  fileName[strcspn(fileName, "\n")] = '\0';
}

int getTimeout() {
    int valid;
    int timeout;

    printf("\nChoose time-out time in seconds (0 - default): ");
    valid = scanf("%d", &timeout);
    clean();

    while (valid < 0 || timeout < 2 || timeout > 10) {
      if (timeout == 0) {
        return DEFAULT_TIMEOUT;
      }

      printf("Invalid value! Insert a value between 2 and 10: ");
      valid = scanf("%d", &timeout);
      clean();
    }

    return timeout;
}

int getRetries(){
  int valid;
  int retries;

  printf("\nChoose number of retries (0 - default): ");
  valid = scanf("%d", &retries);
  clean();


  while (valid < 0 || retries < 1) {
    if (retries == 0) {
      return DEFAULT_RETRIES;
    }

    printf("Invalid value! Insert a value greater than 0: ");
    valid = scanf("%d", &retries);
    clean();
  }

      return retries;
}


int getBaudRate() {
  int baudrate;
  int valid;

  printf("\nChoose baudrate:\n");
  printf("0 - 1200\n1 - 1800\n2 - 2400\n3 - 4800\n4 - 9600\n5 - 38400\n6 - 57600\n7 - 115200\n");
  printf("\nInsert your choice: ");
  valid = scanf("%d", &baudrate);
  clean();

  while (valid < 0 || baudrate < 0 || baudrate > 7) {
    printf("Invalid value! Insert a value between 0 and 7: ");
    valid = scanf("%d", &baudrate);
    clean();
  }
  switch (baudrate) {
    case 0:
      return BAUDRATE1200;
    break;
    case 1:
      return BAUDRATE1800;
    break;
    case 2:
      return BAUDRATE2400;
    break;
    case 3:
      return BAUDRATE4800;
    break;
    case 4:
      return BAUDRATE9600;
    break;
    case 5:
      return BAUDRATE38400;
    break;
    case 6:
      return BAUDRATE57600;
    break;
    case 7:
      return BAUDRATE115200;
    break;

  }

  return -1;
}


struct SettingsTransmitter getSettingsTransmitter()
{
  return settingsT;
}
struct SettingsReceiver getSettingsReceiver()
{
  return settingsR;
}

void clean() {
  char c;

  while ( (c = getchar()) != '\n' && c != EOF ) ;
}
