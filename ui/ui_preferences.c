// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//

#include "ui_local.h"

#define PREFERENCES_X_POS		360

/*
=======================================================================

ADVANCED EFFECTS MENU

=======================================================================
*/

#define EFFECTS_FRAMEL	"menu/art/frame2_l"
#define EFFECTS_FRAMER	"menu/art/frame1_r"
#define EFFECTS_BACK0	"menu/art/back_0"
#define EFFECTS_BACK1	"menu/art/back_1"

static char* AdvancedEffects_artlist[] = 
{
	EFFECTS_FRAMEL,
	EFFECTS_FRAMER,
	EFFECTS_BACK0,
	EFFECTS_BACK1,
	NULL,
};

#define ID_EFFECTSBACK		100

#define ID_RAIL				200
#define ID_RAIL_TIME		201
#define ID_ROCKET32			202
#define ID_ROCKET_TRAIL		203
#define ID_LG_BEAM			204
#define ID_LG_SPARKS		205
#define	ID_BULLET_SPARKS	206
#define ID_PLASMA_TRAIL		207
#define ID_EXPLOSION_SPARKS	208

#define MAX_EFFECTS_INFO_MESSAGES 9

static void UI_AdvancedEffects_StatusBar( void *self ) {	
	static const char *info_messages[MAX_EFFECTS_INFO_MESSAGES][2] = {
		{ "Sets rail trail style", "" },
		{ "Sets rail trail time", "How fast rail trail will disappear" },
		{ "Sets rocket explosion type", "" },
		{ "Toggles rocket trail sparks", "" },
		{ "Sets lightning gun beam type", "" },
		{ "Toggles lightning gun sparks", "" },
		{ "Toggles bullet sparks", "For shotgun and machinegun" },
		{ "Sets plasma trail effect", "" },
		{ "Toggles explosion sparks", "For rockets and grenades" }
	};

	UIX_CommonStatusBar(self, ID_RAIL, MAX_EFFECTS_INFO_MESSAGES, info_messages);
}

typedef struct {
	menuframework_s	menu;
	menutext_s		banner;
	menubitmap_s	back;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menulist_s	rail;
	menulist_s	railtime;
	menulist_s	rocketexpl;
	menulist_s	lgbeam;
	menulist_s	plasmatrail;

	menuradiobutton_s	explosparks;
	menuradiobutton_s	rockettrail;
	menuradiobutton_s	lgsparks;
	menuradiobutton_s	bulletsparks;
} AdvancedEffects_t;

static AdvancedEffects_t	s_effects;

static const char *rocketexpl_items[] =
{
	"default",
	"1.30",
	0
};

static const char *rail_items[] =
{
	"default",
	"spiral",
	"simple line",
	0
};

static const char *railtime_items[] =
{
	"fast",
	"default",
	"slow",
	"very slow",
	0
};

static const char *lgbeam_items[] =
{
	"default",
	"vanilla Q3",
	0
};

static const char *plasmatrail_items[] =
{
	"off",
	"1.30",
	"sparks",
	0
};

static void AdvancedEffects_SetMenuItems( void ) {
	int i = trap_Cvar_VariableValue( "cg_weaponEffects" );

	s_effects.rail.curvalue			= (i & WE_RAIL32) ? 1 : (i & WE_RAILSIMPLE) ? 2 : 0;
	s_effects.rocketexpl.curvalue	= i & WE_ROCKET32 ? 1 : 0;
	s_effects.lgbeam.curvalue		= i & WE_LG32 ? 0 : 1;
	s_effects.rockettrail.curvalue	= i & WE_Z_ROCKET_TRAIL ? 1 : 0;
	s_effects.plasmatrail.curvalue	= (i & WE_Z_PLASMA_TRAIL) ? 2 : (i & WE_PLASMA32) ? 1 : 0;
	s_effects.explosparks.curvalue	= i & WE_Z_EXPLOSIONS ? 1 : 0;
	s_effects.lgsparks.curvalue		= i & WE_Z_LG_SPARKS ? 1 : 0;
	s_effects.bulletsparks.curvalue	= i & WE_Z_BULLET_SPARKS ? 1 : 0;

	i = trap_Cvar_VariableValue( "cg_railTrailTime" );

	if (i < 400) s_effects.railtime.curvalue = 0;
	else if (i < 800) s_effects.railtime.curvalue = 1;
	else if (i < 1200) s_effects.railtime.curvalue = 2;
	else s_effects.railtime.curvalue = 3;
}

/*
=================
AdvancedEffects_SaveChanges
=================
*/
static void AdvancedEffects_SaveChanges() {
	int i = 0;

	if (s_effects.rail.curvalue == 1)
		i |= WE_RAIL32;
	else if (s_effects.rail.curvalue == 2)
		i |= WE_RAILSIMPLE;

	if (s_effects.rocketexpl.curvalue)
		i |= WE_ROCKET32;
	if (!s_effects.lgbeam.curvalue)
		i |= WE_LG32;
	if (s_effects.explosparks.curvalue)
		i |= WE_Z_EXPLOSIONS;
	if (s_effects.lgsparks.curvalue)
		i |= WE_Z_LG_SPARKS;
	if (s_effects.bulletsparks.curvalue)
		i |= WE_Z_BULLET_SPARKS;
	if (s_effects.rockettrail.curvalue)
		i |= WE_Z_ROCKET_TRAIL;

	if (s_effects.plasmatrail.curvalue == 2)
		i |= WE_Z_PLASMA_TRAIL;
	else if (s_effects.plasmatrail.curvalue == 1)
		i |= WE_PLASMA32;

	trap_Cvar_SetValue( "cg_weaponEffects", i );

	if( trap_Cvar_VariableValue( "cl_paused" ) )
		trap_Cmd_ExecuteText( EXEC_APPEND, "xmod reload effects\n" );
}

