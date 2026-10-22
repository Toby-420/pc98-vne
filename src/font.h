#ifndef FONT_H
#define FONT_H

#include "GFXI.H"

int font_load(const char *path);

void font_draw_char(gfx_state_t *gfx, unsigned x, unsigned y, char c,
                    unsigned color);

void font_draw_string(gfx_state_t *gfx, unsigned x, unsigned y,
                      const char *text, unsigned color);

#endif
