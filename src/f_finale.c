//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2010 James Haley, Samuel Villarreal
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
//	Game completion, final screen animation.
//
// [STRIFE] Module marked finished 2010-09-13 22:56
//


#include <stdio.h>
#include <ctype.h>

// Functions.
#include "deh_str.h"
#include "i_system.h"
#include "i_swap.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"

// Data.
#include "d_main.h"
#include "dstrings.h"
#include "sounds.h"

#include "doomstat.h"
#include "r_state.h"

#include "p_dialog.h" // [STRIFE]

typedef enum
{
    F_STAGE_TEXT,
    F_STAGE_ARTSCREEN,
    F_STAGE_CAST,
} finalestage_t;

// ?
//#include "doomstat.h"
//#include "r_local.h"
//#include "f_finale.h"

// Stage of animation:
finalestage_t finalestage;

unsigned int finalecount;

// haleyjd 09/12/10: [STRIFE] Slideshow variables
char         *slideshow_panel;
unsigned int  slideshow_tics;
int           slideshow_state;

boolean scroll_finished = false;					// ADDED FOR DEMO

// haleyjd   09/13/10: [STRIFE] All this is unused...
// nitr8  [2014/12/30] ...until now.

#define	TEXTSPEED	3						// REACTIVATED FOR DEMO
//#define	TEXTWAIT	250

typedef struct								// REACTIVATED FOR DEMO
{									// REACTIVATED FOR DEMO
    GameMission_t mission;						// REACTIVATED FOR DEMO
    int episode, level;							// REACTIVATED FOR DEMO
    char *background;							// REACTIVATED FOR DEMO
    char *text;								// REACTIVATED FOR DEMO
} textscreen_t;								// REACTIVATED FOR DEMO

static textscreen_t textscreens[] =					// REACTIVATED FOR DEMO
{									// REACTIVATED FOR DEMO
/*
    { doom,      1, 8,  "FLOOR4_8",  E1TEXT},
    { doom,      2, 8,  "SFLR6_1",   E2TEXT},
    { doom,      3, 8,  "MFLR8_4",   E3TEXT},
    { doom,      4, 8,  "MFLR8_3",   E4TEXT},

    { doom2,     1, 6,  "SLIME16",   C1TEXT},
    { doom2,     1, 11, "RROCK14",   C2TEXT},
    { doom2,     1, 20, "RROCK07",   C3TEXT},
    { doom2,     1, 30, "RROCK17",   C4TEXT},
    { doom2,     1, 15, "RROCK13",   C5TEXT},
    { doom2,     1, 31, "RROCK19",   C6TEXT},

    { pack_tnt,  1, 6,  "SLIME16",   T1TEXT},
    { pack_tnt,  1, 11, "RROCK14",   T2TEXT},
    { pack_tnt,  1, 20, "RROCK07",   T3TEXT},
    { pack_tnt,  1, 30, "RROCK17",   T4TEXT},
    { pack_tnt,  1, 15, "RROCK13",   T5TEXT},
    { pack_tnt,  1, 31, "RROCK19",   T6TEXT},

    { pack_plut, 1, 6,  "SLIME16",   P1TEXT},
    { pack_plut, 1, 11, "RROCK14",   P2TEXT},
    { pack_plut, 1, 20, "RROCK07",   P3TEXT},
    { pack_plut, 1, 30, "RROCK17",   P4TEXT},
    { pack_plut, 1, 15, "RROCK13",   P5TEXT},
    { pack_plut, 1, 31, "RROCK19",   P6TEXT},
*/
    { strife,    1, 34, "PANEL7",    DEMOTEXT},				// ADDED FOR DEMO
};									// REACTIVATED FOR DEMO

char*	finaletext;							// REACTIVATED FOR DEMO
char*	finaleflat;							// REACTIVATED FOR DEMO

void	F_StartCast (void);
void	F_CastTicker (void);
boolean F_CastResponder (event_t *ev);
void	F_CastDrawer (void);

// [STRIFE] - Slideshow states enumeration
enum
{
    // Exit states
    SLIDE_EXITHACK    = -99, // Hacky exit - start a new dialog
    SLIDE_HACKHACK    =  -9, // Bizarre unused state
    SLIDE_EXIT        =  -1, // Exit to next finale state
    SLIDE_CHOCO       =  -2, // haleyjd: This state is Choco-specific... see below.

    // Unknown
    SLIDE_UNKNOWN     =   0, // Dunno what it's for, possibly unused

    // MAP03 - Macil's Programmer exposition
    SLIDE_PROGRAMMER1 =   1, 
    SLIDE_PROGRAMMER2,
    SLIDE_PROGRAMMER3,
    SLIDE_PROGRAMMER4, // Next state = -99

    // MAP10 - Macil's Sigil exposition
    SLIDE_SIGIL1      =   5,
    SLIDE_SIGIL2,
    SLIDE_SIGIL3,
    SLIDE_SIGIL4, // Next state = -99

    // MAP29 - Endings
    // Good Ending
    SLIDE_GOODEND1    =  10,
    SLIDE_GOODEND2,
    SLIDE_GOODEND3,
    SLIDE_GOODEND4, // Next state = -1

    // Bad Ending
    SLIDE_BADEND1     =  14,
    SLIDE_BADEND2,
    SLIDE_BADEND3, // Next state = -1

    // Blah Ending
    SLIDE_BLAHEND1    =  17,
    SLIDE_BLAHEND2,
    SLIDE_BLAHEND3, // Next state = -1

    // Demo Ending - haleyjd 20130301: v1.31 only
    SLIDE_DEMOEND1    =  25,
    SLIDE_DEMOEND2 // Next state = -1
};

