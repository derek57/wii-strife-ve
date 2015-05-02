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
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"
#include "dstrings.h"

#include "d_main.h"
#include "deh_str.h"
#include "deh_misc.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"

#include "hu_stuff.h"

#include "g_game.h"

//#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_saves.h"    // [STRIFE]
#include "p_saveg.h"
#include "p_inter.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"
#include "p_dialog.h"

#include "doomfeatures.h"
#include "fe_funcs.h"
#include "hu_lib.h"
#include "f_wipe.h"
#include "f_finale.h"
#include "st_stuff.h"
/*
#include <psppower.h>
#include <psprtc.h>	// FOR PSP: FILE 'PSPRTC.H' HAS TO BE HERE AT THIS POSITION - HELL WHY ???
#include "pspsysmem_kernel.h"

extern void		M_QuitStrife(int choice);
*/
extern default_t	doom_defaults_list[];		// FOR PSP: KEY BINDINGS

extern patch_t*         hu_font[HU_FONTSIZE];
extern boolean          message_dontfuckwithme;

//extern boolean          chat_on;        // in heads-up code
extern boolean          sendsave;       // [STRIFE]

#ifdef SHAREWARE
extern boolean		STRIFE_1_0_SHAREWARE;
extern boolean		STRIFE_1_1_SHAREWARE;
#endif

extern boolean		STRIFE_1_0_REGISTERED;
extern boolean		STRIFE_1_X_REGISTERED;

extern boolean		display_ticker;
extern boolean		BorderNeedRefresh;
extern boolean		merchantOn;

extern int		cheating;
extern int		mspeed;
extern int		mouselook;

extern int		dots_enabled;
extern int		fps_enabled;
extern int		dont_show;
extern int		display_fps;
extern int		load_voices;
/*
extern int		allocated_ram_size;

extern u32		tickResolution;
extern u64		fpsTickLast;
extern u64		systemTime;
*/
static int		FirstKey = 0;		// FOR PSP: SPECIAL MENU FUNCTIONS (ITEMCOUNT) ;-)
static int		keyaskedfor;
static boolean		askforkey = false;

int			cheeting;
int			map_cheat = 0;
//int			spot = 0;
int			musnum = 1;
int			crosshair = 0;
int			button_layout = 0;
/*
int			mhz333 = 0;
int			frame_limit = 0;
int			memory_info = 0;
int			battery_info = 0;
int			cpu_info = 0;
int			other_info = 0;
*/
int			coordinates_info = 0;
int			timer_info = 0;
int			version_info = 0;
int			fps = 0;		// FOR PSP: calculating the frames per second

int			followplayer = 1;
int			drawgrid;
//int			show_stats = 0;
int			mus_engine = 1;
int			show_talk = 1;
int			messages_disabled = 0;
/*
						// (PSP): USED TO BE FOR SET CFG VARS 'TICKER' & 'FPS',
						// BUT CAUSES A CRASH AS SOON AS THE SCREEN WIPES
int			key_controls_start_in_cfg_at_pos = 19;	// FOR PSP: ACTUALLY IT'S +2 !!!
int			key_controls_end_in_cfg_at_pos = 32;	// FOR PSP: ACTUALLY IT'S +2 !!!
*/
int			key_controls_start_in_cfg_at_pos = 26;	// FOR PSP: ACTUALLY IT'S +2 !!!
int			key_controls_end_in_cfg_at_pos = 43;	// FOR PSP: ACTUALLY IT'S +2 !!!

boolean			am_rotate;
boolean			text_flag_boomstix = false;
boolean			text_flag_donnytrump = false;
boolean			text_flag_stonecold = false;
boolean			text_flag_pumpup = false;
boolean			text_flag_warping = false;
boolean			text_flag_end = false;
boolean			quitting = false;
/*
int			max_free_ram = 0;
char			allocated_ram_textbuffer[50];
char			free_ram_textbuffer[50];
char			max_free_ram_textbuffer[50];
char			fpsDisplay[100];
u64			fpsTickNow;
*/
char			map_coordinates_textbuffer[50];
//
// defaulted values
//
int			mouseSensitivity;

// [STRIFE]: removed this entirely
// Show messages has default, 0 = off, 1 = on
int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10; // [SVE]: default 10

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;
boolean                 messageLastMenuPause; // haleyjd [SVE]: forgotten by Rogue

int		turnspeed = 7;
fixed_t         forwardmove = 29;
fixed_t         sidemove = 21; 

// timed message = no input from user
boolean			messageNeedsInput;

void			(*messageRoutine)(int response);

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean                 inhelpscreens;
boolean                 menuactive;
boolean                 menupause;      // haleyjd 08/29/10: [STRIFE] New global
int                     menupausetime;  // haleyjd 09/04/10: [STRIFE] New global
boolean                 menuindialog;   // haleyjd 09/04/10: ditto
boolean                 music_cheat_used = false;

// haleyjd 08/27/10: [STRIFE] SKULLXOFF == -28, LINEHEIGHT == 19
#define CURSORXOFF		-28
#define LINEHEIGHT		19

#define CURSORXOFF_SMALL	-20
#define LINEHEIGHT_SMALL	10
#define FIRSTKEY_MAX		0

#define FIRSTKEYSETAMOUNT	16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char			endstring[160];

// haleyjd 09/04/10: [STRIFE] Moved menuitem / menu structures into header
// because they are needed externally by the dialog engine.

// haleyjd 08/27/10: [STRIFE] skull* stuff changed to cursor* stuff
short			itemOn;			// menu item skull is on
short			cursorAnimCounter;	// skull animation counter
short			whichCursor;		// which skull to draw

// graphic name of cursors
// haleyjd 08/27/10: [STRIFE] M_SKULL* -> M_CURS*
char    		*cursorName[8] = {"M_CURS1", "M_CURS2", "M_CURS3", "M_CURS4", 
                          		"M_CURS5", "M_CURS6", "M_CURS7", "M_CURS8" };

char    		*cursorNameSmall[8] = {"M_CURS1S", "M_CURS2S", "M_CURS3S", "M_CURS4S", 
                          		"M_CURS5S", "M_CURS6S", "M_CURS7S", "M_CURS8S" };

// haleyjd 20110210 [STRIFE]: skill level for menus
int			menuskill;
boolean			menuepisode = false; // [SVE]: allow starting demo maps from menus

// current menudef
menu_t*			currentMenu;

// haleyjd 03/01/13: [STRIFE] v1.31-only:
// Keeps track of whether the save game menu is being used to name a new
// character slot, or to just save the current game. In the v1.31 disassembly
// this was the new dword_8632C variable.
boolean			namingCharacter; 


//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_StartCast(int choice); // [SVE]
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_ReadThis3(int choice); // [STRIFE]
void M_QuitStrife(int choice);

//void M_ChangeMessages(int choice); [STRIFE]
void M_WalkingSpeed(int choice);
void M_TurningSpeed(int choice);
void M_StrafingSpeed(int choice);
//void M_ChangeDetail(int choice);

void M_GameFiles(int choice);
void M_Brightness(int choice);
void M_Freelook(int choice);
void M_FreelookSpeed(int choice);

//void M_KeyBindingsButtonLayout(int choice);
void M_KeyBindingsClearControls (int ch);
void M_KeyBindingsClearAll (int choice);
void M_KeyBindingsReset (int choice);
void M_KeyBindingsSetKey(int choice);
void M_KeyBindings(int choice);
/*
void M_CpuSpeed(int choice);
void M_CPU(int choice);
void M_FrameLimiter(int choice);
void M_Battery(int choice);
void M_Other(int choice);
void M_ShowMemory(int choice);
*/
void M_FPS(int choice);
void M_DisplayTicker(int choice);
void M_Coordinates(int choice);
void M_Version(int choice);
void M_MapGrid(int choice);
void M_MapRotation(int choice);
void M_WeaponChange(int choice);
void M_FollowMode(int choice);
//void M_Statistics(int choice);
void M_Messages(int choice);
void M_DialogText(int choice);
void M_Crosshair(int choice);

void M_HUD(int choice);			// [SVE]
void M_BloodGore(int choice);		// [SVE]
void M_ClassicMode(int choice);		// [SVE]
void M_WeaponRecoil(int choice);	// [SVE]
void M_Autoaim(int choice);		// [SVE]
void M_Spawn(int choice);		// [SVE]
void M_FastMonsters(int choice);	// [SVE]

void M_SfxVol(int choice);
void M_Voices(int choice);
void M_VoiceVol(int choice); // [STRIFE]
void M_MusicVol(int choice);
void M_Engine(int choice);
void M_Map(int choice);
void M_God(int choice);
void M_Noclip(int choice);
void M_Weapons(int choice);
void M_Keys(int choice);
void M_Artifacts(int choice);
void M_Stonecold(int choice);
void M_Lego(int choice);
void M_Pumpup(int choice);
void M_Topo(int choice);
/*
void M_Gripper(int choice);
void M_GPS(int choice);
void M_Dots(int choice);
*/
void M_Rift(int choice);
void M_RiftNow(int choice);
void M_Spin(int choice);
/*
void M_Scoot(int choice);
void M_ScootNow(int choice);
*/
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Screen(int choice);
void M_FPSCounter(int display_fps);
void M_Controls(int choice);
void M_System(int choice);
void M_Sound(int choice);
void M_Game(int choice);
void M_Debug(int choice);
void M_Cheats(int choice);

//void M_FinishReadThis(int choice); - [STRIFE] unused
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
/*
void M_QuickSave(void);
void M_QuickLoad(void);
*/
void M_DrawFilesMenu(void);
void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawReadThis3(void); // [STRIFE]
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);

void M_DrawScreen(void);
void M_DrawKeyBindings(void);
void M_DrawControls(void);
void M_DrawSystem(void);
void M_DrawGame(void);
void M_DrawDebug(void);

void M_DrawSound(void);
void M_DrawCheats(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawThermoSVE(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);

char *maptext[] =			// ADDED FOR PSP
{
	" ",
	"SANCTUARY",
	"TOWN",
	"FRONT BASE",
	"POWER STATION",
	"PRISON",
	"SEWERS",
	"CASTLE",
	"AUDIENCE CHAMBER",
	"CASTLE: PROGRAMMER'S KEEP",
	"NEW FRONT BASE",
	"BORDERLANDS",
	"THE TEMPLE OF THE ORACLE",
	"CATACOMBS",
	"MINES",
	"FORTRESS: ADMINISTRATION",
	"FORTRESS: BISHOP'S TOWER",
	"FORTRESS: THE BAILEY",
	"FORTRESS: STORES",
	"FORTRESS: SECURITY COMPLEX",
	"FACTORY: RECEIVING",
	"FACTORY: MANUFACTURING",
	"FACTORY: FORGE",
	"ORDER COMMONS",
	"FACTORY: CONVERSION CHAPEL",
	"CATACOMBS: RUINED TEMPLE",
	"PROVING GROUNDS",
	"THE LAB",
	"ALIEN SHIP",
	"ENTITY'S LAIR",
	"ABANDONED FRONT BASE",
	"TRAINING FACILITY",
//					DEMO ADDITIONS ???
	"SANCTUARY",
	"TOWN",
	"MOVEMENT BASE (DEMO VER.)",
	"MAP35",
	"MAP36",
};

char *mustext[] =			// ADDED FOR PSP
{
    "",
    "01",
    "02",
    "03",
    "04",
    "05",
    "06",
    "07",
    "08",
    "09",
    "10",
    "11",
    "",
    "12",
    "13",
    "",
    "",
    "14",
    "15",
    "",
    "",
    "16",
    "17",
    "",
    "18",
    "19",
    "20",
    "21",
    "",
    "",
    "",
    "",
    "",
    "22",
    "23",
};

char *mustextdemo[] =			// ADDED FOR PSP
{
    "",
    "01",
    "",
    "02",
    "",
    "03",
    "04",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "05",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "06",
    "07",
    "08",
};
/*
char *spottext[] =			// FOR PSP (BUT DOESN'T WORK (YET))
{
	" ",
	"01",
	"02",
	"03",
	"04",
	"05",
	"06",
	"07",
	"08",
	"09",
	"10",
};
*/
char *stupidtable[] =
{
    "A","B","C","D","E",
    "F","G","H","I","J",
    "K","L","M","N","O",
    "P","Q","R","S","T",
    "U","V","W","X","Y",
    "Z"
};

#define CLASSIC_CONTROLLER_A		0x1
#define CLASSIC_CONTROLLER_R		0x2
#define CLASSIC_CONTROLLER_PLUS		0x4
#define CLASSIC_CONTROLLER_L		0x8
#define CLASSIC_CONTROLLER_MINUS	0x10
#define CLASSIC_CONTROLLER_B		0x20
#define CLASSIC_CONTROLLER_LEFT		0x40
#define CLASSIC_CONTROLLER_DOWN		0x80
#define CLASSIC_CONTROLLER_RIGHT	0x100
#define CLASSIC_CONTROLLER_UP		0x200
#define CLASSIC_CONTROLLER_ZR		0x400
#define CLASSIC_CONTROLLER_ZL		0x800
#define CLASSIC_CONTROLLER_HOME		0x1000
#define CLASSIC_CONTROLLER_X		0x2000
#define CLASSIC_CONTROLLER_Y		0x4000
#define CONTROLLER_1			0x8000
#define CONTROLLER_2			0x10000

char *Key2String (int ch)
{
// S.A.: return "[" or "]" or "\"" doesn't work
// because there are no lumps for these chars,
// therefore we have to go with "RIGHT BRACKET"
// and similar for much punctuation.  Probably
// won't work with international keyboards and
// dead keys, either.
//
    switch (ch)
    {
	case CLASSIC_CONTROLLER_UP:	return "UP ARROW";
	case CLASSIC_CONTROLLER_DOWN:	return "DOWN ARROW";
	case CLASSIC_CONTROLLER_LEFT:	return "LEFT ARROW";
	case CLASSIC_CONTROLLER_RIGHT:	return "RIGHT ARROW";
	case CLASSIC_CONTROLLER_MINUS:	return "MINUS";
	case CLASSIC_CONTROLLER_PLUS:	return "PLUS";
	case CLASSIC_CONTROLLER_HOME:	return "HOME";
	case CLASSIC_CONTROLLER_A:	return "A";
	case CLASSIC_CONTROLLER_B:	return "B";
	case CLASSIC_CONTROLLER_X:	return "X";
	case CLASSIC_CONTROLLER_Y:	return "Y";
	case CLASSIC_CONTROLLER_ZL:	return "ZL";
	case CLASSIC_CONTROLLER_ZR:	return "ZR";
	case CLASSIC_CONTROLLER_L:	return "LEFT TRIGGER";
	case CLASSIC_CONTROLLER_R:	return "RIGHT TRIGGER";
	case CONTROLLER_1:		return "1";
	case CONTROLLER_2:		return "2";
    }

    // Handle letter keys
    // S.A.: could also be done with toupper
    if (ch >= 'a' && ch <= 'z')
	return stupidtable[(ch - 'a')];

    return "?";		// Everything else
}

//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
/*
    loadgame,
    savegame,
*/
    gamefiles,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
/*
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
*/
    {1,"M_GFILES",M_GameFiles,'f'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'h'}, // haleyjd 08/28/10: 'r' -> 'h'
    {1,"M_QUITG",M_QuitStrife,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,65, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};

enum
{
    loadgame,
    savegame,
    endgame,
    cheats,
    startcast,
    files_end
} files_e;

menuitem_t FilesMenu[]=
{
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    {1,"M_ENDGAM",M_EndGame,'e'},
    {1,"M_CHEATS",M_Cheats,'c'},
//    {1,"Show Cast",M_StartCast,'x'} // [SVE]
    {1,"M_SHWCST",M_StartCast,'x'}
};

menu_t  FilesDef =
{
    files_end,
    &MainDef,
    FilesMenu,
    M_DrawFilesMenu,
    97,45, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    // haleyjd 08/28/10: [STRIFE] changed all shortcut letters
    {1,"M_JKILL",   M_ChooseSkill, 't'},
    {1,"M_ROUGH",   M_ChooseSkill, 'r'},
    {1,"M_HURT",    M_ChooseSkill, 'v'},
    {1,"M_ULTRA",   M_ChooseSkill, 'e'},
    {1,"M_NMARE",   M_ChooseSkill, 'b'}
};

menu_t  NewDef =
{
    newg_end,           // # of menu items
    &MainDef,           // previous menu - haleyjd [STRIFE] changed to MainDef
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    toorough            // lastOn - haleyjd [STRIFE]: default to skill 1
};

//
// EPISODE SELECT
//
// haleyjd [SVE] 20141014: Restored to allow selecting the demo episode
// from the menus
//

enum
{
    ep1,
    ep2,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
/*
    {1, "Quest for the Sigil", M_Episode, 'q'},
    {1, "Trust No One (Demo)", M_Episode, 't'},
*/
    {1, "M_QUEST", M_Episode, 'q'},
    {1, "M_TRUST", M_Episode, 't'},
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &NewDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    44,63,              // x,y
    ep1			// lastOn
};

//
// OPTIONS MENU
//
enum
{
/*
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    endgame,
    scrnsize,
    option_empty1,
    soundvol,
    cheats,
*/
    screen,
    controls,
    sound,
    sys,
    game,
    debug,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_SCRSET",	M_Screen,'s'},
    {1,"M_CTLSET",	M_Controls,'c'},
    {1,"M_SNDSET",	M_Sound,'v'},
    {1,"M_SYSSET",	M_System,'y'},
    {1,"M_GMESET",	M_Game,'g'},
    {1,"M_DBGSET",	M_Debug,'d'}
/*
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    {1,"M_ENDGAM",	M_EndGame,'e'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {1,"M_SVOL",	M_Sound,'s'},
    {1,"M_CHEATS",	M_Cheats,'c'}
*/
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,57,
    0
};

//
// Read This! MENU 1 & 2 & [STRIFE] 3
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_ReadThis3,0} // haleyjd 08/28/10: [STRIFE] Go to ReadThis3
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    250,185, // haleyjd 08/28/10: [STRIFE] changed coords
    0
};

// haleyjd 08/28/10: Added Read This! menu 3
enum
{
    rdthsempty3,
    read3_end
} read_e3;

