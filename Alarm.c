#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


void (*handler)(char *, unsigned int);
char *message;
unsigned int messageLength;

unsigned int numberTries;
unsigned int attempt;
unsigned int waitTime;

int timedOut;


void handleAlarm() {
  if (attempt <= numberTries) {
    fprintf(stdout, "No response received. Retrying... (%d)\n", attempt);
    handler(message, messageLength);
    alarm(waitTime);
  }
  else {
    timedOut = 1;
    alarm(0);
  }

  attempt++;
  return;
}


void configAlarm(unsigned int nrTries, unsigned int waitPeriod) {
  (void) signal(SIGALRM, handleAlarm);   // Install routine to re-send SET if receiver doesn't respond
  numberTries = nrTries;
  waitTime = waitPeriod;
}


void setAlarm(void (*func)(char *, unsigned int), char *buffer, unsigned int length) {
  handler = func;
  message = buffer;
  messageLength = length;

  attempt = 1;
  timedOut = 0;
  alarm(waitTime);
}

void disableAlarm() {
  alarm(0);
}


int connectionTimedOut() {
  return timedOut;
}