//
// F_StartFinale
//
// [STRIFE]
// haleyjd 09/13/10: Modified to drive slideshow sequences.
//
void F_StartFinale (void)
{
#if 0
    // haleyjd 20111006: see below...
    patch_t *panel;
#endif
    size_t i;								// ADDED FOR DEMO

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    viewactive = false;
    automapactive = false;
    wipegamestate = -1; // [STRIFE]

    // [STRIFE] Setup the slide show
//    slideshow_panel = DEH_String("PANEL0");				// MODIFIED FOR DEMO

    // haleyjd 20111006: These two lines of code *are* in vanilla Strife; 
    // however, there, they were completely inconsequential due to the dirty
    // rects system. No intervening V_MarkRect call means PANEL0 was never 
    // drawn to the framebuffer. In Chocolate Strife, however, with no such
    // system in place, this only manages to fuck up the fade-out that is
    // supposed to happen at the beginning of all finales. So, don't do it!
#if 0
    panel = (patch_t *)W_CacheLumpName(slideshow_panel, PU_CACHE);
    V_DrawPatch(0, 0, panel);
#endif

    switch(gamemap)
    {
    case 3:  // Macil's exposition on the Programmer
	finalestage = F_STAGE_TEXT;					// ADDED FOR DEMO
	finalecount = 0;						// ADDED FOR DEMO
	slideshow_tics = 7;						// ADDED FOR DEMO
	slideshow_panel = DEH_String("PANEL0");				// ADDED FOR DEMO
	S_ChangeMusic(mus_dark, 1);					// ADDED FOR DEMO
        slideshow_state = SLIDE_PROGRAMMER1;
        break;
    case 9:  // Super hack for death of Programmer
	finalestage = F_STAGE_TEXT;					// ADDED FOR DEMO
	finalecount = 0;						// ADDED FOR DEMO
	slideshow_tics = 7;						// ADDED FOR DEMO
	slideshow_panel = DEH_String("PANEL0");				// ADDED FOR DEMO
	S_ChangeMusic(mus_dark, 1);					// ADDED FOR DEMO
        slideshow_state = SLIDE_EXITHACK; 
        break;
    case 10: // Macil's exposition on the Sigil
	finalestage = F_STAGE_TEXT;					// ADDED FOR DEMO
	finalecount = 0;						// ADDED FOR DEMO
	slideshow_tics = 7;						// ADDED FOR DEMO
	slideshow_panel = DEH_String("PANEL0");				// ADDED FOR DEMO
	S_ChangeMusic(mus_dark, 1);					// ADDED FOR DEMO
        slideshow_state = SLIDE_SIGIL1;
        break;
    case 29: // Endings
//        if(!netgame)
        {
	    finalestage = F_STAGE_TEXT;					// ADDED FOR DEMO
	    finalecount = 0;						// ADDED FOR DEMO
	    slideshow_tics = 7;						// ADDED FOR DEMO
	    slideshow_panel = DEH_String("PANEL0");			// ADDED FOR DEMO
	    S_ChangeMusic(mus_dark, 1);					// ADDED FOR DEMO
            if(players[0].health <= 0)            // Bad ending 
                slideshow_state = SLIDE_BADEND1;  // - Humanity goes extinct
            else
            {
                if((players[0].questflags & QF_QUEST25) && // Converter destroyed
                   (players[0].questflags & QF_QUEST27))   // Computer destroyed (wtf?!)
                {
                    // Good ending - You get the hot babe.
                    slideshow_state = SLIDE_GOODEND1; 
                }
                else
                {
                    // Blah ending - You win the battle, but fail at life.
                    slideshow_state = SLIDE_BLAHEND1;
                }
            }
        }
        break;
    case 34: // For the demo version ending
	{								// ADDED FOR DEMO
//	    slideshow_state = SLIDE_EXIT;				// MODIFIED FOR DEMO

    	    // haleyjd 20130301: Somebody noticed the demo levels were missing the
    	    // ending they used to have in the demo version EXE, I guess. But the
    	    // weird thing is, this will only trigger if you run with strife0.wad,
    	    // and no released version thereof actually works with the 1.31 EXE
    	    // due to differing dialog formats... was there to be an updated demo
    	    // that never got released?!

//	    if(gameversion == exe_strife_1_31 && isdemoversion)		// MODIFIED FOR DEMO

	    // Find the right screen and set the text and background

	    scroll_finished = false;					// ADDED FOR DEMO
	    slideshow_tics = 0;						// ADDED FOR DEMO
	    slideshow_panel = DEH_String("PANEL7");			// ADDED FOR DEMO
	    slideshow_state = SLIDE_DEMOEND1;				// ADDED FOR DEMO

	    for (i=0; i<arrlen(textscreens); ++i)			// ADDED FOR DEMO
	    {								// ADDED FOR DEMO
		textscreen_t *screen = &textscreens[i];			// ADDED FOR DEMO

		finaletext = screen->text;				// ADDED FOR DEMO
		finaleflat = screen->background;			// ADDED FOR DEMO
	    }								// ADDED FOR DEMO

	    // Do dehacked substitutions of strings
  
	    finaletext = DEH_String(finaletext);			// ADDED FOR DEMO
	    finaleflat = DEH_String(finaleflat);			// ADDED FOR DEMO
    
	    finalestage = F_STAGE_TEXT;					// ADDED FOR DEMO
	    finalecount = 0;						// ADDED FOR DEMO

	    S_StopMusic();						// ADDED FOR DEMO
	}								// ADDED FOR DEMO
	break;
    }
/*
    S_ChangeMusic(mus_dark, 1);						// MODIFIED FOR DEMO

    slideshow_tics = 7;							// MODIFIED FOR DEMO
    finalestage = F_STAGE_TEXT;						// MODIFIED FOR DEMO
    finalecount = 0;							// MODIFIED FOR DEMO
*/
}