menuitem_t ReadMenu3[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3 =
{
    read3_end,
    &ReadDef2,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
    0
};

enum
{
    gamma,
    screen_empty1,
    scrnsize,
    screen_empty2,
//    screen_detail,
    screen_end
} screen_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t ScreenMenu[]=
{
    {2,"M_BRGTNS",M_Brightness,'b'},
    {-1,"",0,'\0'},
    {2,"M_SCRNSZ",M_SizeDisplay,'s'},
    {-1,"",0,'\0'}/*,
    {2,"M_DETAIL",M_ChangeDetail,'d'}*/
};

menu_t  ScreenDef =
{
    screen_end,
    &OptionsDef,
    ScreenMenu,
    M_DrawScreen,
    80,55,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    mousesens,
    controls_empty1,
    turnsens,
    controls_empty2,
    strafesens,
    controls_empty3,
    controls_freelook,
    mousespeed,
    controls_empty4,
    controls_keybindings,
    controls_end
} controls_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t ControlsMenu[]=
{
//    {2,"M_WSPEED",M_WalkingSpeed,'m'},
    {2,"",M_WalkingSpeed,'m'},
    {-1,"",0,'\0'},
//    {2,"M_TSPEED",M_TurningSpeed,'t'},
    {2,"",M_TurningSpeed,'t'},
    {-1,"",0,'\0'},
//    {2,"M_SSPEED",M_StrafingSpeed,'s'},
    {2,"",M_StrafingSpeed,'s'},
    {-1,"",0,'\0'},
/*
    {2,"M_FRLOOK",M_Freelook,'f'},
    {2,"M_FLKSPD",M_FreelookSpeed,'l'},
*/
    {3,"",M_Freelook,'f'},
    {2,"",M_FreelookSpeed,'l'},
    {-1,"",0,'\0'},
//    {1,"M_KBNDGS",M_KeyBindings,'b'}
    {1,"",M_KeyBindings,'b'}
};

menu_t  ControlsDef =
{
    controls_end,
    &OptionsDef,
    ControlsMenu,
    M_DrawControls,
    50,0,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    keybindings_up,
    keybindings_down,
    keybindings_left,
    keybindings_right,
    keybindings_triangle,
    keybindings_cross,
    keybindings_square,
    keybindings_circle,
    keybindings_select,
    keybindings_start,
    keybindings_lefttrigger,
    keybindings_righttrigger,
    keybindings_special_1,
    keybindings_special_2,
    keybindings_special_3,
    keybindings_special_4,
    keybindings_special_5,
//    keybindings_layout,
    keybindings_clearall,
    keybindings_reset,
    keybindings_end
} keybindings_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t KeyBindingsMenu[]=
{
    {3,"",M_KeyBindingsSetKey,0},
    {3,"",M_KeyBindingsSetKey,1},
    {3,"",M_KeyBindingsSetKey,2},
    {3,"",M_KeyBindingsSetKey,3},
    {3,"",M_KeyBindingsSetKey,4},
    {3,"",M_KeyBindingsSetKey,5},
    {3,"",M_KeyBindingsSetKey,6},
    {3,"",M_KeyBindingsSetKey,7},
    {3,"",M_KeyBindingsSetKey,8},
    {3,"",M_KeyBindingsSetKey,9},
    {3,"",M_KeyBindingsSetKey,10},
    {3,"",M_KeyBindingsSetKey,11},
    {3,"",M_KeyBindingsSetKey,12},
    {3,"",M_KeyBindingsSetKey,13},
    {3,"",M_KeyBindingsSetKey,14},
    {3,"",M_KeyBindingsSetKey,15},
    {3,"",M_KeyBindingsSetKey,16},
//    {2,"",M_KeyBindingsButtonLayout,'l'},
    {3,"",M_KeyBindingsClearAll,'c'},
    {3,"",M_KeyBindingsReset,'r'}
};

menu_t  KeyBindingsDef =
{
    keybindings_end,
    &ControlsDef,
    KeyBindingsMenu,
    M_DrawKeyBindings,
    50,10,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
//    system_cpuspeed,
    system_fps,
    system_ticker,
//    system_framelimiter,
    system_end
} system_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SystemMenu[]=
{
//    {2,"M_CPUSPD",M_CpuSpeed,'c'},
    {3,"M_FRAMES",M_FPS,'f'},
    {3,"M_TICKER",M_DisplayTicker,'t'}/*,
    {2,"M_FLIMIT",M_FrameLimiter,'l'}*/
};

menu_t  SystemDef =
{
    system_end,
    &OptionsDef,
    SystemMenu,
    M_DrawSystem,
    40,85,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
    game_mapgrid,
    game_maprotation,
    game_followmode,
//    game_statistics,
    game_messages,
    game_dialogtext,
    game_crosshair,
//    game_weapon,				// FIXME: IF ACTIVATED, CRASHES GAME IF WEAPON CHANGES
    game_classicmode,
    game_gore,
    game_fullscreenhud,
    game_recoil,
    game_autoaim,
    game_respawn,
    game_fast,
    game_end
} game_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t GameMenu[]=
{
/*
    {2,"M_MAPGRD",M_MapGrid,'g'},
    {2,"M_MAPROT",M_MapRotation,'r'},
    {2,"M_FLWMDE",M_FollowMode,'f'},
//    {2,"M_AMSTAT",M_Statistics,'s'},
    {-1,"",0,'\0'},
    {2,"",M_Messages,'m'},
    {2,"M_DLGTXT",M_DialogText,'t'},
    {2,"M_XHAIR",M_Crosshair,'c'},
//    {2,"M_WPNCHG",M_WeaponChange,'w'}		// FIXME: IF ACTIVATED, CRASHES GAME IF WEAPON CHANGES
*/
    {3,"",M_MapGrid,'g'},
    {3,"",M_MapRotation,'r'},
    {3,"",M_FollowMode,'f'},
    {3,"",M_Messages,'t'},
    {3,"",M_DialogText,'d'},
    {3,"",M_Crosshair,'c'},
    {3,"",M_ClassicMode,'c'},
    {3,"",M_BloodGore,'b'},
    {3,"",M_HUD,'h'},
    {3,"",M_WeaponRecoil,'w'},
    {3,"",M_Autoaim,'a'},
    {3,"",M_Spawn,'s'},
    {3,"",M_FastMonsters,'m'}
};

menu_t  GameDef =
{
    game_end,
    &OptionsDef,
    GameMenu,
    M_DrawGame,
    80,30,       // [STRIFE] changed y coord 64 -> 35
    0
};

enum
{
/*
    debug_battery,
    debug_cpu,
    debug_memory,
    debug_other,
*/
    debug_coordinates,
    debug_version,
    debug_end
} debug_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t DebugMenu[]=
{
/*
    {2,"M_BATNFO",M_Battery,'b'},
    {2,"M_CPUNFO",M_CPU,'p'},
    {2,"M_MEMORY",M_ShowMemory,'m'},
    {2,"M_OTHNFO",M_Other,'o'},
*/
    {3,"M_CRDNTS",M_Coordinates,'c'},
    {3,"M_VRSN",M_Version,'v'}
};

menu_t  DebugDef =
{
    debug_end,
    &OptionsDef,
    DebugMenu,
    M_DrawDebug,
    45,75,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sfx_voices,
    voice_vol,
    sfx_empty3,
    music_engine,
/*
    sfx_mouse,
    sfx_empty4,
*/
    sound_end
} sound_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'},
    {3,"M_VOICES",M_Voices,'d'}, 
    {2,"",M_VoiceVol,'v'}, 
    {-1,"",0,'\0'},
    {3,"",M_Engine,'e'}
/*
    {2,"M_MSENS",M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'}
*/
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,30,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// CHEATS MENU
//
enum
{
    cheats_map,
    cheats_god,
    cheats_noclip,
    cheats_weapons,
    cheats_keys,
    cheats_artifacts,
    cheats_stonecold,
    cheats_lego,
    cheats_pumpup,
    cheats_topo,
/*
    cheats_gripper,
    cheats_gps,
    cheats_dots,
*/
    cheats_rift,
    cheats_empty1,
    cheats_riftnow,
/*
    cheats_scoot,
    cheats_scootnow,
*/
    cheats_spin,
    cheats_end
} cheats_e;

menuitem_t CheatsMenu[]=
{
    {3,"",M_Map,'g'},
    {3,"",M_God,'g'},
    {3,"",M_Noclip,'n'},
    {3,"",M_Weapons,'w'}, 
    {3,"",M_Keys,'k'},
    {3,"",M_Artifacts,'a'},
    {3,"",M_Stonecold,'c'},
    {3,"",M_Lego,'l'},
    {3,"",M_Pumpup,'p'},
    {3,"",M_Topo,'t'},
/*
    {2,"",M_Gripper,'b'},
    {2,"",M_GPS,'c'},
    {2,"",M_Dots,'d'},
*/
    {3,"",M_Rift,'r'},
    {-1,"",0,'\0'},
    {3,"",M_RiftNow,'e'},
/*
    {2,"",M_Scoot,'j'},
    {2,"",M_ScootNow,'s'},
*/
    {3,"",M_Spin,'m'}
};

menu_t  CheatsDef =
{
    cheats_end,
    &FilesDef,
    CheatsMenu,
    M_DrawCheats,
    45,20,       // [STRIFE] changed y coord 64 -> 35
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    load_end,
    &FilesDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    load_end,
    &FilesDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};

void M_DrawNameChar(void);

//
// NAME CHARACTER MENU
//
// [STRIFE]
// haleyjd 20110210: New "Name Your Character" Menu
//
menu_t NameCharDef =
{
    load_end,
    &EpiDef,
    SaveMenu,
    M_DrawNameChar,
    80,54,
    0
};

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// [STRIFE]
// haleyjd 20110210: Rewritten to read "name" file in each slot directory
//
void M_ReadSaveStrings(void)
{
    FILE *handle;
    int   i;
    char *fname = NULL;

    for(i = 0; i < load_end; i++)
    {
        if(fname)
            Z_Free(fname, "M_ReadSaveStrings");
        fname = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(i, "\\name"));

        handle = fopen(fname, "rb");
        if(handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING,
                         sizeof(savegamestrings[i]));
            LoadMenu[i].status = 0;
            continue;
        }
        fread(savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = 1;
    }

    if(fname)
        Z_Free(fname, "M_ReadSaveStrings");
}

//
// M_DrawNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for drawing the "Name Your Character" menu.
//
void M_DrawNameChar(void)
{
    int i;

    M_WriteText(72, 28, DEH_String("Name Your Character"));

    for (i = 0;i < load_end; i++)
    {
        // [SVE]
        NameCharDef.menuitems[i].x = LoadDef.x - 10;
        NameCharDef.menuitems[i].y = (LoadDef.y + LINEHEIGHT*i) - 4;
        NameCharDef.menuitems[i].w = 209;
        NameCharDef.menuitems[i].h = LINEHEIGHT - 4;

        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_DoNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for items in the "Name Your Character" menu.
//
void M_DoNameChar(int choice)
{
    int map_init;

    // 20130301: clear naming character flag for 1.31 save logic
    if(gameversion == exe_strife_1_31)
        namingCharacter = false;
    sendsave = 1;
    ClearTmp();
    G_WriteSaveName(choice, savegamestrings[choice]);
    quickSaveSlot = choice;  
    SaveDef.lastOn = choice;
    ClearSlot();
    FromCurr();
    
    if(menuepisode)			// [SVE]: allow demo episode select
    {					// HACK AGAINST [SVE]: add more demo style
        map_init = 33;

	isdemoversion = true;		// HACK AGAINST [SVE]: add more demo style

	if(!devparm)
	    ST_Init ();
    }					// HACK AGAINST [SVE]: add more demo style
    else
    {					// HACK AGAINST [SVE]: add more demo style
        map_init = 2;

	isdemoversion = false;		// HACK AGAINST [SVE]: add more demo style

	if(!devparm)
	    ST_Init ();
    }					// HACK AGAINST [SVE]: add more demo style
    G_DeferedInitNew(menuskill, map_init);
    M_ClearMenus(0);
}

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;

    V_DrawPatch(72, 28, 
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
        // [SVE]
        LoadDef.menuitems[i].x = LoadDef.x - 10;
        LoadDef.menuitems[i].y = (LoadDef.y + LINEHEIGHT*i) - 4;
        LoadDef.menuitems[i].w = 209;
        LoadDef.menuitems[i].h = LINEHEIGHT - 4;

        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    M_WriteText(42, 163, "* INDICATES A SAVEGAME THAT WAS");
    M_WriteText(42, 173, "CREATED USING AN OPTIONAL PWAD!");
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;

    V_DrawPatch(x - 8, y + 7,
                      W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));

    for (i = 0;i < 24;i++)
    {
        V_DrawPatch(x, y + 7,
                          W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
        x += 8;
    }

    V_DrawPatch(x, y + 7,
                      W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    // [STRIFE]: completely rewritten
    char *name = NULL;

    G_WriteSaveName(choice, savegamestrings[choice]);
    ToCurr();

    // use safe & portable filepath concatenation for Choco
    name = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(choice, ""));

    G_ReadCurrent(name);
    quickSaveSlot = choice;
    M_ClearMenus(0);

    Z_Free(name, "M_LoadSelect");
}

//
// Selected from DOOM menu
//
// [STRIFE] Verified unmodified
//
void M_LoadGame (int choice)
{
/*
    if (netgame)
    {
        M_StartMessage(DEH_String(LOADNET), NULL, false);
        return;
    }
*/
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;

    V_DrawPatch(72, 28, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
        // [SVE]
        SaveDef.menuitems[i].x = SaveDef.x - 10;
        SaveDef.menuitems[i].y = (SaveDef.y + LINEHEIGHT*i) - 4;
        SaveDef.menuitems[i].w = 209;
        SaveDef.menuitems[i].h = LINEHEIGHT - 4;

        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }

    M_WriteText(42, 163, "* INDICATES A SAVEGAME THAT WAS");
    M_WriteText(42, 173, "CREATED USING AN OPTIONAL PWAD!");
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    // [STRIFE]: completely rewritten
    if(slot >= 0)
    {
        sendsave = 1;
        G_WriteSaveName(slot, savegamestrings[slot]);
        M_ClearMenus(0);
        quickSaveSlot = slot;        
        // haleyjd 20130922: slight divergence. We clear the destination slot 
        // of files here, which vanilla did not do. As a result, 1.31 had 
        // broken save behavior to the point of unusability. fraggle agrees 
        // this is detrimental enough to be fixed - unconditionally, for now.
        ClearSlot();        
        FromCurr();
    }
    else
        M_StartMessage(DEH_String(QSAVESPOT), NULL, false);
}

//
// User wants to save. Start string input for M_Responder
//
#include <time.h>

void M_SaveSelect(int choice)
{
    frontend_wipe = false;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // [STRIFE]
    quickSaveSlot = choice;
    //saveSlot = choice;

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);

    int day = aTime->tm_mday;
    int month = aTime->tm_mon + 1;
    int year = aTime->tm_year + 1900;
    int hour = aTime->tm_hour;
    int min = aTime->tm_min;
//    int gamemapx = 0;

    if(load_extra_wad == 1 && load_voices == 0)
	sprintf(savegamestrings[choice], "MAP%2.2d %d/%d/%d %2.2d:%2.2d *",
		gamemap, year, month, day, hour, min);
    else
	sprintf(savegamestrings[choice], "MAP%2.2d %d/%d/%d %2.2d:%2.2d",
		gamemap, year, month, day, hour, min);

    M_StringCopy(saveOldString, savegamestrings[choice], sizeof(saveOldString));
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
/*
    // [STRIFE]
    if (netgame)
    {
        // haleyjd 20110211: Hooray for Rogue's awesome multiplayer support...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }
*/
    if (!usergame)
    {
        M_StartMessage(DEH_String(SAVEDEAD),NULL,true);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    // [STRIFE]
    if(gameversion == exe_strife_1_31)
    {
        // haleyjd 20130301: in 1.31, we can choose a slot again.
        M_SetupNextMenu(&SaveDef);
        M_ReadSaveStrings();
    }
    else
    {
        // In 1.2 and lower, you save over your character slot exclusively
        M_ReadSaveStrings();
        M_DoSave(quickSaveSlot);
    }
}



//
//      M_QuickSave
//
/*
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

void M_QuickSave(void)
{
    if (netgame)
    {
        // haleyjd 20110211 [STRIFE]: More fun...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }

    if (!usergame)
    {
        S_StartSound(NULL, sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;	// means to pick a slot now
        return;
    }
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoadResponse
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

//
// M_QuickLoad
//
// [STRIFE] Verified unmodified
//
void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(QLOADNET),NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(QSAVESPOT),NULL,false);
        return;
    }
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}
*/



//
// Read This Menus
// Had a "quick hack to fix romero bug"
// haleyjd 08/28/10: [STRIFE] Draw HELP1, unconditionally.
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatch (0, 0, W_CacheLumpName(DEH_String("HELP1"), PU_CACHE));
}



//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] Not optional, draws HELP2
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    if(!isdemoversion)
	V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP2"), PU_CACHE));
    else
	V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP22"), PU_CACHE));
}


//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] New function to draw HELP3.
//
void M_DrawReadThis3(void)
{
    inhelpscreens = true;
    
    if(!isdemoversion)
	V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP3"), PU_CACHE));
    else
	V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP32"), PU_CACHE));
}

void M_GameFiles(int choice)
{
    // haleyjd 20141031: [SVE] make cast call context sensitive
    if(usergame)
        FilesDef.numitems = files_end - 1;
    else
        FilesDef.numitems = files_end;
    if(FilesDef.lastOn >= FilesDef.numitems)
        FilesDef.lastOn = 0;

    M_SetupNextMenu(&FilesDef);
}

void M_DrawFilesMenu(void)
{
}

void M_Brightness(int choice)
{
    switch(choice)
    {
    case 0:
        if (usegamma)
            usegamma--;
        break;
    case 1:
        if (usegamma < 4)
            usegamma++;
        break;
    }
    players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
    I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
}

void M_Screen(int choice)
{
    M_SetupNextMenu(&ScreenDef);
}

void M_DrawScreen(void)
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.

    V_DrawPatch(60, 20, W_CacheLumpName(DEH_String("M_SCRSET"), PU_CACHE));

    if(classicmode)
    {
	M_DrawThermo(OptionsDef.x+62,OptionsDef.y+LINEHEIGHT*(gamma+1),
                 5,usegamma);
	M_DrawThermo(OptionsDef.x+45,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);
    }
    else
    {
	M_WriteText(80, 75, DEH_String("BRIGHTNESS"));
	M_WriteText(80, 95, DEH_String("SCREEN SIZE"));

	M_DrawThermoSVE(ScreenDef.x+122,OptionsDef.y-5+LINEHEIGHT*(gamma+1),
                 5,usegamma);
	M_DrawThermoSVE(ScreenDef.x+90,OptionsDef.y-23+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);
    }