/*
=================
AdvancedEffects_Event
=================
*/
static void AdvancedEffects_Event( void* ptr, int event ) {
	int i;

	if (event != QM_ACTIVATED)
		return;

	switch (((menucommon_s*)ptr)->id) {
		case ID_RAIL_TIME:
			if (s_effects.railtime.curvalue == 0) i = 200;
			else if (s_effects.railtime.curvalue == 1) i = 400;
			else if (s_effects.railtime.curvalue == 2) i = 800;
			else i = 1200;

			trap_Cvar_SetValue( "cg_railTrailTime", i );
			break;
		case ID_EFFECTSBACK:
			AdvancedEffects_SaveChanges();
			UI_PopMenu();
			break;
	}
}

/*
=================
AdvancedEffects_MenuDraw
=================
*/
static void AdvancedEffects_MenuDraw( void ) {
	Menu_Draw( &s_effects.menu );
}

/*
=================
AdvancedEffects_MenuKey
=================
*/
static sfxHandle_t AdvancedEffects_MenuKey( int key )
{
	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		AdvancedEffects_SaveChanges();
	}

	return ( Menu_DefaultKey( &s_effects.menu, key ) );
}

/*
=================
AdvancedEffects_Cache
=================
*/
void AdvancedEffects_Cache( void ) {
	int	i;

	// touch all our pics
	for (i=0; ;i++) {
		if (!AdvancedEffects_artlist[i])
			break;
		trap_R_RegisterShaderNoMip(AdvancedEffects_artlist[i]);
	}
}

