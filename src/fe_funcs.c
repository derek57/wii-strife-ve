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

// Characters for exit prompt
static fecharacter_t fecharacters[] =
{
    { "Rowan", "M_RETRIC", "RETBL",  "Come back again and I'll have you killed, okay?" },
    { "Quincy", "M_QUINCY", "QUTBL",  "Don't ya know the meaning of the words, \"Get lost?\"" },
    { "Richter", "M_RICHTR", "RRTBL",  "You're wasting time and lives. Move!" },
    { "Acolyte", "M_GUARD4", "RGTBL", "Move along, or taste metal." },
    { "Sammis", "M_SAMMIS", "SAM05A", 
      "If it's busy work you want, go stare at that screen for a while. It'll bore you to tears." },
    { "Technician", "M_TEKBRN", "TBTBL",  "If you don't get outta my face, maybe I call the guards, huh?" },
    { "Weran", "M_WERAN", "WNTBL", "You give me nothing, you get nothing!" },
    { "Door Guard", "M_GUARD2", "DGG02",  "Oh, okay. Sure, go ahead. Have a nice day." },
    { "Worner", "M_WORNER", "WRTBL", "Get out of here, unless you want to end up Mr. Dead." },
    { "Harris", "M_PEAS1",  "HATBL",  "Get lost kid, ya bother me." },
    { "Judge Wolenick", "M_JUDGE",  "JWTBL",  "Move along, or join your friends." },
    { "Ketrick", "M_KETRIC", "KETBL",  "My bad mood just got worse. Be gone!" },
    { "Keymaster", "M_KEVIN",  "KEV02",  "Okay, but remember, I'm the Keymaster!" },
    { "False Programmer", "M_FALSP3", "F3TBLA", "I told you all I know. You're wasting your time." },
    { "Macil", "M_LEDR0", "MAC03",  
      "You might want to reconsider, seeing that you're surrounded by heavily armed, angry rebels." },
    { "MacGuffin", "M_MCGUFF", "MCTBL",  "Release me! Leave an old man alone." },
    { "Technician", "M_TEKBRN", "TCH03",  "Are you deaf? I just told you how busy I am. Get back to work!" },
    { "Warden Montag", "M_WARDEN", "MOTBL",  "Shackles, or chains? I want you to hang around!" },
    { "False Programmer", "M_FALSP1", "F1TBLA", "Look, if you stay here, you'll be as dead as I am." },
    { "Door Guard", "M_GUARD2", "DGTBL", "A peasant's death rattle is a lovely sound, don't you think?" },
    { "Derwin", "M_DERWIN", "DER02",  "Nuts! If I'm going down then so are you. GUARDS!" },
    { "Governor Mourel", "M_PEAS7",  "GOTBL",  "Give you a hint. When I stop talking to you, you leave." },
    { "Oracle", "M_ORACLE", "ORC04",  
      "Whatever choice you make, your kind shall perish under the will of the One God." },
    { "Rowan", "M_RETRIC", "RET06", "Get going! If you hang around here, we're both dead." },
    { "Macil", "M_LEDR0",  "MLTBL",  "Fight for the Front and Freedom. Move out!" },
    { "Overseer", "M_GUARD5", "AGG01",  "Get back to work, now!" },
    { "Governor Mourel", "M_PEAS7",  "GOV10",  
      "I've wasted enough time with you, all pumped up and nowhere to go. Story of my life." },
    { "Geoff", "M_GEOFF",  "GETBL",  "Walk away, boy. Just walk away." },
    { "Foreman", "M_GUARD3", "FOTBLA", "You are an unpleasant distraction." },
    { "False Programmer", "M_FALSP2", "F2TBLA", "You'll never find anything if you hang around here." },
    { "Oracle", "M_ORACLE", "ORTBL",  "The river of time moves forever onward, while you stand still." }
};

static int frontend_curchar;  // current character #

//
// Get the next character in the progression.
//
fecharacter_t *FE_GetCharacter(void)
{
    int pic = frontend_curchar++;
    frontend_curchar %= arrlen(fecharacters);
    return &fecharacters[pic];
}

// Pointer to currently selected character.
fecharacter_t *curCharacter;

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

//
// Write a centered string in the HUD/menu font.
//
void FE_WriteSmallTextCentered(int y, /*const*/ char *str)
{
    M_WriteText(160 - M_StringWidth(str)/2, y, str);
}

//
// Draw character quit prompt
//
void FE_DrawChar(void)
{
    char msg[128];

    if(!curCharacter)
        return;

    V_DrawPatch(0, 0, W_CacheLumpName(curCharacter->pic, PU_CACHE));
    M_WriteText(12, 18, curCharacter->name);

    M_StringCopy(msg, curCharacter->text, sizeof(msg));
    M_DialogDimMsg(20, 28, msg, false);
    M_WriteText(20, 28, msg);

    FE_WriteSmallTextCentered(160, "Are you sure you want to quit?");
    FE_WriteSmallTextCentered(172, "(Press Y or confirm to quit)");
}

