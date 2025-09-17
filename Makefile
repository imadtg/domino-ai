#
# Updated Makefile to compile multiple C files into a single executable.
#
# - Sarah Mount, November 2011
# Updated to link into a single executable
#

CC=gcc
ifeq ($(OS), Windows_NT)
RM=del /F /Q
else
RM=rm -f
endif
CFLAGS=-c -Wall -O3
LDFLAGS=-lm
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=out/dominoai  # Name your executable

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