/*
=================
UI_AdvancedEffects_Menu
=================
*/
static void UI_AdvancedEffects_Menu( void ) {
	int		y;

	// zero set all our globals
	memset( &s_effects, 0 ,sizeof(AdvancedEffects_t) );

	AdvancedEffects_Cache();

	s_effects.menu.fullscreen = qtrue;
	s_effects.menu.draw       = AdvancedEffects_MenuDraw;
	s_effects.menu.key        = AdvancedEffects_MenuKey;

	s_effects.banner.generic.type  = MTYPE_BTEXT;
	s_effects.banner.generic.x	  = 320;
	s_effects.banner.generic.y	  = 16;
	s_effects.banner.string		  = "ADVANCED EFFECTS";
	s_effects.banner.color	      = color_white;
	s_effects.banner.style	      = UI_CENTER;

	s_effects.framel.generic.type  = MTYPE_BITMAP;
	s_effects.framel.generic.name  = EFFECTS_FRAMEL;
	s_effects.framel.generic.flags = QMF_INACTIVE;
	s_effects.framel.generic.x	   = 0;
	s_effects.framel.generic.y	   = 78;
	s_effects.framel.width  	   = 256;
	s_effects.framel.height  	   = 329;

	s_effects.framer.generic.type  = MTYPE_BITMAP;
	s_effects.framer.generic.name  = EFFECTS_FRAMER;
	s_effects.framer.generic.flags = QMF_INACTIVE;
	s_effects.framer.generic.x	   = 376;
	s_effects.framer.generic.y	   = 76;
	s_effects.framer.width  	   = 256;
	s_effects.framer.height  	   = 334;

	s_effects.back.generic.type	    = MTYPE_BITMAP;
	s_effects.back.generic.name     = EFFECTS_BACK0;
	s_effects.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_effects.back.generic.callback = AdvancedEffects_Event;
	s_effects.back.generic.id	    = ID_EFFECTSBACK;
	s_effects.back.generic.x		= 0;
	s_effects.back.generic.y		= 480-64;
	s_effects.back.width  		    = 128;
	s_effects.back.height  		    = 64;
	s_effects.back.focuspic         = EFFECTS_BACK1;

	y = 40+144 - BIGCHAR_HEIGHT * 3;

	y += BIGCHAR_HEIGHT+2;
	s_effects.rail.generic.type       = MTYPE_SPINCONTROL;
	s_effects.rail.generic.name	      = "Rail Trail:";
	s_effects.rail.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.rail.generic.callback   = AdvancedEffects_Event;
	s_effects.rail.generic.id         = ID_RAIL;
	s_effects.rail.generic.x	      = PREFERENCES_X_POS;
	s_effects.rail.generic.y	      = y;
	s_effects.rail.itemnames          = rail_items;
	s_effects.rail.generic.statusbar  = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.railtime.generic.type       = MTYPE_SPINCONTROL;
	s_effects.railtime.generic.name	      = "Rail Trail Time:";
	s_effects.railtime.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.railtime.generic.callback   = AdvancedEffects_Event;
	s_effects.railtime.generic.id         = ID_RAIL_TIME;
	s_effects.railtime.generic.x	      = PREFERENCES_X_POS;
	s_effects.railtime.generic.y	      = y;
	s_effects.railtime.itemnames          = railtime_items;
	s_effects.railtime.generic.statusbar  = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.rocketexpl.generic.type       = MTYPE_SPINCONTROL;
	s_effects.rocketexpl.generic.name	      = "Rocket Explosion:";
	s_effects.rocketexpl.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.rocketexpl.generic.callback   = AdvancedEffects_Event;
	s_effects.rocketexpl.generic.id         = ID_ROCKET32;
	s_effects.rocketexpl.generic.x	      = PREFERENCES_X_POS;
	s_effects.rocketexpl.generic.y	      = y;
	s_effects.rocketexpl.itemnames = rocketexpl_items;
	s_effects.rocketexpl.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.lgbeam.generic.type       = MTYPE_SPINCONTROL;
	s_effects.lgbeam.generic.name	      = "Lightning Gun Beam:";
	s_effects.lgbeam.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.lgbeam.generic.callback   = AdvancedEffects_Event;
	s_effects.lgbeam.generic.id         = ID_LG_BEAM;
	s_effects.lgbeam.generic.x	      = PREFERENCES_X_POS;
	s_effects.lgbeam.generic.y	      = y;
	s_effects.lgbeam.itemnames = lgbeam_items;
	s_effects.lgbeam.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.explosparks.generic.type          = MTYPE_RADIOBUTTON;
	s_effects.explosparks.generic.name	      = "Explosion Sparks:";
	s_effects.explosparks.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.explosparks.generic.callback      = AdvancedEffects_Event;
	s_effects.explosparks.generic.id            = ID_EXPLOSION_SPARKS;
	s_effects.explosparks.generic.x	          = PREFERENCES_X_POS;
	s_effects.explosparks.generic.y	          = y;
	s_effects.explosparks.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.rockettrail.generic.type       = MTYPE_RADIOBUTTON;
	s_effects.rockettrail.generic.name	      = "Rocket Trail Sparks:";
	s_effects.rockettrail.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.rockettrail.generic.callback   = AdvancedEffects_Event;
	s_effects.rockettrail.generic.id         = ID_ROCKET_TRAIL;
	s_effects.rockettrail.generic.x	      = PREFERENCES_X_POS;
	s_effects.rockettrail.generic.y	      = y;
	s_effects.rockettrail.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.lgsparks.generic.type          = MTYPE_RADIOBUTTON;
	s_effects.lgsparks.generic.name	      = "Lightning Gun Sparks:";
	s_effects.lgsparks.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.lgsparks.generic.callback      = AdvancedEffects_Event;
	s_effects.lgsparks.generic.id            = ID_LG_SPARKS;
	s_effects.lgsparks.generic.x	          = PREFERENCES_X_POS;
	s_effects.lgsparks.generic.y	          = y;
	s_effects.lgsparks.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.bulletsparks.generic.type          = MTYPE_RADIOBUTTON;
	s_effects.bulletsparks.generic.name	      = "Bullet Sparks:";
	s_effects.bulletsparks.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.bulletsparks.generic.callback      = AdvancedEffects_Event;
	s_effects.bulletsparks.generic.id            = ID_BULLET_SPARKS;
	s_effects.bulletsparks.generic.x	          = PREFERENCES_X_POS;
	s_effects.bulletsparks.generic.y	          = y;
	s_effects.bulletsparks.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_effects.plasmatrail.generic.type       = MTYPE_SPINCONTROL;
	s_effects.plasmatrail.generic.name	      = "Plasma Trail:";
	s_effects.plasmatrail.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_effects.plasmatrail.generic.callback   = AdvancedEffects_Event;
	s_effects.plasmatrail.generic.id         = ID_PLASMA_TRAIL;
	s_effects.plasmatrail.generic.x	      = PREFERENCES_X_POS;
	s_effects.plasmatrail.generic.y	      = y;
	s_effects.plasmatrail.itemnames = plasmatrail_items;
	s_effects.plasmatrail.generic.statusbar   = UI_AdvancedEffects_StatusBar;

	Menu_AddItem( &s_effects.menu, &s_effects.rail );
	Menu_AddItem( &s_effects.menu, &s_effects.railtime );
	Menu_AddItem( &s_effects.menu, &s_effects.rocketexpl );
	Menu_AddItem( &s_effects.menu, &s_effects.rockettrail );
	Menu_AddItem( &s_effects.menu, &s_effects.explosparks );
	Menu_AddItem( &s_effects.menu, &s_effects.lgbeam );
	Menu_AddItem( &s_effects.menu, &s_effects.lgsparks );
	Menu_AddItem( &s_effects.menu, &s_effects.bulletsparks );
	Menu_AddItem( &s_effects.menu, &s_effects.plasmatrail );

	Menu_AddItem( &s_effects.menu, &s_effects.banner );
	Menu_AddItem( &s_effects.menu, &s_effects.framel );
	Menu_AddItem( &s_effects.menu, &s_effects.framer );
	Menu_AddItem( &s_effects.menu, &s_effects.back );

	AdvancedEffects_SetMenuItems();

	UI_PushMenu( &s_effects.menu );
}

