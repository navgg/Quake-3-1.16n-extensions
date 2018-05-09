// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
/*
=======================================================================

DISPLAY OPTIONS MENU

=======================================================================
*/

#include "ui_local.h"


#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_SOUND			12
#define ID_NETWORK			13
#define ID_BRIGHTNESS		14
#define ID_SCREENSIZE		15
#define ID_OVERBRIGHT_BITS	16
#define ID_IGNORE_HW_GAMMA	17
#define ID_DRAWFPS			18
#define ID_WIDESCREEN_FIX	19
#define ID_MAXFPS			20
#define ID_PRIMITIVES		21
#define ID_WIDESCREEN_FOV	22

#define ID_BACK				29

#define MAX_INFO_MESSAGES	9
static void UI_Display_StatusBar( void *self ) {	
	static const char *info_messages[MAX_INFO_MESSAGES][2] = {
		{ "Controls display brightness", "Turn off 'Over Bright Bits' to increase even higher" },
		{ "Decreases screen size (not recommended)", "Adds gray frame around screen" },
		{ "Ambient lighting of in-game entities or objects", "Turn this off if you have problems with brightness" },
		{ "If you have problems with brighness", "Or colors after game exit, turn this on" },
		{ "Shows counter of frapmes per second", "" },
		{ "Fixes rendering for widescreens", "All icons and fonts are not stretching in game" },
		{ "Sets max limit for frames per second", "Min - 62, Max - 250" },
		{ "Changes the render method", "'Fast' may increase fps in some cases" },
		{ "Adjusts FOV (field of view) for widescreens", ""}
	};

	UIX_CommonStatusBar(self, ID_BRIGHTNESS, MAX_INFO_MESSAGES, info_messages);
}

static const char *fps_items[] = {		
	"62",
	"76",
	"90",
	"100",
	"111",
	"125",
	"142",
	"166",
	"200",
	"250",
	0
};

static const char *primitives_items[] = {
	"Default",
	"Fast",	
	0
};


typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menutext_s		graphics;
	menutext_s		display;
	menutext_s		sound;
	menutext_s		network;

	menuslider_s	brightness;
	menuslider_s	screensize;
	menuradiobutton_s overbrightbits;
	menuradiobutton_s ignorehwgamma;
	menuradiobutton_s widescreenfix;
	menuradiobutton_s widescreenfov;
	menuradiobutton_s drawfps;
	menulist_s		maxfps;
	menulist_s		primitives;

	menubitmap_s	back;
} displayOptionsInfo_t;

static displayOptionsInfo_t	displayOptionsInfo;

//X-MOD: lock screensize slider
static qboolean screensize_locked = qtrue;

static void UI_ViewSize_Action( qboolean result ) {
	if (!result) {
		displayOptionsInfo.screensize.curvalue = trap_Cvar_VariableValue("cg_viewsize") / 10;
		return;
	}

	trap_Cvar_SetValue("cg_viewsize", displayOptionsInfo.screensize.curvalue * 10);
	screensize_locked = qfalse;	
}

