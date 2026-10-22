#ifndef BG_H
#define BG_H

#include "GFXI.H"

#define BG_WIDTH  640
#define BG_HEIGHT 400
#define BG_COLORS 16

static int load_palette(gfx_state_t *gfx, const char *path);
static int load_background(gfx_state_t *gfx, const char *path);
int display_bg(gfx_state_t *gfx, char *filename);
int prepare_background_pages(gfx_state_t *gfx, const char *name);

#endif
