//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	Preparation of data for rendering,
//	generation of lookups, caching, retrieval by name.
//

//#include <pspdebug.h>

#include <stdio.h>

#include "d_main.h"
#include "deh_str.h"
#include "i_swap.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "doomstat.h"
#include "m_misc.h"
#include "r_sky.h"
#include "r_data.h"
#include "sounds.h" // villsa [STRIFE]

#include "doomfeatures.h"

#include "c_io.h"

//#define printf pspDebugScreenPrintf

//
// Graphics.
// DOOM graphics for walls and sprites
// is stored in vertical runs of opaque pixels (posts).
// A column is composed of zero or more posts,
// a patch or sprite is composed of zero or more columns.
// 



//
// Texture definition.
// Each texture is composed of one or more patches,
// with patches being lumps stored in the WAD.
// The lumps are referenced by number, and patched
// into the rectangular texture space using origin
// and possibly other attributes.
//
typedef struct
{
    short	originx;
    short	originy;
    short	patch;
    //short	stepdir;    // villsa [STRIFE] removed
    //short	colormap;   // villsa [STRIFE] removed
} PACKEDATTR mappatch_t;


//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//
typedef struct
{
    char		name[8];
    int			masked;	
    short		width;
    short		height;
    //int               obsolete;   // villsa [STRIFE] removed
    short		patchcount;
    mappatch_t	patches[1];
} PACKEDATTR maptexture_t;


int		firstflat;
int		lastflat;
int		numflats;

int		firstpatch;
int		lastpatch;
int		numpatches;

int		firstspritelump;
int		lastspritelump;
int		numspritelumps;

int		numtextures;
texture_t**	textures;
texture_t**     textures_hashtable;


int*			texturewidthmask;
// needed for texture pegging
fixed_t*		textureheight;		
int*			texturecompositesize;
short**			texturecolumnlump;
unsigned short**	texturecolumnofs;
byte**			texturecomposite;

// for global animation
int*		flattranslation;
int*		texturetranslation;

// needed for pre rendering
fixed_t*	spritewidth;	
fixed_t*	spriteoffset;
fixed_t*	spritetopoffset;

lighttable_t	*colormaps;

// [SVE] svillarreal
fixed_t*    spriteheight;

#ifdef SHAREWARE
extern boolean STRIFE_1_0_SHAREWARE;
extern boolean STRIFE_1_1_SHAREWARE;
#endif

//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//



//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//
void
R_DrawColumnInCache
( column_t*	patch,
  byte*		cache,
  int		originy,
  int		cacheheight )
{
    int		count;
    int		position;
    byte*	source;

    while (patch->topdelta != 0xff)
    {
	source = (byte *)patch + 3;
	count = patch->length;
	position = originy + patch->topdelta;

	if (position < 0)
	{
	    count += position;
	    position = 0;
	}

	if (position + count > cacheheight)
	    count = cacheheight - position;

	if (count > 0)
	    memcpy (cache + position, source, count);
		
	patch = (column_t *)(  (byte *)patch + patch->length + 4); 
    }
}



//
// R_GenerateComposite
// Using the texture definition,
//  the composite texture is created from the patches,
//  and each column is cached.
//
void R_GenerateComposite (int texnum)
{
    byte*		block;
    texture_t*		texture;
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int			x;
    int			x1;
    int			x2;
    int			i;
    column_t*		patchcol;
    short*		collump;
    unsigned short*	colofs;
	
    texture = textures[texnum];

    block = Z_Malloc (texturecompositesize[texnum],
		      PU_STATIC, 
		      (void**)&texturecomposite[texnum], "R_GenerateComposite");	

    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    
    // Composite the columns together.
    patch = texture->patches;
		
    for (i=0 , patch = texture->patches;
	 i<texture->patchcount;
	 i++, patch++)
    {
	realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
	x1 = patch->originx;
	x2 = x1 + SHORT(realpatch->width);

	if (x1<0)
	    x = 0;
	else
	    x = x1;
	
	if (x2 > texture->width)
	    x2 = texture->width;

	for ( ; x<x2 ; x++)
	{
	    // Column does not have multiple patches?
	    if (collump[x] >= 0)
		continue;
	    
	    patchcol = (column_t *)((byte *)realpatch
				    + LONG(realpatch->columnofs[x-x1]));
	    R_DrawColumnInCache (patchcol,
				 block + colofs[x],
				 patch->originy,
				 texture->height);
	}
						
    }

    // Now that the texture has been built in column cache,
    //  it is purgable from zone memory.
    Z_ChangeTag (block, PU_CACHE);
}



