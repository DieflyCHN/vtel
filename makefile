CC=gcc
CFLAGS=-Wall -Wextra -O2

all: vtel

vtel: main.c sender.c receiver.c decoder.c sender.h receiver.h decoder.h
	$(CC) $(CFLAGS) \
	main.c \
	sender.c \
	receiver.c \
	decoder.c \
	-o vtel

clean:
	rm -f vtel
