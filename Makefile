CC=gcc
CFLAGS = -Wall -O3

all: writenoncanonical noncanonical

writenoncanonical: linklayer.h writenoncanonical.c
	$(CC) $(CFLAGS) writenoncanonical.c linklayer.c -o writenoncanonical
	
noncanonical: linklayer.h noncanonical.c
	$(CC) $(CFLAGS) noncanonical.c linklayer.c -o noncanonical
	
clean:
	rm -f writenoncanonical
	rm -f noncanonical
