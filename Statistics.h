#ifndef STATISTICS_H
#define STATISTICS_H

#define BUFFER_LENGTH 256

struct StatisticsPacket {
  int framesTransmitted;
  int framesReceived;
  int framesRepeated;
  int framesRejected;
  int timeOutCounter;
};

void setStatistics();
void incFramesTransmitted();
void incFramesReceived();
void incFramesRepeated();
void incFramesRejected();
void incTimeOut();
struct StatisticsPacket getStatisticsPacket();
#endif
