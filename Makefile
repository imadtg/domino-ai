#
# Updated Makefile to compile multiple C files into a single executable.
#
# - Sarah Mount, November 2011
# Updated to link into a single executable
#

CC=emcc
RM=rm -f
CFLAGS=-c -Wall -g
LDFLAGS=-lm -mconsole --shell-file html_template/shell_minimal.html -sNO_EXIT_RUNTIME -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sNO_INVOKE_RUN -sASSERTIONS -sEXPORT_ALL -sLINKABLE -g
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=build/index.html  # Name your executable

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) html_template/shell_minimal.html
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)