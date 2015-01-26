#ifndef __FE_FUNCS__
#define __FE_FUNCS__

#include <stdlib.h>

#include "doomstat.h"
#include "fe_funcs.h"
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#define FE_MERCHANT_X 296
#define FE_MERCHANT_Y 193

enum
{
    FE_BG_SIGIL,
    FE_BG_RSKULL,
    FE_BG_TSKULL,
    FE_NUM_BGS
};

//
// Transition the merchant into a specific state
//
void FE_MerchantSetState(int statenum);

//
// Per-tick logic for the merchant; state transitions and randomized animations
//
void FE_MerchantTick(void);

//
// Draw the merchant
//
void FE_DrawMerchant(int x, int y);

//
// Initialize the merchant
//
void FE_InitMerchant(void);

//
// Draw box
//
void FE_DrawBox(int left, int top, int w, int h);

//
// Render a background, using the hi-res version if available in the currently
// active rendering engine, and falling back to a patch otherwise.
//
void FE_DrawBackground(int bgnum);

//
// Clear the screen to black.
//
void FE_ClearScreen(void);

#endif    