//
// F_Responder
//
// [STRIFE] Verified unmodified
//
boolean F_Responder (event_t *event)
{
    if (finalestage == F_STAGE_CAST)
        return F_CastResponder (event);

    return false;
}

//
// F_WaitTicker
//
// [STRIFE] New function
// haleyjd 09/13/10: This is called from G_Ticker if gamestate is 1, but we
// have no idea for what it's supposed to be. It is unused.
//
void F_WaitTicker(void)
{
    if(++finalecount >= 250)
    {
        gamestate   = GS_FINALE;
        finalestage = 0;
        finalecount = 0;
    }
}

//
// F_DrawPatchCol
//
void
F_DrawPatchCol
( int		x,
  patch_t*	patch,
  int		col )
{
    column_t*	column;
    byte*	source;
    byte*	dest;
    byte*	desttop;
//    int		count;						// CHANGED FOR HIRES (ORIGINAL)
    int		count, f;						// CHANGED FOR HIRES
	
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    desttop = I_VideoBuffer + x;

    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
	for (f = 0; f <= hires; f++)					// ADDED FOR HIRES
	{								// ADDED FOR HIRES
	    source = (byte *)column + 3;
//	    dest = desttop + column->topdelta*SCREENWIDTH;		// CHANGED FOR HIRES (ORIGINAL)
	    dest = desttop + column->topdelta*(SCREENWIDTH << hires)
			    + (x * hires) + f;				// CHANGED FOR HIRES
	    count = column->length;

	    while (count--)
	    {
		if (hires)						// ADDED FOR HIRES
		{							// ADDED FOR HIRES
		    *dest = *source;					// ADDED FOR HIRES
		    dest += SCREENWIDTH;				// ADDED FOR HIRES
		}							// ADDED FOR HIRES
		*dest = *source++;
		dest += SCREENWIDTH;
	    }
	}								// ADDED FOR HIRES
	column = (column_t *)(  (byte *)column + column->length + 4 );
    }
}


//
// F_BunnyScroll
//
void F_BunnyScroll (void)						// FUNCTION RE-ADDED FOR DEMO
{
    signed int  scrolled;
    int		x;
    patch_t*	p1;
    patch_t*	p2;

    p1 = W_CacheLumpName (DEH_String("vellogo"), PU_LEVEL);
    p2 = W_CacheLumpName (DEH_String("credit"),  PU_LEVEL);

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);

    scrolled = (320 - ((signed int) finalecount-230)/2);
    if (scrolled > 320)
	scrolled = 320;
    if (scrolled < 0)
	scrolled = 0;
		
//    for ( x=0 ; x<SCREENWIDTH ; x++)					// (ORIGINAL)
    for ( x=0 ; x<ORIGWIDTH ; x++)					// CHANGED FOR HIRES
    {
	if (x+scrolled < 320)
	    F_DrawPatchCol (x, p1, x+scrolled);
	else
	    F_DrawPatchCol (x, p2, x+scrolled - 320);
    }
}

