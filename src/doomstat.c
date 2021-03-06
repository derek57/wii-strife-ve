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
//	Put all global tate variables here.
//

#include <stdio.h>

#include "doomstat.h"


// Game Mode - identify IWAD as shareware, retail etc.
GameMode_t gamemode = indetermined;
GameMission_t	gamemission = doom;
GameVersion_t   gameversion = exe_strife_1_31;
char *gamedescription;

// Set if homebrew PWAD stuff has been added.
//boolean	modifiedgame;

// [SVE] haleyjd: HUD toggle
boolean fullscreenhud = true;

// [SVE] for those Brutal Doom fans...			// FIXME: FOR WII NOT YET FULLY INCLUDED
boolean d_maxgore = true;				// (WALLSPLATS & FLOORSPLATS MISSING -
							// MAYBE TAKE THE CODE FROM DOOM LEGACY???)
// haleyjd 20140816: [SVE] Classic mode toggle
// * true  == behave like vanilla Strife as much as is practical
// * false == fix non-critical bugs and enable new gameplay elements
boolean classicmode = false;

// [SVE] svillarreal - Skip intro movies?
boolean d_skipmovies = false;				// FIXME: FOR WII NOT YET INCLUDED (SMPEG BUG)

// [SVE] svillarreal - player recoil bobbing
boolean d_recoil = true;				// FIXME: PARTIAL (CUSTOM / NON-SVE) SUPPORT

// [SVE] autoaim toggle
int	autoaim = false;

boolean	respawnparm = false;    		// checkparm of -respawn
boolean	fastparm = false;       		// checkparm of -fast

// [SVE] damage indicator
boolean d_dmgindictor = false;				// FIXME: FOR WII NOT YET WORKING

boolean cast_running = false;
boolean release_keys = false;

boolean	wipe_drawer = false;
boolean	frontend_wipe = false;         // wipe wanted

boolean	game_loaded = false;

// [SVE] haleyjd: if true, a netgame played on the current map will be a
// Capture the Chalice game. This is triggered by presence of the appropriate
// starts on the map.
boolean capturethechalice = false;
int     ctcpointlimit     = 5;
int     ctcbluescore;
int     ctcredscore;

