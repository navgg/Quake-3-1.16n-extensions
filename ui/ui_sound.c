// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
/*
=======================================================================

SOUND OPTIONS MENU

=======================================================================
*/

#include "ui_local.h"


#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"
#define ART_ACCEPT0			"menu/art/accept_0"
#define ART_ACCEPT1			"menu/art/accept_1"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_SOUND			12
#define ID_NETWORK			13
#define ID_EFFECTSVOLUME	14
#define ID_MUSICVOLUME		15
#define ID_QUALITY			16
#define ID_A3D				17
#define ID_COMPRESSION		18
#define ID_AMBIENT			19
#define ID_KILLBEEP			20
#define ID_BACK				21

static void UI_Sound_StatusBar( void *self ) {	
	static const char *info_messages[][2] = {
		{ "Controls sound effects volume", "" },
		{ "Controls ingame music volume", "" },
		{ "Sets sound quality, recommended 'High'", "" },
		{ "This setting removed in latest Quake 3", "Recommended 'Off'" },
		{ "Sound 8bit compression", "Removed in latest Quake 3, Recommended 'Off'" },
		{ "Toggles ambient sounds", "" },
		{ "Sets sound beep after killing an enemy", "" }
	};

	UIX_CommonStatusBar(self, ID_EFFECTSVOLUME, ArrLen(info_messages), info_messages);
}

static const char *quality_items[] = {
	"Low", "High", 0
};

static const char *killbeep_items[] = {
	"Off",
	"Ting",
	"Tink",
	"Dramatic",
	"Voosh",
	"Drum",
	"Bang",
	"Ding",
	"Cha-Ching!",
	0
};

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;

	menutext_s			graphics;
	menutext_s			display;
	menutext_s			sound;
	menutext_s			network;

	menuslider_s		sfxvolume;
	menuslider_s		musicvolume;
	menulist_s			quality;
	menuradiobutton_s	compression;
	menuradiobutton_s	a3d;
	menuradiobutton_s	ambient;
	menulist_s			killbeep;
	int					killpeep_initial;

	sfxHandle_t			killbeep_sounds[8];

	menubitmap_s		back;
	menubitmap_s		apply;
} soundOptionsInfo_t;

static soundOptionsInfo_t	soundOptionsInfo;

static char *s_compression;

/*
=================
UI_SoundOptionsMenu_Event
=================
*/
static void UI_SoundOptionsMenu_Event( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_PopMenu();
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		UI_PopMenu();
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		break;

	case ID_NETWORK:
		UI_PopMenu();
		UI_NetworkOptionsMenu();
		break;

	case ID_EFFECTSVOLUME:
		trap_Cvar_SetValue( "s_volume", soundOptionsInfo.sfxvolume.curvalue / 10 );
		break;

	case ID_MUSICVOLUME:
		trap_Cvar_SetValue( "s_musicvolume", soundOptionsInfo.musicvolume.curvalue / 10 );
		break;

	case ID_QUALITY:
		if( soundOptionsInfo.quality.curvalue ) {
			trap_Cvar_SetValue( "s_khz", 22 );
			trap_Cvar_SetValue( s_compression, 0 );
		}
		else {
			trap_Cvar_SetValue( "s_khz", 11 );			
			trap_Cvar_SetValue( s_compression, 1 );
		}
		UI_ForceMenuOff();
		trap_Cmd_ExecuteText( EXEC_APPEND, "snd_restart; wait 3; ui_sound\n" );
		break;

	case ID_A3D:
		if( soundOptionsInfo.a3d.curvalue ) {
			trap_Cmd_ExecuteText( EXEC_NOW, "s_enable_a3d\n" );
		}
		else {
			trap_Cmd_ExecuteText( EXEC_NOW, "s_disable_a3d\n" );
		}
		soundOptionsInfo.a3d.curvalue = (int)trap_Cvar_VariableValue( "s_usingA3D" );
		break;

	case ID_COMPRESSION:
		trap_Cvar_SetValue( s_compression, soundOptionsInfo.compression.curvalue );
		soundOptionsInfo.quality.curvalue = !trap_Cvar_VariableValue( s_compression ) && trap_Cvar_VariableValue( "s_khz" ) >= 22;
		break;

	case ID_AMBIENT:
		trap_Cvar_SetValue( "s_ambient", soundOptionsInfo.ambient.curvalue );
		soundOptionsInfo.ambient.curvalue = (int)trap_Cvar_VariableValue( "s_ambient" );
		break;

	case ID_KILLBEEP:
		trap_Cvar_SetValue( "cg_killBeep", soundOptionsInfo.killbeep.curvalue );

		if (soundOptionsInfo.killbeep.curvalue)
			trap_S_StartLocalSound(soundOptionsInfo.killbeep_sounds[soundOptionsInfo.killbeep.curvalue % 8], CHAN_LOCAL);

		if (trap_Cvar_VariableValue("cl_paused")) {
			soundOptionsInfo.apply.generic.flags |= QMF_HIDDEN | QMF_INACTIVE;
			if (soundOptionsInfo.killpeep_initial != soundOptionsInfo.killbeep.curvalue && soundOptionsInfo.killbeep.curvalue)
				soundOptionsInfo.apply.generic.flags &= ~(QMF_HIDDEN | QMF_INACTIVE);
		}
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}

