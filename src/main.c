#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "GFXI.H"
#include "font.h"
#include "textbox.h"
#include "bg.h"
#include "story.h"
#include "music.h"

#define MASTER_STORY "ASSETS/MASTER.EST"

int seek_line(FILE *file, long line_number)
{
    long current_line;
    int ch;

    if (line_number < 1) return 0;

    rewind(file);

    for (current_line = 1; current_line < line_number; current_line++)
    {
        while ((ch = fgetc(file)) != '\n')
        {
            if (ch == EOF) return 0;
        }
    }

    return 1;
}

static int jump_to_label(FILE *storyFile, const char *target)
{
    char buf[256];
    long original_pos, label_pos;

    original_pos = ftell(storyFile);
    rewind(storyFile);

    while ((label_pos = ftell(storyFile)) >= 0 &&
        fgets(buf, sizeof(buf), storyFile) != NULL)
    {
        buf[strcspn(buf, "\r\n")] = '\0';

        if (buf[0] == '@' && strcmp(buf + 1, target) == 0)
        {
            // File is now positioned after the label line
            return 1;
        }
    }

    // Label not found: restore the previous position
    if (original_pos >= 0) fseek(storyFile, original_pos, SEEK_SET);

    return 0;
}

int main(void)
{
    gfx_state_t gfx;

    FILE *storyFile;
    char line[256]; // Maximum dialogue length is ~200 char anyway
    char *jump_label;
    int auto_advance, typewrite_delay = 0;
    char current_bg[10], current_story[10], prev_story[10];
    long prev_pos, jump_pos;

    music_init();


    if (!gfx_init(&gfx, GFX_MODE_640x400x16))
    {
        printf("GFX init failed: %s\n", gfx_last_error(&gfx));
        goto shutdown_seq;
    }

    if (!font_load("ASSETS/FONT8.FNT"))
    {
        printf("Could not load font\n");
        goto shutdown_seq;
    }

    storyFile = fopen(MASTER_STORY, "r");
    if (storyFile == NULL)
    {
        printf("Could not load story\n");
        goto shutdown_seq;
    }

    while (fgets(line, sizeof(line), storyFile) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';
        switch (line[0])
        {
            case '!':
                goto shutdown_seq;
                break;
            case '$':
                auto_advance = (line[1] == '$');

                textbox_set_text(line + 1);

                /* Ensure both pages contain the textbox. */
                textbox_redraw_both_pages(&gfx, typewrite_delay);

                /* Display the completed rendering page. */
                gfx_wait_vsync(&gfx);

                /* Only swap if rendering and display are different. */
                if (gfx_get_plane(&gfx, GFX_PLANE_RENDERING) !=
                    gfx_get_plane(&gfx, GFX_PLANE_DISPLAY))
                {
                    gfx_swap_planes(&gfx);
                }

                if (!auto_advance) getch();

            break;
            case '%':
                strncpy(current_bg, line + 1, sizeof(current_bg) - 1);
                current_bg[sizeof(current_bg) - 1] = '\0';

                prepare_background_pages(&gfx, current_bg);
                gfx_swap_planes(&gfx);
                break;
            // case '&':
            //     strcpy(save_name, line + 2);
            //     if (line[1] == 'S')
            //     {
            //         saveFile = fopen(save_name, "w");
            //
            //     } else if (line[1] == 'L')
            //     {
            //         saveFile = fopen(save_name, "w");
            //     }
            //     break;
            case '*':
                if (line[1] == '*')
                {
                    sprintf(current_story, MASTER_STORY);
                    fclose(storyFile);
                    storyFile = fopen(current_story, "r");

                    if (storyFile == NULL) break;

                    fseek(storyFile, prev_pos, SEEK_SET);
                } else if (line[1] == '@')
                {
                    prev_pos = ftell(storyFile);
                    sprintf(prev_story, current_story);


                    sprintf(current_story, "ASSETS/%s.EST",
                                        strtok(line + 2, ":"));

                    jump_pos = atol(strtok(NULL, ":"));

                    fclose(storyFile);
                    storyFile = fopen(current_story, "r");

                    if (storyFile == NULL) break;

                    if (!seek_line(storyFile, jump_pos))
                    {
                        printf("Invalid line number: %ld\n", jump_pos);
                        break;
                    }
                } else
                {
                    prev_pos = ftell(storyFile);
                    sprintf(prev_story, current_story);
                    sprintf(current_story, "ASSETS/%s.EST", line + 1);
                    fclose(storyFile);
                    storyFile = fopen(current_story, "r");

                    if (storyFile == NULL) break;
                }

                if (storyFile == NULL)
                {
                    printf("Could not load story %s\n", current_story);
                    goto shutdown_seq;
                }
                break;
            case '/':
                textbox_set_speaker(line + 1);
                break;
            case ':':
                jump_label = (line + 1);
                if (!jump_to_label(storyFile, jump_label))
                {
                    printf("Error: label '%s' not found.\n",
                           jump_label);
                }
                break;
            case ';':
                if (line + 1 != NULL) {
                    typewrite_delay = atoi(line + 1);
                } else
                {
                    typewrite_delay = 0;
                }
                break;
            case '?':
                jump_label = handle_choice(&gfx, line + 1);

                textbox_show(&gfx, typewrite_delay);
                gfx_wait_vsync(&gfx);

                prepare_background_pages(&gfx, current_bg);
                if (jump_label != NULL)
                {
                    if (!jump_to_label(storyFile, jump_label))
                    {
                        printf("Error: label '%s' not found.\n",
                               jump_label);
                    }

                    free(jump_label);
                }
                break;

            case '@':
                // Ignore labels here, they are handled by '?'
                break;
            case '_':
                getch();
                break;
            case '`':
                // Ignore comments distinctly
                break;
            case '~':
                if (line[1] == '!') music_stop();
                else music_play(line + 1);
                break;
            default:
                break;
        }
    }

    shutdown_seq:
        prepare_background_pages(&gfx, "BLACK");
        music_shutdown();
        gfx_fini(&gfx);

    return 0;
}
