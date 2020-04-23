include ../Makefile.inc
# Archivo modificado para la compilacion de archivos assembler e inclusion de archivos .h

MODULE=0000-sampleCodeModule.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard *.asm)
SOURCES_TESTS=$(wildcard tests/*.c)

OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

all: $(MODULE)

$(MODULE): $(SOURCES) $(OBJECTS_ASM) $(SOURCES_TESTS)
	$(GCC) $(GCCFLAGS) -I./include -T sampleCodeModule.ld _loader.c $(SOURCES) $(OBJECTS_ASM) $(SOURCES_TESTS) -o ../$(MODULE)


%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean print