// 
// F_DoSlideShow
//
// [STRIFE] New function
// haleyjd 09/13/10: Handles slideshow states. Begging to be tabulated!
//
static void F_DoSlideShow(void)
{
    patch_t *patch;

    switch(slideshow_state)
    {
    case SLIDE_UNKNOWN: // state #0, seems to be unused
        slideshow_tics = 700;
        slideshow_state = SLIDE_EXIT;
        // falls through into state 1, so above is pointless? ...

    case SLIDE_PROGRAMMER1: // state #1
        slideshow_panel = DEH_String("SS2F1");
        I_StartVoice(DEH_String("MAC10"));
        slideshow_state = SLIDE_PROGRAMMER2;
        slideshow_tics = 315;
        break;
    case SLIDE_PROGRAMMER2: // state #2
        slideshow_panel = DEH_String("SS2F2");
        I_StartVoice(DEH_String("MAC11"));
        slideshow_state = SLIDE_PROGRAMMER3;
        slideshow_tics = 350;
        break;
    case SLIDE_PROGRAMMER3: // state #3
        slideshow_panel = DEH_String("SS2F3");
        I_StartVoice(DEH_String("MAC12"));
        slideshow_state = SLIDE_PROGRAMMER4;
        slideshow_tics = 420;
        break;
    case SLIDE_PROGRAMMER4: // state #4
        slideshow_panel = DEH_String("SS2F4");
        I_StartVoice(DEH_String("MAC13"));
        slideshow_state = SLIDE_EXITHACK; // End of slides
        slideshow_tics = 595;
        break;

    case SLIDE_SIGIL1: // state #5
        slideshow_panel = DEH_String("SS3F1");
        I_StartVoice(DEH_String("MAC16"));
        slideshow_state = SLIDE_SIGIL2;
        slideshow_tics = 350;
        break;
    case SLIDE_SIGIL2: // state #6
        slideshow_panel = DEH_String("SS3F2");
        I_StartVoice(DEH_String("MAC17"));
        slideshow_state = SLIDE_SIGIL3;
        slideshow_tics = 420;
        break;
    case SLIDE_SIGIL3: // state #7
        slideshow_panel = DEH_String("SS3F3");
        I_StartVoice(DEH_String("MAC18"));
        slideshow_tics = 420;
        slideshow_state = SLIDE_SIGIL4;
        break;
    case SLIDE_SIGIL4: // state #8
        slideshow_panel = DEH_String("SS3F4");
        I_StartVoice(DEH_String("MAC19"));
        slideshow_tics = 385;
        slideshow_state = SLIDE_EXITHACK; // End of slides
        break;

    case SLIDE_GOODEND1: // state #10
        slideshow_panel = DEH_String("SS4F1");
        S_StartMusic(mus_happy);
        I_StartVoice(DEH_String("RIE01"));
        slideshow_state = SLIDE_GOODEND2;
        slideshow_tics = 455;
        break;
    case SLIDE_GOODEND2: // state #11
        slideshow_panel = DEH_String("SS4F2");
        I_StartVoice(DEH_String("BBX01"));
        slideshow_state = SLIDE_GOODEND3;
        slideshow_tics = 385;
        break;
    case SLIDE_GOODEND3: // state #12
        slideshow_panel = DEH_String("SS4F3");
        I_StartVoice(DEH_String("BBX02"));
        slideshow_state = SLIDE_GOODEND4;
        slideshow_tics = 490;
        break;
    case SLIDE_GOODEND4: // state #13
        slideshow_panel = DEH_String("SS4F4");
        slideshow_state = SLIDE_EXIT; // Go to end credits
        slideshow_tics = 980;
        break;

    case SLIDE_BADEND1: // state #14
        S_StartMusic(mus_sad);
        slideshow_panel = DEH_String("SS5F1");
        I_StartVoice(DEH_String("SS501b"));
        slideshow_state = SLIDE_BADEND2;
        slideshow_tics = 385;
        break;
    case SLIDE_BADEND2: // state #15
        slideshow_panel = DEH_String("SS5F2");
        I_StartVoice(DEH_String("SS502b"));
        slideshow_state = SLIDE_BADEND3;
        slideshow_tics = 350;
        break;
    case SLIDE_BADEND3: // state #16
        slideshow_panel = DEH_String("SS5F3");
        I_StartVoice(DEH_String("SS503b"));
        slideshow_state = SLIDE_EXIT; // Go to end credits
        slideshow_tics = 385;
        break;

    case SLIDE_BLAHEND1: // state #17
        S_StartMusic(mus_end);
        slideshow_panel = DEH_String("SS6F1");
        I_StartVoice(DEH_String("SS601A"));
        slideshow_state = SLIDE_BLAHEND2;
        slideshow_tics = 280;
        break;
    case SLIDE_BLAHEND2: // state #18
        S_StartMusic(mus_end);
        slideshow_panel = DEH_String("SS6F2");
        I_StartVoice(DEH_String("SS602A"));
        slideshow_state = SLIDE_BLAHEND3;
        slideshow_tics = 280;
        break;
    case SLIDE_BLAHEND3: // state #19
        S_StartMusic(mus_end);
        slideshow_panel = DEH_String("SS6F3");
        I_StartVoice(DEH_String("SS603A"));
        slideshow_state = SLIDE_EXIT; // Go to credits
        slideshow_tics = 315;
        break;

    case SLIDE_DEMOEND1: // state #25 - only exists in 1.31
//        slideshow_panel = DEH_String("PANEL7");			// MODIFIED FOR DEMO
        finalecount = 0;						// ADDED FOR DEMO
        wipegamestate = -1;						// ADDED FOR DEMO
        slideshow_tics = 430;						// MOD. FOR DEMO: INC. BY +255
	S_ChangeMusic(mus_drone, 1);					// ADDED FOR DEMO
        slideshow_state = SLIDE_DEMOEND2;
        break;
    case SLIDE_DEMOEND2: // state #26 - ditto
//        slideshow_panel = DEH_String("VELLOGO");			// MODIFIED FOR DEMO
        wipegamestate = -1;						// ADDED FOR DEMO
        S_StartMusic(mus_fast);						// ADDED FOR DEMO
        finalecount = 0;						// ADDED FOR DEMO
        finalestage = F_STAGE_ARTSCREEN;				// ADDED FOR DEMO
        slideshow_tics = 1000;						// MOD. FOR DEMO: INC. BY +825
        slideshow_state = SLIDE_EXIT; // Go to end credits
        break;
    case SLIDE_EXITHACK: // state -99: super hack state
        gamestate = GS_LEVEL;
        P_DialogStartP1();
        break;
    case SLIDE_HACKHACK: // state -9: unknown bizarre unused state
        S_StartSound(NULL, sfx_rifle);
        slideshow_tics = 3150;
        break;
    case SLIDE_EXIT: // state -1: proceed to next finale stage
        finalecount = 0;
        finalestage = F_STAGE_ARTSCREEN;

	if(gamemap != 34)						// ADDED FOR DEMO
	    wipegamestate = -1;						
	else								// ADDED FOR DEMO
	    scroll_finished = true;					// ADDED FOR DEMO

        S_StartMusic(mus_fast);
        // haleyjd 20130301: The ONLY glitch fixed in 1.31 of Strife
        // *would* be something this insignificant, of course!
        if(gameversion != exe_strife_1_31)
            slideshow_state = SLIDE_CHOCO; // haleyjd: see below...
        break;
    case SLIDE_CHOCO: 
        // haleyjd 09/14/10: This wouldn't be necessary except that Choco
        // doesn't support the V_MarkRect dirty rectangles system. This
        // just so happens to have hidden the fact that the ending
        // does a screenfade every ~19 seconds due to remaining stuck in
        // SLIDE_EXIT state above, UNLESS the menus were active - the
        // V_MarkRect calls in the menu system cause it to be visible. 
        // This means that in order to get the same behavior as the vanilla
        // EXE, I need different code. So, come to this state and only set 
        // wipegamestate if menuactive is true.
        finalecount = 0;
        finalestage = F_STAGE_ARTSCREEN;
/*
        if(menuactive)							// UNSURE... DUNNO... (??) :-/
            wipegamestate = -1;						// UNSURE... DUNNO... (??) :-/
*/
        S_StartMusic(mus_fast);
        slideshow_state = SLIDE_CHOCO; // remain here.
        break;
    default:
        break;
    }

    finalecount = 0;
    if(gameversion != exe_strife_1_31) // See above. This was removed in 1.31.
    {
       patch = (patch_t *)W_CacheLumpName(DEH_String("PANEL0"), PU_CACHE);
       V_DrawPatch(0, 0, patch);
    }
}

