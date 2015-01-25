// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "i_system.h"

#include "doomtype.h"

#include "deh_str.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// [SVE]: kerning data for big font
#include "kerning.h"

// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK

// Blending table used for fuzzpatch, etc.
// Only used in Heretic/Hexen

byte *tinttable = NULL;

// villsa [STRIFE] Blending table used for Strife
byte *xlatab = NULL;

// The screen buffer that the v_video.c code draws to.

static byte *dest_screen = NULL;

int dirtybox[4]; 

// haleyjd 08/28/10: clipping callback function for patches.
// This is needed for Chocolate Strife, which clips patches to the screen.
static vpatchclipfunc_t patchclip_callback = NULL;

//
// V_MarkRect 
// 
void V_MarkRect(int x, int y, int width, int height) 
{ 
    // If we are temporarily using an alternate screen, do not 
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y); 
        M_AddToBox (dirtybox, x + width-1, y + height-1); 
    }
} 
 

//
// V_CopyRect 
// 
void V_CopyRect(int srcx, int srcy, byte *source,
                int width, int height,
                int destx, int desty)
{ 
    byte *src;
    byte *dest; 
 
    srcx <<= hires;
    srcy <<= hires;
    width <<= hires;
    height <<= hires;
    destx <<= hires;
    desty <<= hires;

#ifdef RANGECHECK 
    if (srcx < 0
     || srcx + width > SCREENWIDTH
     || srcy < 0
     || srcy + height > SCREENHEIGHT 
     || destx < 0
     || destx + width > SCREENWIDTH
     || desty < 0
     || desty + height > SCREENHEIGHT)
    {
        I_Error ("Bad V_CopyRect");
    }
#endif 

    V_MarkRect(destx, desty, width, height); 
 
    src = source + SCREENWIDTH * srcy + srcx; 
    dest = dest_screen + SCREENWIDTH * desty + destx; 

    for ( ; height>0 ; height--) 
    { 
        memcpy(dest, src, width); 
        src += SCREENWIDTH; 
        dest += SCREENWIDTH; 
    } 
} 
 
//
// V_SetPatchClipCallback
//
// haleyjd 08/28/10: Added for Strife support.
// By calling this function, you can setup runtime error checking for patch 
// clipping. Strife never caused errors by drawing patches partway off-screen.
// Some versions of vanilla DOOM also behaved differently than the default
// implementation, so this could possibly be extended to those as well for
// accurate emulation.
//
void V_SetPatchClipCallback(vpatchclipfunc_t func)
{
    patchclip_callback = func;
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
//

void V_DrawPatch(int x, int y, patch_t *patch)
{ 
    int count;
    int col;
    column_t *column;
    byte *desttop;
    byte *dest;
    byte *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK
    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatch");
    }
#endif

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*(SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest = *source;
                    dest += SCREENWIDTH;
                }
                *dest = *source++;
                dest += SCREENWIDTH;
            }
          }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

//
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//

void V_DrawPatchFlipped(int x, int y, patch_t *patch)
{
    int count;
    int col; 
    column_t *column; 
    byte *desttop;
    byte *dest;
    byte *source; 
    int w, f; 
 
    y -= SHORT(patch->topoffset); 
    x -= SHORT(patch->leftoffset); 

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK 
    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatchFlipped");
    }
#endif

    V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));

        // step through the posts in a column
        while (column->topdelta != 0xff )
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*(SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest = *source;
                    dest += SCREENWIDTH;
                }
                *dest = *source++;
                dest += SCREENWIDTH;
            }
          }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}



//
// V_DrawPatchDirect
// Draws directly to the screen on the pc. 
//

void V_DrawPatchDirect(int x, int y, patch_t *patch)
{
    V_DrawPatch(x, y, patch); 
} 

//
// V_DrawTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH 
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawTLPatch");
    }

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest = tinttable[((*dest) << 8) + *source];
                    dest += SCREENWIDTH;
                }
                *dest = tinttable[((*dest) << 8) + *source++];
                dest += SCREENWIDTH;
            }
          }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawXlaPatch
//
// villsa [STRIFE] Masks a column based translucent masked pic to the screen.
//

void V_DrawXlaPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for(; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while(column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while(count--)
            {
                if (hires)
                {
                    *dest = xlatab[*dest + ((*source) << 8)];
                    dest += SCREENWIDTH;
                }
                *dest = xlatab[*dest + ((*source) << 8)];
                source++;
                dest += SCREENWIDTH;
            }
          }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawAltTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawAltTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawAltTLPatch");
    }

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest = tinttable[((*dest) << 8) + *source];
                    dest += SCREENWIDTH;
                }
                *dest = tinttable[((*dest) << 8) + *source++];
                dest += SCREENWIDTH;
            }
          }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawShadowedPatch
//
// Masks a column based masked pic to the screen.
//

void V_DrawShadowedPatch(int x, int y, patch_t *patch)
{
    int count, col;
    column_t *column;
    byte *desttop, *dest, *source;
    byte *desttop2, *dest2;
    int w, f;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawShadowedPatch");
    }

    col = 0;
    desttop = dest_screen + (y << hires) * SCREENWIDTH + x;
    desttop2 = dest_screen + ((y + 2) << hires) * SCREENWIDTH + x + 2;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++, desttop2++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
          for (f = 0; f <= hires; f++)
          {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            dest2 = desttop2 + column->topdelta * (SCREENWIDTH << hires) + (x * hires) + f;
            count = column->length;

            while (count--)
            {
                if (hires)
                {
                    *dest2 = tinttable[((*dest2) << 8)];
                    dest2 += SCREENWIDTH;
                    *dest = *source;
                    dest += SCREENWIDTH;
                }
                *dest2 = tinttable[((*dest2) << 8)];
                dest2 += SCREENWIDTH;
                *dest = *source++;
                dest += SCREENWIDTH;

            }
          }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// Load tint table from TINTTAB lump.
//

void V_LoadTintTable(void)
{
    tinttable = W_CacheLumpName("TINTTAB", PU_STATIC);
}

//
// V_LoadXlaTable
//
// villsa [STRIFE] Load xla table from XLATAB lump.
//

void V_LoadXlaTable(void)
{
    if(!xlatab) // [SVE]: once only
        xlatab = W_CacheLumpName("XLATAB", PU_STATIC);
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//

void V_DrawBlock(int x, int y, int width, int height, byte *src) 
{ 
    byte *dest; 
 
#ifdef RANGECHECK 
    if (x < 0
     || x + width >SCREENWIDTH
     || y < 0
     || y + height > SCREENHEIGHT)
    {
	I_Error ("Bad V_DrawBlock");
    }
#endif 
 
    V_MarkRect (x, y, width, height); 
 
    dest = dest_screen + (y << hires) * SCREENWIDTH + x;

    while (height--) 
    { 
	memcpy (dest, src, width); 
	src += width; 
	dest += SCREENWIDTH; 
    } 
} 

void V_DrawScaledBlock(int x, int y, int width, int height, byte *src)
{
    byte *dest;
    int i, j;

#ifdef RANGECHECK
    if (x < 0
     || x + width > ORIGWIDTH
     || y < 0
     || y + height > ORIGHEIGHT)
    {
	I_Error ("Bad V_DrawScaledBlock");
    }
#endif

    V_MarkRect (x, y, width, height);

    dest = dest_screen + (y << hires) * SCREENWIDTH + (x << hires);

    for (i = 0; i < (height << hires); i++)
    {
        for (j = 0; j < (width << hires); j++)
        {
            *(dest + i * SCREENWIDTH + j) = *(src + (i >> hires) * width + (j >> hires));
        }
    }
}

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    uint8_t *buf, *buf1;
    int x1, y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        buf1 = buf;

        for (x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = c;
        }

        buf += SCREENWIDTH;
    }
}

void V_DrawHorizLine(int x, int y, int w, int c)
{
    uint8_t *buf;
    int x1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (x1 = 0; x1 < w; ++x1)
    {
        *buf++ = c;
    }
}

