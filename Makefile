#
# Updated Makefile to compile multiple C files into a single executable.
#
# - Sarah Mount, November 2011
# Updated to link into a single executable
#

CC=emcc
RM=rm -f
CFLAGS=-c -Wall -g -sWASM_WORKERS
LDFLAGS=-lm -mconsole --shell-file html_template/shell_minimal.html -sNO_EXIT_RUNTIME -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,wasmMemory,getValue,setValue -sNO_INVOKE_RUN -sASSERTIONS -sEXPORT_ALL -g -sERROR_ON_WASM_CHANGES_AFTER_LINK -sWASM_BIGINT -sENVIRONMENT=web,webview,worker -sEXPORT_NAME="createModule" -sMODULARIZE -sWASM_WORKERS
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=build/domino-c.html  # Name your executable
EXEC_DIR=$(dir $(EXECUTABLE))

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) html_template/shell_minimal.html | $(EXEC_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(EXEC_DIR):
# HACK: the following wont work for nested directories, i dont expect i will be doing that anytime soon since making that cross platform is a pain
	mkdir $@

%.o: %.c Makefile
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)