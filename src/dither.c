/*
 * Copyright (c) 2014 Hayaki Saito
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif

#include "dither.h"
#include "quant.h"
#include "sixel.h"


static const unsigned char pal_mono_dark[] = {
    0x00, 0x00, 0x00, 0xff, 0xff, 0xff
};

static const unsigned char pal_mono_light[] = {
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00
};

static const unsigned char pal_xterm256[] = {
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x80, 0x00,
    0x00, 0x00, 0x80, 0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xc0,
    0x80, 0x80, 0x80, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x87, 0x00, 0x00, 0xaf,
    0x00, 0x00, 0xd7, 0x00, 0x00, 0xff, 0x00, 0x5f, 0x00, 0x00, 0x5f, 0x5f,
    0x00, 0x5f, 0x87, 0x00, 0x5f, 0xaf, 0x00, 0x5f, 0xd7, 0x00, 0x5f, 0xff,
    0x00, 0x87, 0x00, 0x00, 0x87, 0x5f, 0x00, 0x87, 0x87, 0x00, 0x87, 0xaf,
    0x00, 0x87, 0xd7, 0x00, 0x87, 0xff, 0x00, 0xaf, 0x00, 0x00, 0xaf, 0x5f,
    0x00, 0xaf, 0x87, 0x00, 0xaf, 0xaf, 0x00, 0xaf, 0xd7, 0x00, 0xaf, 0xff,
    0x00, 0xd7, 0x00, 0x00, 0xd7, 0x5f, 0x00, 0xd7, 0x87, 0x00, 0xd7, 0xaf,
    0x00, 0xd7, 0xd7, 0x00, 0xd7, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x5f,
    0x00, 0xff, 0x87, 0x00, 0xff, 0xaf, 0x00, 0xff, 0xd7, 0x00, 0xff, 0xff,
    0x5f, 0x00, 0x00, 0x5f, 0x00, 0x5f, 0x5f, 0x00, 0x87, 0x5f, 0x00, 0xaf,
    0x5f, 0x00, 0xd7, 0x5f, 0x00, 0xff, 0x5f, 0x5f, 0x00, 0x5f, 0x5f, 0x5f,
    0x5f, 0x5f, 0x87, 0x5f, 0x5f, 0xaf, 0x5f, 0x5f, 0xd7, 0x5f, 0x5f, 0xff,
    0x5f, 0x87, 0x00, 0x5f, 0x87, 0x5f, 0x5f, 0x87, 0x87, 0x5f, 0x87, 0xaf,
    0x5f, 0x87, 0xd7, 0x5f, 0x87, 0xff, 0x5f, 0xaf, 0x00, 0x5f, 0xaf, 0x5f,
    0x5f, 0xaf, 0x87, 0x5f, 0xaf, 0xaf, 0x5f, 0xaf, 0xd7, 0x5f, 0xaf, 0xff,
    0x5f, 0xd7, 0x00, 0x5f, 0xd7, 0x5f, 0x5f, 0xd7, 0x87, 0x5f, 0xd7, 0xaf,
    0x5f, 0xd7, 0xd7, 0x5f, 0xd7, 0xff, 0x5f, 0xff, 0x00, 0x5f, 0xff, 0x5f,
    0x5f, 0xff, 0x87, 0x5f, 0xff, 0xaf, 0x5f, 0xff, 0xd7, 0x5f, 0xff, 0xff,
    0x87, 0x00, 0x00, 0x87, 0x00, 0x5f, 0x87, 0x00, 0x87, 0x87, 0x00, 0xaf,
    0x87, 0x00, 0xd7, 0x87, 0x00, 0xff, 0x87, 0x5f, 0x00, 0x87, 0x5f, 0x5f,
    0x87, 0x5f, 0x87, 0x87, 0x5f, 0xaf, 0x87, 0x5f, 0xd7, 0x87, 0x5f, 0xff,
    0x87, 0x87, 0x00, 0x87, 0x87, 0x5f, 0x87, 0x87, 0x87, 0x87, 0x87, 0xaf,
    0x87, 0x87, 0xd7, 0x87, 0x87, 0xff, 0x87, 0xaf, 0x00, 0x87, 0xaf, 0x5f,
    0x87, 0xaf, 0x87, 0x87, 0xaf, 0xaf, 0x87, 0xaf, 0xd7, 0x87, 0xaf, 0xff,
    0x87, 0xd7, 0x00, 0x87, 0xd7, 0x5f, 0x87, 0xd7, 0x87, 0x87, 0xd7, 0xaf,
    0x87, 0xd7, 0xd7, 0x87, 0xd7, 0xff, 0x87, 0xff, 0x00, 0x87, 0xff, 0x5f,
    0x87, 0xff, 0x87, 0x87, 0xff, 0xaf, 0x87, 0xff, 0xd7, 0x87, 0xff, 0xff,
    0xaf, 0x00, 0x00, 0xaf, 0x00, 0x5f, 0xaf, 0x00, 0x87, 0xaf, 0x00, 0xaf,
    0xaf, 0x00, 0xd7, 0xaf, 0x00, 0xff, 0xaf, 0x5f, 0x00, 0xaf, 0x5f, 0x5f,
    0xaf, 0x5f, 0x87, 0xaf, 0x5f, 0xaf, 0xaf, 0x5f, 0xd7, 0xaf, 0x5f, 0xff,
    0xaf, 0x87, 0x00, 0xaf, 0x87, 0x5f, 0xaf, 0x87, 0x87, 0xaf, 0x87, 0xaf,
    0xaf, 0x87, 0xd7, 0xaf, 0x87, 0xff, 0xaf, 0xaf, 0x00, 0xaf, 0xaf, 0x5f,
    0xaf, 0xaf, 0x87, 0xaf, 0xaf, 0xaf, 0xaf, 0xaf, 0xd7, 0xaf, 0xaf, 0xff,
    0xaf, 0xd7, 0x00, 0xaf, 0xd7, 0x5f, 0xaf, 0xd7, 0x87, 0xaf, 0xd7, 0xaf,
    0xaf, 0xd7, 0xd7, 0xaf, 0xd7, 0xff, 0xaf, 0xff, 0x00, 0xaf, 0xff, 0x5f,
    0xaf, 0xff, 0x87, 0xaf, 0xff, 0xaf, 0xaf, 0xff, 0xd7, 0xaf, 0xff, 0xff,
    0xd7, 0x00, 0x00, 0xd7, 0x00, 0x5f, 0xd7, 0x00, 0x87, 0xd7, 0x00, 0xaf,
    0xd7, 0x00, 0xd7, 0xd7, 0x00, 0xff, 0xd7, 0x5f, 0x00, 0xd7, 0x5f, 0x5f,
    0xd7, 0x5f, 0x87, 0xd7, 0x5f, 0xaf, 0xd7, 0x5f, 0xd7, 0xd7, 0x5f, 0xff,
    0xd7, 0x87, 0x00, 0xd7, 0x87, 0x5f, 0xd7, 0x87, 0x87, 0xd7, 0x87, 0xaf,
    0xd7, 0x87, 0xd7, 0xd7, 0x87, 0xff, 0xd7, 0xaf, 0x00, 0xd7, 0xaf, 0x5f,
    0xd7, 0xaf, 0x87, 0xd7, 0xaf, 0xaf, 0xd7, 0xaf, 0xd7, 0xd7, 0xaf, 0xff,
    0xd7, 0xd7, 0x00, 0xd7, 0xd7, 0x5f, 0xd7, 0xd7, 0x87, 0xd7, 0xd7, 0xaf,
    0xd7, 0xd7, 0xd7, 0xd7, 0xd7, 0xff, 0xd7, 0xff, 0x00, 0xd7, 0xff, 0x5f,
    0xd7, 0xff, 0x87, 0xd7, 0xff, 0xaf, 0xd7, 0xff, 0xd7, 0xd7, 0xff, 0xff,
    0xff, 0x00, 0x00, 0xff, 0x00, 0x5f, 0xff, 0x00, 0x87, 0xff, 0x00, 0xaf,
    0xff, 0x00, 0xd7, 0xff, 0x00, 0xff, 0xff, 0x5f, 0x00, 0xff, 0x5f, 0x5f,
    0xff, 0x5f, 0x87, 0xff, 0x5f, 0xaf, 0xff, 0x5f, 0xd7, 0xff, 0x5f, 0xff,
    0xff, 0x87, 0x00, 0xff, 0x87, 0x5f, 0xff, 0x87, 0x87, 0xff, 0x87, 0xaf,
    0xff, 0x87, 0xd7, 0xff, 0x87, 0xff, 0xff, 0xaf, 0x00, 0xff, 0xaf, 0x5f,
    0xff, 0xaf, 0x87, 0xff, 0xaf, 0xaf, 0xff, 0xaf, 0xd7, 0xff, 0xaf, 0xff,
    0xff, 0xd7, 0x00, 0xff, 0xd7, 0x5f, 0xff, 0xd7, 0x87, 0xff, 0xd7, 0xaf,
    0xff, 0xd7, 0xd7, 0xff, 0xd7, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x5f,
    0xff, 0xff, 0x87, 0xff, 0xff, 0xaf, 0xff, 0xff, 0xd7, 0xff, 0xff, 0xff,
    0x08, 0x08, 0x08, 0x12, 0x12, 0x12, 0x1c, 0x1c, 0x1c, 0x26, 0x26, 0x26,
    0x30, 0x30, 0x30, 0x3a, 0x3a, 0x3a, 0x44, 0x44, 0x44, 0x4e, 0x4e, 0x4e,
    0x58, 0x58, 0x58, 0x62, 0x62, 0x62, 0x6c, 0x6c, 0x6c, 0x76, 0x76, 0x76,
    0x80, 0x80, 0x80, 0x8a, 0x8a, 0x8a, 0x94, 0x94, 0x94, 0x9e, 0x9e, 0x9e,
    0xa8, 0xa8, 0xa8, 0xb2, 0xb2, 0xb2, 0xbc, 0xbc, 0xbc, 0xc6, 0xc6, 0xc6,
    0xd0, 0xd0, 0xd0, 0xda, 0xda, 0xda, 0xe4, 0xe4, 0xe4, 0xee, 0xee, 0xee,
};


sixel_dither_t *
sixel_dither_create(int ncolors)
{
    sixel_dither_t *dither;
    int headsize;
    int datasize;
    int wholesize;
    int quality_mode;

    if (ncolors == -1) {
        ncolors = 256;
        quality_mode = QUALITY_FULL;
    }
    else {
        if (ncolors > SIXEL_PALETTE_MAX) {
            ncolors = 256;
        } else if (ncolors < 2) {
            ncolors = 2;
        }
        quality_mode = QUALITY_LOW;
    }
    headsize = sizeof(sixel_dither_t);
    datasize = ncolors * 3;
    wholesize = headsize + datasize;// + cachesize;

    dither = malloc(wholesize);
    if (dither == NULL) {
        return NULL;
    }
    dither->ref = 1;
    dither->palette = (unsigned char*)(dither + 1);
    dither->cachetable = NULL;
    dither->reqcolors = ncolors;
    dither->ncolors = ncolors;
    dither->origcolors = (-1);
    dither->keycolor = (-1);
    dither->optimized = 0;
    dither->complexion = 1;
    dither->bodyonly = 0;
    dither->method_for_largest = LARGE_NORM;
    dither->method_for_rep = REP_CENTER_BOX;
    dither->method_for_diffuse = DIFFUSE_FS;
    dither->quality_mode = quality_mode;

    return dither;
}


void
sixel_dither_destroy(sixel_dither_t *dither)
{
    if (dither) {
        free(dither->cachetable);
        free(dither);
    }
}


void
sixel_dither_ref(sixel_dither_t *dither)
{
    /* TODO: be thread safe */
    ++dither->ref;
}