//
// R_GenerateLookup
//
void R_GenerateLookup (int texnum)
{
    texture_t*		texture;
    byte*		patchcount;	// patchcount[texture->width]
    texpatch_t*		patch;	
    patch_t*		realpatch;
    int			x;
    int			x1;
    int			x2;
    int			i;
    short*		collump;
    unsigned short*	colofs;
	
    texture = textures[texnum];

    // Composited texture not created yet.
    texturecomposite[texnum] = 0;
    
    texturecompositesize[texnum] = 0;
    collump = texturecolumnlump[texnum];
    colofs = texturecolumnofs[texnum];
    
    // Now count the number of columns
    //  that are covered by more than one patch.
    // Fill in the lump / offset, so columns
    //  with only a single patch are all done.
    patchcount = (byte *) Z_Malloc(texture->width, PU_STATIC, (void**)&patchcount, "R_GenerateLookup");
    memset (patchcount, 0, texture->width);
    patch = texture->patches;

    for (i=0 , patch = texture->patches;
	 i<texture->patchcount;
	 i++, patch++)
    {
	realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
	x1 = patch->originx;
	x2 = x1 + SHORT(realpatch->width);
	
	if (x1 < 0)
	    x = 0;
	else
	    x = x1;

	if (x2 > texture->width)
	    x2 = texture->width;
	for ( ; x<x2 ; x++)
	{
	    patchcount[x]++;
	    collump[x] = patch->patch;
	    colofs[x] = LONG(realpatch->columnofs[x-x1])+3;
	}
    }
	
    for (x=0 ; x<texture->width ; x++)
    {
	if (!patchcount[x])
	{
	    C_Printf ("R_GenerateLookup: column without a patch (%s)\n",
		    texture->name);
	    return;
	}
	// I_Error ("R_GenerateLookup: column without a patch");
	
	if (patchcount[x] > 1)
	{
	    // Use the cached block.
	    collump[x] = -1;	
	    colofs[x] = texturecompositesize[texnum];
	    
	    if (texturecompositesize[texnum] > 0x10000-texture->height)
	    {
		I_Error ("R_GenerateLookup: texture %i is >64k",
			 texnum);
	    }
	    
	    texturecompositesize[texnum] += texture->height;
	}
    }

    Z_Free(patchcount, "R_GenerateLookup");
}




//
// R_GetColumn
//
byte*
R_GetColumn
( int		tex,
  int		col )
{
    int		lump;
    int		ofs;
	
    col &= texturewidthmask[tex];
    lump = texturecolumnlump[tex][col];
    ofs = texturecolumnofs[tex][col];
    
    if (lump > 0)
	return (byte *)W_CacheLumpNum(lump,PU_CACHE)+ofs;

    if (!texturecomposite[tex])
	R_GenerateComposite (tex);

    return texturecomposite[tex] + ofs;
}


static void GenerateTextureHashTable(void)
{
    texture_t **rover;
    int i;
    int key;

    textures_hashtable 
            = Z_Malloc(sizeof(texture_t *) * numtextures, PU_STATIC, 0, "GenerateTextureHashTable");

    memset(textures_hashtable, 0, sizeof(texture_t *) * numtextures);

    // Add all textures to hash table

    for (i=0; i<numtextures; ++i)
    {
        // Store index

        textures[i]->index = i;

        // Vanilla Doom does a linear search of the texures array
        // and stops at the first entry it finds.  If there are two
        // entries with the same name, the first one in the array
        // wins. The new entry must therefore be added at the end
        // of the hash chain, so that earlier entries win.

        key = W_LumpNameHash(textures[i]->name) % numtextures;

        rover = &textures_hashtable[key];

        while (*rover != NULL)
        {
            rover = &(*rover)->next;
        }

        // Hook into hash table

        textures[i]->next = NULL;
        *rover = textures[i];
    }
}


