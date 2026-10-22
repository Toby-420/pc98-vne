#include <string.h>
#include <conio.h>
#include "textbox.h"
#include "font.h"

#define BOX_X       0
#define BOX_Y       304
#define BOX_W       640
#define BOX_H       104

#define NAME_X      0
#define NAME_Y      288
#define NAME_W      640
#define NAME_H      24

#define COLOR_BG      0
#define COLOR_BORDER  7
#define COLOR_ACCENT  8
#define COLOR_TEXT   15

#define SPEAKER_MAX  31
#define TEXT_MAX    255

#define TEXT_X       (BOX_X + 20)
#define TEXT_Y       (BOX_Y + 16)
#define TEXT_WIDTH   552
#define LINE_HEIGHT  12
#define MAX_LINES    6

#define MENU_X       340
#define MENU_Y       220
#define MENU_SPACING 20
#define MENU_WIDTH   280
#define MENU_HEIGHT  16

#define MENU_COLOR   15
#define SELECT_COLOR 14


static char speaker[SPEAKER_MAX + 1] = "";
static char dialogue[TEXT_MAX + 1] = "";


static void textbox_redraw_both_pages(gfx_state_t *gfx)
{
    int display;
    int rendering;
    int hidden;

    display = gfx_get_plane(gfx, GFX_PLANE_DISPLAY);
    rendering = gfx_get_plane(gfx, GFX_PLANE_RENDERING);
    hidden = 1 - display;

    /* Draw the dialogue on the hidden page first. */
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, hidden);
    textbox_show(gfx);

    /* Switch to the completed page at vertical retrace. */
    gfx_wait_vsync(gfx);
    gfx_set_plane(gfx, GFX_PLANE_DISPLAY, hidden);

    /* Redraw the old page while it is hidden. */
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, display);
    textbox_show(gfx);

    /* Restore the normal rendering page. */
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, rendering);
}



void textbox_set_speaker(const char *text)
{
    if (!text) text = "";

    strncpy(speaker, text, SPEAKER_MAX);
    speaker[SPEAKER_MAX] = '\0';
}

void textbox_set_text(const char *text)
{
    if (!text) text = "";

    strncpy(dialogue, text, TEXT_MAX);
    dialogue[TEXT_MAX] = '\0';
}

void textbox_draw(gfx_state_t *gfx)
{
    gfx_fill_range(gfx, BOX_X, BOX_Y, BOX_X + BOX_W - 1,
                   BOX_Y + BOX_H - 1, COLOR_BG);

    gfx_fill_range(gfx, NAME_X, NAME_Y, NAME_X + NAME_W - 1,
                   NAME_Y + NAME_H - 1, COLOR_BG);
}


void textbox_show(gfx_state_t *gfx)
{
    const char *p, *word;
    unsigned int x, y, line, word_len, line_chars, max_chars;

    textbox_draw(gfx);

    font_draw_string(gfx, NAME_X + 8, NAME_Y + 8, speaker, COLOR_TEXT);

    max_chars = TEXT_WIDTH / 8;
    p = dialogue;
    x = TEXT_X;
    y = TEXT_Y;
    line = 1;
    line_chars = 0;

    while (*p && line <= MAX_LINES)
    {
        // Preserve written newlines
        if (*p == '#')
        {
            p++;
            line++;
            x = TEXT_X;
            y += LINE_HEIGHT;
            line_chars = 0;
            continue;
        }

        // Skip leading spaces and control chars
        if (*p == ' ' || *p == '\\' || *p == '$')
        {
            p++;
            continue;
        }

        // Find next word
        word = p;
        word_len = 0;

        while (word[word_len] && word[word_len] != ' ' &&
                word[word_len] != '\n')
        {
            word_len++;
        }

        // Wrap before word
        if (line_chars > 0 &&
            line_chars + 1 + word_len > max_chars)
        {
            line++;
            x = TEXT_X;
            y += LINE_HEIGHT;
            line_chars = 0;

            if (line > MAX_LINES) break;
        }

        // Draw word
        while (word_len > 0)
        {
            font_draw_char(gfx, x, y, *p, COLOR_TEXT);
            x += 8;
            p++;
            word_len--;
            line_chars++;
        }

        // Draw spaces
        if (*p == ' ')
        {
            p++;
            x += 8;
            line_chars++;
        }
    }
}


int menu_show(gfx_state_t *gfx, char *options[], int count)
{
    int selected = 0;
    int key;
    int i;
    unsigned y;

    if (count <= 0)
        return -1;

    /* Keep the menu inside the screen. */
    if (MENU_Y + count * MENU_SPACING > gfx->height)
        return -1;

    textbox_show(gfx);

    while (1)
    {
        /* Draw on the current rendering page. */
        gfx_draw_rectangle(gfx, MENU_X - 20, MENU_Y - 8,
                       MENU_X + MENU_WIDTH,
                       MENU_Y + count * MENU_SPACING, COLOR_BG);

        for (i = 0; i < count; i++)
        {
            y = MENU_Y + i * MENU_SPACING;

            if (i == selected)
            {
                font_draw_string(gfx, MENU_X - 16, y, ">",
                                 SELECT_COLOR);

                font_draw_string(gfx, MENU_X, y, options[i],
                                 SELECT_COLOR);
            }
            else
            {
                font_draw_string(gfx, MENU_X, y, options[i],
                                 MENU_COLOR);
            }
        }

        // Wait for vertical retrace, then display
        gfx_wait_vsync(gfx);
        gfx_swap_planes(gfx);

        key = getch();

        if (key == 0 || key == 224)
        {
            key = getch();

            if (key == 72)
            {
                selected--;

                if (selected < 0) selected = count - 1;
            }

            else if (key == 80) selected++;

        }
        else if (key == 0x0B) selected--;
        else if (key == 0x0A) selected++;

        if (selected < 0) selected = count - 1;
        else if (selected >= count) selected = 0;
        else if (key == 13)
        {
            int result = selected;

            textbox_redraw_both_pages(gfx);

            return result;
        }
    }
}
