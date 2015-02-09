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
//	Map Objects, MObj, definition and handling.
//


#ifndef __P_MOBJ__
#define __P_MOBJ__

// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

// [SVE]: Capture the Chalice teams
#define CTC_TEAM_BLUE 8
#define CTC_TEAM_RED  9


// haleyjd 20140902: [SVE]
// prevpos_t represents an Mobj or camera's previous position for purposes of
// frame interpolation in the renderer. - haleyjd 01/04/14
typedef struct prevpos_s
{
   fixed_t x;
   fixed_t y;
   fixed_t z;
   angle_t angle;
} prevpos_t;



//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.  
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL          = 1,

    // Blocks.
    MF_SOLID            = 2,

    // Can be hit.
    MF_SHOOTABLE        = 4,

    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR         = 8,

    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP       = 16,

    // villsa [STRIFE] Stand around until alerted
    MF_STAND            = 32,

    // Will try to attack right back.
    MF_JUSTHIT          = 64,

    // Will take at least one step before attacking.
    MF_JUSTATTACKED     = 128,

    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING     = 256,

    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY        = 512,

    // Movement flags.
    // This allows jumps from high places.
    MF_DROPOFF          = 0x400,

    // villsa [STRIFE] For players, count as quest item
    MF_GIVEQUEST        = 0x800,

    // Player cheat. ???
    MF_NOCLIP           = 0x1000,

    // villsa [STRIFE] are feet clipped into water/slude floor?
    MF_FEETCLIPPED      = 0x2000,

    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT            = 0x4000,

    // villsa [STRIFE] can NPC talk?
    MF_NODIALOG         = 0x8000,

    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE          = 0x10000,

    // Dropped by a demon, not level spawned.
    // E.g. ammo clips dropped by dying former humans.
    MF_DROPPED          = 0x20000,

    // Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    MF_SHADOW           = 0x40000,

    // Flag: don't bleed when shot (use puff),
    //  barrels and shootable furniture shall not bleed.
    MF_NOBLOOD          = 0x80000,

    // Don't stop moving halfway off a step,
    //  that is, have dead bodies slide down all the way.
    MF_CORPSE           = 0x100000,

    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT          = 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL        = 0x400000,

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH           = 0x800000,

    // villsa [STRIFE] flag used for bouncing projectiles
    MF_BOUNCE           = 0x1000000,

    // Don't spawn this object
    //  in death match mode (e.g. key cards).
    MF_NOTDMATCH        = 0x2000000,

    // villsa [STRIFE] friendly towards player with matching flag
    MF_ALLY             = 0x4000000,

    // villsa [STRIFE] 75% or 25% transparency? -- NEEDS VERIFICATION
    MF_MVIS             = 0x8000000,

    // villsa [STRIFE] color translation
    MF_COLORSWAP1       = 0x10000000,

    // villsa [STRIFE] color translation
    MF_COLORSWAP2       = 0x20000000,

    // villsa [STRIFE] color translation
    MF_COLORSWAP3       = 0x40000000,

    // villsa [STRIFE] spectral entity, only damaged by spectral missiles
    MF_SPECTRAL         = 0x80000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // haleyjd 09/06/10: redid for Strife translations
    MF_TRANSLATION      = (MF_COLORSWAP1|MF_COLORSWAP2|MF_COLORSWAP3),

    // Turns 0x10000000 into 0x01 to get a translation index.
    // villsa [STRIFE] change from 26 to 28
    MF_TRANSSHIFT       = 28

} mobjflag_t;

// [SVE] svillarreal - macro for getting the translation index
#define MOBJTRANSLATION(mo) (((mo->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT - 8)) >> 8)

