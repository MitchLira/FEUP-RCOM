#ifndef ALARM_H
#define ALARM_H

void handleAlarm();
void configAlarm(unsigned int nrTries, unsigned int waitPeriod);
void setAlarm(void (*func)(char *), char* buffer, int length);
int connectionTimedOut();

#endif
