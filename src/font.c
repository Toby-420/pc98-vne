#include <stdio.h>
#include "font.h"

#define FONT_CHARS 128
#define FONT_HEIGHT 8
#define FONT_WIDTH 8

static unsigned char font_data[FONT_CHARS][FONT_HEIGHT];
static int font_loaded = 0;

int font_load(const char *path)
{
    FILE *fp = fopen(path, "rb");

    if (!fp) return 0;

    if (fread(font_data, 1, sizeof(font_data), fp)
        != sizeof(font_data))
    {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    font_loaded = 1;
    return 1;
}

void font_draw_char(gfx_state_t *gfx, unsigned x, unsigned y, char c,
                    unsigned color)
{
    unsigned row, col;
    unsigned char bits;
    unsigned char ch = (unsigned char)c;

    if (!font_loaded) return;

    if (ch >= FONT_CHARS) ch = '?';

    for (row = 0; row < FONT_HEIGHT; ++row)
    {
        bits = font_data[ch][row];

        for (col = 0; col < FONT_WIDTH; ++col)
        {
            if (bits & (0x80 >> col))
            {
                gfx_set_pixel(gfx, x + col, y + row, color);
            }
        }
    }
}

void font_draw_string(gfx_state_t *gfx, unsigned x, unsigned y,
                      const char *text, unsigned color)
{
    unsigned start_x = x;

    if (!text)
        return;

    while (*text)
    {
        if (*text == '\n')
        {
            x = start_x;
            y += FONT_HEIGHT + 2;
        } else if (*text == '\r')
        {
            // Ignore carriage returns
        } else
        {
            font_draw_char(gfx, x, y, *text, color);
            x += FONT_WIDTH;
        }

        ++text;
    }
}
