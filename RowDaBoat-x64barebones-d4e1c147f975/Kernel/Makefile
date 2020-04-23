include Makefile.inc
#Se modifico el archivo para que pueda compilar archivos en otras 
# carpetas que no eran la original.
MM=-UBUDDY
KERNEL=kernel.bin
SOURCES=$(wildcard *.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_DRIVERS=$(wildcard drivers/*.c)
SOURCES_UTILITIES=$(wildcard utilities/*.c)
SOURCES_INTERRUPTS=$(wildcard interrupts/*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
OBJECTS_DRIVERS=$(SOURCES_DRIVERS:.c=.o)
OBJECTS_UTILITIES=$(SOURCES_UTILITIES:.c=.o)
OBJECTS_INTERRUPTS=$(SOURCES_INTERRUPTS:.c=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)

STATICLIBS=

ALL_OBJECTS= $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(OBJECTS_INTERRUPTS) $(OBJECTS_DRIVERS) $(OBJECTS_UTILITIES)

all: $(KERNEL)

buddy: MM=-DBUDDY all

$(KERNEL): $(STATICLIBS) $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(ALL_OBJECTS) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I ./drivers/include -I./interrupts/include -I ./utilities/include $(MM) -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(ALL_OBJECTS) $(KERNEL)

.PHONY: all clean buddy