void V_DrawVertLine(int x, int y, int h, int c)
{
    uint8_t *buf;
    int y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        *buf = c;
        buf += SCREENWIDTH;
    }
}

void V_DrawBox(int x, int y, int w, int h, int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y+h-1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x+w-1, y, h, c);
}

//
// Draw a "raw" screen (lump containing raw data to blit directly
// to the screen)
//

void V_CopyScaledBuffer(byte *dest, byte *src, size_t size)
{
    int i, j;

#ifdef RANGECHECK
    if (size < 0
     || size > ORIGWIDTH * ORIGHEIGHT)
    {
        I_Error("Bad V_CopyScaledBuffer");
    }
#endif

    while (size--)
    {
        for (i = 0; i <= hires; i++)
        {
            for (j = 0; j <= hires; j++)
            {
                *(dest + (size << hires) + (hires * (int) (size / ORIGWIDTH) + i) * SCREENWIDTH + j) = *(src + size);
            }
        }
    }
}
 
void V_DrawRawScreen(byte *raw)
{
    V_CopyScaledBuffer(dest_screen, raw, ORIGWIDTH * ORIGHEIGHT);
}
//
// V_Init
// 
void V_Init (void) 
{ 
    // no-op!
    // There used to be separate screens that could be drawn to; these are
    // now handled in the upper layers.
}

// Set the buffer that the code draws to.

void V_UseBuffer(byte *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer(void)
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//

typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} PACKEDATTR pcx_t;


//
// WritePCXfile
//

void WritePCXfile(char *filename, byte *data,
                  int width, int height,
                  byte *palette)
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL, "WritePCXfile");

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx, "WritePCXfile");
}

//
// V_ScreenShot
//

void V_ScreenShot(char *format)
{
    int i;
    char lbmname[16]; // haleyjd 20110213: BUG FIX - 12 is too small!
    
    // find a file name to save it to

    for (i=0; i<=99; i++)
    {
        sprintf(lbmname, format, i);

        if (!M_FileExists(lbmname))
        {
            break;      // file doesn't exist
        }
    }

    if (i == 100)
    {
        // [SVE]: stability
        //I_Error ("V_ScreenShot: Couldn't create a PCX");
        return;
    }

    // save the pcx file
    WritePCXfile(lbmname, I_VideoBuffer,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
}

#define MOUSE_SPEED_BOX_WIDTH  120
#define MOUSE_SPEED_BOX_HEIGHT 9

void V_DrawMouseSpeedBox(int speed)
{
    extern int usemouse;
    int bgcolor, bordercolor, red, black, white, yellow;
    int box_x, box_y;
    int original_speed;
    int redline_x;
    int linelen;

    // Get palette indices for colors for widget. These depend on the
    // palette of the game being played.

    bgcolor = I_GetPaletteIndex(0x77, 0x77, 0x77);
    bordercolor = I_GetPaletteIndex(0x55, 0x55, 0x55);
    red = I_GetPaletteIndex(0xff, 0x00, 0x00);
    black = I_GetPaletteIndex(0x00, 0x00, 0x00);
    yellow = I_GetPaletteIndex(0xff, 0xff, 0x00);
    white = I_GetPaletteIndex(0xff, 0xff, 0xff);

    // If the mouse is turned off or acceleration is turned off, don't
    // draw the box at all.

    if (!usemouse || fabs(mouse_acceleration - 1) < 0.01)
    {
        return;
    }

    // Calculate box position

    box_x = SCREENWIDTH - MOUSE_SPEED_BOX_WIDTH - 10;
    box_y = 15;

    V_DrawFilledBox(box_x, box_y,
                    MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bgcolor);
    V_DrawBox(box_x, box_y,
              MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bordercolor);

    // Calculate the position of the red line.  This is 1/3 of the way
    // along the box.

    redline_x = MOUSE_SPEED_BOX_WIDTH / 3;

    // Undo acceleration and get back the original mouse speed

    if (speed < mouse_threshold)
    {
        original_speed = speed;
    }
    else
    {
        original_speed = speed - mouse_threshold;
        original_speed = (int) (original_speed / mouse_acceleration);
        original_speed += mouse_threshold;
    }

    // Calculate line length

    linelen = (original_speed * redline_x) / mouse_threshold;

    // Draw horizontal "thermometer" 

    if (linelen > MOUSE_SPEED_BOX_WIDTH - 1)
    {
        linelen = MOUSE_SPEED_BOX_WIDTH - 1;
    }

    V_DrawHorizLine(box_x + 1, box_y + 4, MOUSE_SPEED_BOX_WIDTH - 2, black);

    if (linelen < redline_x)
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                      linelen, white);
    }
    else
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        redline_x, white);
        V_DrawHorizLine(box_x + redline_x, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen - redline_x, yellow);
    }

    // Draw red line

    V_DrawVertLine(box_x + redline_x, box_y + 1,
                 MOUSE_SPEED_BOX_HEIGHT - 2, red);
}