void
sixel_dither_unref(sixel_dither_t *dither)
{
    /* TODO: be thread safe */
    if (dither != NULL && --dither->ref == 0) {
        sixel_dither_destroy(dither);
    }
}


sixel_dither_t *
sixel_dither_get(int builtin_dither)
{
    unsigned char *palette;
    int ncolors;
    int keycolor;
    sixel_dither_t *dither;

    switch (builtin_dither) {
    case BUILTIN_MONO_DARK:
        ncolors = 2;
        palette = (unsigned char *)pal_mono_dark;
        keycolor = 0;
        break;
    case BUILTIN_MONO_LIGHT:
        ncolors = 2;
        palette = (unsigned char *)pal_mono_light;
        keycolor = 0;
        break;
    case BUILTIN_XTERM16:
        ncolors = 16;
        palette = (unsigned char *)pal_xterm256;
        keycolor = (-1);
        break;
    case BUILTIN_XTERM256:
        ncolors = 256;
        palette = (unsigned char *)pal_xterm256;
        keycolor = (-1);
        break;
    default:
        return NULL;
    }

    dither = sixel_dither_create(ncolors);
    if (dither) {
        dither->palette = palette;
        dither->keycolor = keycolor;
        dither->optimized = 1;
    }

    return dither;
}


static void
sixel_dither_set_method_for_largest(sixel_dither_t *dither, int method_for_largest)
{
    if (method_for_largest == LARGE_AUTO) {
        method_for_largest = LARGE_NORM;
    }
    dither->method_for_largest = method_for_largest;
}


