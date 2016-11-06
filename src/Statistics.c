#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "Statistics.h"



/* Function headers */
struct StatisticsPacket statistics;

void setStatistics() {
  statistics.framesTransmitted = 0;
  statistics.framesReceived = 0;
  statistics.framesRepeated = 0;
  statistics.framesRejected = 0;
  statistics.timeOutCounter = 0;

  return;
}

void incFramesTransmitted() {
  statistics.framesTransmitted++;
}

void incFramesReceived() {
  statistics.framesReceived++;
}

void incFramesRepeated() {
  statistics.framesRepeated++;
}

void incFramesRejected() {
  statistics.framesRejected++;
}

void incTimeOut() {
  statistics.timeOutCounter++;
}

struct StatisticsPacket getStatisticsPacket() {
  return statistics;
}
