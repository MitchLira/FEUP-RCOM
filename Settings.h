#ifndef SETTINGS_H
#define SETTINGS_H

struct SettingsReceiver {
  int timeout;
};

struct SettingsTransmitter {
  int retries;
  int timeout;
  int baudrate;
};

struct SettingsReceiver loadReceiverSettings();
struct SettingsTransmitter loadTransmitterSettings();


#endif
