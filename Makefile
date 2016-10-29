CC=gcc
CFLAGS = -lm -Wall

transfer: DataLink.h Application.h Alarm.h Settings.h transfer.c
	$(CC) transfer.c DataLink.c Application.c Alarm.c Settings.c Statistics.c -o transfer  $(CFLAGS)

clean:
	rm -f transfer