static void
sixel_dither_set_method_for_rep(sixel_dither_t *dither, int method_for_rep)
{
    if (method_for_rep == REP_AUTO) {
        method_for_rep = REP_CENTER_BOX;
    }
    dither->method_for_rep = method_for_rep;
}


static void
sixel_dither_set_quality_mode(sixel_dither_t *dither, int quality_mode)
{
    if (quality_mode == QUALITY_AUTO) {
        if (dither->ncolors <= 8) {
            quality_mode = QUALITY_HIGH;
        } else {
            quality_mode = QUALITY_LOW;
        }
    }
    dither->quality_mode = quality_mode;
}


int
sixel_dither_initialize(sixel_dither_t *dither, unsigned char *data,
                        int width, int height, int depth,
                        int method_for_largest, int method_for_rep,
                        int quality_mode)
{
    unsigned char *buf;

    sixel_dither_set_method_for_largest(dither, method_for_largest);
    sixel_dither_set_method_for_rep(dither, method_for_rep);
    sixel_dither_set_quality_mode(dither, quality_mode);

    buf = LSQ_MakePalette(data, width, height, depth,
                          dither->reqcolors, &dither->ncolors,
                          &dither->origcolors,
                          dither->method_for_largest,
                          dither->method_for_rep,
                          dither->quality_mode);
    if (buf == NULL) {
        return (-1);
    }
    memcpy(dither->palette, buf, dither->ncolors * depth);
    LSQ_FreePalette(buf);

    dither->optimized = 1;
    if (dither->origcolors <= dither->ncolors) {
        dither->method_for_diffuse = DIFFUSE_NONE;
    }

    return 0;
}


