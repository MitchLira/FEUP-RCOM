CC=gcc
CFLAGS = -lm -Wall

all: writenoncanonical noncanonical

writenoncanonical: DataLink.h Application.h writenoncanonical.c
	$(CC) writenoncanonical.c DataLink.c Application.c -o writenoncanonical $(CFLAGS)
	
noncanonical: DataLink.h Application.h noncanonical.c
	$(CC) noncanonical.c DataLink.c Application.c -o noncanonical $(CFLAGS)
	
clean:
	rm -f writenoncanonical
	rm -f noncanonical
