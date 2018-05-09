// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
/*
=======================================================================

ADVANCED OPTIONS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_FRAMEL				"menu/art/frame2_l"
#define ART_FRAMER				"menu/art/frame1_r"
#define ART_BACK0				"menu/art/back_0"
#define ART_BACK1				"menu/art/back_1"

#define PREFERENCES_X_POS		340
//360
#define PREFERENCES_X_POS_1		230
#define PREFERENCES_X_POS_2		450

#define ID_REWARDS				127
#define ID_BLOOD				128
#define ID_GIBS					129
#define ID_CAMERABOB			130
//#define ID_PLAYERIDS			131
#define ID_FOV					132
#define ID_TIMER				133
#define ID_ENEMYMODEL			134
#define ID_ENEMYCOLORS			135
#define ID_DRAW3DICONS			136
#define ID_ZOOMFOV				137
#define ID_ENEMYMODEL_ENABLED	138
#define ID_CHATBEEP				139
#define ID_ENEMY_TAUNT			140
#define ID_CENTER_PRINT			141
#define ID_SPEED				142
#define ID_DRAW_GUN				143
#define ID_TEAMMODEL			144
#define ID_TEAMCOLORS			145
//#define ID_COLOREDPING			146
#define ID_DEFAULTWEAPON		147
#define ID_LAGOMETER			148
#define ID_HITSOUNDS			149
#define ID_SCOREBOARD			150
#define ID_ACC					151
//#define ID_SCOREBOX				152
#define ID_SHAREDCONFIG			152

#define ID_BACK					190

#define MAX_INFO_MESSAGES		26
static void Preferences2_StatusBar( void *self ) {	
	static const char *info_messages[MAX_INFO_MESSAGES][2] = {
		{ "Toggles display ingame rewards", "On screen center - Excellent, Impressive etc."},
		{ "Toggles display blood after kill", ""},
		{ "Toggles display gibs, if blood is on", ""},
		{ "Toggles camera bobbing when running","Recommended 'Off'"},
		{"",""},
		//{ "Toggles player ids display on scoreboard",""},
		{ "Field of view in degrees","Min - 1 Max - 160"},
		{ "Sets ingame timer display position",""},
		{ "Forces all enemies one model eg. 'keel/pm' 'tankjr' ","Leave emtpy to use pm skin based on model"},
		{ "Forces all enemies colors eg. '2222' '1234' '2!2!'","'?' and '!' - use team color '*' and '.' - random"},
		{ "Toggles 3D icons display (ammo, armor etc.)","If it's on icons will be 3D, off - 2D"},
		{ "Field of view when zoom in degrees","Min - 1 Max - 160"},
		{ "Enables forcing bright enemy and team models",""},
		{ "Toggles beep when chatting",""},
		{ "Toggles enemy taunt sound",""},
		{ "'You fragged...' message transparency level",""},
		{ "Sets display ingame speedometer",""},
		{ "Sets draw gun method", "off - no gun, defaul - bobbing, still - promode"},
		{ "Forces all teammates one model eg. 'bitterman/pm'","Leave emtpy to use pm skin based on model"},
		{ "Forces all teammates colors eg. '!!!!' '5555' 'xyzw'","'?' and '!' - use team color '*' and '.' - random"},
		{"",""},
		//{ "Toggles colored ping on scoreboard","" },
		{ "Sets default weapon switch after respawn","If server sends you BFG but you want shotgun" },
		{ "Draw ingame lagometer", "" },
		{ "Sets hitsounds default - one hit sound", "Other options 4 sounds based on damage done"},		
		{ "Sets ingame scoreboard type", ""},
		{ "Toggles display total weapon accuracy", ""},
		/*{ "Toggles display of scorebox in right lower corner", ""}*/
		{ "Toggles auto saving q3config.cfg into baseq3 folder", "Fixes problems of not saving config after game exit"},
	};

	UIX_CommonStatusBar(self, ID_REWARDS, MAX_INFO_MESSAGES, info_messages);
}

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
	
	menuradiobutton_s	rewards;
	menulist_s			timer;
	menulist_s			speed;
	menulist_s			drawgun;
	menuradiobutton_s	blood;
	menuradiobutton_s	gibs;
	menuradiobutton_s	camerabob;
	//menuradiobutton_s	playerids;
	menuradiobutton_s	draw3dicons;
	menuradiobutton_s	chatbeep;
	menuradiobutton_s	enemytaunt;
	//menuradiobutton_s	coloredping;
	//menuradiobutton_s	scorebox;
	menuradiobutton_s	accuracy;
	menuradiobutton_s	sharedconfig;
	menulist_s			scoreboard;
	menulist_s			centerprint;
	menulist_s			deafultweapon;
	menulist_s			hitsounds;
	menufield_s			fov;
	menufield_s			zoomfov;
	menulist_s			lagometer;
	menuradiobutton_s	enemymodelenabled;
	menufield_s			enemymodel;
	menufield_s			enemycolors;	
	menufield_s			teammodel;
	menufield_s			teamcolors;	

	menubitmap_s		back;	
} preferences_t;

