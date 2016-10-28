#ifndef SETTINGS_H
#define SETTINGS_H

struct SettingsReceiver {
  int timeout;
  int baudrate;
};

struct SettingsTransmitter {
  int retries;
  int timeout;
  int baudrate;
};

void loadReceiverSettings();
void loadTransmitterSettings();


#endif