/*
=================
SoundOptions_ApplyChanges
=================
*/
static void SoundOptions_ApplyChanges( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;

	trap_Cmd_ExecuteText( EXEC_APPEND, "snd_restart\n" );
}

/*
===============
UI_SoundOptionsMenu_Init
===============
*/
static void UI_SoundOptionsMenu_Init( void ) {
	int				y;

	memset( &soundOptionsInfo, 0, sizeof(soundOptionsInfo) );

	s_compression = uis.q3version == 11 ? "s_loadas8bit" : "s_compression";

	UI_SoundOptionsMenu_Cache();
	soundOptionsInfo.menu.wrapAround = qtrue;
	soundOptionsInfo.menu.fullscreen = qtrue;

	soundOptionsInfo.banner.generic.type		= MTYPE_BTEXT;
	soundOptionsInfo.banner.generic.flags		= QMF_CENTER_JUSTIFY;
	soundOptionsInfo.banner.generic.x			= 320;
	soundOptionsInfo.banner.generic.y			= 16;
	soundOptionsInfo.banner.string				= "SYSTEM SETUP";
	soundOptionsInfo.banner.color				= color_white;
	soundOptionsInfo.banner.style				= UI_CENTER;

	soundOptionsInfo.framel.generic.type		= MTYPE_BITMAP;
	soundOptionsInfo.framel.generic.name		= ART_FRAMEL;
	soundOptionsInfo.framel.generic.flags		= QMF_INACTIVE;
	soundOptionsInfo.framel.generic.x			= 0;  
	soundOptionsInfo.framel.generic.y			= 78;
	soundOptionsInfo.framel.width				= 256;
	soundOptionsInfo.framel.height				= 329;

	soundOptionsInfo.framer.generic.type		= MTYPE_BITMAP;
	soundOptionsInfo.framer.generic.name		= ART_FRAMER;
	soundOptionsInfo.framer.generic.flags		= QMF_INACTIVE;
	soundOptionsInfo.framer.generic.x			= 376;
	soundOptionsInfo.framer.generic.y			= 76;
	soundOptionsInfo.framer.width				= 256;
	soundOptionsInfo.framer.height				= 334;

	soundOptionsInfo.graphics.generic.type		= MTYPE_PTEXT;
	soundOptionsInfo.graphics.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	soundOptionsInfo.graphics.generic.id		= ID_GRAPHICS;
	soundOptionsInfo.graphics.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.graphics.generic.x			= 216;
	soundOptionsInfo.graphics.generic.y			= 240 - 2 * PROP_HEIGHT;
	soundOptionsInfo.graphics.string			= "GRAPHICS";
	soundOptionsInfo.graphics.style				= UI_RIGHT;
	soundOptionsInfo.graphics.color				= color_red;

	soundOptionsInfo.display.generic.type		= MTYPE_PTEXT;
	soundOptionsInfo.display.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	soundOptionsInfo.display.generic.id			= ID_DISPLAY;
	soundOptionsInfo.display.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.display.generic.x			= 216;
	soundOptionsInfo.display.generic.y			= 240 - PROP_HEIGHT;
	soundOptionsInfo.display.string				= "DISPLAY";
	soundOptionsInfo.display.style				= UI_RIGHT;
	soundOptionsInfo.display.color				= color_red;

	soundOptionsInfo.sound.generic.type			= MTYPE_PTEXT;
	soundOptionsInfo.sound.generic.flags		= QMF_RIGHT_JUSTIFY;
	soundOptionsInfo.sound.generic.id			= ID_SOUND;
	soundOptionsInfo.sound.generic.callback		= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.sound.generic.x			= 216;
	soundOptionsInfo.sound.generic.y			= 240;
	soundOptionsInfo.sound.string				= "SOUND";
	soundOptionsInfo.sound.style				= UI_RIGHT;
	soundOptionsInfo.sound.color				= color_red;

	soundOptionsInfo.network.generic.type		= MTYPE_PTEXT;
	soundOptionsInfo.network.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	soundOptionsInfo.network.generic.id			= ID_NETWORK;
	soundOptionsInfo.network.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.network.generic.x			= 216;
	soundOptionsInfo.network.generic.y			= 240 + PROP_HEIGHT;
	soundOptionsInfo.network.string				= "NETWORK";
	soundOptionsInfo.network.style				= UI_RIGHT;
	soundOptionsInfo.network.color				= color_red;

	y = 240 - 2.5 * (BIGCHAR_HEIGHT + 2);
	soundOptionsInfo.sfxvolume.generic.type		= MTYPE_SLIDER;
	soundOptionsInfo.sfxvolume.generic.name		= "Effects Volume:";
	soundOptionsInfo.sfxvolume.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	soundOptionsInfo.sfxvolume.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.sfxvolume.generic.id		= ID_EFFECTSVOLUME;
	soundOptionsInfo.sfxvolume.generic.x		= 400;
	soundOptionsInfo.sfxvolume.generic.y		= y;
	soundOptionsInfo.sfxvolume.minvalue			= 0;
	soundOptionsInfo.sfxvolume.maxvalue			= 10;
	soundOptionsInfo.sfxvolume.generic.statusbar = UI_Sound_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	soundOptionsInfo.musicvolume.generic.type		= MTYPE_SLIDER;
	soundOptionsInfo.musicvolume.generic.name		= "Music Volume:";
	soundOptionsInfo.musicvolume.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	soundOptionsInfo.musicvolume.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.musicvolume.generic.id			= ID_MUSICVOLUME;
	soundOptionsInfo.musicvolume.generic.x			= 400;
	soundOptionsInfo.musicvolume.generic.y			= y;
	soundOptionsInfo.musicvolume.minvalue			= 0;
	soundOptionsInfo.musicvolume.maxvalue			= 10;
	soundOptionsInfo.musicvolume.generic.statusbar	= UI_Sound_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	soundOptionsInfo.quality.generic.type		= MTYPE_SPINCONTROL;
	soundOptionsInfo.quality.generic.name		= "Sound Quality:";
	soundOptionsInfo.quality.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	soundOptionsInfo.quality.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.quality.generic.id			= ID_QUALITY;
	soundOptionsInfo.quality.generic.x			= 400;
	soundOptionsInfo.quality.generic.y			= y;
	soundOptionsInfo.quality.itemnames			= quality_items;
	soundOptionsInfo.quality.generic.statusbar	= UI_Sound_StatusBar;

	if (uis.q3version < 32) {
		y += BIGCHAR_HEIGHT+2;
		soundOptionsInfo.compression.generic.type			= MTYPE_RADIOBUTTON;
		soundOptionsInfo.compression.generic.name			= "Compression:";
		soundOptionsInfo.compression.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		soundOptionsInfo.compression.generic.callback		= UI_SoundOptionsMenu_Event;
		soundOptionsInfo.compression.generic.id				= ID_COMPRESSION;
		soundOptionsInfo.compression.generic.x				= 400;
		soundOptionsInfo.compression.generic.y				= y;
		soundOptionsInfo.compression.generic.statusbar		= UI_Sound_StatusBar;

		y += BIGCHAR_HEIGHT+2;
		soundOptionsInfo.a3d.generic.type			= MTYPE_RADIOBUTTON;
		soundOptionsInfo.a3d.generic.name			= "A3D:";
		soundOptionsInfo.a3d.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		soundOptionsInfo.a3d.generic.callback		= UI_SoundOptionsMenu_Event;
		soundOptionsInfo.a3d.generic.id				= ID_A3D;
		soundOptionsInfo.a3d.generic.x				= 400;
		soundOptionsInfo.a3d.generic.y				= y;
		soundOptionsInfo.a3d.generic.statusbar		= UI_Sound_StatusBar;
	}

	y += BIGCHAR_HEIGHT+2;
	soundOptionsInfo.ambient.generic.type		= MTYPE_RADIOBUTTON;
	soundOptionsInfo.ambient.generic.name		= "Ambient:";
	soundOptionsInfo.ambient.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	soundOptionsInfo.ambient.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.ambient.generic.id			= ID_AMBIENT;
	soundOptionsInfo.ambient.generic.x			= 400;
	soundOptionsInfo.ambient.generic.y			= y;
	soundOptionsInfo.ambient.generic.statusbar	= UI_Sound_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	soundOptionsInfo.killbeep.generic.type		= MTYPE_SPINCONTROL;
	soundOptionsInfo.killbeep.generic.name		= "Kill beep:";
	soundOptionsInfo.killbeep.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	soundOptionsInfo.killbeep.generic.callback	= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.killbeep.generic.id		= ID_KILLBEEP;
	soundOptionsInfo.killbeep.generic.x			= 400;
	soundOptionsInfo.killbeep.generic.y			= y;
	soundOptionsInfo.killbeep.itemnames			= killbeep_items;
	soundOptionsInfo.killbeep.generic.statusbar	= UI_Sound_StatusBar;

	soundOptionsInfo.back.generic.type			= MTYPE_BITMAP;
	soundOptionsInfo.back.generic.name			= ART_BACK0;
	soundOptionsInfo.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	soundOptionsInfo.back.generic.callback		= UI_SoundOptionsMenu_Event;
	soundOptionsInfo.back.generic.id			= ID_BACK;
	soundOptionsInfo.back.generic.x				= 0;
	soundOptionsInfo.back.generic.y				= 480-64;
	soundOptionsInfo.back.width					= 128;
	soundOptionsInfo.back.height				= 64;
	soundOptionsInfo.back.focuspic				= ART_BACK1;

	soundOptionsInfo.apply.generic.type     = MTYPE_BITMAP;
	soundOptionsInfo.apply.generic.name     = ART_ACCEPT0;
	soundOptionsInfo.apply.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_HIDDEN|QMF_INACTIVE;
	soundOptionsInfo.apply.generic.callback = SoundOptions_ApplyChanges;
	soundOptionsInfo.apply.generic.x        = 640;
	soundOptionsInfo.apply.generic.y        = 480-64;
	soundOptionsInfo.apply.width  		    = 128;
	soundOptionsInfo.apply.height  			= 64;
	soundOptionsInfo.apply.focuspic         = ART_ACCEPT1;

	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.banner );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.framel );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.framer );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.graphics );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.display );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.sound );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.network );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.sfxvolume );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.musicvolume );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.quality );
	if (uis.q3version < 32) {
		Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.a3d );
		Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.compression );
	}
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.ambient );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.killbeep );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.back );
	Menu_AddItem( &soundOptionsInfo.menu, ( void * ) &soundOptionsInfo.apply );

	soundOptionsInfo.sfxvolume.curvalue = trap_Cvar_VariableValue( "s_volume" ) * 10;
	soundOptionsInfo.musicvolume.curvalue = trap_Cvar_VariableValue( "s_musicvolume" ) * 10;
	soundOptionsInfo.quality.curvalue = !trap_Cvar_VariableValue( s_compression ) && trap_Cvar_VariableValue( "s_khz" ) >= 22;
	soundOptionsInfo.compression.curvalue = trap_Cvar_VariableValue( s_compression );
	soundOptionsInfo.a3d.curvalue = (int)trap_Cvar_VariableValue( "s_usingA3D" );
	soundOptionsInfo.ambient.curvalue = trap_Cvar_VariableValue( "s_ambient" );
	soundOptionsInfo.killbeep.curvalue = (int)trap_Cvar_VariableValue( "cg_killBeep" ) % 9;
	soundOptionsInfo.killpeep_initial = soundOptionsInfo.killbeep.curvalue;
}


/*
===============
UI_SoundOptionsMenu_Cache
===============
*/
void UI_SoundOptionsMenu_Cache( void ) {
	int i;

	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_ACCEPT0 );
	trap_R_RegisterShaderNoMip( ART_ACCEPT1 );

	for (i = 0; i < ArrLen(soundOptionsInfo.killbeep_sounds); i++)
		soundOptionsInfo.killbeep_sounds[i] = trap_S_RegisterSound(va("sound/feedback/impact%i.wav", i));
}


/*
===============
UI_SoundOptionsMenu
===============
*/
void UI_SoundOptionsMenu( void ) {
	UI_SoundOptionsMenu_Init();
	UI_PushMenu( &soundOptionsInfo.menu );
	Menu_SetCursorToItem( &soundOptionsInfo.menu, &soundOptionsInfo.sound );
}