/*
    if(devparm)
    {
	if(detailLevel)
	    V_DrawPatch (180, 134, W_CacheLumpName(DEH_String("M_LOW"), PU_CACHE));
	else
	    V_DrawPatch (180, 135, W_CacheLumpName(DEH_String("M_HIGH"), PU_CACHE));
    }
*/
}
/*
void M_KeyBindingsButtonLayout(int choice)
{
    switch(choice)
    {
    case 0:
        if (button_layout)
            button_layout--;
        break;
    case 1:
        if (button_layout < 1)
            button_layout++;
        break;
    }
}
*/
void M_KeyBindingsSetKey(int choice)
{
    askforkey = true;
    keyaskedfor = choice;
}

void M_KeyBindingsClearControls (int ch)	// XXX (FOR PSP): NOW THIS IS RATHER IMPORTANT: IF...
{						// ...THE CONFIG VARIABLES IN THIS SOURCE EVER GET...
    int i;					// ...SOMEWHAT REARRANGED, THEN IT'S IMPORTANT TO...
						// ...CHANGE THE START- & END-POS INTEGERS AS WELL...
						// ...TO THEIR NEW CORRESPONDING POSITIONS OR ELSE...
						// ...THE KEY BINDINGS MENU WILL BE VERY BUGGY!!!

    for (i = key_controls_start_in_cfg_at_pos; i < key_controls_end_in_cfg_at_pos; i++)
    {
	if (*doom_defaults_list[i].location == ch)
	    *doom_defaults_list[i].location = 0;
    }
}

void M_KeyBindingsClearAll (int choice)
{
    *doom_defaults_list[26].location = 0;
    *doom_defaults_list[27].location = 0;
    *doom_defaults_list[28].location = 0;
    *doom_defaults_list[29].location = 0;
    *doom_defaults_list[30].location = 0;
    *doom_defaults_list[31].location = 0;
    *doom_defaults_list[32].location = 0;
    *doom_defaults_list[33].location = 0;
    *doom_defaults_list[34].location = 0;
    *doom_defaults_list[35].location = 0;
    *doom_defaults_list[36].location = 0;
    *doom_defaults_list[37].location = 0;
    *doom_defaults_list[38].location = 0;
    *doom_defaults_list[39].location = 0;
    *doom_defaults_list[40].location = 0;
    *doom_defaults_list[41].location = 0;
    *doom_defaults_list[42].location = 0;
}

void M_KeyBindingsReset (int choice)
{
    *doom_defaults_list[26].location = CLASSIC_CONTROLLER_R;
    *doom_defaults_list[27].location = CLASSIC_CONTROLLER_L;
    *doom_defaults_list[28].location = CLASSIC_CONTROLLER_MINUS;
    *doom_defaults_list[29].location = CLASSIC_CONTROLLER_LEFT;
    *doom_defaults_list[30].location = CLASSIC_CONTROLLER_DOWN;
    *doom_defaults_list[31].location = CLASSIC_CONTROLLER_RIGHT;
    *doom_defaults_list[32].location = CLASSIC_CONTROLLER_ZL;
    *doom_defaults_list[33].location = CLASSIC_CONTROLLER_ZR;
    *doom_defaults_list[34].location = CLASSIC_CONTROLLER_HOME;
    *doom_defaults_list[35].location = CLASSIC_CONTROLLER_PLUS;
    *doom_defaults_list[36].location = CLASSIC_CONTROLLER_Y;
    *doom_defaults_list[37].location = CLASSIC_CONTROLLER_X;
    *doom_defaults_list[38].location = CLASSIC_CONTROLLER_A;
    *doom_defaults_list[39].location = CLASSIC_CONTROLLER_B;
    *doom_defaults_list[40].location = CLASSIC_CONTROLLER_UP;
    *doom_defaults_list[41].location = CONTROLLER_1;
    *doom_defaults_list[42].location = CONTROLLER_2;
}

void M_DrawKeyBindings(void)
{
    int i;

//    V_DrawPatch (80, 0, W_CacheLumpName(DEH_String("M_KBNDGS"), PU_CACHE));

    M_WriteText(40, 10, DEH_String("FIRE"));
    M_WriteText(40, 20, DEH_String("USE / OPEN"));
    M_WriteText(40, 30, DEH_String("MAIN MENU"));

//    if(button_layout == 0)
    {
    	M_WriteText(40, 40, DEH_String("WEAPON LEFT"));
    	M_WriteText(40, 50, DEH_String("SHOW AUTOMAP"));
    }
//    else
//    {
//    	M_WriteText(40, 60, DEH_String("STRAFE LEFT"));
//    	M_WriteText(40, 70, DEH_String("STRAFE RIGHT"));
//    }

    M_WriteText(40, 60, DEH_String("WEAPON RIGHT"));
    M_WriteText(40, 70, DEH_String("AUTOMAP ZOOM IN"));
    M_WriteText(40, 80, DEH_String("AUTOMAP ZOOM OUT"));
    M_WriteText(40, 90, DEH_String("NEXT INV. ITEM"));
    M_WriteText(40, 100, DEH_String("INVENTORY USE"));
    M_WriteText(40, 110, DEH_String("INVENTORY POPUP"));
    M_WriteText(40, 120, DEH_String("INVENTORY KEYS"));
    M_WriteText(40, 130, DEH_String("MISSION OBJECTIVES"));
    M_WriteText(40, 140, DEH_String("JUMP"));
    M_WriteText(40, 150, DEH_String("INVENTORY DROP"));
    M_WriteText(40, 160, DEH_String("RUN"));
    M_WriteText(40, 170, DEH_String("CONSOLE"));

//    M_WriteText(40, 165, DEH_String("BUTTON LAYOUT:"));

//    if(button_layout == 0)
//    	M_WriteText(195, 165, DEH_String("PS VITA"));
//    else if(button_layout == 1)
//    	M_WriteText(195, 165, DEH_String("PSP"));

    M_WriteText(40, 180, DEH_String("CLEAR ALL CONTROLS"));
    M_WriteText(40, 190, DEH_String("RESET TO DEFAULTS"));

    for (i = 0; i < 17; i++)
    {
	if (askforkey && keyaskedfor == i)
	{
	    M_WriteText(195, (i*10+10), "???");
	}
	else
	{
	    M_WriteText(195, (i*10+10),
		Key2String(*(doom_defaults_list[i+FirstKey+26].location)));
	}
    }
}

void M_KeyBindings(int choice)
{
    M_SetupNextMenu(&KeyBindingsDef);
}

void M_Freelook(int choice)
{
    switch(choice)
    {
    case 0:
        if (mouselook)
            mouselook--;
        break;
    case 1:
        if (mouselook < 2)
            mouselook++;
        break;
    }
}

void M_FreelookSpeed(int choice)
{
    switch(choice)
    {
    case 0:
        if (mspeed)
            mspeed--;
        break;
    case 1:
        if (mspeed < 10)
            mspeed++;
        break;
    }
}

void M_Controls(int choice)
{
    M_SetupNextMenu(&ControlsDef);
}

