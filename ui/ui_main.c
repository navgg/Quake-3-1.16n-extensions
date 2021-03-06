// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322) - X-MOD
// Copyright (C) 2006 Neil Toronto - Unlagged 2.01
//
/*
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/


#include "ui_local.h"


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 ) {
	switch ( command ) {
	case UI_GETAPIVERSION:
		return UIX_GetApiVersion();

	case UI_INIT:
		UI_Init();
		return 0;

	case UI_SHUTDOWN:
		UI_Shutdown();
		return 0;

	case UI_KEY_EVENT:
		UI_KeyEvent( arg0 );
		return 0;

	case UI_MOUSE_EVENT:
		UI_MouseEvent( arg0, arg1 );
		return 0;

	case UI_REFRESH:
		UI_Refresh( arg0 );
		return 0;

	case UI_IS_FULLSCREEN:
		return UI_IsFullscreen();

	case UI_SET_ACTIVE_MENU:
		UI_SetActiveMenu( arg0 );
		return 0;

	case UI_CONSOLE_COMMAND:
		return UI_ConsoleCommand();

	case UI_DRAW_CONNECT_SCREEN:
		UI_DrawConnectScreen( arg0 );
		return 0;
	}

	return -1;
}


/*
================
cvars
================
*/

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

vmCvar_t	ui_ffa_fraglimit;
vmCvar_t	ui_ffa_timelimit;

vmCvar_t	ui_tourney_fraglimit;
vmCvar_t	ui_tourney_timelimit;

vmCvar_t	ui_team_fraglimit;
vmCvar_t	ui_team_timelimit;
vmCvar_t	ui_team_friendly;

vmCvar_t	ui_ctf_capturelimit;
vmCvar_t	ui_ctf_timelimit;
vmCvar_t	ui_ctf_friendly;

vmCvar_t	ui_arenasFile;
vmCvar_t	ui_botsFile;
vmCvar_t	ui_spScores1;
vmCvar_t	ui_spScores2;
vmCvar_t	ui_spScores3;
vmCvar_t	ui_spScores4;
vmCvar_t	ui_spScores5;
vmCvar_t	ui_spAwards;
vmCvar_t	ui_spVideos;
vmCvar_t	ui_spSkill;

vmCvar_t	ui_spSelection;

vmCvar_t	ui_browserMaster;
vmCvar_t	ui_browserGameType;
vmCvar_t	ui_browserSortKey;
vmCvar_t	ui_browserShowFull;
vmCvar_t	ui_browserShowEmpty;

vmCvar_t	ui_brassTime;
vmCvar_t	ui_drawCrosshair;
vmCvar_t	ui_drawCrosshairNames;
vmCvar_t	ui_marks;

vmCvar_t	ui_server1;
vmCvar_t	ui_server2;
vmCvar_t	ui_server3;
vmCvar_t	ui_server4;
vmCvar_t	ui_server5;
vmCvar_t	ui_server6;
vmCvar_t	ui_server7;
vmCvar_t	ui_server8;
vmCvar_t	ui_server9;
vmCvar_t	ui_server10;
vmCvar_t	ui_server11;
vmCvar_t	ui_server12;
vmCvar_t	ui_server13;
vmCvar_t	ui_server14;
vmCvar_t	ui_server15;
vmCvar_t	ui_server16;

vmCvar_t	ui_cdkeychecked;

// X-MOD: ui params
vmCvar_t	uix_serverCache;
vmCvar_t	uix_browserShowMplayer;
vmCvar_t	uix_browserShowNettype;
vmCvar_t	uix_blockedIPs;