/*
=======================================================================

GAME OPTIONS MENU

=======================================================================
*/




#define ART_FRAMEL				"menu/art/frame2_l"
#define ART_FRAMER				"menu/art/frame1_r"
#define ART_BACK0				"menu/art/back_0"
#define ART_BACK1				"menu/art/back_1"


#define ID_CROSSHAIR			127
#define ID_SIMPLEITEMS			128
#define ID_HIGHQUALITYSKY		129
#define ID_EJECTINGBRASS		130
#define ID_WALLMARKS			131
#define ID_DYNAMICLIGHTS		132
#define ID_IDENTIFYTARGET		133
#define ID_SYNCEVERYFRAME		134
#define ID_FORCEMODEL			135
#define ID_DRAWTEAMOVERLAY		136
#define ID_ALLOWDOWNLOAD		137

#define ID_CROSSHAIR_COLOR		138
#define ID_CROSSHAIR_SIZE		139

#define ID_EFFECTS				140
//#define ID_SHAREDCONFIG			140

#define ID_BACK					150

#define	NUM_CROSSHAIRS			10

#define MAX_INFO_MESSAGES	13
static void UI_Preferences_StatusBar( void *self ) {	
	static const char *info_messages[MAX_INFO_MESSAGES][2] = {
		{ "Sets ingame crosshair", "" },
		{ "Sets display of objects in game", "'On' - all weapons and items will be 2D, 'Off' - 3D" },
		{ "If it's off sky will not draw", "May increase slightly FPS on old PC" },
		{ "Toggles ejecting brass", "" },
		{ "Toggles display of marks on walls", "Turning off may increase FPS in some cases" },
		{ "Renders realtime dynamic lights", "Turning off may increase FPS on some PC" },
		{ "Display target name above crosshair", "To show team info 'teamoverlay' must be enabled" },
		{ "Synchronization of rendered frames (Not V-Sync.)", "Engine will wait for GL calls to finish" },
		{ "Forces all player models to be same", "All enemies in game will be using player's model" },
		{ "Sets team overlay draw position", "" },
		{ "Allows autodowloading content from pure servers", "" },
		{ "Sets crosshair color", "" },
		{ "Sets crosshair size", "" },
		//{ "Toggles auto saving q3config.cfg into baseq3 folder", "Fixes problems of not saving config after game exit"}
	};

	UIX_CommonStatusBar(self, ID_CROSSHAIR, MAX_INFO_MESSAGES, info_messages);
}

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;

	menulist_s			crosshair;
	menulist_s			crosshaircolor;
	menulist_s			crosshairsize;
	menuradiobutton_s	simpleitems;
	menuradiobutton_s	brass;
	menuradiobutton_s	wallmarks;
	menuradiobutton_s	dynamiclights;
	menulist_s			identifytarget;
	menuradiobutton_s	highqualitysky;
	menuradiobutton_s	synceveryframe;
	menuradiobutton_s	forcemodel;
	menulist_s			drawteamoverlay;
	menuradiobutton_s	allowdownload;
	menubitmap_s		back;
	//menuradiobutton_s	sharedconfig;
	menutext_s			effects;

	qhandle_t			defaultCrosshair[NUM_CROSSHAIRS];
	qhandle_t			crosshairShader[NUM_CROSSHAIRS];
} preferences_t;

static preferences_t s_preferences;

static int crosshaircolor;
static float crosshairsize;

static const char *teamoverlay_names[] =
{
	"off",
	"upper right",
	"lower right",
	"lower left",
	0
};

static const char *crosshair_colors[] =
{	
	"default",	
	"black",
	"red",
	"green",
	"yellow",
	"blue",
	"cyan",
	"magenta",		
	"white",
	"olive",
	"maroon",
	"purple",
	"teal",
	"office green",
	"navy blue",
	"coral red",
	"15",
	"16",
	"17",
	"18",
	"19",
	"20",
	"21",
	"22",
	"23",
	"24",
	"25",
	"26",
	"27",
	"28",
	"29",
	"30",
	"31",
	"32",
	"33",
	"34",
	"35",	
	"36",
	"37",
	"38",
	0
};

//"orange-red",
//"dark orange",
//"orange",
//"green-yellow",
//"chartreuse",
//"green 1",
//"spring green 1",
//"spring green 2",
//"green-cyan",

static const char *crosshairsize_items[] =
{
	"small",
	"smaller",
	"default",
	"bigger",
	"big",
	0
};

static const char *drawnames_items[] = {
	"off",
	"name only",
	"name+team info text",
	"name+team info bars",
	0
};