static preferences_t s_preferences2;

static const char *centerprint_items[] =
{
	"off",
	"transparent",
	"on",
	0
};

static const char *timer_items[] =
{
	"off",
	"top right",
	"top center",
	0
};

static const char *speed_items[] =
{
	"off",
	"top right",
	"under crosshair",
	0
};

static const char *drawgun_items[] =
{
	"off",
	"default",
	"still",
	0
};

static const char *defaultweapon_items[] = {
	"default",
	"gauntlet",
	"machinegun",
	"shotgun",
	"grenades",
	"rocket",
	"lightning",
	"rail",
	"plasma",
	"BFG",
	0
};

static const char *lagometer_items[] = {
	"off",
	"default",
	"default + ping",
	"when packetloss",
	0
};

static const char *hitsounds_items[] = {
	"default",
	"hi-low",
	"low-hi",
	0
};

static const char *scoreboar_items[] = {
	"default",
	"always small",
	"default Q3",
	0
};

static void Preferences2_SetMenuItems( void ) {
	s_preferences2.rewards.curvalue		= trap_Cvar_VariableValue( "cg_drawRewards" ) != 0;
	s_preferences2.timer.curvalue		= abs((int)trap_Cvar_VariableValue( "cg_drawTimer" ) % 3);
	s_preferences2.speed.curvalue		= abs((int)trap_Cvar_VariableValue("cg_drawSpeed") % 3);
	s_preferences2.blood.curvalue		= trap_Cvar_VariableValue( "com_blood" ) != 0;
	s_preferences2.gibs.curvalue		= trap_Cvar_VariableValue( "cg_gibs" ) != 0;
	//s_preferences2.playerids.curvalue	= trap_Cvar_VariableValue( "cg_drawPlayerIDs" ) != 0;	
	s_preferences2.draw3dicons.curvalue	= trap_Cvar_VariableValue( "cg_draw3Dicons" ) != 0;
	s_preferences2.camerabob.curvalue	= trap_Cvar_VariableValue( "cg_bobup" ) != 0 
										|| trap_Cvar_VariableValue( "cg_bobpitch" ) != 0 
										|| trap_Cvar_VariableValue( "cg_bobroll" ) != 0;	

	s_preferences2.enemymodelenabled.curvalue = trap_Cvar_VariableValue("cg_enemyModel_enabled") != 0;
	s_preferences2.chatbeep.curvalue = trap_Cvar_VariableValue("cg_chatSound") != 0;
	s_preferences2.enemytaunt.curvalue = trap_Cvar_VariableValue("cg_noTaunt") == 0;
	//s_preferences2.coloredping.curvalue = trap_Cvar_VariableValue("cg_coloredPing") != 0;	
	s_preferences2.deafultweapon.curvalue = abs((int)trap_Cvar_VariableValue("cg_defaultWeapon") % (WP_NUM_WEAPONS - 1));
	s_preferences2.lagometer.curvalue = abs((int)trap_Cvar_VariableValue("cg_lagometer") % 4);
	s_preferences2.hitsounds.curvalue = abs((int)trap_Cvar_VariableValue("cg_hitsounds") % 3);
	
	s_preferences2.centerprint.curvalue = (int)(trap_Cvar_VariableValue("cg_centerPrintAlpha") * 2) % 3;
	s_preferences2.drawgun.curvalue = (int)trap_Cvar_VariableValue("cg_drawGun") % 3;

	s_preferences2.scoreboard.curvalue = trap_Cvar_VariableValue("cg_scoreboard") != 0;
	s_preferences2.accuracy.curvalue = trap_Cvar_VariableValue("cg_drawAccuracy") != 0;
	/*s_preferences2.scorebox.curvalue = trap_Cvar_VariableValue("cg_drawScoreBox") != 0;*/
	s_preferences2.sharedconfig.curvalue		= trap_Cvar_VariableValue("cg_sharedConfig") != 0;

	trap_Cvar_VariableStringBuffer("cg_fov", s_preferences2.fov.field.buffer, sizeof(s_preferences2.fov.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_zoomfov", s_preferences2.zoomfov.field.buffer, sizeof(s_preferences2.zoomfov.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_enemyModel", s_preferences2.enemymodel.field.buffer, sizeof(s_preferences2.enemymodel.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_enemyColors", s_preferences2.enemycolors.field.buffer, sizeof(s_preferences2.enemycolors.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_teamModel", s_preferences2.teammodel.field.buffer, sizeof(s_preferences2.teammodel.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_teamColors", s_preferences2.teamcolors.field.buffer, sizeof(s_preferences2.teamcolors.field.buffer));
}

/*
=================
Preferences2_SaveChanges
=================
*/
static void Preferences2_SaveChanges( void ) {	
	float fov, fov2;

	fov = fov2 = atof(s_preferences2.fov.field.buffer);

	if (fov == 0) fov = 100; 
	else if (fov < 1) fov = 1; 
	else if (fov > 160) fov = 160;

	if (fov != fov2)
		Com_sprintf(s_preferences2.fov.field.buffer, 5, "%f", fov);

	fov = fov2 = atof(s_preferences2.zoomfov.field.buffer);

	if (fov == 0) fov = 22.5f;
	else if (fov < 1) fov = 1; 
	else if (fov > 160) fov = 160;

	if (fov != fov2)
		Com_sprintf(s_preferences2.zoomfov.field.buffer, 5, "%f", fov);	

	trap_Cvar_Set( "cg_fov", s_preferences2.fov.field.buffer );	
	trap_Cvar_Set( "cg_zoomfov", s_preferences2.zoomfov.field.buffer );
	trap_Cvar_Set( "cg_enemyModel", QX_trim(s_preferences2.enemymodel.field.buffer) );	
	trap_Cvar_Set( "cg_enemyColors", s_preferences2.enemycolors.field.buffer );	
	trap_Cvar_Set( "cg_teamModel", QX_trim(s_preferences2.teammodel.field.buffer) );	
	trap_Cvar_Set( "cg_teamColors", s_preferences2.teamcolors.field.buffer );	
}

static void Preferences2_Event( void* ptr, int notification ) {
	int fov;

	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_REWARDS:
		trap_Cvar_SetValue( "cg_drawRewards", s_preferences2.rewards.curvalue );
		break;

	case ID_TIMER:
		trap_Cvar_SetValue( "cg_drawTimer", s_preferences2.timer.curvalue );
		break;

	case ID_SPEED:
		trap_Cvar_SetValue("cg_drawSpeed", s_preferences2.speed.curvalue);
		break;

	case ID_BLOOD:
		trap_Cvar_SetValue( "com_blood", s_preferences2.blood.curvalue );
		break;

	case ID_DRAW_GUN:
		trap_Cvar_SetValue( "cg_drawGun", s_preferences2.drawgun.curvalue );
		break;

	case ID_GIBS:
		trap_Cvar_SetValue( "cg_gibs", s_preferences2.gibs.curvalue );
		break;

	case ID_CAMERABOB:
		if (s_preferences2.camerabob.curvalue) {			
			trap_Cvar_SetValue("cg_bobup", 0.005);
			trap_Cvar_SetValue("cg_bobpitch", 0.002);
			trap_Cvar_SetValue("cg_bobroll", 0.002);
		} else {
			trap_Cvar_SetValue("cg_bobup", 0);
			trap_Cvar_SetValue("cg_bobpitch", 0);
			trap_Cvar_SetValue("cg_bobroll", 0);
		}
		break;	

	//case ID_PLAYERIDS:
	//	trap_Cvar_SetValue( "cg_drawPlayerIDs", s_preferences2.playerids.curvalue );
	//	break;

	case ID_DRAW3DICONS:
		trap_Cvar_SetValue( "cg_draw3Dicons", s_preferences2.draw3dicons.curvalue );
		break;

	case ID_ENEMYMODEL_ENABLED:
		trap_Cvar_SetValue("cg_enemyModel_enabled", s_preferences2.enemymodelenabled.curvalue);
		break;

	case ID_ACC:
		trap_Cvar_SetValue("cg_drawAccuracy", s_preferences2.accuracy.curvalue);
		break;

	case ID_CHATBEEP:
		trap_Cvar_SetValue("cg_chatSound", s_preferences2.chatbeep.curvalue);
		break;

	case ID_ENEMY_TAUNT:
		trap_Cvar_SetValue("cg_noTaunt", !s_preferences2.enemytaunt.curvalue);
		break;

	//case ID_COLOREDPING:
	//	trap_Cvar_SetValue("cg_coloredPing", s_preferences2.coloredping.curvalue);
	//	break;

	case ID_DEFAULTWEAPON:
		trap_Cvar_SetValue("cg_defaultWeapon", s_preferences2.deafultweapon.curvalue);
		break;
		
	case ID_LAGOMETER:
		trap_Cvar_SetValue( "cg_lagometer", s_preferences2.lagometer.curvalue );
		break;

	case ID_HITSOUNDS:
		trap_Cvar_SetValue( "cg_hitsounds", s_preferences2.hitsounds.curvalue );
		break;

	case ID_CENTER_PRINT:
		trap_Cvar_SetValue("cg_centerPrintAlpha", s_preferences2.centerprint.curvalue / 2.0f);
		break;

	case ID_SCOREBOARD:
		trap_Cvar_SetValue("cg_scoreboard", s_preferences2.scoreboard.curvalue );
		break;

	//case ID_SCOREBOX:		
	//	trap_Cvar_SetValue("cg_drawScoreBox", s_preferences2.scorebox.curvalue);
	//	break;

	case ID_SHAREDCONFIG:
		trap_Cvar_SetValue("cg_sharedConfig", s_preferences2.sharedconfig.curvalue);
		break;

	case ID_BACK:
		Preferences2_SaveChanges();
		UI_PopMenu();
		break;
	}
}

/*
=================
PlayerSettings_MenuKey
=================
*/
static sfxHandle_t Preferences2_MenuKey( int key ) {
	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		Preferences2_SaveChanges();
	}
	return Menu_DefaultKey( &s_preferences2.menu, key );
}

static void Preferences2_MenuInit( void ) {
	int				y, y2, ystart;

	memset( &s_preferences2, 0 ,sizeof(preferences_t) );	

	Preferences2_Cache();

	s_preferences2.menu.key        = Preferences2_MenuKey;
	s_preferences2.menu.wrapAround = qtrue;
	s_preferences2.menu.fullscreen = qtrue;

	s_preferences2.banner.generic.type  = MTYPE_BTEXT;
	s_preferences2.banner.generic.x		= 320;
	s_preferences2.banner.generic.y		= 16;
	s_preferences2.banner.string		= "ADVANCED SETTINGS";
	s_preferences2.banner.color         = color_white;
	s_preferences2.banner.style         = UI_CENTER;

	s_preferences2.framel.generic.type  = MTYPE_BITMAP;
	s_preferences2.framel.generic.name  = ART_FRAMEL;
	s_preferences2.framel.generic.flags = QMF_INACTIVE;
	s_preferences2.framel.generic.x	   = 0;
	s_preferences2.framel.generic.y	   = 78;
	s_preferences2.framel.width  	   = 256;
	s_preferences2.framel.height  	   = 329;

	s_preferences2.framer.generic.type  = MTYPE_BITMAP;
	s_preferences2.framer.generic.name  = ART_FRAMER;
	s_preferences2.framer.generic.flags = QMF_INACTIVE;
	s_preferences2.framer.generic.x	   = 376;
	s_preferences2.framer.generic.y	   = 76;
	s_preferences2.framer.width  	   = 256;
	s_preferences2.framer.height  	   = 334;

	ystart = 144 - BIGCHAR_HEIGHT * 3;
	y = ystart;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.rewards.generic.type       = MTYPE_RADIOBUTTON;
	s_preferences2.rewards.generic.name	      = "Rewards:";
	s_preferences2.rewards.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.rewards.generic.callback   = Preferences2_Event;
	s_preferences2.rewards.generic.statusbar  = Preferences2_StatusBar;
	s_preferences2.rewards.generic.id         = ID_REWARDS;
	s_preferences2.rewards.generic.x	      = PREFERENCES_X_POS_1;
	s_preferences2.rewards.generic.y	      = y;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.blood.generic.type         = MTYPE_RADIOBUTTON;
	s_preferences2.blood.generic.name	      = "Blood:";
	s_preferences2.blood.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.blood.generic.callback     = Preferences2_Event;
	s_preferences2.blood.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.blood.generic.id           = ID_BLOOD;
	s_preferences2.blood.generic.x	          = PREFERENCES_X_POS_1;
	s_preferences2.blood.generic.y	          = y;	
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.gibs.generic.type         = MTYPE_RADIOBUTTON;
	s_preferences2.gibs.generic.name	     = "Gibs:";
	s_preferences2.gibs.generic.flags	     = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.gibs.generic.callback     = Preferences2_Event;
	s_preferences2.gibs.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.gibs.generic.id           = ID_GIBS;
	s_preferences2.gibs.generic.x	         = PREFERENCES_X_POS_1;
	s_preferences2.gibs.generic.y	         = y;	
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.camerabob.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences2.camerabob.generic.name	  = "Camera Bobbing:";
	s_preferences2.camerabob.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.camerabob.generic.callback = Preferences2_Event;
	s_preferences2.camerabob.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.camerabob.generic.id       = ID_CAMERABOB;
	s_preferences2.camerabob.generic.x	      = PREFERENCES_X_POS_1;
	s_preferences2.camerabob.generic.y	      = y;	
	//y += BIGCHAR_HEIGHT+2;
	//s_preferences2.playerids.generic.type     = MTYPE_RADIOBUTTON;
	//s_preferences2.playerids.generic.name	  = "Show Player ID:";
	//s_preferences2.playerids.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	//s_preferences2.playerids.generic.callback = Preferences2_Event;
	//s_preferences2.playerids.generic.statusbar	= Preferences2_StatusBar;
	//s_preferences2.playerids.generic.id       = ID_PLAYERIDS;
	//s_preferences2.playerids.generic.x	      = PREFERENCES_X_POS_1;
	//s_preferences2.playerids.generic.y	      = y;
	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.chatbeep.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.chatbeep.generic.name = "Chat Beep:";
	s_preferences2.chatbeep.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.chatbeep.generic.callback = Preferences2_Event;
	s_preferences2.chatbeep.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.chatbeep.generic.id = ID_CHATBEEP;
	s_preferences2.chatbeep.generic.x = PREFERENCES_X_POS_1;
	s_preferences2.chatbeep.generic.y = y;
	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.enemytaunt.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.enemytaunt.generic.name = "Enemy Taunt:";
	s_preferences2.enemytaunt.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.enemytaunt.generic.callback = Preferences2_Event;
	s_preferences2.enemytaunt.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.enemytaunt.generic.id = ID_ENEMY_TAUNT;
	s_preferences2.enemytaunt.generic.x = PREFERENCES_X_POS_1;
	s_preferences2.enemytaunt.generic.y = y;
	//y += BIGCHAR_HEIGHT + 2;
	//s_preferences2.coloredping.generic.type = MTYPE_RADIOBUTTON;
	//s_preferences2.coloredping.generic.name = "Colored Ping:";
	//s_preferences2.coloredping.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	//s_preferences2.coloredping.generic.callback = Preferences2_Event;
	//s_preferences2.coloredping.generic.statusbar	= Preferences2_StatusBar;
	//s_preferences2.coloredping.generic.id = ID_COLOREDPING;
	//s_preferences2.coloredping.generic.x = PREFERENCES_X_POS_1;
	//s_preferences2.coloredping.generic.y = y;
	y += BIGCHAR_HEIGHT+2;		
	s_preferences2.draw3dicons.generic.type			= MTYPE_RADIOBUTTON;
	s_preferences2.draw3dicons.generic.name			= "3D Icons:";
	s_preferences2.draw3dicons.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.draw3dicons.generic.callback		= Preferences2_Event;
	s_preferences2.draw3dicons.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.draw3dicons.generic.id			= ID_DRAW3DICONS;
	s_preferences2.draw3dicons.generic.x			= PREFERENCES_X_POS_1;
	s_preferences2.draw3dicons.generic.y			= y;
	//y += BIGCHAR_HEIGHT+2;		
	//s_preferences2.scorebox.generic.type			= MTYPE_RADIOBUTTON;
	//s_preferences2.scorebox.generic.name			= "Scorebox:";
	//s_preferences2.scorebox.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	//s_preferences2.scorebox.generic.callback		= Preferences2_Event;
	//s_preferences2.scorebox.generic.statusbar	= Preferences2_StatusBar;
	//s_preferences2.scorebox.generic.id			= ID_SCOREBOX;
	//s_preferences2.scorebox.generic.x			= PREFERENCES_X_POS_1;
	//s_preferences2.scorebox.generic.y			= y;
	y += BIGCHAR_HEIGHT+2;		
	s_preferences2.accuracy.generic.type			= MTYPE_RADIOBUTTON;
	s_preferences2.accuracy.generic.name			= "Accuracy:";
	s_preferences2.accuracy.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.accuracy.generic.callback		= Preferences2_Event;
	s_preferences2.accuracy.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.accuracy.generic.id			= ID_ACC;
	s_preferences2.accuracy.generic.x			= PREFERENCES_X_POS_1;
	s_preferences2.accuracy.generic.y			= y;

	y += BIGCHAR_HEIGHT+2;		
	s_preferences2.sharedconfig.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.sharedconfig.generic.name = "Shared Config:";
	s_preferences2.sharedconfig.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.sharedconfig.generic.callback = Preferences2_Event;
	s_preferences2.sharedconfig.generic.statusbar = Preferences2_StatusBar;
	s_preferences2.sharedconfig.generic.id = ID_SHAREDCONFIG;
	s_preferences2.sharedconfig.generic.x = PREFERENCES_X_POS_1;
	s_preferences2.sharedconfig.generic.y = y;

	y = ystart;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.fov.generic.type       = MTYPE_FIELD;
	s_preferences2.fov.generic.name		  = "FOV:";
	s_preferences2.fov.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.fov.generic.x	      = PREFERENCES_X_POS_2;
	s_preferences2.fov.generic.callback	  = Preferences2_Event;
	s_preferences2.fov.generic.statusbar = Preferences2_StatusBar;
	s_preferences2.fov.generic.id         = ID_FOV;
	s_preferences2.fov.generic.y	      = y;
	s_preferences2.fov.field.widthInChars = 5;
	s_preferences2.fov.field.maxchars     = 4;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.zoomfov.generic.type       = MTYPE_FIELD;
	s_preferences2.zoomfov.generic.name		  = "Zoom FOV:";
	s_preferences2.zoomfov.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.zoomfov.generic.x	      = PREFERENCES_X_POS_2;
	s_preferences2.zoomfov.generic.callback	  = Preferences2_Event;
	s_preferences2.zoomfov.generic.statusbar = Preferences2_StatusBar;
	s_preferences2.zoomfov.generic.id         = ID_ZOOMFOV;
	s_preferences2.zoomfov.generic.y	      = y;
	s_preferences2.zoomfov.field.widthInChars = 5;
	s_preferences2.zoomfov.field.maxchars     = 4;
	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.drawgun.generic.type			= MTYPE_SPINCONTROL;
	s_preferences2.drawgun.generic.name			= "Draw Gun:";
	s_preferences2.drawgun.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.drawgun.generic.callback		= Preferences2_Event;
	s_preferences2.drawgun.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.drawgun.generic.id			= ID_DRAW_GUN;
	s_preferences2.drawgun.generic.x			= PREFERENCES_X_POS_2;
	s_preferences2.drawgun.generic.y			= y;
	s_preferences2.drawgun.itemnames			= drawgun_items;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.timer.generic.type			= MTYPE_SPINCONTROL;
	s_preferences2.timer.generic.name			= "Draw Timer:";
	s_preferences2.timer.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.timer.generic.callback		= Preferences2_Event;
	s_preferences2.timer.generic.statusbar		= Preferences2_StatusBar;
	s_preferences2.timer.generic.id				= ID_TIMER;
	s_preferences2.timer.generic.x				= PREFERENCES_X_POS_2;
	s_preferences2.timer.generic.y				= y;
	s_preferences2.timer.itemnames				= timer_items;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.speed.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.speed.generic.name = "Draw Speed:";
	s_preferences2.speed.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.speed.generic.callback = Preferences2_Event;
	s_preferences2.speed.generic.statusbar = Preferences2_StatusBar;
	s_preferences2.speed.generic.id = ID_SPEED;
	s_preferences2.speed.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.speed.generic.y = y;
	s_preferences2.speed.itemnames = speed_items;
	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.centerprint.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.centerprint.generic.name = "Center Print:";
	s_preferences2.centerprint.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.centerprint.generic.callback = Preferences2_Event;
	s_preferences2.centerprint.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.centerprint.generic.id = ID_CENTER_PRINT;
	s_preferences2.centerprint.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.centerprint.generic.y = y;
	s_preferences2.centerprint.itemnames = centerprint_items;
	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.deafultweapon.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.deafultweapon.generic.name = "Default Weapon:";
	s_preferences2.deafultweapon.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.deafultweapon.generic.callback = Preferences2_Event;
	s_preferences2.deafultweapon.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.deafultweapon.generic.id = ID_DEFAULTWEAPON;
	s_preferences2.deafultweapon.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.deafultweapon.generic.y = y;
	s_preferences2.deafultweapon.itemnames = defaultweapon_items;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.hitsounds.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.hitsounds.generic.name = "Hitsounds:";
	s_preferences2.hitsounds.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.hitsounds.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.hitsounds.generic.callback = Preferences2_Event;
	s_preferences2.hitsounds.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.hitsounds.generic.id = ID_HITSOUNDS;
	s_preferences2.hitsounds.generic.y = y;
	s_preferences2.hitsounds.itemnames = hitsounds_items;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.scoreboard.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.scoreboard.generic.name = "Scoreboard:";
	s_preferences2.scoreboard.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.scoreboard.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.scoreboard.generic.callback = Preferences2_Event;
	s_preferences2.scoreboard.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.scoreboard.generic.id = ID_SCOREBOARD;
	s_preferences2.scoreboard.generic.y = y;
	s_preferences2.scoreboard.itemnames = scoreboar_items;
	y += BIGCHAR_HEIGHT+2;
	s_preferences2.lagometer.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.lagometer.generic.name = "Lagometer:";
	s_preferences2.lagometer.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.lagometer.generic.x = PREFERENCES_X_POS_2;
	s_preferences2.lagometer.generic.callback = Preferences2_Event;
	s_preferences2.lagometer.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.lagometer.generic.id = ID_LAGOMETER;
	s_preferences2.lagometer.generic.y = y;
	s_preferences2.lagometer.itemnames = lagometer_items;


	y += (BIGCHAR_HEIGHT+2)* 2;
	s_preferences2.enemymodelenabled.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.enemymodelenabled.generic.name = "Bright models:";
	s_preferences2.enemymodelenabled.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.enemymodelenabled.generic.callback = Preferences2_Event;
	s_preferences2.enemymodelenabled.generic.statusbar = Preferences2_StatusBar;
	s_preferences2.enemymodelenabled.generic.id = ID_ENEMYMODEL_ENABLED;
	s_preferences2.enemymodelenabled.generic.x = PREFERENCES_X_POS;
	s_preferences2.enemymodelenabled.generic.y = y;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.enemymodel.generic.type       = MTYPE_FIELD;
	s_preferences2.enemymodel.generic.name		 = "Enemy model:";
	s_preferences2.enemymodel.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.enemymodel.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.enemymodel.generic.callback	 = Preferences2_Event;
	s_preferences2.enemymodel.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.enemymodel.generic.id         = ID_ENEMYMODEL;
	s_preferences2.enemymodel.generic.y			 = y;
	s_preferences2.enemymodel.field.widthInChars = 17;
	s_preferences2.enemymodel.field.maxchars     = 16;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.enemycolors.generic.type       = MTYPE_FIELD;
	s_preferences2.enemycolors.generic.name		 = "Enemy colors:";
	s_preferences2.enemycolors.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.enemycolors.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.enemycolors.generic.callback	 = Preferences2_Event;
	s_preferences2.enemycolors.generic.statusbar = Preferences2_StatusBar;	
	s_preferences2.enemycolors.generic.id         = ID_ENEMYCOLORS;
	s_preferences2.enemycolors.generic.y			 = y;
	s_preferences2.enemycolors.field.widthInChars = 5;
	s_preferences2.enemycolors.field.maxchars     = 4;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.teammodel.generic.type       = MTYPE_FIELD;
	s_preferences2.teammodel.generic.name		 = "Team model:";
	s_preferences2.teammodel.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.teammodel.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.teammodel.generic.callback	 = Preferences2_Event;
	s_preferences2.teammodel.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.teammodel.generic.id         = ID_TEAMMODEL;
	s_preferences2.teammodel.generic.y			 = y;
	s_preferences2.teammodel.field.widthInChars = 17;
	s_preferences2.teammodel.field.maxchars     = 16;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.teamcolors.generic.type       = MTYPE_FIELD;
	s_preferences2.teamcolors.generic.name		 = "Team colors:";
	s_preferences2.teamcolors.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.teamcolors.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.teamcolors.generic.callback	 = Preferences2_Event;
	s_preferences2.teamcolors.generic.statusbar	= Preferences2_StatusBar;
	s_preferences2.teamcolors.generic.id         = ID_TEAMCOLORS;
	s_preferences2.teamcolors.generic.y			 = y;
	s_preferences2.teamcolors.field.widthInChars = 5;
	s_preferences2.teamcolors.field.maxchars     = 4;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.back.generic.type	    = MTYPE_BITMAP;
	s_preferences2.back.generic.name     = ART_BACK0;
	s_preferences2.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_preferences2.back.generic.callback = Preferences2_Event;
	s_preferences2.back.generic.id	    = ID_BACK;
	s_preferences2.back.generic.x		= 0;
	s_preferences2.back.generic.y		= 480-64;
	s_preferences2.back.width  		    = 128;
	s_preferences2.back.height  		    = 64;
	s_preferences2.back.focuspic         = ART_BACK1;

	Menu_AddItem( &s_preferences2.menu, &s_preferences2.banner );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.framel );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.framer );

	Menu_AddItem( &s_preferences2.menu, &s_preferences2.rewards );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.timer );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.speed );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.drawgun );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.blood );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.gibs );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.camerabob );
	//Menu_AddItem( &s_preferences2.menu, &s_preferences2.playerids );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.draw3dicons );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.fov );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.zoomfov );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemymodelenabled );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemymodel );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemycolors );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.teammodel );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.teamcolors );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.chatbeep );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemytaunt );
	//Menu_AddItem( &s_preferences2.menu, &s_preferences2.coloredping );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.deafultweapon );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.centerprint );	
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.lagometer );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.hitsounds );
	/*Menu_AddItem( &s_preferences2.menu, &s_preferences2.scorebox );*/
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.scoreboard );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.accuracy );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.sharedconfig );

	Menu_AddItem( &s_preferences2.menu, &s_preferences2.back );

	Preferences2_SetMenuItems();
}

/*
===============
Preferences_Cache
===============
*/
void Preferences2_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}

/*
===============
UI_PreferencesMenu
===============
*/
void UI_Preferences2Menu( void ) {
	Preferences2_MenuInit();
	UI_PushMenu( &s_preferences2.menu );
}