//
// F_Ticker
//
// [STRIFE] Modifications for new finales
// haleyjd 09/13/10: Calls F_DoSlideShow
//
void F_Ticker (void)
{
    size_t          i;

    // check for skipping
    if (finalecount > 50) // [STRIFE] No commercial check
    {
        // go on to the next level
	if(gamemap != 34)						// ADDED FOR THE WII PORT
	{								// ADDED FOR THE WII PORT
            for (i=0 ; i<MAXPLAYERS ; i++)
		if (players[i].cmd.buttons)
		    break;

	    if (i < MAXPLAYERS)
		finalecount = slideshow_tics; // [STRIFE]
	}								// ADDED FOR THE WII PORT
    }

    if(slideshow_state > 25)							// ADDED FOR DEMO
    {										// ADDED FOR DEMO
	if(finalecount == 15)							// ADDED FOR DEMO
	    S_StartSound(NULL, sfx_mislht);					// ADDED FOR DEMO
    }										// ADDED FOR DEMO
    // advance animation
    finalecount++;

    if (finalestage == F_STAGE_CAST)
        F_CastTicker ();
    else if(finalecount > slideshow_tics) // [STRIFE] Advance slideshow
        F_DoSlideShow();

    // [STRIFE]: Rest is unused
    /*
    if ( gamemode == commercial)
        return;

    if (finalestage == F_STAGE_TEXT
        && finalecount>strlen (finaletext)*TEXTSPEED + TEXTWAIT)
    {
        finalecount = 0;
        finalestage = F_STAGE_ARTSCREEN;
        wipegamestate = -1;		// force a wipe
        if (gameepisode == 3)
            S_StartMusic (mus_logo);
    }
    */
}

// haleyjd 09/13/10: Not present in Strife: Cast drawing functions

#include "hu_stuff.h"
extern	patch_t *hu_font[HU_FONTSIZE];


//
// F_TextWrite
//
void F_TextWrite (void)							// REACTIVATED FOR DEMO
{
/*
    byte*	src;							// MODIFIED FOR DEMO
    byte*	dest;							// MODIFIED FOR DEMO
*/
    patch_t *patch;
    
    int		/*x,y,*/w;						// MODIFIED FOR DEMO
    signed int	count;
    char*	ch;
    int		c;
    int		cx;
    int		cy;
    
    // erase the entire screen to a tiled background
/*
    src = W_CacheLumpName ( finaleflat , PU_CACHE);			// MODIFIED FOR DEMO
    dest = I_VideoBuffer;						// MODIFIED FOR DEMO

    for (y=0 ; y<SCREENHEIGHT ; y++)					// MODIFIED FOR DEMO
    {									// MODIFIED FOR DEMO
	for (x=0 ; x<SCREENWIDTH/64 ; x++)				// MODIFIED FOR DEMO
	{								// MODIFIED FOR DEMO
	    memcpy (dest, src+((y&63)<<6), 64);				// MODIFIED FOR DEMO
	    dest += 64;							// MODIFIED FOR DEMO
	}								// MODIFIED FOR DEMO
	if (SCREENWIDTH&63)						// MODIFIED FOR DEMO
	{								// MODIFIED FOR DEMO
	    memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63);		// MODIFIED FOR DEMO
	    dest += (SCREENWIDTH&63);					// MODIFIED FOR DEMO
	}								// MODIFIED FOR DEMO
    }									// MODIFIED FOR DEMO
*/
    patch = (patch_t *)W_CacheLumpName(finaleflat, PU_CACHE);		// ADDED FOR DEMO
    V_DrawPatch (0, 0, patch);						// ADDED FOR DEMO

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);			// UNSURE... DUNNO... (??) :-/

    // draw some of the text onto the screen
    cx = 10;
    cy = 10;
    ch = finaletext;

    count = ((signed int) finalecount - 10) / TEXTSPEED;
    if (count < 0)
	count = 0;
    for ( ; count ; count-- )
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = 10;
	    cy += 11;
	    continue;
	}

	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}

	w = SHORT (hu_font[c]->width);
//	if (cx+w > SCREENWIDTH)					// (ORIGINAL)
	if (cx+w > ORIGWIDTH)					// CHANGED FOR HIRES
	    break;
	V_DrawPatch(cx, cy, hu_font[c]);
	cx+=w;
    }
}

// CAST ROUTINE TAKEN FROM STRIFE: VETERAN EDITION
//
// Final DOOM 2 animation
// Casting by id Software.
//   in order of appearance
//
typedef struct
{
    int         isindemo; // [STRIFE] Changed from name, which is in mobjinfo
    mobjtype_t  type;
    char       *name;
} castinfo_t;

