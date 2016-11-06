#ifndef ALARM_H
#define ALARM_H

void handleAlarm();
void configAlarm(unsigned int nrTries, unsigned int waitPeriod);
void setAlarm(void (*func)(char *, unsigned int), char *buffer, unsigned int length);
void disableAlarm();
int connectionTimedOut();

#endif
