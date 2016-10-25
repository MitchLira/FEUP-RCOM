/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "DataLink.h"
#include <math.h>
#include "Application.h"

#define BAUDRATE B38400

Interface loadInterface(int argc, char** argv){
  Interface interface;
  if(argc == 5)
  {
    argv[1] = interface.type;
    argv[2] = interface.baudrate;
    argv[3] = interface.retransmissions;
    argv[4] = interface.timeOut;
  }
  else
  {
    argv[1] = interface.type;
    interface.baudrate = BAUDRATE;
    interface.retransmissions = 3;
    interface.timeOut = 3;
  }

  return interface;

}