void
sixel_dither_set_diffusion_type(sixel_dither_t *dither, int method_for_diffuse)
{
    if (method_for_diffuse == DIFFUSE_AUTO) {
        if (dither->ncolors > 16) {
            method_for_diffuse = DIFFUSE_FS;
        } else {
            method_for_diffuse = DIFFUSE_ATKINSON;
        }
    }
    dither->method_for_diffuse = method_for_diffuse;
}


int
sixel_dither_get_num_of_palette_colors(sixel_dither_t *dither)
{
    return dither->ncolors;
}


int
sixel_dither_get_num_of_histgram_colors(sixel_dither_t *dither)
{
    return dither->origcolors;
}


unsigned char *
sixel_dither_get_palette(sixel_dither_t /* in */ *dither)  /* dither context object */
{
    return dither->palette;
}


void
sixel_dither_set_complexion_score(sixel_dither_t /* in */ *dither,  /* dither context object */
                                  int            /* in */ score)    /* complexion score (>= 1) */
{
    dither->complexion = score;
}

void
sixel_dither_set_body_only(sixel_dither_t /* in */ *dither,     /* dither context object */
                           int            /* in */ bodyonly)    /* 0: output palette section
                                                                   1: do not output palette section  */
{
    dither->bodyonly = bodyonly;
}

unsigned char *
sixel_apply_palette(unsigned char *pixels, int width, int height, sixel_dither_t *dither)
{
    int ret;
    int bufsize;
    int cachesize;
    unsigned char *dest;

    bufsize = width * height * sizeof(unsigned char);
    dest = malloc(bufsize);
    if (dest == NULL) {
        return NULL;
    }

    if (dither->cachetable == NULL && dither->optimized) {
        if (dither->palette != pal_mono_dark && dither->palette != pal_mono_light) {
            cachesize = (1 << 3 * 5) * sizeof(unsigned short);
#if HAVE_CALLOC
            dither->cachetable = calloc(cachesize, 1);
#else
            dither->cachetable = malloc(cachesize);
            memset(dither->cachetable, 0, cachesize);
#endif
        }
    }

    ret = LSQ_ApplyPalette(pixels, width, height, 3,
                           dither->palette,
                           dither->ncolors,
                           dither->method_for_diffuse,
                           dither->optimized,
                           dither->complexion,
                           dither->cachetable,
                           dest);
    if (ret != 0) {
        return NULL;
    }

    return dest;
}

/* emacs, -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/* vim: set expandtab ts=4 : */
/* EOF */