void M_DrawControls(void)
{
    if(classicmode)
    {
	V_DrawPatch (50, 0, W_CacheLumpName(DEH_String("M_WSPEED"), PU_CACHE));
	V_DrawPatch (50, 40, W_CacheLumpName(DEH_String("M_TSPEED"), PU_CACHE));
	V_DrawPatch (50, 80, W_CacheLumpName(DEH_String("M_SSPEED"), PU_CACHE));
	V_DrawPatch (50, 120, W_CacheLumpName(DEH_String("M_FRLOOK"), PU_CACHE));
	V_DrawPatch (50, 140, W_CacheLumpName(DEH_String("M_FLKSPD"), PU_CACHE));
	V_DrawPatch (50, 179, W_CacheLumpName(DEH_String("M_KBNDGS"), PU_CACHE));

	if(mouselook == 0)
	    V_DrawPatch(177, 122, 
		    W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
	else if(mouselook == 1)
	    V_DrawPatch(177, 122, 
		    W_CacheLumpName(DEH_String("M_NORMAL"), PU_CACHE));
	else if(mouselook == 2)
	    V_DrawPatch(177, 122, 
		    W_CacheLumpName(DEH_String("M_INVRSE"), PU_CACHE));

	M_DrawThermo(OptionsDef.x-10,OptionsDef.y-56+LINEHEIGHT*(mousesens+1),
                    29,forwardmove-19);

	M_DrawThermo(OptionsDef.x-10,OptionsDef.y-54+LINEHEIGHT*(turnsens+1),
                    6,turnspeed-5);

	M_DrawThermo(OptionsDef.x-10,OptionsDef.y-52+LINEHEIGHT*(strafesens+1),
                    17,sidemove-16);

	M_DrawThermo(OptionsDef.x-10,OptionsDef.y-50+LINEHEIGHT*(mousespeed+1),
                    11,mspeed);
    }
    else
    {
	V_DrawPatch (50, 20, W_CacheLumpName(DEH_String("M_CTLSET"), PU_CACHE));

	M_WriteText(20, 45, DEH_String("MOVEMENT SPEEDS:"));
	M_WriteText(20, 55, DEH_String("WALK"));
	M_WriteText(20, 75, DEH_String("TURN"));
	M_WriteText(20, 95, DEH_String("STRAFE"));
	M_WriteText(20, 115, DEH_String("FREELOOK MODE"));
	M_WriteText(20, 125, DEH_String("FREELOOK"));
	M_WriteText(20, 145, DEH_String("KEY BINDINGS..."));

	if(mouselook == 0)
	    M_WriteText(289, 115, DEH_String("OFF"));
	else if(mouselook == 1)
	    M_WriteText(256, 115, DEH_String("NORMAL"));
	else if(mouselook == 2)
	    M_WriteText(257, 115, DEH_String("INVERSE"));

	M_DrawThermoSVE(ControlsDef.x+20,ControlsDef.y+32+LINEHEIGHT*(mousesens+1),
                    29,forwardmove-19);

	M_DrawThermoSVE(ControlsDef.x+204,ControlsDef.y+15+LINEHEIGHT*(turnsens+1),
                    6,turnspeed-5);

	M_DrawThermoSVE(ControlsDef.x+116,ControlsDef.y-3+LINEHEIGHT*(strafesens+1),
                    17,sidemove-16);

	M_DrawThermoSVE(ControlsDef.x+164,ControlsDef.y-31+LINEHEIGHT*(mousespeed+1),
                    11,mspeed);
    }
}
/*
void M_CpuSpeed(int choice)
{
    mhz333 ^= 1;
    if (mhz333)
    {
	players[consoleplayer].message = DEH_String("CLOCK NOW AT 333MHZ");
	scePowerSetClockFrequency(333, 333, 166);
    }
    else
    {
	players[consoleplayer].message = DEH_String("CLOCK NOW AT 222MHZ");
	scePowerSetClockFrequency(222, 222, 111);
    }
}
*/
void M_FPS(int choice)
{
    if(display_fps < 1)
    {
	display_fps++;
	fps_enabled = 1;
	players[consoleplayer].message = DEH_String("FPS COUNTER ON");
    }
    else if(display_fps)
    {
	display_fps--;
	fps_enabled = 0;
	players[consoleplayer].message = DEH_String("FPS COUNTER OFF");
    }
}

void M_DisplayTicker(int choice)
{
    display_ticker = !display_ticker;
    if (display_ticker)
    {
	dots_enabled = 1;
	players[consoleplayer].message = DEH_String("TICKER ON");
    }
    else
    {
	dots_enabled = 0;
	players[consoleplayer].message = DEH_String("TICKER OFF");
    }
    I_DisplayFPSDots(display_ticker);
}
/*
void M_FrameLimiter(int choice)
{
    switch(choice)
    {
    case 0:
        if (frame_limit)
            frame_limit--;
        break;
    case 1:
        if (frame_limit < 3)
            frame_limit++;
        break;
    }
}

void M_Battery(int choice)
{
    switch(choice)
    {
    case 0:
        if (battery_info)
            battery_info--;
        break;
    case 1:
        if (battery_info < 1)
            battery_info++;
        break;
    }
}

void M_CPU(int choice)
{
    switch(choice)
    {
    case 0:
        if (cpu_info)
            cpu_info--;
        break;
    case 1:
        if (cpu_info < 1)
            cpu_info++;
        break;
    }
}

void M_Other(int choice)
{
    switch(choice)
    {
    case 0:
        if (other_info)
            other_info--;
        break;
    case 1:
        if (other_info < 1)
            other_info++;
        break;
    }
}

void M_ShowMemory(int choice)
{
    switch(choice)
    {
    case 0:
        if (memory_info)
            memory_info--;
        break;
    case 1:
        if (memory_info < 1)
            memory_info++;
        break;
    }
}
*/
void M_Version(int choice)
{
    switch(choice)
    {
    case 0:
        if (version_info)
            version_info--;
        break;
    case 1:
        if (version_info < 1)
            version_info++;
        break;
    }
}

void M_Coordinates(int choice)
{
    switch(choice)
    {
    case 0:
        if (coordinates_info)
            coordinates_info--;
        break;
    case 1:
        if (coordinates_info < 1)
            coordinates_info++;
        break;
    }
}

void M_System(int choice)
{
    M_SetupNextMenu(&SystemDef);
}
/*
char unit_plugged_textbuffer[50];
char battery_present_textbuffer[50];
char battery_charging_textbuffer[50];
char battery_charging_status_textbuffer[50];
char battery_low_textbuffer[50];
char battery_lifetime_percent_textbuffer[50];
char battery_lifetime_int_textbuffer[50];
char battery_temp_textbuffer[50];
char battery_voltage_textbuffer[50];
char processor_clock_textbuffer[50];
char processor_bus_textbuffer[50];
char idle_time_textbuffer[50];
*/
void M_DrawSystem(void)
{
    V_DrawPatch (55, 20, W_CacheLumpName(DEH_String("M_SYSSET"), PU_CACHE));
/*
    if(mhz333)
    V_DrawPatch(188, 67, 
                      W_CacheLumpName(DEH_String("M_333MHZ"), PU_CACHE));
    else
    V_DrawPatch(188, 67, 
                      W_CacheLumpName(DEH_String("M_222MHZ"), PU_CACHE));
*/
    if(classicmode)
    {
	if(display_fps)
	    V_DrawPatch (244, 85, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
	else
	    V_DrawPatch (244, 85, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

	if(display_ticker)
	    V_DrawPatch (244, 106, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
	else
	    V_DrawPatch (244, 106, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    }
    else
    {
	M_WriteText(80, 90, DEH_String("FPS COUNTER"));
	M_WriteText(80, 100, DEH_String("DISPLAY TICKER"));

	if(display_fps)
	    M_WriteText(220, 90, DEH_String("ON"));
	else
	    M_WriteText(220, 90, DEH_String("OFF"));

	if(display_ticker)
	    M_WriteText(220, 100, DEH_String("ON"));
	else
	    M_WriteText(220, 100, DEH_String("OFF"));
    }
/*
    if(frame_limit == 0)
	V_DrawPatch (244, 127, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    else if(frame_limit == 1)
	V_DrawPatch (249, 127, W_CacheLumpName(DEH_String("M_ONE"), PU_CACHE));
    else if(frame_limit == 2)
	V_DrawPatch (246, 127, W_CacheLumpName(DEH_String("M_TWO"), PU_CACHE));
    else if(frame_limit == 3)
	V_DrawPatch (246, 127, W_CacheLumpName(DEH_String("M_THREE"), PU_CACHE));
*/
}

void M_MapGrid(int choice)
{
    switch(choice)
    {
    case 0:
        if (drawgrid)
            drawgrid--;
        players[consoleplayer].message = DEH_String(AMSTR_GRIDOFF);
        break;
    case 1:
        if (drawgrid < 1)
            drawgrid++;
        players[consoleplayer].message = DEH_String(AMSTR_GRIDON);
        break;
    }
}

void M_MapRotation(int choice)
{
    switch(choice)
    {
    case 0:
        if (am_rotate)
            am_rotate = false;
        players[consoleplayer].message = DEH_String(AMSTR_ROTATIONOFF);
        break;
    case 1:
        if (am_rotate == false)
            am_rotate = true;
        players[consoleplayer].message = DEH_String(AMSTR_ROTATIONON);
        break;
    }
}

void M_WeaponChange(int choice)
{
    switch(choice)
    {
    case 0:
        if (use_vanilla_weapon_change == 1)
            use_vanilla_weapon_change = 0;
        players[consoleplayer].message = DEH_String("ORIGINAL WEAPON CHANGING STYLE DISABLED");
        break;
    case 1:
        if (use_vanilla_weapon_change == 0)
            use_vanilla_weapon_change = 1;
        players[consoleplayer].message = DEH_String("ORIGINAL WEAPON CHANGING STYLE ENABLED");
        break;
    }
}

void M_FollowMode(int choice)
{
    switch(choice)
    {
    case 0:
        if (followplayer)
            followplayer--;
        players[consoleplayer].message = DEH_String(AMSTR_FOLLOWOFF);
        break;
    case 1:
        if (followplayer < 1)
            followplayer++;
        players[consoleplayer].message = DEH_String(AMSTR_FOLLOWON);
        break;
    }
}
/*
void M_Statistics(int choice)
{
    switch(choice)
    {
    case 0:
        if (show_stats)
            show_stats--;
	players[consoleplayer].message = DEH_String("LEVEL STATISTICS OFF");
        break;
    case 1:
        if (show_stats < 1)
            show_stats++;
	players[consoleplayer].message = DEH_String("LEVEL STATISTICS ON");
        break;
    }
}
*/
void M_Messages(int choice)
{
    switch(choice)
    {
    case 0:
        if (showMessages)
            showMessages--;
	    messages_disabled = 1;
        players[consoleplayer].message = DEH_String(MSGOFF);
        break;
    case 1:
        if (showMessages < 1)
            showMessages++;
	    messages_disabled = 0;
	    dont_show = 0;
        players[consoleplayer].message = DEH_String(MSGON);
        break;
    }
}

void M_DialogText(int choice)
{
    switch(choice)
    {
    case 0:
        if (dialogshowtext)
            dialogshowtext--;
        players[consoleplayer].message = DEH_String(DLGOFF);
        break;
    case 1:
        if (dialogshowtext < 1)
            dialogshowtext++;
        players[consoleplayer].message = DEH_String(DLGON);
        break;
    }
}

void M_Crosshair(int choice)
{
    switch(choice)
    {
    case 0:
        if (crosshair)
            crosshair--;
        break;
    case 1:
        if (crosshair < 1)
            crosshair++;
        break;
    }
}

void M_HUD(int choice)			// [SVE]
{
    switch(choice)
    {
    case 0:
        if (fullscreenhud)
            fullscreenhud = false;
        players[consoleplayer].message = DEH_String(FHSTR_OFF);
        break;
    case 1:
        if (fullscreenhud == false)
            fullscreenhud = true;
        players[consoleplayer].message = DEH_String(FHSTR_ON);
        break;
    }
}

void M_BloodGore(int choice)		// [SVE]
{
    switch(choice)
    {
    case 0:
        if (d_maxgore)
            d_maxgore = false;
        players[consoleplayer].message = DEH_String(MGSTR_OFF);
        break;
    case 1:
        if (d_maxgore == false)
            d_maxgore = true;
        players[consoleplayer].message = DEH_String(MGSTR_ON);
        break;
    }
}

void M_ClassicMode(int choice)		// [SVE]
{
    switch(choice)
    {
    case 0:
        if (classicmode)
            classicmode = false;
        players[consoleplayer].message = DEH_String(CMSTR_OFF);
        break;
    case 1:
        if (classicmode == false)
            classicmode = true;
        players[consoleplayer].message = DEH_String(CMSTR_ON);
        break;
    }
}

void M_WeaponRecoil(int choice)	// [SVE]
{
    switch(choice)
    {
    case 0:
        if (d_recoil)
            d_recoil = false;
        players[consoleplayer].message = DEH_String(WRSTR_OFF);
        break;
    case 1:
        if (d_recoil == false)
            d_recoil = true;
        players[consoleplayer].message = DEH_String(WRSTR_ON);
        break;
    }
}

void M_Autoaim(int choice)		// [SVE]
{
    switch(choice)
    {
    case 0:
        if (autoaim)
            autoaim = false;
        players[consoleplayer].message = DEH_String(AASTR_OFF);
        break;
    case 1:
        if (autoaim == false)
            autoaim = true;
        players[consoleplayer].message = DEH_String(AASTR_ON);
        break;
    }
}

void M_Spawn(int choice)		// [SVE]
{
    switch(choice)
    {
    case 0:
        if (respawnparm)
            respawnparm = false;
        players[consoleplayer].message = DEH_String(RPSTR_OFF);
        break;
    case 1:
        if (respawnparm == false)
            respawnparm = true;
        players[consoleplayer].message = DEH_String(RPSTR_ON);
        break;
    }
}

void M_FastMonsters(int choice)	// [SVE]
{
    switch(choice)
    {
    case 0:
        if (fastparm)
            fastparm = false;
        players[consoleplayer].message = DEH_String(FPSTR_OFF);
        break;
    case 1:
        if (fastparm == false)
            fastparm = true;
        players[consoleplayer].message = DEH_String(FPSTR_ON);
        break;
    }
}

void M_Game(int choice)
{
    M_SetupNextMenu(&GameDef);
}

void M_DrawGame(void)
{
    if(!classicmode)
    {
	V_DrawPatch (70, 10, W_CacheLumpName(DEH_String("M_GMESET"), PU_CACHE));

	M_WriteText(72, 30, DEH_String("AUTOMAP GRID"));
	M_WriteText(72, 40, DEH_String("AUTOMAP ROTATION"));
	M_WriteText(72, 50, DEH_String("AUTOMAP FOLLOW"));
	M_WriteText(72, 60, DEH_String("SHOW MESSAGES"));
	M_WriteText(72, 70, DEH_String("DIALOG TEXT"));
	M_WriteText(72, 80, DEH_String("CROSSHAIR"));
	M_WriteText(72, 90, DEH_String("CLASSIC MODE"));

	M_WriteText(72, 100, DEH_String("MORE BLOOD & GORE"));
	M_WriteText(72, 110, DEH_String("FULLSCREEN HUD"));
	M_WriteText(72, 120, DEH_String("WEAPON RECOIL"));
	M_WriteText(72, 130, DEH_String("AUTOAIM"));
	M_WriteText(72, 140, DEH_String("RESPAWN MACHINES"));
	M_WriteText(72, 150, DEH_String("FAST ENEMIES"));

	if(d_maxgore)
	    M_WriteText(230, 100, DEH_String("ON"));
	else
	    M_WriteText(230, 100, DEH_String("OFF"));

	if(fullscreenhud)
	    M_WriteText(230, 110, DEH_String("ON"));
	else
	    M_WriteText(230, 110, DEH_String("OFF"));

	if(d_recoil)
	    M_WriteText(230, 120, DEH_String("ON"));
	else
	    M_WriteText(230, 120, DEH_String("OFF"));

	if(autoaim)
	    M_WriteText(230, 130, DEH_String("ON"));
	else
	    M_WriteText(230, 130, DEH_String("OFF"));

	if(respawnparm)
	    M_WriteText(230, 140, DEH_String("ON"));
	else
	    M_WriteText(230, 140, DEH_String("OFF"));

	if(fastparm)
	    M_WriteText(230, 150, DEH_String("ON"));
	else
	    M_WriteText(230, 150, DEH_String("OFF"));
    }
    else
    {
	V_DrawPatch (70, 10, W_CacheLumpName(DEH_String("M_GMESET"), PU_CACHE));

	M_WriteText(72, 60, DEH_String("AUTOMAP GRID"));
	M_WriteText(72, 70, DEH_String("AUTOMAP ROTATION"));
	M_WriteText(72, 80, DEH_String("AUTOMAP FOLLOW"));
	M_WriteText(72, 90, DEH_String("SHOW MESSAGES"));
	M_WriteText(72, 100, DEH_String("DIALOG TEXT"));
	M_WriteText(72, 110, DEH_String("CROSSHAIR"));
	M_WriteText(72, 120, DEH_String("CLASSIC MODE"));
    }

    if(!classicmode)
    {
	if(drawgrid == 1)
	    M_WriteText(230, 30, DEH_String("ON"));
	else if(drawgrid == 0)
	    M_WriteText(230, 30, DEH_String("OFF"));

	if(am_rotate == true)
	    M_WriteText(230, 40, DEH_String("ON"));
	else if(am_rotate == false)
	    M_WriteText(230, 40, DEH_String("OFF"));

	if(followplayer == 1)
	    M_WriteText(230, 50, DEH_String("ON"));
	else if(followplayer == 0)
	    M_WriteText(230, 50, DEH_String("OFF"));

	if(showMessages == 1)
	    M_WriteText(230, 60, DEH_String("ON"));
	else if (showMessages == 0)
	    M_WriteText(230, 60, DEH_String("OFF"));

	if(dialogshowtext == 1)
	    M_WriteText(230, 70, DEH_String("ON"));
	else if (dialogshowtext == 0)
	    M_WriteText(230, 70, DEH_String("OFF"));

	if(crosshair == 1)
	    M_WriteText(230, 80, DEH_String("ON"));
	else if (crosshair == 0)
	    M_WriteText(230, 80, DEH_String("OFF"));

	if(classicmode)
	    M_WriteText(230, 90, DEH_String("ON"));
	else
	    M_WriteText(230, 90, DEH_String("OFF"));
    }
    else
    {
	if(drawgrid == 1)
	    M_WriteText(230, 60, DEH_String("ON"));
	else if(drawgrid == 0)
	    M_WriteText(230, 60, DEH_String("OFF"));

	if(am_rotate == true)
	    M_WriteText(230, 70, DEH_String("ON"));
	else if(am_rotate == false)
	    M_WriteText(230, 70, DEH_String("OFF"));

	if(followplayer == 1)
	    M_WriteText(230, 80, DEH_String("ON"));
	else if(followplayer == 0)
	    M_WriteText(230, 80, DEH_String("OFF"));

	if(showMessages == 1)
	    M_WriteText(230, 90, DEH_String("ON"));
	else if (showMessages == 0)
	    M_WriteText(230, 90, DEH_String("OFF"));

	if(dialogshowtext == 1)
	    M_WriteText(230, 100, DEH_String("ON"));
	else if (dialogshowtext == 0)
	    M_WriteText(230, 100, DEH_String("OFF"));

	if(crosshair == 1)
	    M_WriteText(230, 110, DEH_String("ON"));
	else if (crosshair == 0)
	    M_WriteText(230, 110, DEH_String("OFF"));

	if(classicmode)
	    M_WriteText(230, 120, DEH_String("ON"));
	else
	    M_WriteText(230, 120, DEH_String("OFF"));
    }

    if(classicmode)
    {
	M_WriteText(230, 90, DEH_String("ON"));
	autoaim = true;
	respawnparm = false;
	start_respawnparm = false;
	fastparm = false;
	start_fastparm = false;

	if(!music_cheat_used)
	{
	    if(gamestate == GS_LEVEL)
	    {
		if(!isdemoversion)
		    S_ChangeMusic(gamemap + 1, 1);		// [SVE]
		else
		{
		    if(gamemap == 32)
			S_ChangeMusic(mus_map1, 1);		// [SVE]
		    else if(gamemap == 33)
			S_ChangeMusic(mus_tavern, 1);		// [SVE]
		    else if(gamemap == 34)
			S_ChangeMusic(mus_map3, 1);		// [SVE]
		    else if(gamemap == 35)
			S_ChangeMusic(mus_fight2, 1);		// [SVE]
		    else if(gamemap == 36)
			S_ChangeMusic(mus_fmstl, 1);		// [SVE]
		}
	    }
	}
	else
	    S_ChangeMusic(musnum, 1);		// [SVE]
    }
    else
    {
	M_WriteText(230, 90, DEH_String("OFF"));
	autoaim = false;
	S_ChangeMusic(mus_panthr, 1);	// [SVE]
    }
}

void M_Debug(int choice)
{
    M_SetupNextMenu(&DebugDef);
}

void M_DrawDebug(void)
{
    V_DrawPatch (62, 15, W_CacheLumpName(DEH_String("M_DBGSET"), PU_CACHE));
/*
    if(battery_info)
    {
	sprintf(unit_plugged_textbuffer,"Unit Is Plugged In: %d \n",scePowerIsPowerOnline());
	sprintf(battery_present_textbuffer,"Battery Is Present: %d \n",scePowerIsBatteryExist());
	sprintf(battery_charging_textbuffer,"Battery Is Charging: %d \n",scePowerIsBatteryCharging());
	sprintf(battery_charging_status_textbuffer,"Battery Charging Status: %d \n",scePowerGetBatteryChargingStatus());
	sprintf(battery_low_textbuffer,"Battery Is Low: %d \n",scePowerIsLowBattery());
	sprintf(battery_lifetime_percent_textbuffer,"Battery Lifetime (Perc.): %d \n",scePowerGetBatteryLifePercent());
	sprintf(battery_lifetime_int_textbuffer,"Battery Lifetime (Int.): %d \n",scePowerGetBatteryLifeTime());
	sprintf(battery_temp_textbuffer,"Battery Temp.: %d \n",scePowerGetBatteryTemp());
	sprintf(battery_voltage_textbuffer,"Battery Voltage: %d \n",scePowerGetBatteryVolt());

	V_DrawPatch (234, 55, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 55, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(cpu_info)
    {
	sprintf(processor_clock_textbuffer,"Processor Clock Freq.: %d \n",scePowerGetCpuClockFrequencyInt());
	sprintf(processor_bus_textbuffer,"Processor Bus Freq.: %d \n",scePowerGetBusClockFrequencyInt());

	V_DrawPatch (234, 75, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 75, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(memory_info)
    {
    	sprintf(allocated_ram_textbuffer,"Allocated RAM: %d Bytes\n",allocated_ram_size);

    	sprintf(free_ram_textbuffer,"Curr. Free RAM: %d Bytes\n",sceKernelTotalFreeMemSize());

    	max_free_ram = sceKernelMaxFreeMemSize();

    	sprintf(max_free_ram_textbuffer,"Max. Free RAM: %d Bytes\n",max_free_ram);

	V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(other_info)
    {
	sprintf(idle_time_textbuffer,"Idle Time: %d \n",scePowerGetIdleTimer());

	V_DrawPatch (234, 135, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    }
    else
	V_DrawPatch (234, 135, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

    if(timer_info)
	V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
    else
	V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
*/
    if(classicmode)
    {
	if(coordinates_info)
	    V_DrawPatch (234, 78, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
	else
	    V_DrawPatch (234, 78, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));

	if(version_info)
	    V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));
	else
	    V_DrawPatch (234, 95, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
    }
    else
    {
	M_WriteText(80, 90, DEH_String("COORDINATES"));
	M_WriteText(80, 100, DEH_String("VERSION INFO"));

	if(coordinates_info)
	    M_WriteText(220, 90, DEH_String("ON"));
	else
	    M_WriteText(220, 90, DEH_String("OFF"));

	if(version_info)
	    M_WriteText(220, 100, DEH_String("ON"));
	else
	    M_WriteText(220, 100, DEH_String("OFF"));
    }
}

//
// Change Sfx & Music volumes
//
// haleyjd 08/29/10: [STRIFE]
// * Changed title graphic coordinates
// * Added voice volume and sensitivity sliders
//
void M_DrawSound(void)
{
    if(classicmode)
    {
	V_DrawPatch (60, 5, W_CacheLumpName(DEH_String("M_SNDSET"), PU_CACHE));

	M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
                 16,sfxVolume);

	M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
                 16,musicVolume);

	if(fsize != 9934413)	// HACK: NOT FOR SHARE 1.0 & 1.1
	{
	    if(disable_voices == 1)
		V_DrawPatch (207, 106, W_CacheLumpName(DEH_String("M_SETOFF"), PU_CACHE));
	    else
		V_DrawPatch (207, 106, W_CacheLumpName(DEH_String("M_SETON"), PU_CACHE));

	    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(voice_vol+1),
		    16,voiceVolume);

	    V_DrawPatchDirect(80, 125, W_CacheLumpName(DEH_String("M_VOIVOL"), PU_CACHE));

//	    if(devparm)
	    {
		V_DrawPatchDirect(80, 168, W_CacheLumpName(DEH_String("M_ENGINE"), PU_CACHE));

		if(mus_engine == 1)
		    V_DrawPatch (166, 168, W_CacheLumpName(DEH_String("M_INTERN"), PU_CACHE));
		else
		    V_DrawPatch (159, 168, W_CacheLumpName(DEH_String("M_EXTERN"), PU_CACHE));

		if(mus_engine < 1)
		    mus_engine = 1;
		else if(mus_engine > 2)
		    mus_engine = 2;
	    }
	}
    }
    else
    {
	V_DrawPatch (60, 20, W_CacheLumpName(DEH_String("M_SNDSET"), PU_CACHE));

	M_WriteText(30, 55, DEH_String("SOUND VOLUME"));
	M_WriteText(30, 75, DEH_String("MUSIC VOLUME"));
	M_WriteText(30, 95, DEH_String("VOICES"));

	M_DrawThermoSVE(SoundDef.x+65,SoundDef.y+2+LINEHEIGHT*(sfx_vol+1),
                 16,sfxVolume);

	M_DrawThermoSVE(SoundDef.x+65,SoundDef.y-16+LINEHEIGHT*(music_vol+1),
                 16,musicVolume);

	if(fsize != 9934413)	// HACK: NOT FOR SHARE 1.0 & 1.1
	{
	    if(disable_voices == 1)
		M_WriteText(260, 95, DEH_String("OFF"));
	    else
		M_WriteText(265, 95, DEH_String("ON"));

	    M_DrawThermoSVE(SoundDef.x+65,SoundDef.y-43+LINEHEIGHT*(voice_vol+1),
		    16,voiceVolume);

	    M_WriteText(30, 105, DEH_String("VOICE VOLUME"));

//	    if(devparm)
	    {
		M_WriteText(30, 125, DEH_String("MUSIC ENGINE"));

		if(mus_engine == 1)
		    M_WriteText(187, 125, DEH_String("INTERNAL OPL"));
		else
		    M_WriteText(183, 125, DEH_String("EXTERNAL OGG "));

		if(mus_engine < 1)
		    mus_engine = 1;
		else if(mus_engine > 2)
		    mus_engine = 2;
	    }
	}
    }
/*
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT+10*(sfx_mouse+1),
                 16,mouseSensitivity);
*/
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (sfxVolume)
            sfxVolume--;
        break;
    case 1:
        if (sfxVolume < 15)
            sfxVolume++;
        break;
    }

    S_SetSfxVolume(sfxVolume * 8);
}

void M_Voices(int choice)
{
    switch(choice)
    {
    case 0:
        if (disable_voices)
            disable_voices--;
        break;
    case 1:
        if (disable_voices < 1 && (STRIFE_1_0_REGISTERED || STRIFE_1_X_REGISTERED))
            disable_voices++;
#ifdef SHAREWARE
	else if(STRIFE_1_0_SHAREWARE || STRIFE_1_1_SHAREWARE)	// FOR PSP: (NOT POSSIBLE IN SHAREWARE)
	    M_StartMessage(DEH_String("VOICES CAN'T BE DISABLED IN DEMO"), NULL, false);
#endif
        break;
    }
}

//
// M_VoiceVol
//
// haleyjd 08/29/10: [STRIFE] New function
// Sets voice volume level.
//
void M_VoiceVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (voiceVolume)
            voiceVolume--;
        break;
    case 1:
        if (voiceVolume < 15)
            voiceVolume++;
        break;
    }

    S_SetVoiceVolume(voiceVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (musicVolume)
            musicVolume--;
        break;
    case 1:
        if (musicVolume < 15)
            musicVolume++;
        break;
    }

    S_SetMusicVolume(musicVolume * 8);
}

void M_Engine(int choice)
{
    switch(choice)
    {
    case 0:
        if(mus_engine > 1)
	{
            snd_musicdevice = SNDDEVICE_SB;
	    mus_engine--;
	}
        break;
    case 1:
        if(mus_engine < 2)
	{
            snd_musicdevice = SNDDEVICE_GENMIDI;
	    mus_engine++;
	}
        break;
    }
}

void DetectState(void)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_DEAD)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED - YOU'RE DEAD"), NULL, true);
    }
    else if(/*!netgame && */demoplayback && gamestate == GS_LEVEL
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, true);
    }
    else if(/*!netgame && */demoplayback && gamestate == GS_LEVEL
	&& players[consoleplayer].playerstate == PST_DEAD)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, true);
    }
    else if(/*!netgame && */demoplayback && gamestate != GS_LEVEL)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, true);
    }
    else if(/*!netgame && */!demoplayback && gamestate != GS_LEVEL)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED IN DEMO MODE"), NULL, true);
    }