//
// R_InitTextures
// Initializes the texture list
//  with the textures from the world map.
//
void R_InitTextures (void)
{
    maptexture_t*	mtexture;
    texture_t*		texture;
    mappatch_t*		mpatch;
    texpatch_t*		patch;

    int			i;
    int			j;

    int*		maptex;
    int*		maptex2;
    int*		maptex1;
    
    char		name[9];
    char*		names;
    char*		name_p;
    
    int*		patchlookup;
    
    int			totalwidth;
    int			nummappatches;
    int			offset;
    int			maxoff;
    int			maxoff2;
    int			numtextures1;
    int			numtextures2;

    int*		directory;
    
    int			temp1;
    int			temp2;
    int			temp3;


    // Load the patch names from pnames.lmp.
    names = W_CacheLumpName (DEH_String("PNAMES"), PU_STATIC);
    nummappatches = LONG ( *((int *)names) );
    name_p = names+4;
    patchlookup = Z_Malloc(nummappatches*sizeof(*patchlookup), PU_STATIC, NULL, "R_InitTextures -> patchlookup");

    for (i = 0; i < nummappatches; i++)
    {
        M_StringCopy(name, name_p + i * 8, sizeof(name));
        patchlookup[i] = W_CheckNumForName (name);

	if (patchlookup[i] == -1 && devparm)
	    //jff 8/3/98 use logical output routine
            C_Printf("\nWarning: patch %.8s,\n index %d does not exist\n\n", name, i);
    }
    W_ReleaseLumpName(DEH_String("PNAMES"));

    // Load the map texture definitions from textures.lmp.
    // The data is contained in one or two lumps,
    //  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
    maptex = maptex1 = W_CacheLumpName (DEH_String("TEXTURE1"), PU_STATIC);
    numtextures1 = LONG(*maptex);
    maxoff = W_LumpLength (W_GetNumForName (DEH_String("TEXTURE1")));
    directory = maptex+1;

    if (W_CheckNumForName (DEH_String("TEXTURE2")) != -1)
    {
        maptex2 = W_CacheLumpName (DEH_String("TEXTURE2"), PU_STATIC);
        numtextures2 = LONG(*maptex2);
        maxoff2 = W_LumpLength (W_GetNumForName (DEH_String("TEXTURE2")));
    }
    else
    {
        maptex2 = NULL;
        numtextures2 = 0;
        maxoff2 = 0;
    }
    numtextures = numtextures1 + numtextures2;

    textures = Z_Malloc (numtextures * sizeof(*textures), PU_STATIC, 0, "R_InitTextures -> textures");
    texturecolumnlump = Z_Malloc (numtextures * sizeof(*texturecolumnlump), PU_STATIC, 0, "R_InitTextures -> texturecolumnlump (1)");
    texturecolumnofs = Z_Malloc (numtextures * sizeof(*texturecolumnofs), PU_STATIC, 0, "R_InitTextures -> texturecolumnofs (1)");
    texturecomposite = Z_Malloc (numtextures * sizeof(*texturecomposite), PU_STATIC, 0, "R_InitTextures -> texturecomposite");
    texturecompositesize = Z_Malloc (numtextures * sizeof(*texturecompositesize), PU_STATIC, 0, "R_InitTextures -> texturecompositesize");
    texturewidthmask = Z_Malloc (numtextures * sizeof(*texturewidthmask), PU_STATIC, 0, "R_InitTextures -> texturewidthmask");
    textureheight = Z_Malloc (numtextures * sizeof(*textureheight), PU_STATIC, 0, "R_InitTextures -> textureheight");

    totalwidth = 0;
    
    //	Really complex printing shit...
    temp1 = W_GetNumForName (DEH_String("S_START"));  // P_???????
    temp2 = W_GetNumForName (DEH_String("S_END")) - 1;
    temp3 = ((temp2-temp1+63)/64) + ((numtextures+63)/64);

    // If stdout is a real console, use the classic vanilla "filling
    // up the box" effect, which uses backspace to "step back" inside
    // the box.  If stdout is a file, don't draw the box.

    // haleyjd 20110206 [STRIFE]: box is in devparm only
    if (devparm && I_ConsoleStdout())
    {
        printf("[");
        for (i = 0; i < temp3 - 56/* + 9*/; i++)	// CHANGED FOR PSP DEBUG SCREEN
            printf(" ");
/*						// FOR PSP: looks like SH*T on the PSP debug screen
        printf("]");
        for (i = 0; i < temp3 + 10; i++)
            printf("\b");
*/
    }

    printf("[");

    for (i=0 ; i<numtextures ; i++, directory++)
    {
        if (!(i&63))
        {
            // [STRIFE]: tick intro if not in devparm

//	    if(devparm)
	    {
#ifdef SHAREWARE
		if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)
		    printf (".");
#endif
	    }
//	    else
		D_IntroTick();

        }

        if (i == numtextures1)
        {
            // Start looking in second texture file.
            maptex = maptex2;
            maxoff = maxoff2;
            directory = maptex+1;
        }

        offset = LONG(*directory);

        if (offset > maxoff)
            I_Error ("R_InitTextures: bad texture directory");

        mtexture = (maptexture_t *) ( (byte *)maptex + offset);

        texture = textures[i] =
            Z_Malloc (sizeof(texture_t)
                      + sizeof(texpatch_t)*(SHORT(mtexture->patchcount)-1),
                      PU_STATIC, 0, "R_InitTextures -> texture");

        texture->width = SHORT(mtexture->width);
        texture->height = SHORT(mtexture->height);
        texture->patchcount = SHORT(mtexture->patchcount);

        memcpy (texture->name, mtexture->name, sizeof(texture->name));
        mpatch = &mtexture->patches[0];
        patch = &texture->patches[0];

        for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
        {
            patch->originx = SHORT(mpatch->originx);
            patch->originy = SHORT(mpatch->originy);
            patch->patch = patchlookup[SHORT(mpatch->patch)];
            if (patch->patch == -1)
            {
/*
                I_Error ("R_InitTextures: Missing patch in texture %s",
                         texture->name);
*/
                //jff 8/3/98 use logical output routine
                C_Printf("R_InitTextures: Missing patch\n %d in texture %.8s\n\n",
                    SHORT(mpatch->patch), texture->name); // killough 4/17/98
            }
        }		
        texturecolumnlump[i] = Z_Malloc (texture->width*sizeof(**texturecolumnlump), PU_STATIC,0, "R_InitTextures -> texturecolumnlump (2)");
        texturecolumnofs[i] = Z_Malloc (texture->width*sizeof(**texturecolumnofs), PU_STATIC,0, "R_InitTextures -> texturecolumnofs (2)");

        j = 1;
        while (j*2 <= texture->width)
            j<<=1;

        texturewidthmask[i] = j-1;
        textureheight[i] = texture->height<<FRACBITS;

        totalwidth += texture->width;
    }

    Z_Free(patchlookup, "R_InitTextures");

    W_ReleaseLumpName(DEH_String("TEXTURE1"));
    if (maptex2)
        W_ReleaseLumpName(DEH_String("TEXTURE2"));

    // Precalculate whatever possible.	

    for (i=0 ; i<numtextures ; i++)
    {
        // [STRIFE]: tick intro
        if(!(i & 63))
            D_IntroTick();

        R_GenerateLookup (i);
    }

    // Create translation table for global animation.
    texturetranslation = Z_Malloc ((numtextures+1)*sizeof(*texturetranslation), PU_STATIC, 0, "R_InitTextures -> texturetranslation");

    for (i=0 ; i<numtextures ; i++)
        texturetranslation[i] = i;

    GenerateTextureHashTable();
}



