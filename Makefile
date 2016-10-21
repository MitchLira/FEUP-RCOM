CC=gcc
CFLAGS = -lm -Wall

all: writenoncanonical noncanonical

writenoncanonical: DataLink.h Application.h Alarm.h writenoncanonical.c
	$(CC) writenoncanonical.c DataLink.c Application.c Alarm.c -o writenoncanonical $(CFLAGS)
	
noncanonical: DataLink.h Application.h Alarm.h noncanonical.c
	$(CC) noncanonical.c DataLink.c Application.c Alarm.c -o noncanonical $(CFLAGS)
	
clean:
	rm -f writenoncanonical
	rm -f noncanonical