static void Preferences_SetMenuItems( void ) {
	char buf[32];
	
	s_preferences.crosshair.curvalue		= abs((int)trap_Cvar_VariableValue( "cg_drawCrosshair" )) % NUM_CROSSHAIRS;
	s_preferences.simpleitems.curvalue		= trap_Cvar_VariableValue( "cg_simpleItems" ) != 0;
	s_preferences.brass.curvalue			= trap_Cvar_VariableValue( "cg_brassTime" ) != 0;
	s_preferences.wallmarks.curvalue		= trap_Cvar_VariableValue( "cg_marks" ) != 0;
	s_preferences.identifytarget.curvalue	= abs((int)trap_Cvar_VariableValue( "cg_drawCrosshairNames" )) % ArrLen(drawnames_items);
	s_preferences.dynamiclights.curvalue	= trap_Cvar_VariableValue( "r_dynamiclight" ) != 0;
	s_preferences.highqualitysky.curvalue	= trap_Cvar_VariableValue ( "r_fastsky" ) == 0;
	s_preferences.synceveryframe.curvalue	= trap_Cvar_VariableValue( "r_finish" ) != 0;
	s_preferences.forcemodel.curvalue		= trap_Cvar_VariableValue( "cg_forcemodel" ) != 0;
	s_preferences.drawteamoverlay.curvalue	= Com_Clamp( 0, 3, trap_Cvar_VariableValue( "cg_drawTeamOverlay" ) );
	s_preferences.allowdownload.curvalue	= trap_Cvar_VariableValue( "cl_allowDownload" ) != 0;

	//s_preferences.sharedconfig.curvalue		= trap_Cvar_VariableValue("cg_sharedConfig") != 0;

	trap_Cvar_VariableStringBuffer("cg_crosshairColor", buf, sizeof(buf));

	if (!buf[0])
		crosshaircolor = 0;
	else if (crosshaircolor = QX_StringToColor(buf))
		crosshaircolor = ColorIndex(crosshaircolor) + 1;		
	else
		crosshaircolor = atoi(buf) % ArrLen(g_color_table_ex) + 1;

	s_preferences.crosshaircolor.curvalue = crosshaircolor;

	crosshairsize = trap_Cvar_VariableValue("cg_crosshairSize");	
	if (crosshairsize <= 16)
		s_preferences.crosshairsize.curvalue = 0;
	else if (crosshairsize <= 20)
		s_preferences.crosshairsize.curvalue = 1;
	else if (crosshairsize <= 24)
		s_preferences.crosshairsize.curvalue = 2;
	else if (crosshairsize <= 28)
		s_preferences.crosshairsize.curvalue = 3;
	else
		s_preferences.crosshairsize.curvalue = 4;
	crosshairsize = s_preferences.crosshairsize.curvalue * 4 + 16;
}

static void Preferences_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_CROSSHAIR:
		s_preferences.crosshair.curvalue++;
		if( s_preferences.crosshair.curvalue == NUM_CROSSHAIRS ) {
			s_preferences.crosshair.curvalue = 0;
		}
		trap_Cvar_SetValue( "cg_drawCrosshair", s_preferences.crosshair.curvalue );
		break;

	case ID_CROSSHAIR_COLOR:			
		crosshaircolor = s_preferences.crosshaircolor.curvalue;
		if (crosshaircolor == 0)
			trap_Cvar_Set("cg_crosshairColor", "");
		else
			trap_Cvar_SetValue("cg_crosshairColor", crosshaircolor - 1);
		break;

	case ID_CROSSHAIR_SIZE:		
		crosshairsize = s_preferences.crosshairsize.curvalue * 4 + 16;
		trap_Cvar_SetValue("cg_crosshairSize", crosshairsize);
		break;

	case ID_SIMPLEITEMS:
		trap_Cvar_SetValue( "cg_simpleItems", s_preferences.simpleitems.curvalue );
		break;

	case ID_HIGHQUALITYSKY:
		trap_Cvar_SetValue( "r_fastsky", !s_preferences.highqualitysky.curvalue );
		break;

	case ID_EJECTINGBRASS:
		if ( s_preferences.brass.curvalue )
			trap_Cvar_Reset( "cg_brassTime" );
		else
			trap_Cvar_SetValue( "cg_brassTime", 0 );
		break;

	case ID_WALLMARKS:
		trap_Cvar_SetValue( "cg_marks", s_preferences.wallmarks.curvalue );
		break;

	case ID_DYNAMICLIGHTS:
		trap_Cvar_SetValue( "r_dynamiclight", s_preferences.dynamiclights.curvalue );
		break;		

	case ID_IDENTIFYTARGET:
		trap_Cvar_SetValue( "cg_drawCrosshairNames", s_preferences.identifytarget.curvalue );
		break;

	case ID_SYNCEVERYFRAME:
		trap_Cvar_SetValue( "r_finish", s_preferences.synceveryframe.curvalue );
		break;

	case ID_FORCEMODEL:
		trap_Cvar_SetValue( "cg_forcemodel", s_preferences.forcemodel.curvalue );
		break;

	case ID_DRAWTEAMOVERLAY:
		trap_Cvar_SetValue( "cg_drawTeamOverlay", s_preferences.drawteamoverlay.curvalue );
		break;

	case ID_ALLOWDOWNLOAD:
		trap_Cvar_SetValue( "cl_allowDownload", s_preferences.allowdownload.curvalue );
		break;

	case ID_EFFECTS:
		UI_AdvancedEffects_Menu();
		break;

	//case ID_SHAREDCONFIG:
	//	trap_Cvar_SetValue("cg_sharedConfig", s_preferences.sharedconfig.curvalue);
	//	break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
