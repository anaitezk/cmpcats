CC = gcc
CFLAGS = -Wall -Wextra

all: cmpcats

cmpcats: cmpcats.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f cmpcats
