.DEFAULT_GOAL := native

NATIVE_CC = gcc
WASM_CC = emcc

ifeq ($(OS), Windows_NT)
RM = del /F /Q
NATIVE_EXECUTABLE = out\dominoai.exe
WASM_ARTIFACTS = dist\index.mjs dist\index.wasm
else
RM = rm -f
NATIVE_EXECUTABLE = out/dominoai.out
WASM_ARTIFACTS = dist/index.mjs dist/index.wasm
endif

WASM_TARGET = dist/index.mjs

NATIVE_CFLAGS = -c -Wall -O3
NATIVE_LDFLAGS = -lm
WASM_CFLAGS = -c -Wall -g -sWASM_WORKERS
WASM_LDFLAGS = -lm -sNO_EXIT_RUNTIME -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,wasmMemory,getValue,setValue -sNO_INVOKE_RUN -sASSERTIONS -sEXPORT_ALL -g -sERROR_ON_WASM_CHANGES_AFTER_LINK -sWASM_BIGINT -sENVIRONMENT=web,webview,worker -sEXPORT_NAME="createModule" -sMODULARIZE -sEXPORT_ES6 -sWASM_WORKERS

SOURCES = $(wildcard *.c)
NATIVE_SOURCES = $(filter-out wasm.c,$(SOURCES))
WASM_SOURCES = $(SOURCES)
NATIVE_OBJECTS = $(NATIVE_SOURCES:.c=.native.o)
WASM_OBJECTS = $(WASM_SOURCES:.c=.wasm.o)

.PHONY: all native wasm clean clean-native clean-wasm

all: native wasm

native: $(NATIVE_EXECUTABLE)

wasm: $(WASM_TARGET)

$(NATIVE_EXECUTABLE): $(NATIVE_OBJECTS)
	$(NATIVE_CC) $(NATIVE_OBJECTS) $(NATIVE_LDFLAGS) -o $@

$(WASM_TARGET): $(WASM_OBJECTS) | dist
	$(WASM_CC) $(WASM_OBJECTS) $(WASM_LDFLAGS) -o $@

dist:
	mkdir dist

%.native.o: %.c Makefile
	$(NATIVE_CC) $(NATIVE_CFLAGS) $< -o $@

%.wasm.o: %.c Makefile
	$(WASM_CC) $(WASM_CFLAGS) $< -o $@

clean: clean-native clean-wasm

clean-native:
	$(RM) $(NATIVE_OBJECTS) $(NATIVE_EXECUTABLE)

clean-wasm:
	$(RM) $(WASM_OBJECTS) $(WASM_ARTIFACTS)