/*
    else if(netgame)
    {
	M_StartMessage(DEH_String("CHEATING NOT ALLOWED FOR NET GAME"), NULL, false);
    }
*/
    if(gameskill == sk_nightmare)
    {
	M_StartMessage(DEH_String("CHEATING DISABLED - NIGHTMARE SKILL"), NULL, true);
    }
}

void M_Map(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
	static player_t* player;
	player = &players[consoleplayer];

	P_GivePower(player, pw_allmap);

	players[consoleplayer].message = DEH_String("Full Map Added");
    }
    DetectState();
}

void M_God(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
        players[consoleplayer].cheats ^= CF_GODMODE;
	if (players[consoleplayer].cheats & CF_GODMODE)
	{
            if (players[consoleplayer].mo)
                players[consoleplayer].mo->health = 100;
            players[consoleplayer].health = deh_god_mode_health;
            players[consoleplayer].st_update = true; // [STRIFE]
	    players[consoleplayer].message = DEH_String(STSTR_DQDON);
    	}
    	else
    	{
            players[consoleplayer].st_update = true; // [STRIFE]
	    players[consoleplayer].message = DEH_String(STSTR_DQDOFF);
    	}
    }
    DetectState();
}

void M_Noclip(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
        players[consoleplayer].cheats ^= CF_NOCLIP;
    	if (players[consoleplayer].cheats & CF_NOCLIP)
    	{
	    players[consoleplayer].message = DEH_String(STSTR_NCON);
            players[consoleplayer].mo->flags |= MF_NOCLIP;
    	}
    	else
    	{
	    players[consoleplayer].message = DEH_String(STSTR_NCOFF);
	    players[consoleplayer].mo->flags &= ~MF_NOCLIP;
    	}
    }
    DetectState();
}

void M_Weapons(int choice)
{
    int i;

    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	// [STRIFE]: "BOOMSTIX" cheat for all normal weapons
    	players[consoleplayer].armorpoints = deh_idkfa_armor;
    	players[consoleplayer].armortype = deh_idkfa_armor_class;

    	for (i = 0; i < NUMWEAPONS; i++)
	    if(!isdemoversion || weaponinfo[i].availabledemo)
            	players[consoleplayer].weaponowned[i] = true;
        
    	// Takes away the Sigil, even if you already had it...
    	players[consoleplayer].weaponowned[wp_sigil] = false;

    	for (i=0;i<NUMAMMO;i++)
	    players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];

    	players[consoleplayer].message = DEH_String(STSTR_FAADDED);

	text_flag_boomstix = true;
    }
    DetectState();
}

void M_Keys(int choice)
{
    int i;

    // villsa [STRIFE]: "JIMMY" cheat for all keys
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	if(players[consoleplayer].cards[FIRSTKEYSETAMOUNT - 1])
    	{
            if(players[consoleplayer].cards[NUMCARDS - 1] || isdemoversion)
            {
            	for(i = 0; i < NUMCARDS; i++)
                    players[consoleplayer].cards[i] = false;

            	players[consoleplayer].message = DEH_String("Keys removed");
            }
            else
            {
            	for(i = 0; i < NUMCARDS; i++)
                    players[consoleplayer].cards[i] = true;

            	players[consoleplayer].message = DEH_String("Even More Cheater Keys Added");
            }
    	}
    	else
    	{
            for(i = 0; i < FIRSTKEYSETAMOUNT; i++)
            	players[consoleplayer].cards[i] = true;

            players[consoleplayer].message = DEH_String("Cheater Keys Added");
    	}
    }
    DetectState();
}

void M_Artifacts(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	players[consoleplayer].message = DEH_String("YOU GOT THE MIDAS TOUCH, BABY");
    	P_GiveItemToPlayer(&players[0], SPR_HELT, MT_TOKEN_TOUGHNESS);
	text_flag_donnytrump = true;
    }
    DetectState();
}

void M_Stonecold(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	stonecold = true;
    	players[consoleplayer].message = DEH_String("Kill 'em.  Kill 'em All");
	text_flag_stonecold = true;
    }
    DetectState();
}

void M_Lego(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	if(!isdemoversion)
    	{
            players[consoleplayer].st_update = true;
            if(players[consoleplayer].weaponowned[wp_sigil])
            {
    	        if(++players[consoleplayer].sigiltype > 4)
    	        {
                    players[consoleplayer].sigiltype = -1;
                    players[consoleplayer].pendingweapon = wp_fist;
                    players[consoleplayer].weaponowned[wp_sigil] = false;
                }
            }
            else
            {
                players[consoleplayer].weaponowned[wp_sigil] = true;
                players[consoleplayer].sigiltype = 0;
    	    }
    	    // BUG: This brings up a bad version of the Sigil (sigiltype -1) which
    	    // causes some VERY interesting behavior, when you type LEGO for the
    	    // sixth time. This shouldn't be done when taking it away, and yet it
    	    // is here... verified with vanilla.
//    	    players[consoleplayer].pendingweapon = wp_sigil;	// FOR PSP: DEACTIVATED TO BE ABLE... 
    	}							// ...TO GET BACK TO THE FISTS WHEN...
    }								// ...USING 'LEGO' CHEAT IN CHEAT-MENU
    DetectState();
}

void M_Pumpup(int choice)
{
    int i;

    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	for(i = 0; i < 0 + 3; ++i)
    	{
    	    // [STRIFE]: Handle berserk, invisibility, and envirosuit
//  	      if(cht_CheckCheat(&cheat_powerup[i], ev->data2))
    	    {
    	        if(players[consoleplayer].powers[i])
    	            players[consoleplayer].powers[i] = (i != 1);
    	        else
    	            P_GivePower(&players[0], i);
    	    }
    	}

    	// [STRIFE]: PUMPUPH gives medical inventory items
    	P_GiveItemToPlayer(&players[0], SPR_STMP, MT_INV_MED1);
    	P_GiveItemToPlayer(&players[0], SPR_MDKT, MT_INV_MED2);
    	P_GiveItemToPlayer(&players[0], SPR_FULL, MT_INV_MED3);

    	// [STRIFE]: PUMPUPP gives backpack
    	if(!&players[consoleplayer].backpack)
    	{
	    for(i = 0; i < NUMAMMO; ++i)
	    players[consoleplayer].maxammo[i] = 2 * players[consoleplayer].maxammo[i];
    	}
    	players[consoleplayer].backpack = true;

    	for(i = 0; i < NUMAMMO; ++i)
	    P_GiveAmmo(&players[0], i, 1);

    	// [STRIFE]: PUMPUPS gives stamina and accuracy upgrades
    	P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_STAMINA);
    	P_GiveItemToPlayer(&players[0], SPR_TOKN, MT_TOKEN_NEW_ACCURACY);

    	// [STRIFE] PUMPUPT gives targeter
    	P_GivePower(&players[0], pw_targeter);
    	players[consoleplayer].message = DEH_String("you got the stuff!");
	text_flag_pumpup = true;
    }
    DetectState();
}

void M_Topo(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	cheating = (cheating+1) % 3;
    	cheeting = (cheeting+1) % 3;
    }
    DetectState();
}
/*
void M_Gripper(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	players[consoleplayer].cheats ^= CF_NOMOMENTUM;
    	if(players[consoleplayer].cheats & CF_NOMOMENTUM)
    	    players[consoleplayer].message = DEH_String("STEALTH BOOTS ON");
    	else
    	    players[consoleplayer].message = DEH_String("STEALTH BOOTS OFF");
    }
    DetectState();
}

void M_GPS(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	// [STRIFE] 'GPS' for player position
    	static char buf[52];
    	M_snprintf(buf, sizeof(buf),
    	           "ang=0x%x;x,y=(0x%x,0x%x)",
    	           players[consoleplayer].mo->angle,
    	           players[consoleplayer].mo->x,
    	           players[consoleplayer].mo->y);
    	players[consoleplayer].message = buf;
    }
    DetectState();
}

void M_Dots(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	devparm = !devparm;
    	if (devparm)
    	    players[consoleplayer].message = DEH_String("devparm ON");
    	else
    	    players[consoleplayer].message = DEH_String("devparm OFF");
    }
    DetectState();
}
*/

void M_Rift(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
    	    if   ((map_cheat >=  2 && !isdemoversion)
	    ||    (map_cheat >= 33 && isdemoversion))
		   map_cheat--;
    	    break;
    	case 1:
	    if    (map_cheat <  36)	// FOR PSP: STRIFE v1.0 Shareware includes MAP35 & 36, but...
	    {				// ...the IWAD is missing some textures (maybe DEV-MAPS?)
		if(map_cheat ==  0 && isdemoversion)
		   map_cheat  = 32;
    	    	   map_cheat++;
		if(map_cheat == 35 && !isdemoversion)
		   map_cheat =  34;
	    }
    	    break;
    	}
    }
    DetectState();
}

void M_RiftNow(int choice)
{
    if(/*!netgame && */!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	// So be it.
    	players[consoleplayer].message = DEH_String(STSTR_CLEV);
    	G_RiftExitLevel(map_cheat, 0, players[consoleplayer].mo->angle);

	music_cheat_used = false;
	M_ClearMenus(0);
	text_flag_warping = true;
    }
    DetectState();
}
/*
					// THIS WAS SUPPOSED TO BE FOR THE PSP SOURCE PORT, BUT...
void M_Scoot(int choice)		// ...IT DOESN'T WORK AS EXPECTED (YET)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
    	    break;
    	case 1:
    	    break;
    	}
    }
    DetectState();
}
					// THIS WAS SUPPOSED TO BE FOR THE PSP SOURCE PORT, BUT...
void M_ScootNow(int choice)		// ...IT DOESN'T WORK AS EXPECTED (YET)
{
    if(!demoplayback && gamestate == GS_LEVEL && gameskill != sk_nightmare
	&& players[consoleplayer].playerstate == PST_LIVE)
    {
    	// BUG: should be <= 9. Shouldn't do anything bad though...
    	if(spot <= 10) 
    	{
    	    players[consoleplayer].message = DEH_String("Spawning to spot");
	    G_RiftCheat(spot);
    	}
    }
    DetectState();
}
*/
void M_Spin(int choice)
{
    if(!demoplayback && gamestate == GS_LEVEL
	&& gameskill != sk_nightmare && players[consoleplayer].playerstate == PST_LIVE)
    {
    	switch(choice)
    	{
    	case 0:
    	    if(musnum >  1)
	    {
		musnum--;
		if(isdemoversion)
		{
		    if(musnum == 34)
			musnum = 24;
		    else if(musnum == 23)
			musnum = 6;
		    else if(musnum == 4)
			musnum = 3;
		    else if(musnum == 2)
			musnum = 1;
		}
		else
		{
		    if(musnum == 32)
			musnum = 27;
		    else if(musnum == 23)
			musnum = 22;
		    else if(musnum == 20)
			musnum = 18;
		    else if(musnum == 16)
			musnum = 14;
		    else if(musnum == 12)
			musnum = 11;
		}
	    }
    	    break;
    	case 1:
	    if (musnum < 37 && isdemoversion)
	    {					// 2 MUSIC FILES ARE STILL MISSING HERE (DEMO HAS 10)
		musnum++;
		if(musnum == 2)
		    musnum = 3;
		else if(musnum == 4)
		    musnum = 5;
		else if(musnum == 7)
		    musnum = 24;
		else if(musnum == 25)
		    musnum = 35;
	    }
	    else if(musnum < 34 && !isdemoversion)
	    {
		musnum++;
		if (musnum == 12)
		    musnum = 13;
		else if (musnum == 15)
		    musnum = 17;
		else if (musnum == 19)
		    musnum = 21;
		else if (musnum == 23)
		    musnum = 24;
		else if (musnum == 28)
		    musnum = 33;
	    }
    	    break;
    	}
	music_cheat_used = true;
        players[consoleplayer].message = DEH_String(STSTR_MUS);
        S_ChangeMusic(musnum, 1);
    }
    DetectState();
}

void M_DrawCheats(void)
{
    V_DrawPatch (110, 0, W_CacheLumpName(DEH_String("M_CHEATS"), PU_CACHE));

    M_WriteText(52, 20, DEH_String("SHOW NON-VISITED MAP AREAS"));

    if(!isdemoversion)
	M_WriteText(52, 30, DEH_String("OMNIPOTENT"));
    else
	M_WriteText(52, 30, DEH_String("IBGOD"));

    M_WriteText(150, 30, DEH_String("(GOD)"));

    if (players[consoleplayer].cheats & CF_GODMODE)
	M_WriteText(247, 30, DEH_String("ON"));
    else
	M_WriteText(241, 30, DEH_String("OFF"));

    if(!isdemoversion)
	M_WriteText(52, 40, DEH_String("ELVIS"));
    else
	M_WriteText(52, 40, DEH_String("SPIRIT"));

    M_WriteText(150, 40, DEH_String("(NOCLIP)"));

    if (players[consoleplayer].cheats & CF_NOCLIP)
	M_WriteText(247, 40, DEH_String("ON"));
    else
	M_WriteText(241, 40, DEH_String("OFF"));

    if(!isdemoversion)
	M_WriteText(52, 50, DEH_String("BOOMSTIX"));
    else
	M_WriteText(52, 50, DEH_String("GUNS"));

    M_WriteText(150, 50, DEH_String("(GUNS)"));

    if(text_flag_boomstix && !classicmode)
	M_WriteText(226, 50, DEH_String("GIVEN"));

    if(!isdemoversion)
	M_WriteText(52, 60, DEH_String("JIMMY"));
    else
	M_WriteText(52, 60, DEH_String("OPEN"));

    if(players[consoleplayer].cards[FIRSTKEYSETAMOUNT - 1])
    {
        if(players[consoleplayer].cards[NUMCARDS - 1] || isdemoversion)
	    M_WriteText(259, 60, DEH_String("2"));
        else
	    M_WriteText(261, 60, DEH_String("1"));
    }
    else
	M_WriteText(257, 60, DEH_String("0"));

    M_WriteText(150, 60, DEH_String("(KEYS)"));

    if(!isdemoversion)
	M_WriteText(52, 70, DEH_String("DONNYTRUMP"));
    else
	M_WriteText(52, 70, DEH_String("MONEY"));

    M_WriteText(150, 70, DEH_String("(GOLD)"));

    if(text_flag_donnytrump && !classicmode)
	M_WriteText(226, 70, DEH_String("GIVEN"));

    if(!isdemoversion)
	M_WriteText(52, 80, DEH_String("STONECOLD"));
    else
	M_WriteText(52, 80, DEH_String("KILLEM"));

    M_WriteText(150, 80, DEH_String("(KILL ALL)"));

    if(text_flag_stonecold && !classicmode)
	M_WriteText(229, 80, DEH_String("DONE"));

    if(!isdemoversion)
    {
	M_WriteText(52, 90, DEH_String("LEGO"));
	M_WriteText(150, 90, DEH_String("(SIGIL PARTS)"));
    }
    else
	M_WriteText(52, 90, DEH_String("- NOT FOR TEASER CAMPAIGN -"));

    if(!isdemoversion && !classicmode)
    {
	if(players[consoleplayer].weaponowned[wp_sigil] == false)
	    M_WriteText(257, 90, DEH_String("0"));

	if(players[consoleplayer].sigiltype == 0 && players[consoleplayer].weaponowned[wp_sigil] == true)
	    M_WriteText(261, 90, DEH_String("1"));
	else if(players[consoleplayer].sigiltype == 1)
	    M_WriteText(259, 90, DEH_String("2"));
	else if(players[consoleplayer].sigiltype == 2)
	    M_WriteText(259, 90, DEH_String("3"));
	else if(players[consoleplayer].sigiltype == 3)
	    M_WriteText(259, 90, DEH_String("4"));
	else if(players[consoleplayer].sigiltype == 4)
	    M_WriteText(259, 90, DEH_String("5"));
    }
    if(!isdemoversion)
	M_WriteText(52, 100, DEH_String("PUMPUP"));
    else
	M_WriteText(52, 100, DEH_String("LISTIT"));

    M_WriteText(150, 100, DEH_String("(ITEMS)"));

    if(text_flag_pumpup && !classicmode)
	M_WriteText(226, 100, DEH_String("GIVEN"));

    if(!isdemoversion)
	M_WriteText(52, 110, DEH_String("TOPO"));
    else
	M_WriteText(52, 110, DEH_String("IDDT"));

    M_WriteText(150, 110, DEH_String("(MAP)"));

    if(!cheating)
	M_WriteText(241, 110, DEH_String("OFF"));
    else if (cheating && cheeting!=2)
	M_WriteText(221, 110, DEH_String("WALLS"));
    else if (cheating && cheeting==2)	  
	M_WriteText(241, 110, DEH_String("ALL"));
/*
    if(!isdemoversion)
	M_WriteText(52, 120, DEH_String("GRIPPER"));
    else
	M_WriteText(52, 120, DEH_String("STIC"));

    M_WriteText(143, 120, DEH_String("(SLOW SPEED)"));
*/
    if(!isdemoversion)
	M_WriteText(52, 120, DEH_String("RIFT:"));
    else
	M_WriteText(52, 120, DEH_String("GOTO:"));

    M_WriteText(150, 120, DEH_String("(WARP TO MAP)"));

    if(map_cheat == 0)
    {
	if(!isdemoversion)
	    map_cheat =  1;
	else
	    map_cheat = 32;
    }

    M_WriteText(52, 130, maptext[map_cheat]);

    if(isdemoversion)
    {
	if(map_cheat == 32 || map_cheat == 33)
	    M_WriteText(150, 130, "(DEMO VER.)");
	else if(map_cheat == 35 || map_cheat == 36)
	    M_WriteText(150, 130, "(DEV. MAP)");
    }
    M_WriteText(52, 140, DEH_String("EXECUTE WARPING"));

    if(text_flag_warping && !classicmode)
	M_WriteText(236, 50, DEH_String("DONE"));

    if(!isdemoversion)
	M_WriteText(52, 150, DEH_String("SPIN:"));
    else
	M_WriteText(52, 150, DEH_String("IDMUS:"));

    M_WriteText(150, 150, DEH_String("(PLAY MUSIC)"));

    if (musnum == 0)
	musnum =  1;

    if(isdemoversion)
	M_WriteText(248, 150, mustextdemo[musnum]);
    else
	M_WriteText(248, 150, mustext[musnum]);
}

