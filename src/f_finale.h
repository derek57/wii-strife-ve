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
//
//    


#ifndef __F_FINALE__
#define __F_FINALE__


#include "doomtype.h"
#include "d_event.h"
//
// FINALE
//

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

// Called by main loop.
boolean F_Responder (event_t* ev);

// Called by main loop.
void F_Ticker (void);

// haleyjd: [STRIFE] Called from G_Ticker as well...
void F_WaitTicker(void);

// Called by main loop.
void F_Drawer (void);


void F_StartFinale (void);




#endif
