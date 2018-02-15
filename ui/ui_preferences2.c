// Copyright (C) 1999-2000 Id Software, Inc.
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

#define PREFERENCES_X_POS		360

#define ID_REWARDS				127
#define ID_BLOOD				128
#define ID_GIBS					129
#define ID_CAMERABOB			130
#define ID_PLAYERIDS			131
#define ID_FOV					132
#define ID_TIMER				133
#define ID_ENEMYMODEL			134
#define ID_ENEMYCOLORS			135
#define ID_DRAW3DICONS			136
#define ID_ZOOMFOV				137
#define ID_ENEMYMODEL_ENABLED	139
#define ID_CHATBEEP				140
#define ID_ENEMY_TAUNT			141
#define ID_CENTER_PRINT			142
#define ID_SPEED				143
#define ID_DRAW_GUN				144
//#define ID_FORCEMODEL			135
//#define ID_DRAWTEAMOVERLAY		136
//#define ID_ALLOWDOWNLOAD			137
#define ID_BACK					138


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
	menuradiobutton_s	playerids;
	menuradiobutton_s	draw3dicons;
	menuradiobutton_s	chatbeep;
	menuradiobutton_s	enemytaunt;
	menulist_s			centerprint;
	menufield_s			fov;
	menufield_s			zoomfov;
	menuradiobutton_s	enemymodelenabled;
	menufield_s			enemymodel;
	menufield_s			enemycolors;		
	//menuradiobutton_s	dynamiclights;	

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

