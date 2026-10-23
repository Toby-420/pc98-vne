#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <i86.h>

#include "music.h"

#define PMD_INTERRUPT       0x60
#define PMD_GET_MUSIC_BUF   0x06
#define PMD_GET_SIZES       0x22
#define PMD_PLAY            0x00
#define PMD_STOP            0x01

static unsigned char __far *music_buffer;
static unsigned long music_buffer_size;
static int music_ready;

static int pmd_call(unsigned char function, union REGS *out)
{
    union REGS in;

    memset(&in, 0, sizeof(in));
    in.h.ah = function;

    int86(PMD_INTERRUPT, &in, out);

    return 1;
}

int music_init(void)
{
    union REGS in;
    union REGS out;
    struct SREGS segs;

    music_ready = 0;
    music_buffer = NULL;
    music_buffer_size = 0;

    /* Check that PMD is installed and has a sound driver. */
    memset(&in, 0, sizeof(in));
    in.h.ah = 0x09;
    int86(PMD_INTERRUPT, &in, &out);

    if (out.h.al == 0xFF)
    {
        printf("Music failed to init.");
        return 0;
    }

    /* Get the music buffer address (DS:DX). */
    memset(&in, 0, sizeof(in));
    in.h.ah = PMD_GET_MUSIC_BUF;

    segread(&segs);
    int86x(PMD_INTERRUPT, &in, &out, &segs);

    music_buffer = (unsigned char __far *)MK_FP(segs.ds, out.x.dx);

    /* Get the music buffer size in KB. */
    memset(&in, 0, sizeof(in));
    in.h.ah = PMD_GET_SIZES;
    int86(PMD_INTERRUPT, &in, &out);

    music_buffer_size = (unsigned long)out.h.al * 1024UL;

    if (music_buffer == NULL || music_buffer_size == 0)
    {
        printf("Music failed to init.");
        return 0;
    }

    music_ready = 1;
    return 1;
}

int music_play(const char *filename)
{
    FILE *file;
    unsigned long size;
    unsigned long read_size;
    char music_path[20];
    unsigned char temp[1024];
    unsigned long total_read = 0;
    unsigned int chunk;

    union REGS out;

    if (!music_ready || filename == NULL)
        return 0;

    sprintf(music_path, "ASSETS/%s.M", filename);

    file = fopen(music_path, "rb");
    if (file == NULL)
        return 0;

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return 0;
    }

    size = (unsigned long)ftell(file);

    if (size == 0 || size > music_buffer_size)
    {
        fclose(file);
        return 0;
    }

    rewind(file);

    {

        while (total_read < size)
        {
            unsigned long remaining = size - total_read;

            chunk = (remaining > sizeof(temp))
            ? sizeof(temp)
            : (unsigned int)remaining;

            read_size = (unsigned long)fread(temp, 1, chunk, file);

            if (read_size != chunk)
            {
                fclose(file);
                return 0;
            }

            _fmemcpy(&music_buffer[total_read], temp, chunk);

            total_read += read_size;
        }
    }

    fclose(file);

    /* Keep your existing PMD playback call here. */
    pmd_call(PMD_PLAY, &out);

    return 1;
}

void music_stop(void)
{
    union REGS out;

    if (!music_ready) return;

    pmd_call(PMD_STOP, &out);
}

void music_shutdown(void)
{
    music_stop();

    music_ready = 0;
    music_buffer = NULL;
    music_buffer_size = 0;
}