vmCvar_t	uix_wideScreenFix;
vmCvar_t	uix_defaultWeapon;
vmCvar_t	uix_enemyModel_enabled;
vmCvar_t	uix_enemyModel;
vmCvar_t	uix_enemyColors;
vmCvar_t	uix_teamModel;
vmCvar_t	uix_teamColors;
vmCvar_t	uix_deadBodyDarken;
vmCvar_t	uix_chatSound;
vmCvar_t	uix_noTaunt;
vmCvar_t	uix_hitsounds;
vmCvar_t	uix_centerPrintAlpha;
vmCvar_t	uix_crosshairColor;
vmCvar_t	uix_crosshairSize;
vmCvar_t	uix_drawSpeed;
vmCvar_t	uix_lagometer;
vmCvar_t	uix_networkAdjustments;
vmCvar_t	uix_scoreboard;
vmCvar_t	uix_drawScoreBox;
vmCvar_t	uix_sharedConfig;
vmCvar_t	uix_nomip;
vmCvar_t	uix_weaponEffects;
// some q3 defaults to init
vmCvar_t	uix_fov;
vmCvar_t	uix_zoomfov;
vmCvar_t	uix_drawgun;
vmCvar_t	uix_drawrewards;
vmCvar_t	uix_draw3dicons;
vmCvar_t	uix_gibs;
vmCvar_t	uix_draw2D;
vmCvar_t	uix_railTrailTime;

vmCvar_t	s_ambient;

#if CGX_UNLAGGED
vmCvar_t	cg_delag;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
#endif