static void Preferences2_SetMenuItems( void ) {
	s_preferences2.rewards.curvalue		= trap_Cvar_VariableValue( "cg_drawRewards" ) != 0;
	s_preferences2.timer.curvalue		= abs((int)trap_Cvar_VariableValue( "cg_drawTimer" ) % 3);
	s_preferences2.speed.curvalue		= abs((int)trap_Cvar_VariableValue("cg_drawSpeed") % 3);
	s_preferences2.blood.curvalue		= trap_Cvar_VariableValue( "com_blood" ) != 0;
	s_preferences2.gibs.curvalue		= trap_Cvar_VariableValue( "cg_gibs" ) != 0;
	s_preferences2.playerids.curvalue	= trap_Cvar_VariableValue( "cgx_drawPlayerIDs" ) != 0;	
	s_preferences2.draw3dicons.curvalue	= trap_Cvar_VariableValue( "cg_draw3Dicons" ) != 0;
	s_preferences2.camerabob.curvalue	= trap_Cvar_VariableValue( "cg_bobup" ) != 0 
										&& trap_Cvar_VariableValue( "cg_bobpitch" ) != 0 
										&& trap_Cvar_VariableValue( "cg_bobroll" ) != 0;	

	s_preferences2.enemymodelenabled.curvalue = trap_Cvar_VariableValue("cg_enemyModel_enabled") != 0;
	s_preferences2.chatbeep.curvalue = trap_Cvar_VariableValue("cg_chatSound") != 0;
	s_preferences2.enemytaunt.curvalue = trap_Cvar_VariableValue("cg_noTaunt") == 0;

	s_preferences2.centerprint.curvalue = (int)(trap_Cvar_VariableValue("cg_centerPrintAlpha") * 2) % 3;
	s_preferences2.drawgun.curvalue = (int)trap_Cvar_VariableValue("cg_drawGun") % 3;

	trap_Cvar_VariableStringBuffer("cg_fov", s_preferences2.fov.field.buffer, sizeof(s_preferences2.fov.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_zoomfov", s_preferences2.zoomfov.field.buffer, sizeof(s_preferences2.zoomfov.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_enemyModel", s_preferences2.enemymodel.field.buffer, sizeof(s_preferences2.enemymodel.field.buffer));
	trap_Cvar_VariableStringBuffer("cg_enemyColors", s_preferences2.enemycolors.field.buffer, sizeof(s_preferences2.enemycolors.field.buffer));
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
			trap_Print("reset bob");
			trap_Cvar_Reset("cg_bobup");
			trap_Cvar_Reset("cg_bobpitch");
			trap_Cvar_Reset("cg_bobroll");
		} else {
			trap_Cvar_SetValue("cg_bobup", 0);
			trap_Cvar_SetValue("cg_bobpitch", 0);
			trap_Cvar_SetValue("cg_bobroll", 0);
		}
		break;	

	case ID_PLAYERIDS:
		trap_Cvar_SetValue( "cgx_drawPlayerIDs", s_preferences2.gibs.curvalue );
		break;

	case ID_DRAW3DICONS:
		trap_Cvar_SetValue( "cg_draw3Dicons", s_preferences2.draw3dicons.curvalue );
		break;

	case ID_ENEMYMODEL_ENABLED:
		trap_Cvar_SetValue("cg_enemyModel_enabled", s_preferences2.enemymodelenabled.curvalue);
		break;

	case ID_CHATBEEP:
		trap_Cvar_SetValue("cg_chatSound", s_preferences2.chatbeep.curvalue);
		break;

	case ID_ENEMY_TAUNT:
		trap_Cvar_SetValue("cg_noTaunt", !s_preferences2.enemytaunt.curvalue);
		break;

	case ID_CENTER_PRINT:
		trap_Cvar_SetValue("cg_centerPrintAlpha", s_preferences2.centerprint.curvalue / 2.0f);
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
	int				y;

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

	y = 144 - BIGCHAR_HEIGHT * 5;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.rewards.generic.type       = MTYPE_RADIOBUTTON;
	s_preferences2.rewards.generic.name	      = "Draw Rewards:";
	s_preferences2.rewards.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.rewards.generic.callback   = Preferences2_Event;
	s_preferences2.rewards.generic.id         = ID_REWARDS;
	s_preferences2.rewards.generic.x	      = PREFERENCES_X_POS;
	s_preferences2.rewards.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.timer.generic.type			= MTYPE_SPINCONTROL;
	s_preferences2.timer.generic.name			= "Draw Timer:";
	s_preferences2.timer.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.timer.generic.callback		= Preferences2_Event;
	s_preferences2.timer.generic.id				= ID_TIMER;
	s_preferences2.timer.generic.x				= PREFERENCES_X_POS;
	s_preferences2.timer.generic.y				= y;
	s_preferences2.timer.itemnames				= timer_items;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.speed.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.speed.generic.name = "Draw Speed:";
	s_preferences2.speed.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.speed.generic.callback = Preferences2_Event;
	s_preferences2.speed.generic.id = ID_SPEED;
	s_preferences2.speed.generic.x = PREFERENCES_X_POS;
	s_preferences2.speed.generic.y = y;
	s_preferences2.speed.itemnames = speed_items;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.draw3dicons.generic.type			= MTYPE_RADIOBUTTON;
	s_preferences2.draw3dicons.generic.name			= "Draw 3D Icons:";
	s_preferences2.draw3dicons.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.draw3dicons.generic.callback		= Preferences2_Event;
	s_preferences2.draw3dicons.generic.id			= ID_DRAW3DICONS;
	s_preferences2.draw3dicons.generic.x			= PREFERENCES_X_POS;
	s_preferences2.draw3dicons.generic.y			= y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.drawgun.generic.type			= MTYPE_SPINCONTROL;
	s_preferences2.drawgun.generic.name			= "Draw Gun:";
	s_preferences2.drawgun.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.drawgun.generic.callback		= Preferences2_Event;
	s_preferences2.drawgun.generic.id			= ID_DRAW_GUN;
	s_preferences2.drawgun.generic.x			= PREFERENCES_X_POS;
	s_preferences2.drawgun.generic.y			= y;
	s_preferences2.drawgun.itemnames			= drawgun_items;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.blood.generic.type         = MTYPE_RADIOBUTTON;
	s_preferences2.blood.generic.name	      = "Blood:";
	s_preferences2.blood.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.blood.generic.callback     = Preferences2_Event;
	s_preferences2.blood.generic.id           = ID_BLOOD;
	s_preferences2.blood.generic.x	          = PREFERENCES_X_POS;
	s_preferences2.blood.generic.y	          = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.gibs.generic.type         = MTYPE_RADIOBUTTON;
	s_preferences2.gibs.generic.name	     = "Gibs:";
	s_preferences2.gibs.generic.flags	     = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.gibs.generic.callback     = Preferences2_Event;
	s_preferences2.gibs.generic.id           = ID_GIBS;
	s_preferences2.gibs.generic.x	         = PREFERENCES_X_POS;
	s_preferences2.gibs.generic.y	         = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.camerabob.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences2.camerabob.generic.name	  = "Camera Bobbing:";
	s_preferences2.camerabob.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.camerabob.generic.callback = Preferences2_Event;
	s_preferences2.camerabob.generic.id       = ID_CAMERABOB;
	s_preferences2.camerabob.generic.x	      = PREFERENCES_X_POS;
	s_preferences2.camerabob.generic.y	      = y;	

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.playerids.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences2.playerids.generic.name	  = "Show Player ID:";
	s_preferences2.playerids.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.playerids.generic.callback = Preferences2_Event;
	s_preferences2.playerids.generic.id       = ID_PLAYERIDS;
	s_preferences2.playerids.generic.x	      = PREFERENCES_X_POS;
	s_preferences2.playerids.generic.y	      = y;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.chatbeep.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.chatbeep.generic.name = "Chat Beep:";
	s_preferences2.chatbeep.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.chatbeep.generic.callback = Preferences2_Event;
	s_preferences2.chatbeep.generic.id = ID_CHATBEEP;
	s_preferences2.chatbeep.generic.x = PREFERENCES_X_POS;
	s_preferences2.chatbeep.generic.y = y;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.enemytaunt.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.enemytaunt.generic.name = "Enemy Taunt:";
	s_preferences2.enemytaunt.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.enemytaunt.generic.callback = Preferences2_Event;
	s_preferences2.enemytaunt.generic.id = ID_ENEMY_TAUNT;
	s_preferences2.enemytaunt.generic.x = PREFERENCES_X_POS;
	s_preferences2.enemytaunt.generic.y = y;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.centerprint.generic.type = MTYPE_SPINCONTROL;
	s_preferences2.centerprint.generic.name = "Center Print:";
	s_preferences2.centerprint.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.centerprint.generic.callback = Preferences2_Event;
	s_preferences2.centerprint.generic.id = ID_CENTER_PRINT;
	s_preferences2.centerprint.generic.x = PREFERENCES_X_POS;
	s_preferences2.centerprint.generic.y = y;
	s_preferences2.centerprint.itemnames = centerprint_items;
	
	y += (BIGCHAR_HEIGHT+2) * 2;	
	s_preferences2.fov.generic.type       = MTYPE_FIELD;
	s_preferences2.fov.generic.name		  = "FOV (80-130):";
	s_preferences2.fov.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.fov.generic.x	      = PREFERENCES_X_POS;
	s_preferences2.fov.generic.callback	  = Preferences2_Event;
	s_preferences2.fov.generic.id         = ID_FOV;
	s_preferences2.fov.generic.y	      = y;
	s_preferences2.fov.field.widthInChars = 4;
	s_preferences2.fov.field.maxchars     = 3;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.zoomfov.generic.type       = MTYPE_FIELD;
	s_preferences2.zoomfov.generic.name		  = "Zoom FOV (10-80):";
	s_preferences2.zoomfov.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.zoomfov.generic.x	      = PREFERENCES_X_POS;
	s_preferences2.zoomfov.generic.callback	  = Preferences2_Event;
	s_preferences2.zoomfov.generic.id         = ID_ZOOMFOV;
	s_preferences2.zoomfov.generic.y	      = y;
	s_preferences2.zoomfov.field.widthInChars = 4;
	s_preferences2.zoomfov.field.maxchars     = 2;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.enemymodelenabled.generic.type = MTYPE_RADIOBUTTON;
	s_preferences2.enemymodelenabled.generic.name = "Bright enemies enabled:";
	s_preferences2.enemymodelenabled.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	s_preferences2.enemymodelenabled.generic.callback = Preferences2_Event;
	s_preferences2.enemymodelenabled.generic.id = ID_ENEMYMODEL_ENABLED;
	s_preferences2.enemymodelenabled.generic.x = PREFERENCES_X_POS;
	s_preferences2.enemymodelenabled.generic.y = y;

	y += BIGCHAR_HEIGHT + 2;
	s_preferences2.enemymodel.generic.type       = MTYPE_FIELD;
	s_preferences2.enemymodel.generic.name		 = "Enemy model:";
	s_preferences2.enemymodel.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences2.enemymodel.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.enemymodel.generic.callback	 = Preferences2_Event;
	s_preferences2.enemymodel.generic.id         = ID_ENEMYMODEL;
	s_preferences2.enemymodel.generic.y			 = y;
	s_preferences2.enemymodel.field.widthInChars = 20;
	s_preferences2.enemymodel.field.maxchars     = 19;

	y += BIGCHAR_HEIGHT+2;
	s_preferences2.enemycolors.generic.type       = MTYPE_FIELD;
	s_preferences2.enemycolors.generic.name		 = "Enemy colors:";
	s_preferences2.enemycolors.generic.flags		 = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_preferences2.enemycolors.generic.x			 = PREFERENCES_X_POS;
	s_preferences2.enemycolors.generic.callback	 = Preferences2_Event;
	s_preferences2.enemycolors.generic.id         = ID_ENEMYCOLORS;
	s_preferences2.enemycolors.generic.y			 = y;
	s_preferences2.enemycolors.field.widthInChars = 5;
	s_preferences2.enemycolors.field.maxchars     = 4;

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
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.playerids );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.draw3dicons );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.fov );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.zoomfov );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemymodelenabled );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemymodel );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemycolors );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.chatbeep );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.enemytaunt );
	Menu_AddItem( &s_preferences2.menu, &s_preferences2.centerprint );	

	Menu_AddItem( &s_preferences2.menu, &s_preferences2.back );

	Preferences2_SetMenuItems();
}

/*
=================
Preferences2_SaveChanges
=================
*/
static void Preferences2_SaveChanges( void ) {	
	int fov;

	fov = atoi(s_preferences2.fov.field.buffer);
	if (fov < 80) fov = 80; else if (fov > 130) fov = 130;
	Com_sprintf(s_preferences2.fov.field.buffer, 4, "%d", fov);

	fov = atoi(s_preferences2.zoomfov.field.buffer);
	if (fov < 10) fov = 10; else if (fov > 80) fov = 80;
	Com_sprintf(s_preferences2.zoomfov.field.buffer, 4, "%d", fov);

	trap_Cvar_Set( "cg_fov", s_preferences2.fov.field.buffer );
	trap_Cvar_Set( "cg_zoomfov", s_preferences2.zoomfov.field.buffer );
	trap_Cvar_Set( "cg_enemyModel", s_preferences2.enemymodel.field.buffer );	
	trap_Cvar_Set( "cg_enemyColors", s_preferences2.enemycolors.field.buffer );	
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