// haleyjd: [STRIFE] A new cast order was defined, however it is unused in any
// of the released versions of Strife, even including the demo version :(
castinfo_t      castorder[] = 
{
    { 1, MT_PLAYER,     "THE MERCENARY"  },
    { 1, MT_BEGGAR1,    "BEGGAR"         },
    { 1, MT_PEASANT2_A, "PEASANT"        },
    { 1, MT_REBEL1,     "REBEL"          },
    { 1, MT_RLEADER2,   "MACIL"          },
    { 1, MT_GUARD1,     "ACOLYTE"        },
    { 1, MT_CRUSADER,   "CRUSADER"       },
    { 0, MT_SENTINEL,   "SENTINEL"       },
    { 0, MT_STALKER,    "STALKER"        },
    { 0, MT_REAVER,     "REAVER"         },
    { 0, MT_PGUARD,     "TEMPLAR"        },
    { 0, MT_INQUISITOR, "INQUISITOR"     },
    { 0, MT_PROGRAMMER, "THE PROGRAMMER" },
    { 0, MT_BISHOP,     "THE BISHOP"     },
    { 0, MT_PRIEST,     "THE LOREMASTER" },
    { 0, MT_SPECTRE_A,  "SPECTRE"        },
    { 0, MT_SUBENTITY,  "THE ONE GOD???" },
    { 1, NUMMOBJTYPES  }
};

int		castnum;
int		casttics;
state_t*	caststate;
boolean		castdeath;
int		castframes;
int		castonmelee;
boolean		castattacking;

//
// F_StartCast
//
// haleyjd   09/13/10: [STRIFE] Heavily modified, yet unused...
// nitr8  [2014/12/30] ...until now.
// Evidence suggests this was meant to be started from a menu item.
// See m_menu.c for more info.
//
void F_StartCast (void)
{
    usergame = false;
    gameaction = ga_nothing;
    viewactive = false;
    automapactive = false;
    castnum = 0;
    gamestate = GS_FINALE;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = caststate->tics;
    if(casttics > 50)
        casttics = 50;
    wipegamestate = -1;             // force a screen wipe
    castdeath = false;
    finalestage = F_STAGE_CAST;
    castframes = 0;
    castonmelee = 0;
    castattacking = false;
    S_ChangeMusic(mus_action, 1);
    I_StartVoice(NULL); // sorry Macil, be quiet for a sec :P
}

//
// haleyjd 20141116: [SVE] Don't go to weird gib states during the cast call.
//
static boolean F_IsWeirdGibState(int nextstate)
{
    switch(nextstate)
    {
    case S_RGIB_07: // MT_PLAYER
    case S_GIBS_08: // MT_PEASANT2_A
    case S_GIBS_20: // MT_GUARD1
        return true;
    default:
        return false;
    }
}

//
// F_CastTicker
//
// [STRIFE] Heavily modified, but unused...
// nitr8  [2014/12/30] ...until now.
// haleyjd   09/13/10: Yeah, I bothered translating this even though it isn't
// going to be seen, in part because I hope some Strife port or another will
// pick it up and finish it, adding it as the optional menu item it was 
// meant to be, or just adding it as part of the ending sequence.
//
void F_CastTicker (void)
{
    int st;
    int type;

    if (--casttics > 0)
        return;                  // not time to change state yet

    if (caststate->tics == -1 || caststate->nextstate == S_NULL ||
        F_IsWeirdGibState(caststate->nextstate))
    {
        // switch from deathstate to next monster
        castnum++;
        castdeath = false;
        if (isdemoversion)
        {
            // [STRIFE] Demo version had a shorter cast
            if(!castorder[castnum].isindemo)
                castnum = 0;
        }
        // [STRIFE] Break on type == NUMMOBJTYPES rather than name == NULL
        if(castorder[castnum].type == NUMMOBJTYPES)
            castnum = 0;
        type = castorder[castnum].type;
        if(mobjinfo[type].seesound && type != MT_PEASANT2_A && 
            type != MT_REBEL1 && type != MT_RLEADER2)
        {
            S_StartSound(NULL, mobjinfo[castorder[castnum].type].seesound);
        }
        st = mobjinfo[castorder[castnum].type].seestate;

        // see state hacks
        switch(st)
        {
        case S_SPID_03: st = S_SPID_18; break;
        default:
            break;
        }
        caststate = &states[st];
        castframes = 0;
    }
    else
    {
        int sfx = 0;

        // just advance to next state in animation
        if (caststate == &states[S_PLAY_05])    // villsa [STRIFE] - updated
            goto stopattack;	// Oh, gross hack!
        st = caststate->nextstate;

        // next state hacks
        switch(st)
        {
        case S_SPID_03: st = S_SPID_18; break;
        case S_ROB2_11:
            if(castonmelee)
                st = S_ROB2_01;
            break;
        default:
            break;
        }

        caststate = &states[st];
        castframes++;

        if (st != mobjinfo[castorder[castnum].type].meleestate &&
            st != mobjinfo[castorder[castnum].type].missilestate)
        {
            if (st == S_PLAY_05)
                sfx = sfx_rifle;
            else
                sfx = 0;
        }
        else
            sfx = mobjinfo[castorder[castnum].type].attacksound;

        // sound hacks
        switch(st)
        {
        case S_PEAS_10: sfx = sfx_meatht; break;
        case S_HMN1_20: 
        case S_HMN1_21: 
        case S_LEAD_13:
        case S_LEAD_18:
        case S_AGRD_18:
        case S_AGRD_19:
        case S_AGRD_20: sfx = sfx_rifle;  break;
        case S_ROB1_11: sfx = sfx_revbld; break;
        case S_ROB1_14: sfx = sfx_reavat; break;
        case S_PGRD_13: sfx = sfx_revbld; break;
        case S_PGRD_15: sfx = sfx_pgrdat; break;
        case S_ROB2_10:
            if(castonmelee)
            {
                sfx = sfx_rlaunc;
                break;
            }
            // fall-through
        case S_ROB2_11:
        case S_ROB2_12:
        case S_ROB2_13:
        case S_ROB2_14:
        case S_ROB2_15:
        case S_ROB2_16:
        case S_ROB2_17: sfx = sfx_flburn; break;
        case S_MLDR_10: sfx = sfx_rlaunc; break;
        case S_PRST_11: sfx = sfx_revbld; break;
        case S_PRST_14: sfx = sfx_chain;  break;
        case S_ALN1_14: sfx = sfx_revbld; break;
        case S_ALN1_17: sfx = sfx_sglhit; break;
        case S_MNAL_35: sfx = sfx_revbld; break;
        case S_MNAL_38: sfx = sfx_sglhit; break;
        case S_SEWR_04: sfx = sfx_plasma; break;
        case S_SPID_10: sfx = sfx_spdatk; break;
        case S_SPID_18:
        case S_SPID_22: sfx = sfx_spdwlk; break;
        case S_ROB3_02:
        case S_ROB3_08: sfx = sfx_inqact; break;
        case S_ROB3_12: sfx = sfx_reavat; break;
        case S_ROB3_15: sfx = sfx_phoot;  break;
        case S_PRGR_03: sfx = sfx_progac; break;
        case S_PRGR_13: sfx = sfx_revbld; break;
        case S_PRGR_17: sfx = sfx_sglhit; break;
        default:
            break;
        }

        if (sfx)
            S_StartSound (NULL, sfx);
    }

    if (!castdeath && castframes == 12)
    {
        // go into attack frame
        castattacking = true;
        if (castonmelee)
        {
            if(castorder[castnum].type == MT_INQUISITOR)
                caststate = &states[S_ROB3_14];
            else
                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
        }
        else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        castonmelee ^= 1;
        if (caststate == &states[S_NULL])
        {
            if (castonmelee)
                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
                caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
        }
    }

    if (castattacking)
    {
        if (castframes == 24
            ||	caststate == &states[mobjinfo[castorder[castnum].type].seestate] )
        {
stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder[castnum].type].seestate];
            if(caststate == &states[S_SPID_03])
                caststate = &states[S_SPID_18];
            if(caststate == &states[S_PRGR_02])
                caststate = &states[S_PRGR_06];
        }
    }

    if(caststate == &states[S_MLDR_27])
        casttics = 30;
    else
        casttics = caststate->tics;
    if (casttics > 50) // [STRIFE] Cap tics
        casttics = 50;
    else if (casttics == -1)
        casttics = 15;
}