=================
Crosshair_Draw
=================
*/
static void Crosshair_Draw( void *self ) {
	menulist_s	*s;
	float		*color;
	int			x, y;
	int			style;
	qboolean	focus;

	s = (menulist_s *)self;
	x = s->generic.x;
	y =	s->generic.y;

	style = UI_SMALLFONT;
	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if ( s->generic.flags & QMF_GRAYED )
		color = text_color_disabled;
	else if ( focus )
	{
		color = text_color_highlight;
		style |= UI_PULSE;
	}
	else if ( s->generic.flags & QMF_BLINK )
	{
		color = text_color_highlight;
		style |= UI_BLINK;
	}
	else
		color = text_color_normal;

	if ( focus )
	{
		// draw cursor
		UI_FillRect( s->generic.left, s->generic.top, s->generic.right-s->generic.left+1, s->generic.bottom-s->generic.top+1, listbar_color ); 
		UI_DrawChar( x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	UI_DrawString( x - SMALLCHAR_WIDTH, y, s->generic.name, style|UI_RIGHT, color );
	if( !s->curvalue ) {
		return;
	}

	if (crosshaircolor > 0) {
		trap_R_SetColor(g_color_table_ex[crosshaircolor - 1]);

		UI_DrawHandlePic(x + SMALLCHAR_WIDTH + (24 - crosshairsize) / 2, y - 4 + (24 - crosshairsize) / 2,
			crosshairsize, crosshairsize, s_preferences.crosshairShader[s->curvalue]);
	} else if (crosshaircolor == 0) {
		if (!s_preferences.defaultCrosshair[s->curvalue])
			s_preferences.defaultCrosshair[s->curvalue] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + s->curvalue ) );
		UI_DrawHandlePic(x + SMALLCHAR_WIDTH + (24 - crosshairsize) / 2, y - 4 + (24 - crosshairsize) / 2,
			crosshairsize, crosshairsize, s_preferences.defaultCrosshair[s->curvalue]);
	}

	trap_R_SetColor(NULL);
}