//
// R_InitFlats
//
void R_InitFlats (void)
{
    int		i;
	
    firstflat = W_GetNumForName (DEH_String("F_START")) + 1;
    lastflat = W_GetNumForName (DEH_String("F_END")) - 1;
    numflats = lastflat - firstflat + 1;
	
    // Create translation table for global animation.
    flattranslation = Z_Malloc ((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0, "R_InitFlats");
    
    for (i=0 ; i<numflats ; i++)
	flattranslation[i] = i;
}


//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//
void R_InitSpriteLumps (void)
{
    int		i;
    patch_t	*patch;

    firstspritelump = W_GetNumForName (DEH_String("S_START")) + 1;
    lastspritelump = W_GetNumForName (DEH_String("S_END")) - 1;

    numspritelumps = lastspritelump - firstspritelump + 1;
    spritewidth = Z_Malloc (numspritelumps*sizeof(*spritewidth), PU_STATIC, 0, "R_InitSpriteLumps -> spritewidth");
    spriteoffset = Z_Malloc (numspritelumps*sizeof(*spriteoffset), PU_STATIC, 0, "R_InitSpriteLumps -> spriteoffset");
    spritetopoffset = Z_Malloc (numspritelumps*sizeof(*spritetopoffset), PU_STATIC, 0, "R_InitSpriteLumps -> spritetopoffset");

    // [SVE] svillarreal
    spriteheight = Z_Malloc(numspritelumps*sizeof(*spriteheight), PU_STATIC, 0, "R_InitSpriteLumps -> spriteheight");

    for (i=0 ; i< numspritelumps ; i++)
    {
        if(!(i&63))
        {
            // [STRIFE] tick intro if not in devparm
            if(devparm 
#ifdef SHAREWARE
			|| STRIFE_1_1_SHAREWARE
#endif
						)
                printf (".");
            else
                D_IntroTick();
        }

        patch = W_CacheLumpNum (firstspritelump+i, PU_CACHE);
        spritewidth[i] = SHORT(patch->width)<<FRACBITS;
        spriteoffset[i] = SHORT(patch->leftoffset)<<FRACBITS;
        spritetopoffset[i] = SHORT(patch->topoffset)<<FRACBITS;

        // [SVE] svillarreal
        spriteheight[i] = SHORT(patch->height)<<FRACBITS;
    }
}



//
// R_InitColormaps
//
void R_InitColormaps (void)
{
    int	lump;

    // Load in the light tables, 256 byte align tables.
    lump = W_GetNumForName(DEH_String("COLORMAP"));
    colormaps = W_CacheLumpNum(lump, PU_STATIC);
}



//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData (void)
{
    C_Printf("\n Textures\n");
    R_InitTextures ();
    if(devparm
#ifdef SHAREWARE
		|| STRIFE_1_1_SHAREWARE
#endif
					)
        printf (".");
    else
        D_IntroTick(); // [STRIFE] tick intro

    C_Printf("Flats\n");
    R_InitFlats ();

//    if(devparm)
    {
#ifdef SHAREWARE
	if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)
	    printf (".");
#endif
    }
//    else
	D_IntroTick();

    C_Printf("Sprites\n\n");
    R_InitSpriteLumps ();

//    if(devparm)
    {
#ifdef SHAREWARE
	if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)
	    printf (".");
#endif
    }
