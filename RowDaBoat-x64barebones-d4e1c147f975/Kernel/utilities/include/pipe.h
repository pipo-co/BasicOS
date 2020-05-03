#ifndef __PIPE_H__
#define __PIPE_H__

int initPipes();

int32_t openPipe(char * pipeName);

int writePipe(uint16_t pipe, char c);

char readPipe(uint16_t pipe);

void closePipe(uint16_t pipe);

void dumpPipes();

#endif
