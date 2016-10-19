CC=gcc
CFLAGS = -Wall

all: writenoncanonical noncanonical

writenoncanonical: DataLink.h Application.h writenoncanonical.c
	$(CC) $(CFLAGS) writenoncanonical.c DataLink.c Application.c -o writenoncanonical
	
noncanonical: DataLink.h Application.h noncanonical.c
	$(CC) $(CFLAGS) noncanonical.c DataLink.c Application.c -o noncanonical
	
clean:
	rm -f writenoncanonical
	rm -f noncanonical