cvarTable_t		cvarTable[] = {
	{ &ui_ffa_fraglimit, "ui_ffa_fraglimit", "20", CVAR_ARCHIVE },
	{ &ui_ffa_timelimit, "ui_ffa_timelimit", "0", CVAR_ARCHIVE },

	{ &ui_tourney_fraglimit, "ui_tourney_fraglimit", "0", CVAR_ARCHIVE },
	{ &ui_tourney_timelimit, "ui_tourney_timelimit", "10", CVAR_ARCHIVE },

	{ &ui_team_fraglimit, "ui_team_fraglimit", "0", CVAR_ARCHIVE },
	{ &ui_team_timelimit, "ui_team_timelimit", "20", CVAR_ARCHIVE },
	{ &ui_team_friendly, "ui_team_friendly",  "1", CVAR_ARCHIVE },

	{ &ui_ctf_capturelimit, "ui_ctf_capturelimit", "8", CVAR_ARCHIVE },
	{ &ui_ctf_timelimit, "ui_ctf_timelimit", "30", CVAR_ARCHIVE },
	{ &ui_ctf_friendly, "ui_ctf_friendly",  "0", CVAR_ARCHIVE },

	{ &ui_arenasFile, "g_arenasFile", "", CVAR_INIT | CVAR_ROM },
	{ &ui_botsFile, "g_botsFile", "", CVAR_INIT | CVAR_ROM },
	{ &ui_spScores1, "g_spScores1", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores2, "g_spScores2", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores3, "g_spScores3", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores4, "g_spScores4", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores5, "g_spScores5", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spAwards, "g_spAwards", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spVideos, "g_spVideos", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spSkill, "g_spSkill", "2", CVAR_ARCHIVE | CVAR_LATCH },

	{ &ui_spSelection, "ui_spSelection", "", CVAR_ROM },

	{ &ui_browserMaster, "ui_browserMaster", "2", CVAR_ARCHIVE },
	{ &ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE },
	{ &ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE },
	{ &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
	{ &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },

	{ &ui_brassTime, "cg_brassTime", "1250", CVAR_ARCHIVE },
	{ &ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &ui_marks, "cg_marks", "1", CVAR_ARCHIVE },

	// X-Mod: server cache
	{ &uix_serverCache, "ui_serverCache", "1", CVAR_TEMP },
	{ &uix_browserShowMplayer, "ui_browserShowMplayer", "0", CVAR_TEMP },
	{ &uix_browserShowNettype, "ui_browserShowNettype", "1", CVAR_TEMP },
	{ &uix_blockedIPs, "ui_blockedIPs", "78.141.221.220 62.35.11.0", CVAR_TEMP },

	// X-MOD: default parameters
	{ &uix_wideScreenFix, "cg_wideScreenFix", "1", CVAR_ARCHIVE },
	{ &uix_defaultWeapon, "cg_defaultWeapon", "0", CVAR_ARCHIVE },

	{ &uix_enemyModel_enabled, "cg_enemyModel_enabled", "0", CVAR_ARCHIVE },
	{ &uix_enemyModel, "cg_enemyModel", "", CVAR_ARCHIVE },		
	{ &uix_enemyColors, "cg_enemyColors", "", CVAR_ARCHIVE },
	//{ &uix_teamModel_enabled, "cg_teamModel_enabled", "0", CVAR_ARCHIVE },
	{ &uix_teamModel, "cg_teamModel", "", CVAR_ARCHIVE },		
	{ &uix_teamColors, "cg_teamColors", "", CVAR_ARCHIVE },
	{ &uix_deadBodyDarken, "cg_deadBodyDarken", "3", CVAR_ARCHIVE },	

	{ &uix_chatSound, "cg_chatSound", "1", CVAR_ARCHIVE },
	{ &uix_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE },
	{ &uix_hitsounds, "cg_hitsounds", "0", CVAR_ARCHIVE },
	{ &uix_centerPrintAlpha, "cg_centerPrintAlpha", "1.0", CVAR_ARCHIVE },
	{ &uix_crosshairColor, "cg_crosshairColor", "", CVAR_ARCHIVE },	
	{ &uix_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE },
	{ &uix_networkAdjustments, "cg_autoNetworkSettings", "1", CVAR_ARCHIVE },
	{ &uix_scoreboard, "cg_scoreboard", "0", CVAR_ARCHIVE },
	{ &uix_drawScoreBox, "cg_drawScoreBox", "1", CVAR_ARCHIVE },
	{ &uix_sharedConfig, "cg_sharedConfig", "0", CVAR_ARCHIVE },

	{ &uix_nomip, "cg_nomip", "-1", CVAR_ARCHIVE | CVAR_LATCH },
	{ &uix_weaponEffects, "cg_weaponEffects", "8", CVAR_ARCHIVE },
	{ &uix_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE },

	{ &s_ambient, "s_ambient", "1", CVAR_ARCHIVE },

#if CGX_UNLAGGED
	{ &cg_delag, "cg_delag", "1", CVAR_ARCHIVE },	
	{ &cg_projectileNudge, "cg_delag_projectileNudge", "0", CVAR_ARCHIVE },
	{ &cg_optimizePrediction, "cg_delag_optimizePrediction", "1", CVAR_ARCHIVE },
#endif

	// some quake3 not initialized variables
	{ &uix_zoomfov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },	
	{ &uix_fov, "cg_fov", "90", CVAR_ARCHIVE },	
	{ &uix_drawgun, "cg_drawGun", "1", CVAR_ARCHIVE },	
	{ &uix_drawrewards, "cg_drawRewards", "1", CVAR_ARCHIVE },	
	{ &uix_draw3dicons, "cg_draw3dIcons", "1", CVAR_ARCHIVE },	
	{ &uix_gibs, "cg_gibs", "1", CVAR_ARCHIVE },	
	{ &uix_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &uix_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &uix_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },

	{ &ui_server1, "server1", "", CVAR_ARCHIVE },
	{ &ui_server2, "server2", "", CVAR_ARCHIVE },
	{ &ui_server3, "server3", "", CVAR_ARCHIVE },
	{ &ui_server4, "server4", "", CVAR_ARCHIVE },
	{ &ui_server5, "server5", "", CVAR_ARCHIVE },
	{ &ui_server6, "server6", "", CVAR_ARCHIVE },
	{ &ui_server7, "server7", "", CVAR_ARCHIVE },
	{ &ui_server8, "server8", "", CVAR_ARCHIVE },
	{ &ui_server9, "server9", "", CVAR_ARCHIVE },
	{ &ui_server10, "server10", "", CVAR_ARCHIVE },
	{ &ui_server11, "server11", "", CVAR_ARCHIVE },
	{ &ui_server12, "server12", "", CVAR_ARCHIVE },
	{ &ui_server13, "server13", "", CVAR_ARCHIVE },
	{ &ui_server14, "server14", "", CVAR_ARCHIVE },
	{ &ui_server15, "server15", "sodmod.ml:27962", CVAR_ARCHIVE },
	{ &ui_server16, "server16", "sodmod.ml:27963", CVAR_ARCHIVE },

	{ &ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM }
};

int		cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);


/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}
}
