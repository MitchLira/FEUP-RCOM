CC=gcc
CFLAGS = -Wall -O3

all: writenoncanonical noncanonical

writenoncanonical: DataLink.h writenoncanonical.c
	$(CC) $(CFLAGS) writenoncanonical.c DataLink.c -o writenoncanonical
	
noncanonical: DataLink.h noncanonical.c
	$(CC) $(CFLAGS) noncanonical.c DataLink.c -o noncanonical
	
clean:
	rm -f writenoncanonical
	rm -f noncanonical
