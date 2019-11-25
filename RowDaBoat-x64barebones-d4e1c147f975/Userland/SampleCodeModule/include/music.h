//music.c
//Funciones que tocan canciones :D
//¿ O sustos que dan gustos ?
#ifndef __MUSIC_H__
#define __MUSIC_H__

//enum con los datos necesarios para tocar la nota correspondiente utilizando sysBeep (syscall 8)
enum chords{A=880, AS= 932, B=988, C=523, CS=554, D=587, DS=622, 
E=659, F=698, FS=740, G=784, GS=831, S=250000 };

void Victory();

void Defeat();

void forElisa();

void Evangelion();

void Lavander();

void Sadness();

#endif