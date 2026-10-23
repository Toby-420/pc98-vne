#ifndef MUSIC_H
#define MUSIC_H

#include "GFXI.H"

static int pmd_call(unsigned char function, union REGS *out);

int music_init(void);

int music_play(const char *filename);

void music_stop(void);

void music_shutdown(void);

#endif