//
// F_CastResponder
//
// [STRIFE] This still exists in Strife but is never used.
// It was used at some point in development, however, as they made
// numerous modifications to the cast call system.
//
boolean F_CastResponder (event_t* ev)
{
//    if (ev->type != ev_keydown)
    if (ev->type == ev_joystick) 
    {
	if ((ev->data1 & 1) == 0) 
	    return false;
    }

    if (castdeath)
        return true;                    // already in dying frames

    // go into death frame
    castdeath = true;
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
    casttics = caststate->tics;
    if(casttics > 50) // [STRIFE] Upper bound on casttics
        casttics = 50;
    castframes = 0;
    castattacking = false;
    if (mobjinfo[castorder[castnum].type].deathsound)
        S_StartSound (NULL, mobjinfo[castorder[castnum].type].deathsound);

    return true;
}

//
// F_CastPrint
//
// [STRIFE] Verified unmodified, and unused...
// nitr8  [2014/12/30] ...until now.
//
void F_CastPrint (char* text)
{
    char*	ch;
    int		c;
    int		cx;
    int		w;
    int		width;
    
    // find width
    ch = text;
    width = 0;
	
    while (ch)
    {
	c = *ch++;
	if (!c)
	    break;
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    width += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	width += w;
    }
    
    // draw it
    cx = 160-width/2;
    ch = text;
    while (ch)
    {
	c = *ch++;
	if (!c)
	    break;
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	V_DrawPatch(cx, 180, hu_font[c]);
	cx+=w;
    }
	
}

// haleyjd 09/13/10: [STRIFE] Unfortunately they removed whatever was
// partway finished of this function from the binary, as there is no
// trace of it. This means we cannot know for sure what the cast call
// would have looked like. :(

//
// F_CastDrawer
//
void F_CastDrawer (void)
{
    spritedef_t   *sprdef;
    spriteframe_t *sprframe;
    int            lump;
    boolean        flip;
    patch_t       *patch;
    int           x = 160, y = 170;
    
    // erase the entire screen to a background
    V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP0"), PU_CACHE));

    // draw the current frame in the middle of the screen
    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    if(castorder[castnum].type == MT_SUBENTITY && !castdeath)
        y = 196;

    patch = W_CacheLumpNum(lump+firstspritelump, PU_CACHE);
    if(flip)
        V_DrawPatchFlipped(x, y, patch);
    else
        V_DrawPatch(x, y, patch);

    // title
    V_WriteBigText("Cast of Characters", 
                   160 - V_BigFontStringWidth("Cast of Characters")/2, 8);

    // name
    F_CastPrint(DEH_String(castorder[castnum].name));
}

// haleyjd   09/13/10: [STRIFE] Unused...
// nitr8  [2014/12/30] ...until now.