// haleyjd 20140818: [SVE] flags2 enumeration
// [SVE] svillarreal - updated 20141109
typedef enum
{
    MF2_NOSTONECOLD         = 0x00000001,   // not killed by STONECOLD cheat
    MF2_DRAWBILLBOARD       = 0x00000002,   // render sprite as a billboard
    MF2_DRAWOUTLINE         = 0x00000004,   // render outline around sprite
    MF2_NOTHINGBLOCK        = 0x00000008,   // does not collide with other mobjs
    MF2_MARKDECAL           = 0x00000010,   // leave a decal in some way
    MF2_NOREBELATTACK       = 0x00000020,   // rebels shouldn't attack w/o provocation
    MF2_IGNORENOMONSTERS    = 0x00000040,   // will always spawn even if -nomonsters is set
    MF2_MIRRORED            = 0x00002000,   // Mirrored horizontally
    MF2_NOFOOTCLIP          = 0x00040000,   // Object's feet won't be clipped in liquid
    MF2_BLOOD               = 0x00400000,   // Object is blood
    MF2_DRAWFIRST           = 0x00800000,   // Object is drawn first
    MF2_DONOTMAP            = 0x01000000    // Object's thing triangle is not displayed in automap

} mobjflag2_t;

typedef unsigned long long uint_64_t;			// ADDED FOR PSP-STATS
// On picking up, count this item object
//  towards intermission item total.
#define MF_COUNTITEM    (uint_64_t)(0x0000000000800000)

// Map Object definition.
//
// [STRIFE]: Amazingly, only one modification was made to mobj_t over DOOM 
// 1.666, and that was the addition of the single-byte allegiance field for
// tracking with which player friendly monsters are allied.
//
typedef struct mobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    // Info for drawing: position.
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    // More list: links in sector (if needed)
    struct mobj_s*      snext;
    struct mobj_s*      sprev;

    //More drawing info: to determine current sprite.
    angle_t             angle;  // orientation
    spritenum_t         sprite; // used to find patch_t and flip value
    int                 frame;  // might be ORed with FF_FULLBRIGHT

    // haleyjd 20140902: [SVE] interpolation data
    prevpos_t           prevpos;

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s*      bnext;
    struct mobj_s*      bprev;
    
    struct subsector_s* subsector;

    // The closest interval over all contacted Sectors.
    fixed_t             floorz;
    fixed_t             ceilingz;

    // For movement checking.
    fixed_t             radius;
    fixed_t             height;

    // Momentums, used to update position.
    fixed_t             momx;
    fixed_t             momy;
    fixed_t             momz;

    // If == validcount, already checked.
    int                 validcount;

    mobjtype_t          type;
    mobjinfo_t*         info;   // &mobjinfo[mobj->type]
    
    int                 tics;   // state tic counter
    state_t*            state;
    int                 flags;
    int                 health;

    // Movement direction, movement generation (zig-zagging).
    int                 movedir;        // 0-7
    int                 movecount;      // when 0, select a new dir

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    struct mobj_s*      target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int                 reactiontime;   

    // If >0, the target will be chased
    // no matter what (even if shot)
    int                 threshold;

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s*    player;

    // Player number last looked for.
    int                 lastlook;

    // For nightmare respawn.
    mapthing_t          spawnpoint;

    // Thing being chased/attacked for tracers.
    struct mobj_s*      tracer;

    // [STRIFE] haleyjd 09/05/10: 
    // * In multiplayer this stores allegiance, for friends and teleport beacons
    // * In single-player this tracks dialog state.
    byte                miscdata;
    
    void                (*colfunc)(void);

    int                 bloodsplats;

    int                 blood;

} mobj_t;

// haleyjd [STRIFE] Exported
void P_CheckMissileSpawn (mobj_t* th);

// [SVE] interpolation
void P_MobjBackupPosition(mobj_t *mo);

// [SVE] referential integrity
void P_SetTarget(mobj_t **mop, mobj_t *target);

#define FUZZYBLOOD              -1
#define CORPSEBLOODSPLATS       512

//
// P_NullMobjThinker
//
void P_NullMobjThinker(mobj_t *mobj);

//
// P_SpawnBloodSplat
//
void P_SpawnBloodSplat(fixed_t x, fixed_t y, int blood, int maxheight);
void P_SpawnBloodSplat2(fixed_t x, fixed_t y, int blood, int maxheight);
void P_NullBloodSplatSpawner(fixed_t x, fixed_t y, int blood, int maxheight);

#endif