//    else
	D_IntroTick();

    R_InitColormaps ();
}



//
// R_FlatNumForName
// Retrieval, get a flat number for a flat name.
//
int R_FlatNumForName (char* name)
{
    int		i;
    char	namet[9];

    if (name[0] == 'F' && name[1] == '_' && name[2] == 'G' && name[3] == 'R' && name[4] == 'A' &&
            name[5] == 'S' && name[6] == 'S' && isdemoversion)
	name = "F_GRASSD";

    i = W_CheckNumForName (name);

    if (i == -1)
    {
	namet[8] = 0;
	memcpy (namet, name,8);
//	I_Error ("R_FlatNumForName: %s not found",namet);
	C_Printf("R_FlatNumForName: %.8s not found", name);
    }
    return i - firstflat;
}




//
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int	R_CheckTextureNumForName (char *name)
{
    texture_t *texture;
    int key;

    // "NoTexture" marker.
    if (name[0] == '-')		
	return 0;
		
    key = W_LumpNameHash(name) % numtextures;

    texture=textures_hashtable[key]; 
    
    while (texture != NULL)
    {
	if (!strncasecmp (texture->name, name, 8) )
	    return texture->index;

        texture = texture->next;
    }
    
    return -1;
}



//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int	R_TextureNumForName (char* name)
{
    int		i;

    if (name[0] == 'A' &&
	    name[1] == 'A' &&
	    name[2] == 'S' &&
	    name[3] == 'T' &&
	    name[4] == 'I' &&
            name[5] == 'N' &&
	    name[6] == 'K' &&
	    name[7] == 'Y' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("AASTINKD");

    else if(name[0] == 'B' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'S' &&
	    name[4] == 'T' &&
            name[5] == 'N' &&
	    name[6] == '0' &&
	    name[7] == '1' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("BIGSTN1D");

    else if(name[0] == 'C' &&
	    name[1] == 'O' &&
	    name[2] == 'M' &&
	    name[3] == 'P' &&
	    name[4] == '0' &&
            name[5] == '4' &&
	    name[6] == 'B' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("COMP04BD");

    else if(name[0] == 'C' &&
	    name[1] == 'O' &&
	    name[2] == 'M' &&
	    name[3] == 'P' &&
	    name[4] == '1' &&
            name[5] == '2' &&
	    name[6] == 'B' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("COMP12BD");

    else if(name[0] == 'R' &&
	    name[1] == 'O' &&
	    name[2] == 'B' &&
	    name[3] == 'O' &&
	    name[4] == 'T' &&
            name[5] == '0' &&
	    name[6] == '2' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("ROBOT02D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '2' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN02D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '3' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN03D");
    
    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '4' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN04D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '5' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN05D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '6' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN06D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '7' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN07D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '0' &&
            name[5] == '9' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN09D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '1' &&
            name[5] == '0' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN10D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '1' &&
            name[5] == '1' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN11D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '2' &&
            name[5] == '0' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN20D");

    else if(name[0] == 'S' &&
	    name[1] == 'I' &&
	    name[2] == 'G' &&
	    name[3] == 'N' &&
	    name[4] == '2' &&
            name[5] == '2' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SIGN22D");

    else if(name[0] == 'S' &&
	    name[1] == 'K' &&
	    name[2] == 'Y' &&
	    name[3] == 'M' &&
	    name[4] == 'N' &&
            name[5] == 'T' &&
            name[6] == '0' &&
            name[7] == '2' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SKYMNT2D");

    else if(name[0] == 'S' &&
	    name[1] == 'W' &&
	    name[2] == 'C' &&
	    name[3] == 'H' &&
	    name[4] == 'N' &&
            name[5] == '0' &&
            name[6] == '1' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SWCHN1D");

    else if(name[0] == 'S' &&
	    name[1] == 'W' &&
	    name[2] == 'C' &&
	    name[3] == 'H' &&
	    name[4] == 'N' &&
            name[5] == '0' &&
            name[6] == '2' &&
	    isdemoversion)

	i = R_CheckTextureNumForName ("SWCHN2D");
    else
	i = R_CheckTextureNumForName (name);

    if (i==-1)
    {
//	I_Error ("R_TextureNumForName: %s not found", name);
	C_Printf("R_TextureNumForName: %.8s not found", name);
    }
    return i;
}

//
// R_SoundNumForDoor
//
// villsa [STRIFE] - new function
// Set sounds associated with door though why
// on earth is this function placed here?
//
void R_SoundNumForDoor(vldoor_t* door)
{
    int         i;
    sector_t    *sector;
    line_t      *line;
    texture_t   *texture;
    char        name[8];
    char        c1;
    char        c2;

    // set default sounds
    door->opensound = sfx_drsmto;
    door->closesound = sfx_drsmtc;

    for(sector = door->sector, i = 0; i < sector->linecount; i++)
    {
        line = sector->lines[i];

        if(!(line->flags & ML_TWOSIDED))
            continue;

        texture = textures[sides[line->sidenum[0]].toptexture];
        memcpy(name, texture->name, 8);

        if(strncmp(name, "DOR", 3))
            continue;

        c1 = name[3];
        c2 = name[4];

        // S type
        if(c1 == 'S')
        {
            door->opensound = sfx_drston;
            door->closesound = sfx_drston;
            return;
        }

        // M type
        if(c1 == 'M')
        {
            // L subtype
            if(c2 == 'L')
            {
                door->opensound = sfx_drlmto;
                door->closesound = sfx_drlmtc;
            }
            // S subtype
            else if(c2 == 'S')
            {
                door->opensound = sfx_drsmto;
                door->closesound = sfx_drsmtc;
            }
            return;
        }
        // W type
        else if(c1 == 'W')
        {
            // L subtype
            if(c2 == 'L')
            {
                door->opensound = sfx_drlwud;
                door->closesound = sfx_drlwud;
            }
            // S subtype
            else if(c2 == 'S')
            {
                door->opensound = sfx_drswud;
                door->closesound = sfx_drswud;
            }
            return;
        }
    }
}




//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//
int		flatmemory;
int		texturememory;
int		spritememory;

void R_PrecacheLevel (void)
{
    char*		flatpresent;
    char*		texturepresent;
    char*		spritepresent;

    int			i;
    int			j;
    int			k;
    int			lump;
    
    texture_t*		texture;
    thinker_t*		th;
    spriteframe_t*	sf;

    if (demoplayback)
	return;
    
    // Precache flats.
    flatpresent = Z_Malloc(numflats, PU_STATIC, NULL, "R_PrecacheLevel -> flatpresent");
    memset (flatpresent,0,numflats);	

    for (i=0 ; i<numsectors ; i++)
    {
	flatpresent[sectors[i].floorpic] = 1;
	flatpresent[sectors[i].ceilingpic] = 1;
    }
	
    flatmemory = 0;

    for (i=0 ; i<numflats ; i++)
    {
	if (flatpresent[i])
	{
	    lump = firstflat + i;
	    flatmemory += lumpinfo[lump].size;
	    W_CacheLumpNum(lump, PU_CACHE);
	}
    }

    Z_Free(flatpresent, "R_PrecacheLevel -> flatpresent");
    
    // Precache textures.
    texturepresent = Z_Malloc(numtextures, PU_STATIC, NULL, "R_PrecacheLevel -> texturepresent");
    memset (texturepresent,0, numtextures);
	
    for (i=0 ; i<numsides ; i++)
    {
	texturepresent[sides[i].toptexture] = 1;
	texturepresent[sides[i].midtexture] = 1;
	texturepresent[sides[i].bottomtexture] = 1;
    }

    // Sky texture is always present.
    // Note that F_SKY1 is the name used to
    //  indicate a sky floor/ceiling as a flat,
    //  while the sky texture is stored like
    //  a wall texture, with an episode dependend
    //  name.
    texturepresent[skytexture] = 1;
	
    texturememory = 0;
    for (i=0 ; i<numtextures ; i++)
    {
	if (!texturepresent[i])
	    continue;

	texture = textures[i];
	
	for (j=0 ; j<texture->patchcount ; j++)
	{
	    lump = texture->patches[j].patch;
	    texturememory += lumpinfo[lump].size;
	    W_CacheLumpNum(lump , PU_CACHE);
	}
    }

    Z_Free(texturepresent, "R_PrecacheLevel -> texturepresent");
    
    // Precache sprites.
    spritepresent = Z_Malloc(numsprites, PU_STATIC, NULL, "R_PrecacheLevel -> spritepresent");
    memset (spritepresent,0, numsprites);
	
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acp1 == (actionf_p1)P_MobjThinker)
	    spritepresent[((mobj_t *)th)->sprite] = 1;
    }
	
    spritememory = 0;
    for (i=0 ; i<numsprites ; i++)
    {
	if (!spritepresent[i])
	    continue;

	for (j=0 ; j<sprites[i].numframes ; j++)
	{
	    sf = &sprites[i].spriteframes[j];
	    for (k=0 ; k<8 ; k++)
	    {
		lump = firstspritelump + sf->lump[k];
		spritememory += lumpinfo[lump].size;
		W_CacheLumpNum(lump , PU_CACHE);
	    }
	}
    }

    Z_Free(spritepresent, "R_PrecacheLevel -> spritepresent");
}