static void F_ArtScreenDrawer(void)					// REACTIVATED FOR DEMO
{
/*
    char *lumpname;							// MODIFIED FOR DEMO
    
    if (gameepisode == 3)						// MODIFIED FOR DEMO
    {									// MODIFIED FOR DEMO
*/
	if(finalecount == 15)						// ADDED FOR DEMO
	    S_StartSound(NULL, sfx_mislht);				// ADDED FOR DEMO

        F_BunnyScroll();
/*
    }									// MODIFIED FOR DEMO
    else								// MODIFIED FOR DEMO
    {									// MODIFIED FOR DEMO
        switch (gameepisode)						// MODIFIED FOR DEMO
        {								// MODIFIED FOR DEMO
            case 1:							// MODIFIED FOR DEMO
                if (gamemode == retail)					// MODIFIED FOR DEMO
                {							// MODIFIED FOR DEMO
                    lumpname = "CREDIT";				// MODIFIED FOR DEMO
                }							// MODIFIED FOR DEMO
                else							// MODIFIED FOR DEMO
                {							// MODIFIED FOR DEMO
                    lumpname = "HELP2";					// MODIFIED FOR DEMO
                }							// MODIFIED FOR DEMO
                break;							// MODIFIED FOR DEMO
            case 2:							// MODIFIED FOR DEMO
                lumpname = "VICTORY2";					// MODIFIED FOR DEMO
                break;							// MODIFIED FOR DEMO
            case 4:							// MODIFIED FOR DEMO
                lumpname = "ENDPIC";					// MODIFIED FOR DEMO
                break;							// MODIFIED FOR DEMO
            default:							// MODIFIED FOR DEMO
                return;							// MODIFIED FOR DEMO
        }								// MODIFIED FOR DEMO

        lumpname = DEH_String(lumpname);				// MODIFIED FOR DEMO

        V_DrawPatch (0, 0, W_CacheLumpName(lumpname, PU_CACHE));	// MODIFIED FOR DEMO
    }									// MODIFIED FOR DEMO
*/
}


//
// F_Drawer
//
// [STRIFE]
// haleyjd 09/13/10: Modified for slideshow, demo version, etc.
//
void F_Drawer (void)
{
    switch (finalestage)
    {
    case F_STAGE_CAST:
        // Cast didn't have a drawer in any released version
        F_CastDrawer();
        break;
    case F_STAGE_TEXT:
        // Draw slideshow panel
	if(gamemap == 34)						// ADDED FOR DEMO
            F_TextWrite();						// ADDED FOR DEMO
	else								// ADDED FOR DEMO
        {
            patch_t *slide =
		    W_CacheLumpName(slideshow_panel, PU_CACHE);
            V_DrawPatch(0, 0, slide);
        }
        break;
    case F_STAGE_ARTSCREEN:
        if(gamemap <= 29)
        {
            // draw credits
            patch_t *credits =
		    W_CacheLumpName(DEH_String("CREDIT"), PU_CACHE);
            V_DrawPatch(0, 0, credits);
        }
        else if(gamemap == 34 && !scroll_finished)			// MODIFIED FOR DEMO
            // demo version - does nothing meaningful
	    // in the final version
            F_ArtScreenDrawer();					// ADDED FOR DEMO
//            F_DrawMap34End();						// MODIFIED FOR DEMO
	else if(scroll_finished)					// ADDED FOR DEMO
	{								// ADDED FOR DEMO
            patch_t *vellogo =						// ADDED FOR DEMO
		    W_CacheLumpName(DEH_String("VELLOGO"), PU_CACHE);	// ADDED FOR DEMO
            V_DrawPatch(0, 0, vellogo);					// ADDED FOR DEMO
	}								// ADDED FOR DEMO
        break;
    }
}


/*
#ifdef STRIFE_DEMO_CODE
//
// F_DrawPatchCol
//
// [STRIFE] Verified unmodified, but not present in 1.2
// It WAS present in the demo version, however...
//
void
F_DrawPatchCol
( int		x,
  patch_t*	patch,
  int		col )
{
    column_t*	column;
    byte*	source;
    byte*	dest;
    byte*	desttop;
    int		count;
	
    column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));
    desttop = screens[0] + x;

    // step through the posts in a column
    while (column->topdelta != 0xff )
    {
	source = (byte *)column + 3;
	dest = desttop + column->topdelta*SCREENWIDTH;
	count = column->length;
		
	while (count--)
	{
	    *dest = *source++;
	    dest += SCREENWIDTH;
	}
	column = (column_t *)(  (byte *)column + column->length + 4 );
    }
}

//
// F_DrawMap34End
//
// [STRIFE] Modified from F_BunnyScroll
// * In 1.2 and up this just causes a weird black screen.
// * In the demo version, it was an actual scroll between two screens.
// I have implemented both code segments, though only the black screen
// one will currently be used, as full demo version support isn't looking
// likely right now.
//

void F_DrawMap34End (void)
{
    signed int  scrolled;
    int         x;
    patch_t*    p1;
    patch_t*    p2;

    p1 = W_CacheLumpName (DEH_String("credit"),  PU_LEVEL);
    p2 = W_CacheLumpName (DEH_String("vellogo"), PU_LEVEL);

    V_MarkRect (0, 0, SCREENWIDTH, SCREENHEIGHT);


    scrolled = (320 - ((signed int) finalecount-430)/2);
    if (scrolled > 320)
        scrolled = 320;
    if (scrolled < 0)
        scrolled = 0;

//#ifdef STRIFE_DEMO_CODE
    for ( x=0 ; x<SCREENWIDTH ; x++)
    {
        if (x+scrolled < 320)
            F_DrawPatchCol (x, p1, x+scrolled);
        else
            F_DrawPatchCol (x, p2, x+scrolled - 320);
    }
//#else
    // wtf this is supposed to do, I have no idea!
    x = 1;
    do
    {
        x += 11;
    }
    while(x < 320);
//#endif
}
#endif
*/
