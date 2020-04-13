CC = gcc
CFLAGS = -pedantic -Wall -std=c99 -pthread -lrt -g -DBUDDY -lm
RM = rm -f

SOURCES = $(wildcard *.c)
BINS = $(SOURCES:.c=.out)

CPPANS = $(SOURCES:.c=.cppout)


all: $(BINS)

%.out: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean: clean_test
	$(RM) $(BINS)

clean_test:
	$(RM) $(CPPANS) *.valout report.tasks

test: clean $(CPPANS) 
	./pvs.sh

%.cppout: %.c
	cppcheck --quiet --enable=all --force --inconclusive  $^ 2> $@


.PHONY: all clean test clean_test