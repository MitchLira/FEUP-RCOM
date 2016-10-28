#ifndef SETTINGS_H
#define SETTINGS_H

#define BUFFER_LENGTH 256

struct SettingsReceiver {
  char fileName[BUFFER_LENGTH];
  int timeout;
  int baudrate;
};

struct SettingsTransmitter {
  char fileName[BUFFER_LENGTH];
  int retries;
  int timeout;
  int baudrate;
};

void loadReceiverSettings();
void loadTransmitterSettings();
struct SettingsTransmitter getSettingsTransmitter();
struct SettingsReceiver getSettingsReceiver();


#endif
