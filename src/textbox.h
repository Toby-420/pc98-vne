#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "GFXI.H"

void textbox_redraw_both_pages(gfx_state_t *gfx, int typewrite_delay);
void textbox_draw(gfx_state_t *gfx);
void textbox_set_speaker(const char *speaker);
void textbox_set_text(const char *text);
void textbox_show(gfx_state_t *gfx, int typewrite_delay);
int menu_show(gfx_state_t *gfx, char *options[], int count);

#endif