static void UI_ViewSize_Draw( void ) {
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 0, "WARNING: This will decrease screen size", UI_CENTER|UI_SMALLFONT, color_yellow );
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 1, "change it if you know what you are doing.", UI_CENTER|UI_SMALLFONT, color_yellow );
}
/*
=================
UI_DisplayOptionsMenu_Event
=================
*/
static void UI_DisplayOptionsMenu_Event( void* ptr, int event ) {
	int i;

	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_PopMenu();
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		break;

	case ID_SOUND:
		UI_PopMenu();
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		UI_PopMenu();
		UI_NetworkOptionsMenu();
		break;

	case ID_BRIGHTNESS:
		trap_Cvar_SetValue( "r_gamma", displayOptionsInfo.brightness.curvalue / 10.0f );
		break;
	
	case ID_SCREENSIZE:
		if (displayOptionsInfo.screensize.curvalue < trap_Cvar_VariableValue( "cg_viewsize") / 10 && screensize_locked)
			UI_ConfirmMenu("ARE YOU SURE?", UI_ViewSize_Draw, UI_ViewSize_Action);
		else
			UI_ViewSize_Action(qtrue);
		break;

	case ID_OVERBRIGHT_BITS:
		trap_Cvar_SetValue( "r_overbrightbits", displayOptionsInfo.overbrightbits.curvalue );		
		trap_Cmd_ExecuteText( EXEC_APPEND, "r_gamma 1; vid_restart; wait 3; ui_display\n" );
		break;

	case ID_IGNORE_HW_GAMMA:
		trap_Cvar_SetValue( "r_ignorehwgamma", displayOptionsInfo.ignorehwgamma.curvalue );		
		trap_Cmd_ExecuteText( EXEC_APPEND, "r_gamma 1; vid_restart; wait 3; ui_display\n" );
		break;

	case ID_DRAWFPS: 		
		trap_Cvar_SetValue( "cg_drawFPS", displayOptionsInfo.drawfps.curvalue );
		break;

	case ID_MAXFPS:
		trap_Cvar_Set( "com_maxfps", fps_items[displayOptionsInfo.maxfps.curvalue] );		
		break;

	case ID_WIDESCREEN_FIX:				
		i = trap_Cvar_VariableValue("cg_wideScreenFix");		
		i = displayOptionsInfo.widescreenfix.curvalue ? i | CGX_WFIX_SCREEN : i & ~(CGX_WFIX_SCREEN);
		trap_Cvar_SetValue( "cg_wideScreenFix", i );		
		break;

	case ID_WIDESCREEN_FOV:				
		i = trap_Cvar_VariableValue("cg_wideScreenFix");		
		i = displayOptionsInfo.widescreenfov.curvalue ? i | CGX_WFIX_FOV : i & ~(CGX_WFIX_FOV);
		trap_Cvar_SetValue( "cg_wideScreenFix", i );		
		break;

	case ID_PRIMITIVES:
		trap_Cvar_SetValue("r_primitives", displayOptionsInfo.primitives.curvalue * 2);
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
UI_DisplayOptionsMenu_Init
===============
*/
static void UI_DisplayOptionsMenu_Init( void ) {
	int		y;
	int		fps;

	memset( &displayOptionsInfo, 0, sizeof(displayOptionsInfo) );

	UI_DisplayOptionsMenu_Cache();
	displayOptionsInfo.menu.wrapAround = qtrue;
	displayOptionsInfo.menu.fullscreen = qtrue;	

	displayOptionsInfo.banner.generic.type		= MTYPE_BTEXT;
	displayOptionsInfo.banner.generic.flags		= QMF_CENTER_JUSTIFY;
	displayOptionsInfo.banner.generic.x			= 320;
	displayOptionsInfo.banner.generic.y			= 16;
	displayOptionsInfo.banner.string			= "SYSTEM SETUP";
	displayOptionsInfo.banner.color				= color_white;
	displayOptionsInfo.banner.style				= UI_CENTER;

	displayOptionsInfo.framel.generic.type		= MTYPE_BITMAP;
	displayOptionsInfo.framel.generic.name		= ART_FRAMEL;
	displayOptionsInfo.framel.generic.flags		= QMF_INACTIVE;
	displayOptionsInfo.framel.generic.x			= 0;  
	displayOptionsInfo.framel.generic.y			= 78;
	displayOptionsInfo.framel.width				= 256;
	displayOptionsInfo.framel.height			= 329;

	displayOptionsInfo.framer.generic.type		= MTYPE_BITMAP;
	displayOptionsInfo.framer.generic.name		= ART_FRAMER;
	displayOptionsInfo.framer.generic.flags		= QMF_INACTIVE;
	displayOptionsInfo.framer.generic.x			= 376;
	displayOptionsInfo.framer.generic.y			= 76;
	displayOptionsInfo.framer.width				= 256;
	displayOptionsInfo.framer.height			= 334;

	displayOptionsInfo.graphics.generic.type		= MTYPE_PTEXT;
	displayOptionsInfo.graphics.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	displayOptionsInfo.graphics.generic.id			= ID_GRAPHICS;
	displayOptionsInfo.graphics.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.graphics.generic.x			= 216;
	displayOptionsInfo.graphics.generic.y			= 240 - 2 * PROP_HEIGHT;
	displayOptionsInfo.graphics.string				= "GRAPHICS";
	displayOptionsInfo.graphics.style				= UI_RIGHT;
	displayOptionsInfo.graphics.color				= color_red;

	displayOptionsInfo.display.generic.type			= MTYPE_PTEXT;
	displayOptionsInfo.display.generic.flags		= QMF_RIGHT_JUSTIFY;
	displayOptionsInfo.display.generic.id			= ID_DISPLAY;
	displayOptionsInfo.display.generic.callback		= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.display.generic.x			= 216;
	displayOptionsInfo.display.generic.y			= 240 - PROP_HEIGHT;
	displayOptionsInfo.display.string				= "DISPLAY";
	displayOptionsInfo.display.style				= UI_RIGHT;
	displayOptionsInfo.display.color				= color_red;

	displayOptionsInfo.sound.generic.type			= MTYPE_PTEXT;
	displayOptionsInfo.sound.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	displayOptionsInfo.sound.generic.id				= ID_SOUND;
	displayOptionsInfo.sound.generic.callback		= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.sound.generic.x				= 216;
	displayOptionsInfo.sound.generic.y				= 240;
	displayOptionsInfo.sound.string					= "SOUND";
	displayOptionsInfo.sound.style					= UI_RIGHT;
	displayOptionsInfo.sound.color					= color_red;

	displayOptionsInfo.network.generic.type			= MTYPE_PTEXT;
	displayOptionsInfo.network.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	displayOptionsInfo.network.generic.id			= ID_NETWORK;
	displayOptionsInfo.network.generic.callback		= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.network.generic.x			= 216;
	displayOptionsInfo.network.generic.y			= 240 + PROP_HEIGHT;
	displayOptionsInfo.network.string				= "NETWORK";
	displayOptionsInfo.network.style				= UI_RIGHT;
	displayOptionsInfo.network.color				= color_red;

	y = 240 - 3 * (BIGCHAR_HEIGHT + 2);	
	displayOptionsInfo.ignorehwgamma.generic.type = MTYPE_RADIOBUTTON;
	displayOptionsInfo.ignorehwgamma.generic.name = "Ignore Game Gamma:";
	displayOptionsInfo.ignorehwgamma.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	displayOptionsInfo.ignorehwgamma.generic.callback = UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.ignorehwgamma.generic.id = ID_IGNORE_HW_GAMMA;
	displayOptionsInfo.ignorehwgamma.generic.x = 400;
	displayOptionsInfo.ignorehwgamma.generic.y = y;
	displayOptionsInfo.ignorehwgamma.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT + 2;
	displayOptionsInfo.overbrightbits.generic.type = MTYPE_RADIOBUTTON;
	displayOptionsInfo.overbrightbits.generic.name = "Over Bright Bits:";
	displayOptionsInfo.overbrightbits.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	displayOptionsInfo.overbrightbits.generic.callback = UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.overbrightbits.generic.id = ID_OVERBRIGHT_BITS;
	displayOptionsInfo.overbrightbits.generic.x = 400;
	displayOptionsInfo.overbrightbits.generic.y = y;
	displayOptionsInfo.overbrightbits.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT + 2;
	displayOptionsInfo.brightness.generic.type		= MTYPE_SLIDER;
	displayOptionsInfo.brightness.generic.name		= "Brightness:";
	displayOptionsInfo.brightness.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.brightness.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.brightness.generic.id		= ID_BRIGHTNESS;
	displayOptionsInfo.brightness.generic.x			= 400;
	displayOptionsInfo.brightness.generic.y			= y;
	displayOptionsInfo.brightness.minvalue			= 5;
	displayOptionsInfo.brightness.maxvalue			= 20;
	displayOptionsInfo.brightness.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	displayOptionsInfo.screensize.generic.type		= MTYPE_SLIDER;
	displayOptionsInfo.screensize.generic.name		= "Screen Size:";
	displayOptionsInfo.screensize.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.screensize.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.screensize.generic.id		= ID_SCREENSIZE;
	displayOptionsInfo.screensize.generic.x			= 400;
	displayOptionsInfo.screensize.generic.y			= y;
	displayOptionsInfo.screensize.minvalue			= 3;
    displayOptionsInfo.screensize.maxvalue			= 10;
	displayOptionsInfo.screensize.generic.statusbar = UI_Display_StatusBar;

	if (!uis.glconfig.deviceSupportsGamma) {
		displayOptionsInfo.brightness.generic.flags |= QMF_GRAYED;
		displayOptionsInfo.overbrightbits.generic.flags |= QMF_GRAYED;
	}

	y += BIGCHAR_HEIGHT+2;
	displayOptionsInfo.drawfps.generic.type		= MTYPE_RADIOBUTTON;
	displayOptionsInfo.drawfps.generic.name		= "Show FPS:";
	displayOptionsInfo.drawfps.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.drawfps.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.drawfps.generic.id		= ID_DRAWFPS;
	displayOptionsInfo.drawfps.generic.x		= 400;
	displayOptionsInfo.drawfps.generic.y		= y;
	displayOptionsInfo.drawfps.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	displayOptionsInfo.maxfps.generic.type		= MTYPE_SPINCONTROL;
	displayOptionsInfo.maxfps.generic.name		= "Max FPS:";
	displayOptionsInfo.maxfps.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.maxfps.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.maxfps.generic.id		= ID_MAXFPS;
	displayOptionsInfo.maxfps.generic.x			= 400;
	displayOptionsInfo.maxfps.generic.y			= y;
	displayOptionsInfo.maxfps.itemnames			= fps_items;
	displayOptionsInfo.maxfps.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT + 2;
	displayOptionsInfo.primitives.generic.type = MTYPE_SPINCONTROL;
	displayOptionsInfo.primitives.generic.name = "Primitives:";
	displayOptionsInfo.primitives.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	displayOptionsInfo.primitives.generic.callback = UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.primitives.generic.id = ID_PRIMITIVES;
	displayOptionsInfo.primitives.generic.x = 400;
	displayOptionsInfo.primitives.generic.y = y;
	displayOptionsInfo.primitives.itemnames = primitives_items;
	displayOptionsInfo.primitives.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	y += BIGCHAR_HEIGHT+2;
	displayOptionsInfo.widescreenfix.generic.type		= MTYPE_RADIOBUTTON;
	displayOptionsInfo.widescreenfix.generic.name		= "Widescreen fix:";
	displayOptionsInfo.widescreenfix.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.widescreenfix.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.widescreenfix.generic.id			= ID_WIDESCREEN_FIX;
	displayOptionsInfo.widescreenfix.generic.x			= 400;
	displayOptionsInfo.widescreenfix.generic.y			= y;
	displayOptionsInfo.widescreenfix.generic.statusbar = UI_Display_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	displayOptionsInfo.widescreenfov.generic.type		= MTYPE_RADIOBUTTON;
	displayOptionsInfo.widescreenfov.generic.name		= "Widescreen fov fix:";
	displayOptionsInfo.widescreenfov.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	displayOptionsInfo.widescreenfov.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.widescreenfov.generic.id			= ID_WIDESCREEN_FOV;
	displayOptionsInfo.widescreenfov.generic.x			= 400;
	displayOptionsInfo.widescreenfov.generic.y			= y;
	displayOptionsInfo.widescreenfov.generic.statusbar = UI_Display_StatusBar;

	displayOptionsInfo.back.generic.type		= MTYPE_BITMAP;
	displayOptionsInfo.back.generic.name		= ART_BACK0;
	displayOptionsInfo.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	displayOptionsInfo.back.generic.callback	= UI_DisplayOptionsMenu_Event;
	displayOptionsInfo.back.generic.id			= ID_BACK;
	displayOptionsInfo.back.generic.x			= 0;
	displayOptionsInfo.back.generic.y			= 480-64;
	displayOptionsInfo.back.width				= 128;
	displayOptionsInfo.back.height				= 64;
	displayOptionsInfo.back.focuspic			= ART_BACK1;

	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.banner );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.framel );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.framer );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.graphics );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.display );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.sound );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.network );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.brightness );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.screensize );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.overbrightbits );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.ignorehwgamma );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.drawfps );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.widescreenfix );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.widescreenfov );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.maxfps );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.primitives );
	Menu_AddItem( &displayOptionsInfo.menu, ( void * ) &displayOptionsInfo.back );

	displayOptionsInfo.brightness.curvalue  = trap_Cvar_VariableValue("r_gamma") * 10;
	displayOptionsInfo.screensize.curvalue  = trap_Cvar_VariableValue( "cg_viewsize")/10;
	displayOptionsInfo.overbrightbits.curvalue  = trap_Cvar_VariableValue("r_overbrightbits") != 0;
	displayOptionsInfo.ignorehwgamma.curvalue  = trap_Cvar_VariableValue("r_ignorehwgamma") != 0;
	displayOptionsInfo.drawfps.curvalue  = trap_Cvar_VariableValue("cg_drawFPS") != 0;
	displayOptionsInfo.maxfps.curvalue  = trap_Cvar_VariableValue("cg_drawFPS") != 0;
	displayOptionsInfo.primitives.curvalue = trap_Cvar_VariableValue("r_primitives") / 2;
	
	y = (int)(abs(trap_Cvar_VariableValue("cg_wideScreenFix")) % 4);
	displayOptionsInfo.widescreenfix.curvalue = y & CGX_WFIX_SCREEN;
	displayOptionsInfo.widescreenfov.curvalue = y & CGX_WFIX_FOV;

	if (displayOptionsInfo.overbrightbits.curvalue || displayOptionsInfo.ignorehwgamma.curvalue)
		displayOptionsInfo.brightness.maxvalue = 10;

	fps = trap_Cvar_VariableValue( "com_maxfps" );
	if (fps <= 62)
		displayOptionsInfo.maxfps.curvalue = 0;
	else if( fps <= 76 )
		displayOptionsInfo.maxfps.curvalue = 1;
	else if( fps <= 90 )
		displayOptionsInfo.maxfps.curvalue = 2;	
	else if( fps <= 100 )
		displayOptionsInfo.maxfps.curvalue = 3;
	else if( fps <= 111 )
		displayOptionsInfo.maxfps.curvalue = 4;
	else if( fps <= 125 )
		displayOptionsInfo.maxfps.curvalue = 5;
	else if( fps <= 142 )
		displayOptionsInfo.maxfps.curvalue = 6;
	else if( fps <= 166 )
		displayOptionsInfo.maxfps.curvalue = 7;
	else if( fps <= 200 )
		displayOptionsInfo.maxfps.curvalue = 8;	
	else
		displayOptionsInfo.maxfps.curvalue = 9;		
}


/*
===============
UI_DisplayOptionsMenu_Cache
===============
*/
void UI_DisplayOptionsMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}


/*
===============
UI_DisplayOptionsMenu
===============
*/
void UI_DisplayOptionsMenu( void ) {
	UI_DisplayOptionsMenu_Init();
	UI_PushMenu( &displayOptionsInfo.menu );
	Menu_SetCursorToItem( &displayOptionsInfo.menu, &displayOptionsInfo.display );
}
