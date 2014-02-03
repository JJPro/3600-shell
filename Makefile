TARGET = 3600sh
CC = gcc
CFLAGS = -std=c99 -g -c -Wall -pedantic -Werror -Wextra
LDFLAGS = -lm -O0

$(TARGET): $(TARGET).o lib.o
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET).o : lib.o

lib.o : lib.c
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET).o : $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $<

all: $(TARGET)

test: all
	./test

clean:
	rm $(TARGET) 3600sh.o lib.o *~
