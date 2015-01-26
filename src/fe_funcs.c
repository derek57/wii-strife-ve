#include <stdlib.h>

#include "z_zone.h"
#include "w_wad.h"
#include "v_video.h"
#include "m_menu.h"
#include "m_misc.h"
#include "i_video.h"
#include "fe_funcs.h"
#include "f_wipe.h"
#include "doomstat.h"

typedef struct febackground_s
{
    const char *lowname;
/*
    const char *highname;
    rbTexture_t texture;
*/
} febackground_t;

static febackground_t backgrounds[FE_NUM_BGS] =
{
    { "FESIGIL"/*,  "FHESIGIL"*/  },
    { "FERSKULL"/*, "FHERSKULL"*/ },
    { "FETSKULL"/*, "FHETSKULL"*/ }
};


//=============================================================================
//
// Merchant Assistant
//

static state_t *merchantState;
static int      merchantStateNum;
static int      merchantTics;

boolean 	merchantOn = true;
boolean		frontend_ingame;

//
// Transition the merchant into a specific state
//
void FE_MerchantSetState(int statenum)
{
    merchantStateNum = statenum;
    merchantState    = &states[merchantStateNum];
    merchantTics     = merchantState->tics * (frontend_ingame ? 2 : 3);
}

//
// Initialize the merchant
//
void FE_InitMerchant(void)
{
    FE_MerchantSetState(S_MRST_00);
}

//
// Per-tick logic for the merchant; state transitions and randomized animations
//
void FE_MerchantTick(void)
{
    if(!merchantOn)
        return;

    if(--merchantTics == 0)
    {
        if(merchantStateNum == S_MRST_00)
        {
            int r = abs(rand());
            merchantStateNum += r % 4;
            FE_MerchantSetState(merchantStateNum);
        }
        else
            FE_MerchantSetState(merchantState->nextstate);
    }
}

//
// Construct a merchant sprite lump name from the animation state
//
static void FE_SpriteLumpName(state_t *state, char name[9])
{
    M_snprintf(name, 9, "%s%c0", 
               sprnames[state->sprite], 'A' + (state->frame & FF_FRAMEMASK));
}

//
// Draw the merchant
//
void FE_DrawMerchant(int x, int y)
{
    char lumpname[9];

    if(!merchantOn)
        return;

    FE_SpriteLumpName(merchantState, lumpname);
    V_DrawPatch(x, y, W_CacheLumpName(lumpname, PU_CACHE));
}

//
// Draw box
//
void FE_DrawBox(int left, int top, int w, int h)
{
    int x, y;

    if(left + w > SCREENWIDTH || top + h > SCREENHEIGHT)
        return;

    if(w > SCREENWIDTH)
        w = SCREENWIDTH;
    if(h > 64)
        h = 64;

    for(x = left; x < ((left+w) << hires); x += 64)					// ADDED HIRES
        V_DrawBlock(x, top + 4, 64, h << hires, W_CacheLumpName("F_PAVE02", PU_CACHE));	// + 4 = BUGFIX

    for(x = left; x < left+w; x += 8)
    {
        V_DrawPatch(x, top-4,   W_CacheLumpName("BRDR_T", PU_CACHE));
        V_DrawPatch(x, top+h-4, W_CacheLumpName("BRDR_B", PU_CACHE));
    }
    
    for(y = top; y < top+h; y += 8)
    {
        V_DrawPatch(left-4,   y, W_CacheLumpName("BRDR_L", PU_CACHE));
        V_DrawPatch(left+w-4, y, W_CacheLumpName("BRDR_R", PU_CACHE));
    }
    V_DrawPatch(left-4,   top-4,   W_CacheLumpName("BRDR_TL", PU_CACHE));
    V_DrawPatch(left+w-4, top-4,   W_CacheLumpName("BRDR_TR", PU_CACHE));
    V_DrawPatch(left-4,   top+h-4, W_CacheLumpName("BRDR_BL", PU_CACHE));
    V_DrawPatch(left+w-4, top+h-4, W_CacheLumpName("BRDR_BR", PU_CACHE));
}

//
// Clear the screen to black.
//
void FE_ClearScreen(void)
{
    V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
}

//
// Render a background, using the hi-res version if available in the currently
// active rendering engine, and falling back to a patch otherwise.
//
void FE_DrawBackground(int bgnum)
{
    febackground_t *bg = &backgrounds[bgnum];
/*
    if(use3drenderer)
        FE_DrawTexture(&bg->texture);
    else
*/
    {
        int lumpnum;
        if((lumpnum = W_CheckNumForName((char*)bg->lowname)) >= 0)
            V_DrawPatch(0, 0, W_CacheLumpNum(lumpnum, PU_CACHE));
        else
            FE_ClearScreen();
    }
}