//=============================================================================
//
// haleyjd 20140928: [SVE] Big font support
//

static int bfkerntbl[128*128];

patch_t *bigfont[BIG_FONTSIZE];

//
// V_LoadBigFont
//

void V_LoadBigFont(void)
{
    size_t i;

    // load patches
    for(i = 0; i < BIG_FONTSIZE; i++)
    {
        int lumpnum;
        int charnum = BIG_FONTSTART + i;
        char lumpname[9];

        M_snprintf(lumpname, sizeof(lumpname), "BFONT%02d", charnum);
        if((lumpnum = W_CheckNumForName(lumpname)) >= 0)
            bigfont[i] = W_CacheLumpNum(lumpnum, PU_STATIC);
    }

    // init kerning table
    for(i = 0; i < arrlen(bfkerntbl); i++)
        bfkerntbl[i] = -1; // default step is -1

    for(i = 0; i < numkernings; i++)
    {
        kerndata_t *kd = &kernings[i];
        bfkerntbl[kd->first + (((unsigned int)kd->second) << 7)] = kd->offset;
    }
}

//
// V_WriteBigText
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchar-subscripts"

void V_WriteBigText(const char *str, int x, int y)
{
    const char *rover;
    char c;            // current char
    char lastc = '\0'; // last char seen that wasn't a space
    patch_t *patch;
    int initx = x;

    for(rover = str; *rover; rover++)
    {
        c = toupper(*rover);

        if(c == ' ')
        {
            x += 8;
            continue;
        }
        if(c == '\n')
        {
            y += 20;
            x = initx;
            lastc = '\0';
            continue;
        }

        if(c < BIG_FONTSTART || c > BIG_FONTEND || !(patch = bigfont[c - BIG_FONTSTART]))
            continue;

        if(lastc) // kerning - move toward the left depending on the previous char
            x += bfkerntbl[lastc + (((unsigned int)c) << 7)];

        V_DrawPatch(x, y, patch);

        x += SHORT(patch->width);
        lastc = c;
    }
}

//
// V_BigFontStringWidth
//

int V_BigFontStringWidth(const char *str)
{
    const char *rover;
    char c;            // current char
    char lastc = '\0'; // last char seen that wasn't a space
    patch_t *patch;
    int widestwidth = 0;
    int width = 0;

    for(rover = str; *rover; rover++)
    {
        c = toupper(*rover);

        if(c == ' ')
        {
            width += 8;
            continue;
        }
        if(c == '\n')
        {
            if(width > widestwidth)
                widestwidth = width;
            width = 0;
            lastc = '\0';
            continue;
        }

        if(c < BIG_FONTSTART || c > BIG_FONTEND || !(patch = bigfont[c - BIG_FONTSTART]))
            continue;

        if(lastc)
            width += bfkerntbl[lastc + (((unsigned int)c) << 7)];
        width += SHORT(patch->width);

        lastc = c;
    }

    if(width > widestwidth)
        widestwidth = width;

    return widestwidth;
}

//
// V_BigFontStringHeight
//

int V_BigFontStringHeight(const char *str)
{
    const char *rover;
    int height = 20; // always at least 20px

    for(rover = str; *rover; rover++)
    {
        if(*rover == '\n')
            height += 20; // +20 for every linebreak
    }

    return height;
}