void M_Cheats(int choice)
{
    M_SetupNextMenu(&CheatsDef);
}

//
// M_DrawMainMenu
//
// haleyjd 08/27/10: [STRIFE] Changed x coordinate; M_DOOM -> M_STRIFE
//
void M_DrawMainMenu(void)
{
    V_DrawPatch(84, 2,
                      W_CacheLumpName(DEH_String("M_STRIFE"), PU_CACHE));
}




//
// M_NewGame
//
// haleyjd 08/31/10: [STRIFE] Changed M_NEWG -> M_NGAME
//
void M_DrawNewGame(void)
{
    V_DrawPatch(96, 14, W_CacheLumpName(DEH_String("M_NGAME"), PU_CACHE));
    V_DrawPatch(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
/*
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(NEWGAME),NULL,false);
        return;
    }
*/
    // haleyjd 09/07/10: [STRIFE] Removed Chex Quest and DOOM gamemodes
    if(gameversion == exe_strife_1_31)
       namingCharacter = true; // for 1.31 save logic
    M_SetupNextMenu(&NewDef);
}


//
//      M_Episode
//

// haleyjd: [STRIFE] Unused
// haleyjd 20141014: [SVE] restored for allowing selection of demo maps

void M_DrawEpisode(void)
{
//    V_WriteBigText("Choose Campaign", 54, 38);
    V_DrawPatch (50, 38, W_CacheLumpName(DEH_String("M_CCMPGN"), PU_CACHE));
}

void M_ChooseSkill(int choice)
{
    // haleyjd 09/07/10: Removed nightmare confirmation
    menuskill = choice;
    M_SetupNextMenu(&EpiDef); // [SVE]: episode menu
}

// haleyjd [STRIFE] Unused
// haleyjd 20141014: [SVE] restored for allowing selection of demo maps
void M_Episode(int choice)
{
    // [STRIFE]: start "Name Your Character" menu
    // [SVE]: moved to after episode selection

    if(choice > 0)
	menuepisode = true;
    else
	menuepisode = false;

    currentMenu = &NameCharDef;
    itemOn = NameCharDef.lastOn;
    M_ReadSaveStrings();
}

/*
// haleyjd [STRIFE] Unused
void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}
*/


//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawPatch(108, 15, 
                      W_CacheLumpName(DEH_String("M_OPTION"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.
/*
    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 9,screenSize);
*/
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// M_AutoUseHealth
//
// [STRIFE] New function
// haleyjd 20110211: toggle autouse health state
//
void M_AutoUseHealth(void)
{
    if(/*!netgame && */usergame)
    {
        players[consoleplayer].cheats ^= CF_AUTOHEALTH;

        if(players[consoleplayer].cheats & CF_AUTOHEALTH)
            players[consoleplayer].message = DEH_String("Auto use health ON");
        else
            players[consoleplayer].message = DEH_String("Auto use health OFF");
    }
}
/*
//
// M_ChangeShowText
//
// [STRIFE] New function
//
void M_ChangeShowText(void)
{
    dialogshowtext ^= true;

    if(dialogshowtext)
        players[consoleplayer].message = DEH_String("Conversation Text On");
    else
        players[consoleplayer].message = DEH_String("Conversation Text Off");
}
*/
//
//      Toggle messages on/off
//
// [STRIFE] Messages cannot be disabled in Strife
/*
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;

    if (!showMessages)
        players[consoleplayer].message = DEH_String(MSGOFF);
    else
        players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}
*/


//
// M_EndGame
//

#include <wiiuse/wpad.h>

u32 WaitButtons(void);

void M_EndGameResponse(int ch)
{
//    if (key != key_menu_confirm)
    if (ch != key_menu_forward || (usergame && devparm))
    {
	if(!music_cheat_used)
	{
	    if(gamestate == GS_LEVEL)
	    {
		if(!isdemoversion)
		    S_ChangeMusic(gamemap + 1, 1);		// [SVE]
		else
		{
		    if(gamemap == 32)
			S_ChangeMusic(mus_map1, 1);		// [SVE]
		    else if(gamemap == 33)
			S_ChangeMusic(mus_tavern, 1);		// [SVE]
		    else if(gamemap == 34)
			S_ChangeMusic(mus_map3, 1);		// [SVE]
		    else if(gamemap == 35)
			S_ChangeMusic(mus_fight2, 1);		// [SVE]
		    else if(gamemap == 36)
			S_ChangeMusic(mus_fmstl, 1);		// [SVE]
		}
	    }
	}
	else
	    S_ChangeMusic(musnum, 1);		// [SVE]
        return;
    }
/*
    u32 buttons = WaitButtons();

    while (true)
    {
	if(buttons & WPAD_CLASSIC_BUTTON_B)
	    break;
	else if(buttons & WPAD_CLASSIC_BUTTON_A)
	    return;
    }
*/
    currentMenu->lastOn = itemOn;
    M_ClearMenus (0);
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    text_flag_end = true;
    choice = 0;
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }
/*
    if (netgame)
    {
        M_StartMessage(DEH_String(NETEND),NULL,false);
        return;
    }
*/
    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,true);		// NOT WORKING FOR THE WII
//    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,false);	// THIS ONE WORKS FOR THE WII
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

//
// M_ReadThis2
//
// haleyjd 08/28/10: [STRIFE] Eliminated DOOM stuff.
//
void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

//
// M_ReadThis3
//
// haleyjd 08/28/10: [STRIFE] New function.
//
void M_ReadThis3(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef3);
}

/*
// haleyjd 08/28/10: [STRIFE] Not used.
void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}
*/

// haleyjd [SVE]: Implemented cast call!

void M_CheckStartCast(void);

static void M_CastCallResponse(int ch)
{
//    if (key != key_menu_confirm)
    if (ch != key_menu_forward)
        return;
    M_CheckStartCast();
}

void M_StartCast(int choice)
{
    boolean i_seejoysticks = true;

    if(usergame)
    {
        M_StartMessage(DEH_String("You have to end your game first."), NULL, false);
        return;
    }
    M_StartMessage(i_seejoysticks ? CASTPROMPTGP : CASTPROMPT, M_CastCallResponse, true);
}

#if 1
extern void F_StartCast(void);

//
// M_CheckStartCast
//
// [STRIFE] New but unused function. Was going to start a cast
//   call from within the menu system... not functional even in
//   the earliest demo version.
//
void M_CheckStartCast(void)
{
    F_StartCast();
    M_ClearMenus(0);
}
#endif

//
// M_QuitResponse
//
// haleyjd 09/11/10: [STRIFE] Modifications to start up endgame
// demosequence.
//
void M_QuitResponse(int ch)
{
    char buffer[20];

//    if (key != key_menu_confirm)
    if (ch != key_menu_forward)
        return;
/*
    u32 buttons = WaitButtons();

    while (true)
    {
	if(buttons & WPAD_CLASSIC_BUTTON_B)
	    break;
	else if(buttons & WPAD_CLASSIC_BUTTON_A)
	    return;
    }

    if(netgame)
        I_Quit();
    else
*/
    {
        int i = gametic % 8 + 1;
        S_StartMusic(mus_fast);
        DEH_snprintf(buffer, sizeof(buffer), "qfmrm%i", i);
        I_StartVoice(buffer);
        D_QuitGame(3*TICRATE + ((i == 7) * 51)); // [SVE]: fix for QFMRM7
    }
}

/*
// haleyjd 09/11/10: [STRIFE] Unused
static char *M_SelectEndMessage(void)
{
}
*/

//
// M_QuitStrife
//
// [STRIFE] Renamed from M_QuitDOOM
// haleyjd 09/11/10: No randomized text message; that's taken care of
// by the randomized voice message after confirmation.
//
void M_QuitStrife(int choice)
{
    text_flag_end = true;

    if(classicmode)
	DEH_snprintf(endstring, sizeof(endstring),
                 "Do you really want to leave?\n\n" DOSY);
    else if(!classicmode)
    {
	DEH_snprintf(endstring, sizeof(endstring),
                 "");
	quitting = true;
	curCharacter   = FE_GetCharacter();
	I_StartVoice(curCharacter->voice);
    }
//    M_StartMessage(endstring,M_QuitResponse,false);	// MODDED (WORKS FOR WII)
    M_StartMessage(endstring,M_QuitResponse,true);	// ORIGINAL (DOESN'T WORK FOR WII CONTROLS)
}

void M_WalkingSpeed(int choice)
{
    switch(choice)
    {
      case 0:
	if(forwardmove > 19)
	    forwardmove--;
	break;
      case 1:
	if(forwardmove < 47)
	    forwardmove++;
	break;
    }
}

void M_TurningSpeed(int choice)
{
    switch(choice)
    {
      case 0:
	if(turnspeed > 5)
	    turnspeed--;
	break;
      case 1:
	if(turnspeed < 10)
	    turnspeed++;
	break;
    }
}

void M_StrafingSpeed(int choice)
{
    switch(choice)
    {
      case 0:
	if(sidemove > 16)
	    sidemove--;
	break;
      case 1:
	if (sidemove < 32)
	    sidemove++;
	break;
    }
}

/*
// haleyjd [STRIFE] Unused
void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DEH_String(DETAILHI);
    else
	players[consoleplayer].message = DEH_String(DETAILLO);
}
*/

// [STRIFE] Verified unmodified
void M_SizeDisplay(int choice)
{
    switch(choice)
    {
    case 0:
        if (screenSize > 0)
        {
            screenblocks--;
            screenSize--;
        }
        break;
    case 1:
        if (screenSize < 8)
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawPatch(xx, yy, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatch(xx, yy, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }
    V_DrawPatch(xx, yy, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    // [STRIFE] +2 to initial y coordinate
    V_DrawPatch((x + 8) + thermDot * 8, y + 2,
                      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));
}

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermoSVE
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawPatch(xx + 2, yy - 2, W_CacheLumpName(DEH_String("M_SLIDEL"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatch(xx, yy - 2, W_CacheLumpName(DEH_String("M_SLIDEM"), PU_CACHE));
        xx += 8;
    }
    V_DrawPatch(xx, yy - 2, W_CacheLumpName(DEH_String("M_SLIDER"), PU_CACHE));

    // [STRIFE] +2 to initial y coordinate
    V_DrawPatch((x + 9) + thermDot * 8, y + 4,
                      W_CacheLumpName(DEH_String("M_SLIDEC"), PU_CACHE));
}

// haleyjd: These are from DOOM v0.5 and the prebeta! They drew those ugly red &
// blue checkboxes... preserved for historical interest, as not in Strife.
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 
                      W_CacheLumpName(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1,
                      W_CacheLumpName(DEH_String("M_CELL2"), PU_CACHE));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    messageLastMenuPause = menupause; // haleyjd: [SVE] forgotten by Rogue
    menuactive = true;
    menupause  = true;                // haleyjd: [SVE] ditto
    return;
}



void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    menupause  = messageLastMenuPause;  // [SVE]
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchar-subscripts"

int M_StringWidth(char* string)
{
    size_t             i;
    int             w = 0;
    int             c;

    for (i = 0;i < strlen(string);i++)
    {
        c = toupper(string[i]) - HU_FONTSTART;
        if (c < 0 || c >= HU_FONTSIZE)
            w += 4;
        else
            w += SHORT (hu_font[c]->width);
    }

    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);

    h = height;
    for (i = 0;i < strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// M_WriteText
//
// Write a string using the hu_font
// haleyjd 09/04/10: [STRIFE]
// * Rogue made a lot of changes to this for the dialog system.
//
int
M_WriteText
( int           x,
  int           y,
  const char*   string) // haleyjd: made const for safety w/dialog engine
{
    int         c;
    int         cx;
    int         cy;
    int	        w;

    const char* ch;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;

        if (!c)
            break;

        // haleyjd 09/04/10: [STRIFE] Don't draw spaces at the start of lines.
        if(c == ' ' && cx == x)
            continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11; // haleyjd 09/04/10: [STRIFE]: Changed 12 -> 11

            continue;
        }

	if(c != 123 && c != 124 && c != 125)
	    c = toupper(c) - HU_FONTSTART;

        if (c < 0 || (c>= HU_FONTSIZE && c != 123 && c != 124 && c != 125))
        {
            cx += 4;

            continue;
        }

	if(c == 123 || c == 125)
	    w = 7;
	else if(c == 124)
	    w = 10;
	else
	    w = SHORT (hu_font[c]->width);

        // haleyjd 09/04/10: [STRIFE] Different linebreak handling
//        if (cx + w > SCREENWIDTH - 20)				// CHANGED FOR HIRES
	if (cx + w > ORIGWIDTH - 20 && currentMenu != &ControlsDef)	// CHANGED FOR HIRES
        {
            cx = x;
            cy += 11;
            --ch;
        }
	else if (cx + w > ORIGWIDTH && currentMenu == &ControlsDef)	// CHANGED FOR HIRES
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
	    if(c == 123)
		V_DrawPatch(cx, cy, W_CacheLumpName(DEH_String("STCFN123"), PU_CACHE));
	    else if(c == 124)
		V_DrawPatch(cx, cy, W_CacheLumpName(DEH_String("STCFN124"), PU_CACHE));
	    else if(c == 125)
	    {
		V_DrawPatch(cx + 295, cy - 11, W_CacheLumpName(DEH_String("STCFN124"), PU_CACHE));
		V_DrawPatch(cx + 305, cy - 11, W_CacheLumpName(DEH_String("STCFN125"), PU_CACHE));
	    }
	    else
		V_DrawPatch(cx, cy, hu_font[c]);

            cx += w;
        }
    }

    // [STRIFE] Return final y coordinate.
    return cy + 12;
}

//
// M_DialogDimMsg
//
// [STRIFE] New function
// haleyjd 09/04/10: Painstakingly transformed from the assembly code, as the
// decompiler could not touch it. Redimensions a string to fit on screen, leaving
// at least a 20 pixel margin on the right side. The string passed in must be
// writable.
//
void M_DialogDimMsg(int x, int y, char *str, boolean useyfont)
{
//    int rightbound = (SCREENWIDTH - 20) - x;			// CHANGED FOR HIRES
    int rightbound = (ORIGWIDTH - 20) - x;			// CHANGED FOR HIRES
    patch_t **fontarray;  // ebp
    int linewidth = 0;    // esi
    int i = 0;            // edx
    char *message = str;  // edi
    char  bl;             // bl

    if(useyfont)
       fontarray = yfont;
    else
       fontarray = hu_font;

    bl = toupper(*message);

    if(!bl)
        return;

    // outer loop - run to end of string
    do
    {
        if(bl != '\n')
        {
            int charwidth; // eax
            int tempwidth; // ecx

            if(bl < HU_FONTSTART || bl > HU_FONTEND)
                charwidth = 4;
            else
                charwidth = SHORT(fontarray[bl - HU_FONTSTART]->width);

            tempwidth = linewidth + charwidth;

            // Test if the line still fits within the boundary...
            if(tempwidth >= rightbound)
            {
                // Doesn't fit...
                char *tempptr = &message[i]; // ebx
                char  al;                    // al

                // inner loop - run backward til a space (or the start of the
                // string) is found, subtracting width off the current line.
                // BUG: shouldn't we stop at a previous '\n' too?
                while(*tempptr != ' ' && i > 0)
                {
                    tempptr--;
                    // BUG: they didn't add the first char to linewidth yet...
                    linewidth -= charwidth; 
                    i--;
                    al = toupper(*tempptr);
                    if(al < HU_FONTSTART || al > HU_FONTEND)
                        charwidth = 4;
                    else
                        charwidth = SHORT(fontarray[al - HU_FONTSTART]->width);
                }
                // Replace the space with a linebreak.
                // BUG: what if i is zero? ... infinite loop time!
                message[i] = '\n';
                linewidth = 0;
            }
            else
            {
                // The line does fit.
                // Spaces at the start of a line don't count though.
                if(!(bl == ' ' && linewidth == 0))
                    linewidth += charwidth;
            }
        }
        else
            linewidth = 0; // '\n' seen, so reset the line width
    }
    while((bl = toupper(message[++i])) != 0); // step to the next character
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.
/*
static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}
*/
//
// CONTROL PANEL
//

//
// M_Responder
//

boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
/*
    static  int     joywait = 0;

    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
*/
    ch = -1; // will be changed to a legit char if we're going to use it here

    // Process joystick input
    // For some reason, polling ev.data for joystick input here in the menu code doesn't work when
    // using the twilight hack to launch wiidoom. At the same time, it works fine if you're using the
    // homebrew channel. I don't know why this is so for the meantime I'm polling the wii remote directly.

    WPADData *data = WPAD_Data(0);
    
    //Classic Controls
    if(data->exp.type == WPAD_EXP_CLASSIC)
    {
	if ((data->btns_d & WPAD_CLASSIC_BUTTON_UP))
	{
	    ch = key_menu_up;                                // phares 3/7/98

//	    joywait = I_GetTime() + 5;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_DOWN))
	{
	    ch = key_menu_down;                              // phares 3/7/98

//	    joywait = I_GetTime() + 5;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_LEFT))
	{
	    ch = key_menu_left;                              // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_RIGHT))
	{
	    ch = key_menu_right;                             // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_B))
	{
	    ch = key_menu_forward;                             // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}

	if ((data->btns_d & WPAD_CLASSIC_BUTTON_A))
	{
	    ch = key_menu_back;                         // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}
/*
	if (data->btns_d & WPAD_CLASSIC_BUTTON_MINUS)
	{
	    ch = key_menu_activate;                         // phares 3/7/98

//	    joywait = I_GetTime() + 10;
	}
*/
	if ((data->exp.classic.ljs.pos.y > (data->exp.classic.ljs.center.y + 50)))
	{
	    ch = key_menu_up;

//	    joywait = I_GetTime() + 5;
	}
	else if ((data->exp.classic.ljs.pos.y < (data->exp.classic.ljs.center.y - 50)))
	{
	    ch = key_menu_down;

//	    joywait = I_GetTime() + 5;
	}

	if ((data->exp.classic.ljs.pos.x > (data->exp.classic.ljs.center.x + 50)))
	{
	    ch = key_menu_right;

//	    joywait = I_GetTime() + 5;
	}
	else if ((data->exp.classic.ljs.pos.x < (data->exp.classic.ljs.center.x - 50)))
	{
	    ch = key_menu_left;

//	    joywait = I_GetTime() + 5;
	}
    }

    if (askforkey && data->btns_d)		// KEY BINDINGS
    {
	M_KeyBindingsClearControls(ev->data1);
	*doom_defaults_list[keyaskedfor + 26 + FirstKey].location = ev->data1;
	askforkey = false;
	return true;
    }

    if (askforkey && ev->type == ev_mouse)
    {
	if (ev->data1 & 1)
	    return true;
	if (ev->data1 & 2)
	    return true;
	if (ev->data1 & 4)
	    return true;
	return false;
    }

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.
/*
    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
*/
    key = -1;
/*
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
        if (ev->data3 < 0)
        {
            key = key_menu_up;
            joywait = I_GetTime() + 5;
        }
        else if (ev->data3 > 0)
        {
            key = key_menu_down;
            joywait = I_GetTime() + 5;
        }

        if (ev->data2 < 0)
        {
            key = key_menu_left;
            joywait = I_GetTime() + 2;
        }
        else if (ev->data2 > 0)
        {
            key = key_menu_right;
            joywait = I_GetTime() + 2;
        }

        if (ev->data1&1)
        {
            key = key_menu_forward;
            joywait = I_GetTime() + 5;
        }
        if (ev->data1&2)
        {
            key = key_menu_back;
            joywait = I_GetTime() + 5;
        }
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else

    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            mousey += ev->data3;
            if (mousey < lasty-30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty+30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            mousex += ev->data2;
            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 15;
//                mouse_fire_countdown = 5;   // villsa [STRIFE]
            }

            if (ev->data1&2)
            {
                key = key_menu_back;
                mousewait = I_GetTime() + 15;
            }
        }

        else
        {
            if (ev->type == ev_keydown)
            {
                key = ev->data1;
                ch = ev->data2;
            }
        }
//    }

    if (key == -1)
        return false;
*/
    // Save Game string input
    if (saveStringEnter)
    {
        switch(ch)
        {
/*					// FOR PSP (we don't need any other input than confirmation)
        case KEY_SQUARE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
*/
        case KEY_BACKSPACE:
            saveStringEnter = 0;
            M_StringCopy(savegamestrings[quickSaveSlot], saveOldString,
                         sizeof(savegamestrings[quickSaveSlot]));
            break;

        case KEY_ENTER:			// CHANGED FOR PSP (WAS KEY_ENTER BEFORE)
            // [STRIFE]
            saveStringEnter = 0;
            if(gameversion == exe_strife_1_31 && !namingCharacter)
            {
               // In 1.31, we can be here as a result of normal saving again,
               // whereas in 1.2 this only ever happens when naming your
               // character to begin a new game.
               M_DoSave(quickSaveSlot);
               return true;
            }
//            if (savegamestrings[quickSaveSlot][0])			// PSP workaround
                M_DoNameChar(quickSaveSlot);
            break;

        default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

//            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }

            ch = toupper(ch);

            if (ch != ' '
                && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                M_StringWidth(savegamestrings[quickSaveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[quickSaveSlot][saveCharIndex++] = ch;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
        if (messageNeedsInput)
        {
	    if (!(ch == key_menu_confirm || ch == key_menu_forward || ch == key_menu_back))
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(ch);

        menupause = false;                // [STRIFE] unpause
        menuactive = false;
        S_StartSound(NULL, sfx_mtalht);   // [STRIFE] sound
        return true;
    }

    // [STRIFE]:
    // * In v1.2 this is moved to F9 (quickload)
    // * In v1.31 it is moved to F12 with DM spy, and quicksave
    //   functionality is restored separate from normal saving
    /*
    if (devparm && key == key_menu_help)
    {
        G_ScreenShot ();
        return true;
    }
    */
/*
    // F-Keys
    if (!menuactive)
    {

        if (key == key_menu_decscreen)      // Screen size down
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(0);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(1);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();
            // haleyjd 08/29/10: [STRIFE] always ReadDef1
            currentMenu = &ReadDef1; 

            itemOn = 0;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // just saving normally, in 1.31

            if(netgame || players[consoleplayer].health <= 0 ||
                players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
            {
                // 1.31: normal save loading
                namingCharacter = false;
                M_StartControlPanel();
                M_LoadGame(0);
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                // Pre 1.31: quickload only
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = sfx_vol;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            //M_ChangeDetail(0);
            M_AutoUseHealth(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // for 1.31 save changes

            if(netgame || players[consoleplayer].health <= 0 ||
               players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuickSave();
            }
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            //M_ChangeMessages(0);
            M_ChangeShowText(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            // [STRIFE]
            // * v1.2: takes a screenshot
            // * v1.31: does quickload again
            if(gameversion == exe_strife_1_31)
            {
                namingCharacter = false;
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            else
                G_ScreenShot();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
            return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
            usegamma++;
            if (usegamma > 4)
                usegamma = 0;
            players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
            return true;
        }
        else if(gameversion == exe_strife_1_31 && key == key_spy)
        {
            // haleyjd 20130301: 1.31 moved screenshots to F12.
            G_ScreenShot();
            return true;
        }
        else if (key != 0 && key == key_menu_screenshot)
        {
            G_ScreenShot();
            return true;
        }
    }
*/
    // Pop-up menu?
    if (!menuactive)
    {
        if (ch == key_menu_activate)
        {
//	    frontend_wipe = true;
	    frontend_wipe = false;

            M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        return false;
    }

    // Keys usable within menu

    if(release_keys)
    {
	if (ch == key_menu_down)
	{
            // Move down to next item
            int oldItemOn = itemOn;

            do
            {
                if (itemOn+1 > currentMenu->numitems-1)
                {
		    if (FirstKey == FIRSTKEY_MAX)	// FOR PSP (if too many menu items) ;-)
		    {
	                itemOn = 0;
		        FirstKey = 0;
		    }
		    else
		    {
		        FirstKey++;
		    }
	        }
                else
		    itemOn++;
	        if(currentMenu == &CheatsDef && itemOn == 7 && isdemoversion)
		    itemOn++;
	        if(classicmode && currentMenu == &GameDef && itemOn == 7)
		    itemOn = 0;
            } while(currentMenu->menuitems[itemOn].status==-1);

	    if(!classicmode && currentMenu == &ScreenDef && itemOn == 3)
		itemOn = 0;

            // [SVE]: play sound only once, and only if it actually moved
            if(itemOn != oldItemOn)
                S_StartSound(NULL, sfx_pstop);

	    if(currentMenu->lastOn != itemOn)
	    {
	    	S_StartSound(NULL, sfx_pstop);
	    	if(merchantOn)
		    FE_MerchantSetState(S_MRGT_00);
	    }

            return true;
        }
        else if (ch == key_menu_up)
        {
            // Move back up to previous item
            int oldItemOn = itemOn;

            do
            {
                if (!itemOn)
	        {
		    if (FirstKey == 0)			// FOR PSP (if too many menu items) ;-)
		    {
                        itemOn = currentMenu->numitems-1;
	    	        FirstKey = FIRSTKEY_MAX;
		    }
		    else
		    {
			FirstKey--;
		    }
	        }
                else
		    itemOn--;
	        if(currentMenu == &CheatsDef && itemOn == 7 && isdemoversion)
		    itemOn--;
	        if(classicmode && currentMenu == &GameDef && itemOn == 12)
		    itemOn = 6;
            } while(currentMenu->menuitems[itemOn].status==-1);

	    if(!classicmode && currentMenu == &ScreenDef && itemOn == 4)
		itemOn = 1;

            // [SVE]: play sound only once, and only if it actually moved
            if(itemOn != oldItemOn)
                S_StartSound(NULL, sfx_pstop);

	    if(currentMenu->lastOn != itemOn)
	    {
	        S_StartSound(NULL, sfx_pstop);
	        if(merchantOn)
		    FE_MerchantSetState(S_MRGT_00);
	    }

            return true;
        }
        else if (ch == key_menu_left)
        {
            // Slide slider left

            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
            {
		S_StartSound(NULL, sfx_stnmov);

                currentMenu->menuitems[itemOn].routine(0);
            }
	    else if(currentMenu->menuitems[itemOn].routine &&
		    currentMenu->menuitems[itemOn].status == 3)
            {
		if(!classicmode)
		    S_StartSound(NULL, sfx_swtchn);
		else
		    S_StartSound(NULL, sfx_stnmov);

                currentMenu->menuitems[itemOn].routine(0);
	    }
            return true;
        }
        else if (ch == key_menu_right)
        {
            // Slide slider right

            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
            {
		S_StartSound(NULL, sfx_stnmov);

                currentMenu->menuitems[itemOn].routine(1);
            }
	    else if(currentMenu->menuitems[itemOn].routine &&
		    currentMenu->menuitems[itemOn].status == 3)
            {
		if(!classicmode)
		    S_StartSound(NULL, sfx_swtchn);
		else
		    S_StartSound(NULL, sfx_stnmov);

                currentMenu->menuitems[itemOn].routine(1);
	    }
            return true;
        }
        else if (ch == key_menu_forward)
        {
            // Activate menu item

	    if     (currentMenu != &ScreenDef && currentMenu != &GameDef && currentMenu != &CheatsDef &&
		    currentMenu != &SoundDef && currentMenu != &SystemDef && currentMenu != &DebugDef &&
		    currentMenu != &KeyBindingsDef && currentMenu != &ControlsDef)
	    {
//                frontend_wipe       = true;
                frontend_wipe       = false;
	    }

            if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status)
            {
                currentMenu->lastOn = itemOn;
                if (currentMenu->menuitems[itemOn].status == 2 && classicmode)
                {
                    currentMenu->menuitems[itemOn].routine(1);      // right arrow

		    S_StartSound(NULL, sfx_stnmov);
                }
                else if (currentMenu->menuitems[itemOn].status == 3 && !classicmode)
                {
                    currentMenu->menuitems[itemOn].routine(1);      // right arrow

		    S_StartSound(NULL, sfx_swtchn);
                }
//                else	// SERIOUSLY NOT: MESSES UP THE KEY BINDINGS MENU WHEN NOT RUNNING CLASSIC MODE
                {
                    currentMenu->menuitems[itemOn].routine(itemOn);
                    //S_StartSound(NULL, sfx_swish); [STRIFE] No sound is played here.

		    if     ((!classicmode && currentMenu != &FilesDef) ||
			    (!classicmode && currentMenu == &FilesDef && itemOn != 2))
			S_StartSound(NULL, sfx_swston);
                }
            }
            return true;
        }
        else if (ch == key_menu_activate)
        {
            // Deactivate menu
            if(gameversion == exe_strife_1_31) // [STRIFE]: 1.31 saving
                namingCharacter = false;

            if(menuindialog) // [STRIFE] - Get out of dialog engine semi-gracefully
                P_DialogDoChoice(-1);

            currentMenu->lastOn = itemOn;
            M_ClearMenus (0);
            S_StartSound(NULL, sfx_mtalht); // villsa [STRIFE]: sounds

            return true;
        }
        else if (ch == key_menu_back)
        {
            // Go back to previous menu

	    if(currentMenu != &MainDef)
	    {
//		frontend_wipe       = true;
		frontend_wipe       = false;
	    }

            currentMenu->lastOn = itemOn;
            if (currentMenu->prevMenu)
            {
                currentMenu = currentMenu->prevMenu;
                itemOn = currentMenu->lastOn;

		if(classicmode)
		    S_StartSound(NULL, sfx_swtchn);
		else
		    S_StartSound(NULL, sfx_swston);

		text_flag_boomstix = false;
		text_flag_donnytrump = false;
		text_flag_stonecold = false;
		text_flag_pumpup = false;
		text_flag_warping = false;
		text_flag_end = false;
            }
            return true;
        }

        // Keyboard shortcut?
        // Vanilla Strife has a weird behavior where it jumps to the scroll bars
        // when certain keys are pressed, so emulate this.

        else if (ch != 0/* || IsNullKey(key)*/)
        {
            // Keyboard shortcut?

            for (i = itemOn+1;i < currentMenu->numitems;i++)
            {
                if (currentMenu->menuitems[i].alphaKey == ch)
                {
                    itemOn = i;
                    S_StartSound(NULL, sfx_pstop);
                    return true;
                }
            }

            for (i = 0;i <= itemOn;i++)
            {
                if (currentMenu->menuitems[i].alphaKey == ch)
                {
                    // [SVE]: sound only if actually moves
                    if(i != itemOn)
                        S_StartSound(NULL, sfx_pstop);
                    itemOn = i;
                    return true;
                }
            }
        }
    }
    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;
/*    
    if(!classicmode && !menuindialog)
	S_ChangeMusic(mus_panthr, 1);	// [SVE]
*/
    menuactive = 1;
    menupause = true;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC

    if(!menuindialog)
//	frontend_wipe       = true;
	frontend_wipe       = false;

    text_flag_boomstix = false;
    text_flag_donnytrump = false;
    text_flag_stonecold = false;
    text_flag_pumpup = false;
    text_flag_warping = false;
    text_flag_end = false;
    quitting = false;
}

#include "st_stuff.h"
#include <SDL/SDL.h>

char			fpsDisplay[100];

u64 GetTicks(void)
{
    return (u64)SDL_GetTicks();
}

void M_FPSCounter(int FramesPerSecond)
{
    int tickfreq = 1000;

    static int fpsframecount = 0;
    static u64 fpsticks;

    fpsframecount++;

    if(GetTicks() >= fpsticks + tickfreq)
    {
	fps = fpsframecount;
	fpsframecount = 0;
	fpsticks = GetTicks();
    }
    sprintf( fpsDisplay, "FPS: %d", fps );

    if(FramesPerSecond)
    {
	M_WriteText(0, 30, fpsDisplay);
    }

    BorderNeedRefresh = true;
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    char               *name;
    int			start;

    if(menuactive && !classicmode && !menuindialog)
    {
	if(!cast_running)
	{
	    // draw background
	    if (currentMenu == &OptionsDef || currentMenu == &ControlsDef || currentMenu == &SoundDef ||
		    currentMenu == &SystemDef || currentMenu == &GameDef || currentMenu == &ScreenDef ||
		    currentMenu == &KeyBindingsDef)
		FE_DrawBackground(currentMenu->background + 1);
	    else if(currentMenu == &FilesDef || currentMenu == &LoadDef || currentMenu == &SaveDef ||
		    currentMenu == &CheatsDef)
		FE_DrawBackground(currentMenu->background + 2);
	    else
	    {
		if(!inhelpscreens)
		    FE_DrawBackground(currentMenu->background);
		else
		{
		    frontend_wipe = false;
		    wipe = false;

		    if(isdemoversion)
			V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("HELP0"), PU_CACHE));
		    else
			V_DrawPatch(0, 0, W_CacheLumpName(DEH_String("PANEL0"), PU_CACHE));
		}
	    }
	}

	if(frontend_wipe)
	{
            frontend_wipe = false;
//            wipe = true;
            wipe = false;
	}

	if((currentMenu != &CheatsDef && !cast_running) || (currentMenu == &CheatsDef && itemOn != 13 &&
		gamestate == GS_LEVEL && !cast_running))
	    S_ChangeMusic(mus_panthr, 1);	// [SVE]

	if (currentMenu == &MainDef && !text_flag_end)
	    HUlib_drawYellowText(100, 50,	"VETERAN EDITION *LITE*");

	if ((currentMenu != &OptionsDef && currentMenu != &MainDef && currentMenu != &KeyBindingsDef &&
	     currentMenu != &LoadDef && currentMenu != &SaveDef && currentMenu != &NewDef &&
	     currentMenu != &EpiDef && currentMenu != &FilesDef && currentMenu != &NameCharDef) ||
	    (currentMenu == &EpiDef && itemOn == 1))
	{
            FE_DrawBox(0, 160, SCREENWIDTH >> hires, 40);	// ADDED HIRES SUPPORT
            FE_DrawMerchant(FE_MERCHANT_X, FE_MERCHANT_Y);

	    if(currentMenu == &EpiDef && itemOn == 1)
	    {
		HUlib_drawYellowText(4, 164,	"THIS MODE TRIES TO EMULATE THE BEHAVIOR OF");
		HUlib_drawYellowText(4, 177,	"THE STRIFE TEASER DEMO AS CLOSE TO THE");
		HUlib_drawYellowText(4, 190,	"ORIGINAL AS POSSIBLE.");
	    }

	    if(currentMenu == &CheatsDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"Makes all map areas visible whether they");
		HUlib_drawYellowText(4, 177,	"have been visited before or not.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 1)
	    {
		HUlib_drawYellowText(4, 164,	"This cheat will turn on / off invincibility");
		HUlib_drawYellowText(4, 177,	"(GOD MODE).");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 2)
	    {
		HUlib_drawYellowText(4, 164,	"No clipping for walls: you are able to walk");
		HUlib_drawYellowText(4, 177,	"right through them.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 3)
	    {
		HUlib_drawYellowText(4, 164,	"Maximum fire power will be given to you.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 4)
	    {
		HUlib_drawYellowText(4, 164,	"Grab all keys that are available in the");
		HUlib_drawYellowText(4, 177,	"whole game (you need to trigger this");
		HUlib_drawYellowText(4, 190,	"multiple times).");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 5)
	    {
		HUlib_drawYellowText(4, 164,	"You will gain lots of gold using this cheat.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 6)
	    {
		HUlib_drawYellowText(4, 164,	"DEATH TO ALL OF THEM (excluding NPC'S like");
		HUlib_drawYellowText(4, 177,	"barkeepers and figures that effect gameplay).");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 7)
	    {
		HUlib_drawYellowText(4, 164,	"Here you can choose the type of sigil that");
		HUlib_drawYellowText(4, 177,	"you wanna use.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 8)
	    {
		HUlib_drawYellowText(4, 164,	"This will give all items to your inventory.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 9)
	    {
		HUlib_drawYellowText(4, 164,	"Show / hide items and enemies on the");
		HUlib_drawYellowText(4, 177,	"automap or draw walls only.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 10)
	    {
		HUlib_drawYellowText(4, 164,	"Choose any map you like...");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 12)
	    {
		HUlib_drawYellowText(4, 164,	"...and do warp to it.");
	    }
	    else if(currentMenu == &CheatsDef && itemOn == 13)
	    {
		HUlib_drawYellowText(4, 164,	"Plays a different music than the actual one.");
	    }

	    if(currentMenu == &ControlsDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"Walking speed is being changed to your");
		HUlib_drawYellowText(4, 177,	"needs using this slider.");
	    }
	    else if(currentMenu == &ControlsDef && itemOn == 2)
	    {
		HUlib_drawYellowText(4, 164,	"Need faster turning? Adjust it right here");
		HUlib_drawYellowText(4, 177,	"with this slider.");
	    }
	    else if(currentMenu == &ControlsDef && itemOn == 4)
	    {
		HUlib_drawYellowText(4, 164,	"This slider will take care of the speed that");
		HUlib_drawYellowText(4, 177,	"your player uses for strafing.");
	    }
	    else if(currentMenu == &ControlsDef && itemOn == 6)
	    {
		HUlib_drawYellowText(4, 164,	"Change the type of how freelook is assigned:");
		HUlib_drawYellowText(4, 177,	"Normal, inversed or even no freelook (off).");
	    }
	    else if(currentMenu == &ControlsDef && itemOn == 7)
	    {
		HUlib_drawYellowText(4, 164,	"With this setting you will be able to adjust");
		HUlib_drawYellowText(4, 177,	"the speed of the freelook feature.");
	    }
	    else if(currentMenu == &ControlsDef && itemOn == 9)
	    {
		HUlib_drawYellowText(4, 164,	"Here you can define your personal key");
		HUlib_drawYellowText(4, 177,	"bindings to your likes for the best gaming");
		HUlib_drawYellowText(4, 190,	"experience.");
	    }

	    if(currentMenu == &DebugDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"This function draws the player coordinates");
		HUlib_drawYellowText(4, 177,	"static to the screen while playing any map.");
	    }
	    else if(currentMenu == &DebugDef && itemOn == 1)
	    {
		HUlib_drawYellowText(4, 164,	"This function will draw the version of the");
		HUlib_drawYellowText(4, 177,	"actual game binary you are playing.");
	    }

	    if(currentMenu == &ScreenDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"This function enlightens the screen more,");
		HUlib_drawYellowText(4, 177,	"depending on how much brightness you need.");
	    }
	    else if(currentMenu == &ScreenDef && itemOn == 2)
	    {
		HUlib_drawYellowText(4, 164,	"This function will draw a screen border the");
		HUlib_drawYellowText(4, 177,	"more you reduce the size.");
	    }

	    if(currentMenu == &SystemDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"This function draws a static frames per");
		HUlib_drawYellowText(4, 177,	"second counter to the screen output.");
	    }
	    else if(currentMenu == &SystemDef && itemOn == 1)
	    {
		HUlib_drawYellowText(4, 164,	"This function draws the display ticker to");
		HUlib_drawYellowText(4, 177,	"the lower right of the screen, showing the");
		HUlib_drawYellowText(4, 190,	"actual CPU performance of the game.");
	    }

	    if(currentMenu == &SoundDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"Volume of digital sound effects, excluding");
		HUlib_drawYellowText(4, 177,	"voice acting.");
	    }
	    else if(currentMenu == &SoundDef && itemOn == 2)
	    {
		HUlib_drawYellowText(4, 164,	"Volume of music.");
	    }
	    else if(currentMenu == &SoundDef && itemOn == 4)
	    {
		HUlib_drawYellowText(4, 164,	"Turn all kind of voices on or off.");
	    }
	    else if(currentMenu == &SoundDef && itemOn == 5)
	    {
		HUlib_drawYellowText(4, 164,	"Volume of voices.");
	    }
	    else if(currentMenu == &SoundDef && itemOn == 7)
	    {
		HUlib_drawYellowText(4, 164,	"Select external OGG file playback or");
		HUlib_drawYellowText(4, 177,	"emulated OPL music. You must restart for");
		HUlib_drawYellowText(4, 190,	"this setting to take effect.");
	    }

	    if(currentMenu == &GameDef && itemOn == 0)
	    {
		HUlib_drawYellowText(4, 164,	"The good old automap grid will be enabled");
		HUlib_drawYellowText(4, 177,	"with this setting.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 1)
	    {
		HUlib_drawYellowText(4, 164,	"For those who like it more special, the");
		HUlib_drawYellowText(4, 177,	"automap will be rotating around the player");
		HUlib_drawYellowText(4, 190,	"while turning.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 2)
	    {
		HUlib_drawYellowText(4, 164,	"This setting - if disabled - allows the");
		HUlib_drawYellowText(4, 177,	"player to explore the complete automap");
		HUlib_drawYellowText(4, 190,	"using the left controller joystick.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 3)
	    {
		HUlib_drawYellowText(4, 164,	"Status messages like item pickups can be");
		HUlib_drawYellowText(4, 177,	"enabled or disabled using this setting.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 4)
	    {
		HUlib_drawYellowText(4, 164,	"This option handles the behavior of whether");
		HUlib_drawYellowText(4, 177,	"showing the text while a dialog is up and");
		HUlib_drawYellowText(4, 190,	"running or not.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 5)
	    {
		HUlib_drawYellowText(4, 164,	"If enabled, an aiming help will be drawn");
		HUlib_drawYellowText(4, 177,	"directly to the screen.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 6)
	    {
		HUlib_drawYellowText(4, 164,	"The game will behave as much as possible");
		HUlib_drawYellowText(4, 177,	"like the original \"Strife: Quest for the");
		HUlib_drawYellowText(4, 190,	"Sigil\", even including some bugs.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 7)
	    {
		HUlib_drawYellowText(4, 164,	"More blood, more gibs, more awesome. For");
		HUlib_drawYellowText(4, 177,	"those who like it \"brutal\"...");
	    }
	    else if(currentMenu == &GameDef && itemOn == 8)
	    {
		HUlib_drawYellowText(4, 164,	"Toggle drawing of the fullscreen heads-up");
		HUlib_drawYellowText(4, 177,	"display.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 9)
	    {
		HUlib_drawYellowText(4, 164,	"If enabled, your view will kick back in");
		HUlib_drawYellowText(4, 177,	"response to heavy arms fire.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 10)
	    {
		HUlib_drawYellowText(4, 164,	"Toggle automatic aim assistance on or off.");
		HUlib_drawYellowText(4, 177,	"If off, you'll need to be exactly on the");
		HUlib_drawYellowText(4, 190,	"mark to hit enemies above or below.");
	    }
	    else if(currentMenu == &GameDef && itemOn == 11)
	    {
		HUlib_drawYellowText(4, 164,	"For this game only, robots and mechanical");
		HUlib_drawYellowText(4, 177,	"enemies will respawn after 16 seconds. Be");
		HUlib_drawYellowText(4, 190,	"quick, or die in shame and dishonor!");
	    }
	    else if(currentMenu == &GameDef && itemOn == 12)
	    {
		HUlib_drawYellowText(4, 164,	"For this game only, enemies attack with");
		HUlib_drawYellowText(4, 177,	"extreme aggression! Not for the assassin");
		HUlib_drawYellowText(4, 190,	"with a faint heart.");
	    }
	}

	if(quitting)
            FE_DrawChar();
    }

    if(classicmode && currentMenu == &EpiDef && itemOn == 1)
    {
	M_WriteText(49, 120, DEH_String("THIS MODE TRIES TO EMULATE THE"));
	M_WriteText(49, 130, DEH_String("BEHAVIOR OF THE ORIGINAL STRIFE"));
	M_WriteText(49, 140, DEH_String("TEASER DEMO AS CLOSE TO THE ORIGINAL"));
	M_WriteText(49, 150, DEH_String("AS POSSIBLE, INCLUDING: SOUND,"));
	M_WriteText(49, 160, DEH_String("MUSIC, MENU, SLIDESHOWS, THE FINALE,"));
	M_WriteText(49, 170, DEH_String("PLAYER MESSAGES & DIALOGUES."));
    }

    if(classicmode && currentMenu == &SoundDef && itemOn == 7)
	M_WriteText(34, 190, "You must restart to take effect.");

    inhelpscreens = false;

    if(version_info)
	M_WriteText(65, 36, STRIFE_VERSIONTEXT);		// DISPLAYS BINARY VERSION

    if(display_fps == 1)
    {
	M_FPSCounter(1);
    }
    else if(display_fps == 0)
    {
	M_FPSCounter(0);
    }
/*
    if(memory_info)
    {
    	M_WriteText(0, 40, allocated_ram_textbuffer);
    	M_WriteText(0, 50, free_ram_textbuffer);
    	M_WriteText(0, 60, max_free_ram_textbuffer);
    }

    if(battery_info)
    {
  	M_WriteText(0, 70, unit_plugged_textbuffer);
    	M_WriteText(0, 80, battery_present_textbuffer);
    	M_WriteText(0, 90, battery_charging_textbuffer);
    	M_WriteText(0, 100, battery_charging_status_textbuffer);
    	M_WriteText(0, 110, battery_low_textbuffer);
    	M_WriteText(0, 120, battery_lifetime_percent_textbuffer);
    	M_WriteText(0, 130, battery_lifetime_int_textbuffer);
    	M_WriteText(0, 140, battery_temp_textbuffer);
    	M_WriteText(0, 150, battery_voltage_textbuffer);
    }

    if(cpu_info)
    {
    	M_WriteText(0, 160, processor_clock_textbuffer);
    	M_WriteText(0, 170, processor_bus_textbuffer);
    }

    if(other_info)
    	M_WriteText(0, 180, idle_time_textbuffer);
*/
    if(coordinates_info)
    {
    	if(gamestate == GS_LEVEL)
    	{
	    static player_t* player;
	    player = &players[consoleplayer];

    	    sprintf(map_coordinates_textbuffer, "MAP: %d - X:%5d - Y:%5d - Z:%5d",
		    gamemap,
		    player->mo->x >> FRACBITS,
		    player->mo->y >> FRACBITS,
		    player->mo->z >> FRACBITS);

            M_WriteText(40, 24, map_coordinates_textbuffer);
    	}
    }

    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
        start = 0;
        y = 100 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start,
                             sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
    }

    if (!menuactive)
        return;

    if (currentMenu->routine)
        currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    if (currentMenu->menuitems[itemOn].status == 5)		// FOR PSP (if too menu items) ;-)
	max += FirstKey;
/*
    if(!devparm && currentMenu == &SoundDef)
	currentMenu->numitems = 7;
*/
    if(!devparm && currentMenu == &OptionsDef)
	currentMenu->numitems = 5;
#ifdef SHAREWARE
    if(STRIFE_1_1_SHAREWARE && currentMenu == &SoundDef)
	currentMenu->numitems = 4;
#endif
    for (i=0;i<max;i++)
    {
        // haleyjd 20141014: [SVE] allow item->name to be a big font string
//        name = DEH_String(currentMenu->menuitems[i].name);
        menuitem_t *item = &(currentMenu->menuitems[i]);
        name = DEH_String(item->name);

//        if (name[0])
        if(*name)
        {
//            V_DrawPatch (x, y, W_CacheLumpName(name, PU_CACHE));
            int lumpnum = W_CheckNumForName(name);

            if ((lumpnum >= 0 && classicmode) || (!classicmode && (currentMenu == &MainDef	||
								   currentMenu == &OptionsDef	||
								   currentMenu == &NewDef	||
								   currentMenu == &EpiDef	||
								   currentMenu == &FilesDef)))
            {
                patch_t *p = W_CacheLumpName(name, PU_CACHE);
                item->x = x - SHORT(p->leftoffset);
                item->y = y - SHORT(p->topoffset);
                item->w = SHORT(p->width);
                item->h = SHORT(p->height);
                V_DrawPatchDirect(x, y, p);
            }
/*
            else
            {
                item->x = x;
                item->y = y+4;
                item->w = V_BigFontStringWidth(item->name);
                item->h = V_BigFontStringHeight(item->name);
                V_WriteBigText(item->name, x, y+4);
            }
*/
        }

	if((!classicmode && (currentMenu != &MainDef	 && currentMenu != &OptionsDef		&&
			     currentMenu != &NewDef	 && currentMenu != &EpiDef		&&
			     currentMenu != &FilesDef	 && currentMenu != &ControlsDef		&&
			     currentMenu != &SoundDef	 && currentMenu != &DebugDef		&&
			     currentMenu != &SystemDef	 && currentMenu != &ScreenDef		&&
			     currentMenu != &LoadDef	 && currentMenu != &SaveDef		&&
			     currentMenu != &NameCharDef && !menuindialog && !inhelpscreens))	||
	    (classicmode && (currentMenu == &CheatsDef	 || currentMenu == &KeyBindingsDef)))
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF_SMALL, currentMenu->y - 5 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(classicmode && currentMenu == &GameDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF_SMALL, currentMenu->y + 25 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(!classicmode && currentMenu == &ControlsDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x - 30 + CURSORXOFF_SMALL, currentMenu->y + 50 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(!classicmode && currentMenu == &SoundDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x - 45 + CURSORXOFF_SMALL, currentMenu->y + 20 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(!classicmode && currentMenu == &DebugDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + 35 + CURSORXOFF_SMALL, currentMenu->y + 10 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(!classicmode && currentMenu == &ScreenDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF_SMALL, currentMenu->y + 14 + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(!classicmode && currentMenu == &SystemDef)
	{
            y += LINEHEIGHT_SMALL;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + 45 + CURSORXOFF_SMALL, currentMenu->y + itemOn*LINEHEIGHT_SMALL,
		              W_CacheLumpName(DEH_String(cursorNameSmall[whichCursor]),
		                              PU_CACHE));
	}
	else if(classicmode && currentMenu == &ControlsDef)
	{
	    y += LINEHEIGHT;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF, currentMenu->y + 5 + itemOn*LINEHEIGHT,
		              W_CacheLumpName(DEH_String(cursorName[whichCursor]),
		                              PU_CACHE));
	}
        else if ((classicmode && (currentMenu != &KeyBindingsDef&& currentMenu != &GameDef	&&
				  currentMenu != &CheatsDef))	||
		(!classicmode && (currentMenu == &MainDef	|| currentMenu == &OptionsDef	||
				  currentMenu == &NewDef	|| currentMenu == &EpiDef	||
				  currentMenu == &FilesDef	|| currentMenu == &LoadDef	||
				  currentMenu == &NameCharDef	|| currentMenu == &SaveDef	||
				  menuindialog			|| inhelpscreens)))
	{
	    y += LINEHEIGHT;
	    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
	    // DRAW SIGIL
	    V_DrawPatch(x + CURSORXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
		              W_CacheLumpName(DEH_String(cursorName[whichCursor]),
		                              PU_CACHE));
	}
    }
}


//
// M_ClearMenus
//
// haleyjd 08/28/10: [STRIFE] Added an int param so this can be called by menus.
//         09/08/10: Added menupause.
//
void M_ClearMenus (int choice)
{
    extern int slideshow_state;

    choice = 0;     // haleyjd: for no warning; not from decompilation.
    menuactive = 0;
    menupause = 0;
    quitting = false;

//    if(!classicmode && !menuindialog)
    {
	if(!music_cheat_used)
	{
	    if(gamestate == GS_FINALE && !cast_running)
	    {
		if(slideshow_state == SLIDE_EXITHACK || slideshow_state == SLIDE_PROGRAMMER1 ||
			slideshow_state == SLIDE_SIGIL1)
		    S_ChangeMusic(mus_dark, 1);			// ADDED FOR DEMO
		else if(slideshow_state >= 17 && slideshow_state <= 19)
		    S_ChangeMusic(mus_end, 1);			// ADDED FOR DEMO
		else if(slideshow_state >= 14 && slideshow_state < 17)
		    S_ChangeMusic(mus_sad, 1);			// ADDED FOR DEMO
		else if(slideshow_state >= 10 && slideshow_state < 14)
		    S_ChangeMusic(mus_happy, 1);		// ADDED FOR DEMO
		else if(slideshow_state == 25)
		    S_ChangeMusic(mus_drone, 1);		// ADDED FOR DEMO
		else if(slideshow_state == 26 || (slideshow_state >= -2 && slideshow_state < -1))
		    S_ChangeMusic(mus_fast, 1);			// ADDED FOR DEMO

		if(players[0].health <= 0 && slideshow_state == 14)  // Bad ending 
		    S_ChangeMusic(mus_dark, 1);			// ADDED FOR DEMO
		else
		{
		    if     ((players[0].questflags & QF_QUEST25) && // Converter destroyed
			    (players[0].questflags & QF_QUEST27 && slideshow_state == 10))
			S_ChangeMusic(mus_dark, 1);		// ADDED FOR DEMO
		    else
			S_ChangeMusic(mus_dark, 1);		// ADDED FOR DEMO
		}
	    }
	    else if(gamestate == GS_LEVEL)
	    {
		if(!isdemoversion)
		    S_ChangeMusic(gamemap + 1, 1);		// [SVE]
		else
		{
		    if(gamemap == 32)
			S_ChangeMusic(mus_map1, 1);		// [SVE]
		    else if(gamemap == 33)
			S_ChangeMusic(mus_tavern, 1);		// [SVE]
		    else if(gamemap == 34)
			S_ChangeMusic(mus_map3, 1);		// [SVE]
		    else if(gamemap == 35)
			S_ChangeMusic(mus_fight2, 1);		// [SVE]
		    else if(gamemap == 36)
			S_ChangeMusic(mus_fmstl, 1);		// [SVE]
		}
	    }
	    if(!usergame && gamestate != GS_LEVEL && gamestate != GS_FINALE)
	    {
		if(demosequence < -2 && demosequence >= -5)
		    S_ChangeMusic(mus_fast, 1);
		else if(demosequence >= -2 && demosequence < 1)
		    S_ChangeMusic(mus_logo, 1);
		else if(demosequence >= 1 && demosequence < 8)
		    S_ChangeMusic(mus_intro, 1);
		else if(demosequence >= 8)
		    S_ChangeMusic(mus_dark, 1);
	    }
	}
	else
	    S_ChangeMusic(musnum, 1);		// [SVE]
    }
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
// haleyjd 08/27/10: [STRIFE] Rewritten for Sigil cursor
//
void M_Ticker (void)
{
    if (--cursorAnimCounter <= 0)
    {
        whichCursor = (whichCursor + 1) % 8;
        cursorAnimCounter = 5;
    }

    // animate merchant
    FE_MerchantTick();
}


//
// M_Init
//
// haleyjd 08/27/10: [STRIFE] Removed DOOM gamemode stuff
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichCursor = 0;
    cursorAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive; // STRIFE-FIXME: assigns 0 here...
    quickSaveSlot = -1;

    // [STRIFE]: Initialize savegame paths and clear temporary directory
    G_WriteSaveName(5, "ME");
    ClearTmp();

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    FE_InitMerchant();			// [SVE]
}

