#include "bg.h"

static int load_palette(gfx_state_t *gfx, const char *path)
{
    FILE *fp;
    unsigned palette[BG_COLORS * 3];
    unsigned char raw[BG_COLORS * 3];
    unsigned i;

    fp = fopen(path, "rb");
    if (!fp)
        return 0;

    if (fread(raw, 1, sizeof(raw), fp) != sizeof(raw)) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    for (i = 0; i < BG_COLORS * 3; ++i)
        palette[i] = raw[i];

    gfx_fill_palette(gfx, palette, BG_COLORS);
    return 1;
}

static int load_background(gfx_state_t *gfx, const char *path)
{
    FILE *fp;
    int ok;

    fp = fopen(path, "rb");
    if (!fp)
        return 0;

    ok = gfx_load_planar_640x400(gfx, fp);

    fclose(fp);
    return ok;
}

int display_bg(gfx_state_t *gfx, char *filename)
{
    char pal_path[20], bg_path[20];
    sprintf(pal_path, "ASSETS/%s.PAL", filename);
    sprintf(bg_path, "ASSETS/%s.P4", filename);

    if (!load_palette(gfx, pal_path))
    {
        printf("Could not load palette %s\n", pal_path);
        gfx_fini(gfx);
        return 1;
    }

    if (!load_background(gfx, bg_path))
    {
        printf("Could not load background\n");
        gfx_fini(gfx);
        return 1;
    }

    return 0;
}

int prepare_background_pages(gfx_state_t *gfx, const char *name)
{
    /* Load the background into page 0. */
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, 0);

    if (display_bg(gfx, (char *)name)) return 0;

    /* Load the same background into page 1. */
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, 1);

    if (display_bg(gfx, (char *)name)) return 0;

    /* Start by displaying page 0 and drawing on page 1. */
    gfx_set_plane(gfx, GFX_PLANE_DISPLAY, 0);
    gfx_set_plane(gfx, GFX_PLANE_RENDERING, 1);

    return 1;
}
