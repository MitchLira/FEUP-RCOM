CC=gcc
CFLAGS = -lm -Wall

transfer: src/DataLink.h src/Application.h src/Alarm.h src/Settings.h src/transfer.c
	$(CC) src/transfer.c src/DataLink.c src/Application.c src/Alarm.c src/Settings.c src/Statistics.c -o transfer  $(CFLAGS)

clean:
	rm -f transfer
