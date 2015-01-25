// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//      Timer functions.
//
//-----------------------------------------------------------------------------

#include <SDL/SDL.h>

#include "i_timer.h"
#include "doomtype.h"

static unsigned int start_displaytime;
static unsigned int displaytime;

static unsigned int rendertic_start;
static unsigned int rendertic_next;

unsigned int        rendertic_step;
float               rendertic_msec;

const float realtic_clock_rate = 100.0f;

//
// I_GetTime
// returns time in 1/35th second tics
//

static Uint32 basetime = 0;

int  I_GetTime (void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    ticks -= basetime;

    return (ticks * TICRATE) / 1000;    
}

//
// Same as I_GetTime, but returns time in milliseconds
//

int I_GetTimeMS(void)
{
    Uint32 ticks;

    ticks = SDL_GetTicks();

    if (basetime == 0)
        basetime = ticks;

    return ticks - basetime;
}

// Sleep for a specified number of ms

void I_Sleep(int ms)
{
    SDL_Delay(ms);
}

void I_WaitVBL(int count)
{
    I_Sleep((count * 1000) / 70);
}

//
// I_TimerStartDisplay
//
// Calculate the starting display time.
//
void I_TimerStartDisplay(void)
{
    start_displaytime = SDL_GetTicks();
}

//
// I_TimerEndDisplay
//
// Calculate the ending display time.
//
void I_TimerEndDisplay(void)
{
    displaytime = SDL_GetTicks() - start_displaytime;
}

//
// I_TimerSaveMS
//
// Update interpolation state variables at the end of gamesim logic.
//

void I_TimerSaveMS(void)
{
    rendertic_start = SDL_GetTicks();
    rendertic_next  = (unsigned int)((rendertic_start * rendertic_msec + 1.0f) / rendertic_msec);
    rendertic_step  = rendertic_next - rendertic_start;
}

//
// I_setMSec
//
// Module private; set the milliseconds per render frame.
//
static void I_setMSec(void)
{
    rendertic_msec = realtic_clock_rate * TICRATE / 100000.0f;
}

void I_InitTimer(void)
{
    // initialize timer

    SDL_Init(SDL_INIT_TIMER);

    // haleyjd: init interpolation
    I_setMSec();
}