static void Preferences_MenuInit( void ) {
	int				y;

	memset( &s_preferences, 0 ,sizeof(preferences_t) );

	Preferences_Cache();

	s_preferences.menu.wrapAround = qtrue;
	s_preferences.menu.fullscreen = qtrue;

	s_preferences.banner.generic.type  = MTYPE_BTEXT;
	s_preferences.banner.generic.x	   = 320;
	s_preferences.banner.generic.y	   = 16;
	s_preferences.banner.string		   = "GAME OPTIONS";
	s_preferences.banner.color         = color_white;
	s_preferences.banner.style         = UI_CENTER;

	s_preferences.framel.generic.type  = MTYPE_BITMAP;
	s_preferences.framel.generic.name  = ART_FRAMEL;
	s_preferences.framel.generic.flags = QMF_INACTIVE;
	s_preferences.framel.generic.x	   = 0;
	s_preferences.framel.generic.y	   = 78;
	s_preferences.framel.width  	   = 256;
	s_preferences.framel.height  	   = 329;

	s_preferences.framer.generic.type  = MTYPE_BITMAP;
	s_preferences.framer.generic.name  = ART_FRAMER;
	s_preferences.framer.generic.flags = QMF_INACTIVE;
	s_preferences.framer.generic.x	   = 376;
	s_preferences.framer.generic.y	   = 76;
	s_preferences.framer.width  	   = 256;
	s_preferences.framer.height  	   = 334;

	y = 144 - BIGCHAR_HEIGHT * 3;
	s_preferences.crosshair.generic.type		= MTYPE_TEXT;
	s_preferences.crosshair.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NODEFAULTINIT|QMF_OWNERDRAW;
	s_preferences.crosshair.generic.x			= PREFERENCES_X_POS;
	s_preferences.crosshair.generic.y			= y;
	s_preferences.crosshair.generic.name		= "Crosshair:";
	s_preferences.crosshair.generic.callback	= Preferences_Event;
	s_preferences.crosshair.generic.ownerdraw	= Crosshair_Draw;
	s_preferences.crosshair.generic.id			= ID_CROSSHAIR;
	s_preferences.crosshair.generic.top			= y - 4;
	s_preferences.crosshair.generic.bottom		= y + 20;
	s_preferences.crosshair.generic.left		= PREFERENCES_X_POS - ( ( strlen(s_preferences.crosshair.generic.name) + 1 ) * SMALLCHAR_WIDTH );
	s_preferences.crosshair.generic.right		= PREFERENCES_X_POS + 48;
	s_preferences.crosshair.generic.statusbar   = UI_Preferences_StatusBar;


	y += BIGCHAR_HEIGHT + 2 + 4 + 8;
	s_preferences.crosshaircolor.generic.type = MTYPE_SPINCONTROL;
	s_preferences.crosshaircolor.generic.name = "Crosshair color:";
	s_preferences.crosshaircolor.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences.crosshaircolor.generic.callback = Preferences_Event;
	s_preferences.crosshaircolor.generic.id = ID_CROSSHAIR_COLOR;
	s_preferences.crosshaircolor.generic.x = PREFERENCES_X_POS;
	s_preferences.crosshaircolor.generic.y = y;
	s_preferences.crosshaircolor.itemnames = crosshair_colors;
	s_preferences.crosshaircolor.generic.statusbar   = UI_Preferences_StatusBar;


	y += BIGCHAR_HEIGHT + 2;
	s_preferences.crosshairsize.generic.type = MTYPE_SPINCONTROL;
	s_preferences.crosshairsize.generic.name = "Crosshair size:";
	s_preferences.crosshairsize.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences.crosshairsize.generic.callback = Preferences_Event;
	s_preferences.crosshairsize.generic.id = ID_CROSSHAIR_SIZE;
	s_preferences.crosshairsize.generic.x = PREFERENCES_X_POS;
	s_preferences.crosshairsize.generic.y = y;
	s_preferences.crosshairsize.itemnames = crosshairsize_items;
	s_preferences.crosshairsize.generic.statusbar   = UI_Preferences_StatusBar;


	y += BIGCHAR_HEIGHT * 2 + 2;
	s_preferences.simpleitems.generic.type        = MTYPE_RADIOBUTTON;
	s_preferences.simpleitems.generic.name	      = "Simple Items:";
	s_preferences.simpleitems.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.simpleitems.generic.callback    = Preferences_Event;
	s_preferences.simpleitems.generic.id          = ID_SIMPLEITEMS;
	s_preferences.simpleitems.generic.x	          = PREFERENCES_X_POS;
	s_preferences.simpleitems.generic.y	          = y;
	s_preferences.simpleitems.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT;
	s_preferences.wallmarks.generic.type          = MTYPE_RADIOBUTTON;
	s_preferences.wallmarks.generic.name	      = "Marks on Walls:";
	s_preferences.wallmarks.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.wallmarks.generic.callback      = Preferences_Event;
	s_preferences.wallmarks.generic.id            = ID_WALLMARKS;
	s_preferences.wallmarks.generic.x	          = PREFERENCES_X_POS;
	s_preferences.wallmarks.generic.y	          = y;
	s_preferences.wallmarks.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.brass.generic.type              = MTYPE_RADIOBUTTON;
	s_preferences.brass.generic.name	          = "Ejecting Brass:";
	s_preferences.brass.generic.flags	          = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.brass.generic.callback          = Preferences_Event;
	s_preferences.brass.generic.id                = ID_EJECTINGBRASS;
	s_preferences.brass.generic.x	              = PREFERENCES_X_POS;
	s_preferences.brass.generic.y	              = y;
	s_preferences.brass.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.dynamiclights.generic.type      = MTYPE_RADIOBUTTON;
	s_preferences.dynamiclights.generic.name	  = "Dynamic Lights:";
	s_preferences.dynamiclights.generic.flags     = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.dynamiclights.generic.callback  = Preferences_Event;
	s_preferences.dynamiclights.generic.id        = ID_DYNAMICLIGHTS;
	s_preferences.dynamiclights.generic.x	      = PREFERENCES_X_POS;
	s_preferences.dynamiclights.generic.y	      = y;
	s_preferences.dynamiclights.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.identifytarget.generic.type     = MTYPE_SPINCONTROL;
	s_preferences.identifytarget.generic.name	  = "Identify Target:";
	s_preferences.identifytarget.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.identifytarget.generic.callback = Preferences_Event;
	s_preferences.identifytarget.generic.id       = ID_IDENTIFYTARGET;
	s_preferences.identifytarget.generic.x	      = PREFERENCES_X_POS;
	s_preferences.identifytarget.generic.y	      = y;
	s_preferences.identifytarget.generic.statusbar   = UI_Preferences_StatusBar;
	s_preferences.identifytarget.itemnames        = drawnames_items;


	y += BIGCHAR_HEIGHT+2;
	s_preferences.highqualitysky.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.highqualitysky.generic.name	  = "High Quality Sky:";
	s_preferences.highqualitysky.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.highqualitysky.generic.callback = Preferences_Event;
	s_preferences.highqualitysky.generic.id       = ID_HIGHQUALITYSKY;
	s_preferences.highqualitysky.generic.x	      = PREFERENCES_X_POS;
	s_preferences.highqualitysky.generic.y	      = y;
	s_preferences.highqualitysky.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.synceveryframe.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.synceveryframe.generic.name	  = "Sync Every Frame:";
	s_preferences.synceveryframe.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.synceveryframe.generic.callback = Preferences_Event;
	s_preferences.synceveryframe.generic.id       = ID_SYNCEVERYFRAME;
	s_preferences.synceveryframe.generic.x	      = PREFERENCES_X_POS;
	s_preferences.synceveryframe.generic.y	      = y;
	s_preferences.synceveryframe.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.forcemodel.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.forcemodel.generic.name	  = "Force Player Models:";
	s_preferences.forcemodel.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.forcemodel.generic.callback = Preferences_Event;
	s_preferences.forcemodel.generic.id       = ID_FORCEMODEL;
	s_preferences.forcemodel.generic.x	      = PREFERENCES_X_POS;
	s_preferences.forcemodel.generic.y	      = y;
	s_preferences.forcemodel.generic.statusbar   = UI_Preferences_StatusBar;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.drawteamoverlay.generic.type     = MTYPE_SPINCONTROL;
	s_preferences.drawteamoverlay.generic.name	   = "Draw Team Overlay:";
	s_preferences.drawteamoverlay.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.drawteamoverlay.generic.callback = Preferences_Event;
	s_preferences.drawteamoverlay.generic.id       = ID_DRAWTEAMOVERLAY;
	s_preferences.drawteamoverlay.generic.x	       = PREFERENCES_X_POS;
	s_preferences.drawteamoverlay.generic.y	       = y;
	s_preferences.drawteamoverlay.itemnames			= teamoverlay_names;
	s_preferences.drawteamoverlay.generic.statusbar   = UI_Preferences_StatusBar;
	if (uis.q3version >= 16)
	y += BIGCHAR_HEIGHT+2;
	s_preferences.allowdownload.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.allowdownload.generic.name	   = "Automatic Downloading:";
	s_preferences.allowdownload.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.allowdownload.generic.callback = Preferences_Event;
	s_preferences.allowdownload.generic.id       = ID_ALLOWDOWNLOAD;
	s_preferences.allowdownload.generic.x	       = PREFERENCES_X_POS;
	s_preferences.allowdownload.generic.y	       = y;
	s_preferences.allowdownload.generic.statusbar   = UI_Preferences_StatusBar;

	//y += (BIGCHAR_HEIGHT+2)*2;	
	//s_preferences.sharedconfig.generic.type = MTYPE_RADIOBUTTON;
	//s_preferences.sharedconfig.generic.name = "Shared Config:";
	//s_preferences.sharedconfig.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	//s_preferences.sharedconfig.generic.callback = Preferences_Event;
	//s_preferences.sharedconfig.generic.statusbar = UI_Preferences_StatusBar;
	//s_preferences.sharedconfig.generic.id = ID_SHAREDCONFIG;
	//s_preferences.sharedconfig.generic.x = PREFERENCES_X_POS;
	//s_preferences.sharedconfig.generic.y = y;

	y += (BIGCHAR_HEIGHT+2) * 2;
	s_preferences.effects.generic.type     = MTYPE_PTEXT;
	s_preferences.effects.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_preferences.effects.generic.callback = Preferences_Event;
	s_preferences.effects.generic.id       = ID_EFFECTS;
	s_preferences.effects.generic.x        = 320;
	s_preferences.effects.generic.y        = y;
	s_preferences.effects.string           = "Advanced Effects";
	s_preferences.effects.style            = UI_CENTER|UI_SMALLFONT;
	s_preferences.effects.color            = color_red;

	s_preferences.back.generic.type	    = MTYPE_BITMAP;
	s_preferences.back.generic.name     = ART_BACK0;
	s_preferences.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_preferences.back.generic.callback = Preferences_Event;
	s_preferences.back.generic.id	    = ID_BACK;
	s_preferences.back.generic.x		= 0;
	s_preferences.back.generic.y		= 480-64;
	s_preferences.back.width  		    = 128;
	s_preferences.back.height  		    = 64;
	s_preferences.back.focuspic         = ART_BACK1;

	Menu_AddItem( &s_preferences.menu, &s_preferences.banner );
	Menu_AddItem( &s_preferences.menu, &s_preferences.framel );
	Menu_AddItem( &s_preferences.menu, &s_preferences.framer );

	Menu_AddItem( &s_preferences.menu, &s_preferences.crosshair );
	Menu_AddItem( &s_preferences.menu, &s_preferences.simpleitems );
	Menu_AddItem( &s_preferences.menu, &s_preferences.wallmarks );
	Menu_AddItem( &s_preferences.menu, &s_preferences.brass );
	Menu_AddItem( &s_preferences.menu, &s_preferences.dynamiclights );
	Menu_AddItem( &s_preferences.menu, &s_preferences.identifytarget );
	Menu_AddItem( &s_preferences.menu, &s_preferences.highqualitysky );
	Menu_AddItem( &s_preferences.menu, &s_preferences.synceveryframe );
	Menu_AddItem( &s_preferences.menu, &s_preferences.forcemodel );
	Menu_AddItem( &s_preferences.menu, &s_preferences.drawteamoverlay );
	if (uis.q3version >= 16)
	Menu_AddItem( &s_preferences.menu, &s_preferences.allowdownload );
	Menu_AddItem( &s_preferences.menu, &s_preferences.crosshaircolor );
	Menu_AddItem( &s_preferences.menu, &s_preferences.crosshairsize);

	//Menu_AddItem( &s_preferences.menu, &s_preferences.sharedconfig );
#if CGX_Z_EFFECTS
	Menu_AddItem( &s_preferences.menu, &s_preferences.effects );
#endif

	Menu_AddItem( &s_preferences.menu, &s_preferences.back );

	Preferences_SetMenuItems();
}


/*
===============
Preferences_Cache
===============
*/
void Preferences_Cache( void ) {
	int		n;

	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	for( n = 0; n < NUM_CROSSHAIRS; n++ ) {		
		s_preferences.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/xm_crosshair%c", 'a' + n ) );
		/*s_preferences.defaultCrosshair[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );*/
	}
}


/*
===============
UI_PreferencesMenu
===============
*/
void UI_PreferencesMenu( void ) {
	Preferences_MenuInit();
	UI_PushMenu( &s_preferences.menu );
}
