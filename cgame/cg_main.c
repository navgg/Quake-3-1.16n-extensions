// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

void CG_Init( int serverMessageNum, int serverCommandSequence );
void CG_Shutdown( void );

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 ) {
	switch ( command ) {
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CONSOLE_COMMAND:		
		return CG_ConsoleCommand();
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_INIT:
		CG_Init( arg0, arg1 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}

vScreen_t			vScreen;
xhud_t				hud;

cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[MAX_WEAPONS];
itemInfo_t			cg_items[MAX_ITEMS];

//OSP stats
clientStats_t		stats;

vmCvar_t	cgx_wideScreenFix;
vmCvar_t	cgx_enemyModel;
vmCvar_t	cgx_enemyModel_enabled;
vmCvar_t	cgx_enemyColors;
vmCvar_t	cgx_teamModel;
//vmCvar_t	cgx_teamModel_enabled;
vmCvar_t	cgx_teamColors;
vmCvar_t	cgx_deadBodyDarken;
vmCvar_t	cgx_defaultWeapon;
vmCvar_t	cgx_chatSound;
vmCvar_t	cgx_noTaunt;
vmCvar_t	cgx_centerPrintAlpha;
vmCvar_t	cgx_crosshairColor;
vmCvar_t	cgx_drawSpeed;
vmCvar_t	cgx_hitsounds;
vmCvar_t	cgx_networkAdjustments;
vmCvar_t	cgx_drawScoreBox;
vmCvar_t	cgx_scoreboard;
vmCvar_t	cgx_drawAccuracy;
vmCvar_t	cgx_weaponEffects;
vmCvar_t	cgx_nomip;
vmCvar_t	cgx_sharedConfig;
vmCvar_t	cgx_chatFilter;
vmCvar_t	cgx_killBeep;
vmCvar_t	cgx_winterEffects;
vmCvar_t	cgx_modelCache;
vmCvar_t	cgx_intermissionStats;
vmCvar_t	cgx_hud;
vmCvar_t	cgx_predictWeaponTime;
vmCvar_t	cgx_kickScale;
vmCvar_t	cgx_playerLean;

vmCvar_t	s_ambient;

vmCvar_t	com_maxfps;
vmCvar_t	cl_maxpackets;
vmCvar_t	cl_timeNudge;
vmCvar_t	sv_fps;

vmCvar_t	cgx_debug;

//1.32
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	pmove_accurate;

//unlagged - client options
vmCvar_t	cg_delag;
vmCvar_t	cg_cmdTimeNudge;
#if CGX_UNLAGGED
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cg_delag_interp32;
#endif
#if CGX_DEBUG
vmCvar_t	cg_debugDelag;
vmCvar_t	cg_drawBBox;
vmCvar_t	cg_latentSnaps;
vmCvar_t	cg_latentCmds;
vmCvar_t	cg_plOut;
#endif
//unlagged - client options

vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
#if CGX_DEBUG
vmCvar_t	cg_drawSnapshot;
#endif
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
#if CGX_DEBUG
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
#endif
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
#if CGX_DEBUG
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_footsteps;
#endif
vmCvar_t	cg_showmiss;

vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
#if CGX_DEBUG
vmCvar_t	cg_gun_frame;
#endif
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
#if CGX_DEBUG
vmCvar_t	cg_ignore;
#endif
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_syncronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
#if CGX_DEBUG
vmCvar_t 	cg_stats;
#endif
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;

#if CGX_FREEZE
vmCvar_t	cg_enableBreath;
#endif


typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

cvarTable_t		cvarTable[] = {
#if CGX_DEBUG
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
#endif
	{ &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
#if CGX_DEBUG
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
#endif
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
	{ &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairHealth, "cg_crosshairHealth", "0", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_brassTime, "cg_brassTime", "1250", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	// X-MOD: nethgraph + ping
	{ &cg_lagometer, "cg_lagometer", "1", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	// X-MOD: cg_gun no more cheats
	{ &cg_gun_x, "cg_gunX", "0", CVAR_ARCHIVE },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_ARCHIVE },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_ARCHIVE },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
#if CGX_DEBUG
	{ &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
#endif
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
#if CGX_DEBUG
	{ &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
#endif
	{ &cg_tracerChance, "cg_tracerchance", "0.4", CVAR_CHEAT },
	{ &cg_tracerWidth, "cg_tracerwidth", "1", CVAR_CHEAT },
	{ &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", 0 },
	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	//wtf?
	//	/*freeze
	//	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
	//	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
	//	freeze*/
	//{ &cg_thirdPersonRange, "cg_thirdPersonRange", "40", 0 },
	//{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", 0 },
	//	//freeze
	{ &cg_thirdPerson, "cg_thirdPerson", "0", 0 },
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
	{ &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
#if CGX_DEBUG
	{ &cg_stats, "cg_stats", "0", 0 },
#endif
	// X-MOD: extended cgx commands

	{ &cgx_wideScreenFix, "cg_wideScreenFix", "1", CVAR_ARCHIVE },
	{ &cgx_defaultWeapon, "cg_defaultWeapon", "0", CVAR_ARCHIVE },
	
	{ &cgx_enemyModel_enabled, "cg_enemyModel_enabled", "0", CVAR_ARCHIVE },
	{ &cgx_enemyModel, "cg_enemyModel", "", CVAR_ARCHIVE },		
	{ &cgx_enemyColors, "cg_enemyColors", "", CVAR_ARCHIVE },
	//{ &cgx_teamModel_enabled, "cg_teamModel_enabled", "0", CVAR_ARCHIVE },
	{ &cgx_teamModel, "cg_teamModel", "", CVAR_ARCHIVE },		
	{ &cgx_teamColors, "cg_teamColors", "", CVAR_ARCHIVE },
	{ &cgx_deadBodyDarken, "cg_deadBodyDarken", "3", CVAR_ARCHIVE },	

	{ &cgx_chatSound, "cg_chatSound", "1", CVAR_ARCHIVE },
	{ &cgx_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE },
	{ &cgx_hitsounds, "cg_hitsounds", "0", CVAR_ARCHIVE },
	{ &cgx_centerPrintAlpha, "cg_centerPrintAlpha", "1.0", CVAR_ARCHIVE },
	{ &cgx_crosshairColor, "cg_crosshairColor", "", CVAR_ARCHIVE },
	{ &cgx_drawSpeed, "cg_drawSpeed", "0", CVAR_ARCHIVE },
	{ &cgx_networkAdjustments, "cg_autoNetworkSettings", "1", CVAR_ARCHIVE },
	{ &cgx_scoreboard, "cg_scoreboard", "0", CVAR_ARCHIVE },
	{ &cgx_drawScoreBox, "cg_drawScoreBox", "1", CVAR_ARCHIVE },
	{ &cgx_drawAccuracy, "cg_drawAccuracy", "0", CVAR_ARCHIVE },
	{ &cgx_weaponEffects, "cg_weaponEffects", "8", CVAR_ARCHIVE },
	{ &cgx_sharedConfig, "cg_sharedConfig", "0", CVAR_ARCHIVE },
	{ &cgx_nomip, "cg_nomip", "-1", CVAR_ARCHIVE | CVAR_LATCH },
	{ &cgx_chatFilter, "cg_chatFilter", "1", CVAR_ARCHIVE },
	{ &cgx_killBeep, "cg_killBeep", "0", CVAR_ARCHIVE | CVAR_LATCH },

	{ &cgx_winterEffects, "cg_winterEffects", "0", CVAR_TEMP },

	{ &cgx_hud, "hud", "", CVAR_ARCHIVE },

	{ &cgx_predictWeaponTime, "cg_predictWeaponTime", "0", CVAR_TEMP },
	{ &cgx_kickScale, "cg_kickScale", "1", CVAR_ARCHIVE },
	{ &cgx_playerLean, "cg_playerLean", "1", CVAR_ARCHIVE },
	{ &s_ambient, "s_ambient", "1", CVAR_ARCHIVE },

	//unlagged - client options
	{ &cg_delag, "cg_delag", "1", CVAR_ARCHIVE | CGX_NOGHOST_COMPATIBLE},
	{ &cg_cmdTimeNudge, "cg_delag_cmdTimeNudge", "0", CVAR_ARCHIVE | CGX_NOGHOST_COMPATIBLE },
#if CGX_UNLAGGED
	{ &cg_projectileNudge, "cg_delag_projectileNudge", "0", CVAR_ARCHIVE },
	{ &cg_optimizePrediction, "cg_delag_optimizePrediction", "1", CVAR_ARCHIVE },
	{ &cg_delag_interp32, "cg_delag_interp32", "1", CVAR_TEMP },
#if CGX_DEBUG
	{ &cg_debugDelag, "cg_debugDelag", "0", CVAR_CHEAT },
	{ &cg_drawBBox, "cg_drawBBox", "0", CVAR_CHEAT },
	{ &cg_latentSnaps, "cg_latentSnaps", "0", CVAR_CHEAT },
	{ &cg_latentCmds, "cg_latentCmds", "0", CVAR_CHEAT },
	{ &cg_plOut, "cg_plOut", "0", CVAR_CHEAT },
#endif
	//unlagged - client options
#else
	{ &cg_delag, "cg_delag", "1", CVAR_ARCHIVE },
#endif
#if CGX_FREEZE
	{ &cg_enableBreath, "cg_enableBreath", "0", CVAR_TEMP },
#endif

	{ &com_maxfps, "com_maxfps", "125", CVAR_ARCHIVE | CGX_NOGHOST_COMPATIBLE },
	{ &cl_maxpackets, "cl_maxpackets", "40", CVAR_ARCHIVE | CGX_NOGHOST_COMPATIBLE },				
	{ &cl_timeNudge, "cl_timeNudge", "0", CVAR_ARCHIVE | CGX_NOGHOST_COMPATIBLE },

#if CGX_DEBUG
	{ &cgx_debug, "cgx_debug", "1", CVAR_TEMP },
#endif
	// resolving favorite servers by domain name
	// colored server names shifting left bug fixed
	// optimization: removed many debug info in CG_EntityEvent
	// other commands
	// cinematics menu fixed
	// model sarge^^^ fixed	
	// cgx_version - show version	
	// unlagged
	// g_delag	
	// fov adjust for widescreen

	// the following variables are created in other parts of the system,
	// but we also reference them here
#if CGX_DEBUG
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures

	{ &pmove_fixed, "pmove_fixed", "0", CVAR_TEMP },
	{ &pmove_msec, "pmove_msec", "8", CVAR_TEMP},
	{ &pmove_accurate, "pmove_accurate", "0", CVAR_TEMP },
#endif
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
	{ &cg_syncronousClients, "g_syncronousClients", "0", 0 },	// communicated by systeminfo	
};

int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );
int		cgx_wideScreenFixmodificationCount = 1;
int		cgx_enemyModelModificationCount = 1;
int		cgx_teamModelModificationCount = 1;
int		cgx_enemyColorsModificationCount = 1;
int		cgx_teamColorsModificationCount = 1;
int		cgx_deadBodyDarkenModificationCount = 1;
int		cgx_enemyModel_enabledModificationCount = 1;
int		cgx_fps_modificationCount = 1;
int		cgx_sharedConfigModificationCount = 1;
int		cgx_draw2DModificationCount = 1;
int		cgx_hudModificationCount = 1;

// some temp info
vmCvar_t	cgx_version;
vmCvar_t	cgx_last_error;
vmCvar_t	cgx_maploadingfix;
vmCvar_t	cgx_fixedmaps;
vmCvar_t	cgx_r_picmip;
vmCvar_t	cgx_profanity;

vmCvar_t	r_vertexLight;
vmCvar_t	r_picmip;

vmCvar_t	cgx_downloadedbytes;
vmCvar_t	cgx_totalbytes;
vmCvar_t	cgx_downloadname;
vmCvar_t	cgx_dl_host;
vmCvar_t	cgx_dl_page;
vmCvar_t	cgx_dl_page;
vmCvar_t	cgx_dl_dynb;
vmCvar_t	cgx_dl_tobaseq3;

vmCvar_t	cgx_helpShowed;

cvarTable_t		cgx_cvarTable_temp[] = {
	//handle map loading errors
	{ &cgx_last_error, "cgx_last_error", "", CVAR_TEMP | CVAR_ROM },
	//save for nomip
	{ &cgx_r_picmip, "cgx_r_picmip", "", CVAR_TEMP | CVAR_ROM },
	// X-MOD: fixes loading of some big maps 0: default 1: r_vertexLight 1 loading
	{ &cgx_maploadingfix, "cgx_fix_mapload", "0", CVAR_TEMP | CVAR_ROM },
	// stored fixed maplist, so if it once was fixed nextime will just read from this list
	//{ &cgx_fixedmaps, "cl_fixedmaps", "", CVAR_ROM | CVAR_ARCHIVE },
	{ &cgx_fixedmaps, "cg_fixedmaps", "", CVAR_TEMP | CVAR_ROM },
	{ &cgx_profanity, "cg_profanity", CGX_PROFANITY, CVAR_TEMP | CVAR_LATCH },
#if !CGX_DEBUG
	//mod version
	{ &cgx_version, "cgx_version", CGX_FULLVER, CVAR_ROM | CVAR_TEMP | CVAR_USERINFO },
#endif
	//for help message showing in intermission
	{ &cgx_helpShowed, "cgx_help_showed", "0", CVAR_TEMP },
	//for model cahe
	{ &cgx_modelCache, "cg_modelCache", "1", CVAR_TEMP | CVAR_LATCH },
	//ops stats window in intermission
	{ &cgx_intermissionStats, "cg_intermissionStats", "0", CVAR_TEMP },
	//for unlagged.c
	//better not register here or servefs will screw clients sv_fps
	// this will be automagically copied from the server	
	//{ &sv_fps, "sv_fps", "20", 0 },
	{ &r_vertexLight, "r_vertexLight", "0", 0 },	
	{ &r_picmip, "r_picmip", "0", 0 },

	//for downloader
	{ &cgx_downloadedbytes, "cgx_downloadedbytes", "", CVAR_TEMP | CVAR_ROM },
	{ &cgx_totalbytes, "cgx_totalbytes", "", CVAR_TEMP | CVAR_ROM },
	{ &cgx_downloadname, "cgx_downloadname", "", CVAR_TEMP | CVAR_ROM },
	{ &cgx_dl_dynb, "cgx_dl_dynb", "1", CVAR_TEMP },
	{ &cgx_dl_host, "cgx_dl_host", "ws.q3df.org", CVAR_TEMP },
	{ &cgx_dl_page, "cgx_dl_page", "/maps/download/%s", CVAR_TEMP },
	{ &cgx_dl_tobaseq3, "cgx_dl_tobaseq3", "1", CVAR_TEMP },

#if !CGX_DEBUG
	//1.32
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_ROM | CVAR_TEMP },
	{ &pmove_msec, "pmove_msec", "8", CVAR_ROM | CVAR_TEMP},
	{ &pmove_accurate, "pmove_accurate", "0", CVAR_ROM | CVAR_TEMP },
#endif
};

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer("version", var, sizeof var);

	cg.q3version = atoi(&var[5]);

	//vanilla defaults
	if (cg.q3version != 16 || var[7] != 'x') {
		for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
			if (cv->vmCvar == &cl_maxpackets)
				cv->defaultString = "30";
			else if (cv->vmCvar == &com_maxfps)
				cv->defaultString = "85";

		for (i = 0, cv = cgx_cvarTable_temp; i < ArrLen(cgx_cvarTable_temp); i++, cv++)
			if (cv->vmCvar == &r_picmip)
				cv->defaultString = "1";
	}

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	//X-MOD: temp table, no update (only manual with trap_Cvar_Update)
	for (i = 0, cv = cgx_cvarTable_temp; i < ArrLen(cgx_cvarTable_temp); i++, cv++) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	sv_fps.integer = 20;

	//save current picmip
	CGX_SavePicmip();

	//modification counts should be initialized here
	//TODO: make it initializing in loop, from -1 initial value
	cgx_wideScreenFixmodificationCount = cgx_wideScreenFix.modificationCount;
	cgx_enemyModelModificationCount = cgx_enemyModel.modificationCount;
	cgx_teamModelModificationCount = cgx_teamModel.modificationCount;
	cgx_enemyColorsModificationCount = cgx_enemyColors.modificationCount;
	cgx_teamColorsModificationCount = cgx_teamColors.modificationCount;
	cgx_deadBodyDarkenModificationCount = cgx_deadBodyDarken.modificationCount;
	cgx_enemyModel_enabledModificationCount = cgx_enemyModel_enabled.modificationCount;
	cgx_fps_modificationCount = cg_drawFPS.modificationCount;
	cgx_sharedConfigModificationCount = cgx_sharedConfig.modificationCount;
	cgx_draw2DModificationCount = cg_draw2D.modificationCount;
	cgx_hudModificationCount = cgx_hud.modificationCount;
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;	

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
#if CGX_UNLAGGED
		//unlagged - client options
		// clamp the value between 0 and 999
		// negative values would suck - people could conceivably shoot other
		// players *long* after they had left the area, on purpose
		if ( cv->vmCvar == &cg_cmdTimeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 999 );
		}
		// cl_timenudge less than -50 or greater than 50 doesn't actually
		// do anything more than -50 or 50 (actually the numbers are probably
		// closer to -30 and 30, but 50 is nice and round-ish)
		// might as well not feed the myth, eh?
		// X-Mod: limit to -20 and 20 like in quake live
		else if ( cv->vmCvar == &cl_timeNudge ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, -20, 20 );
		}
#if CGX_DEBUG
		// don't let this go too high - no point
		else if ( cv->vmCvar == &cg_latentSnaps ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 10 );
		}
		// don't let this get too large
		else if ( cv->vmCvar == &cg_latentCmds ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, MAX_LATENT_CMDS - 1 );
		}
		// no more than 100% packet loss
		else if ( cv->vmCvar == &cg_plOut ) {
			CG_Cvar_ClampInt( cv->cvarName, cv->vmCvar, 0, 100 );
		}
#endif //  CGX_DEBUG
		//unlagged - client options		
#endif //  CGX_UNLAGGED

		trap_Cvar_Update( cv->vmCvar );
	}
	//X-MOD: validate maxpackets
	if (cl_maxpackets.integer < CGX_MIN_MAXPACKETS) {
		cl_maxpackets.integer = CGX_MIN_MAXPACKETS;
		CG_Printf("Min cl_maxpackets is %i\n", CGX_MIN_MAXPACKETS);
		trap_Cvar_Set("cl_maxpackets", va("%i", CGX_MIN_MAXPACKETS));
	}

	// check for modications here
	// X-MOD: reinit vScreen if value changed
	if (cgx_wideScreenFixmodificationCount != cgx_wideScreenFix.modificationCount ||
		cgx_draw2DModificationCount != cg_draw2D.modificationCount ||
		cgx_hudModificationCount != cgx_hud.modificationCount) {
		cgx_wideScreenFixmodificationCount = cgx_wideScreenFix.modificationCount;
		cgx_draw2DModificationCount = cg_draw2D.modificationCount;
		cgx_hudModificationCount = cgx_hud.modificationCount;
		CGX_Init_vScreen();
		D_Printf(("^6CG_UpdateCvars value changed\n"));
	}

	// X-MOD: reinit enemymodels if value or player's team changed
	if (cgx_enemyModelModificationCount != cgx_enemyModel.modificationCount ||
		cgx_teamModelModificationCount != cgx_teamModel.modificationCount ||
		cgx_enemyModel_enabledModificationCount != cgx_enemyModel_enabled.modificationCount) {
		
		if (!cgx_enemyModel_enabled.integer && cgx_enemyModel.string[0] &&
			cgx_enemyModel_enabled.modificationCount == cgx_enemyModel_enabledModificationCount) {
			trap_Cvar_Set("cg_enemyModel_enabled", "1");
			D_Printf(("^6cg_enemyModel_enabled 1\n"));
		}
		else {
			CGX_Init_enemyModels();			
			CGX_CheckEnemyModelAll(qtrue);
			D_Printf(("^6CG_UpdateCvars value changed\n"));
		}

		cgx_enemyModelModificationCount = cgx_enemyModel.modificationCount;
		cgx_teamModelModificationCount = cgx_teamModel.modificationCount;
		cgx_enemyModel_enabledModificationCount = cgx_enemyModel_enabled.modificationCount;	
	}	

	// X-MOD: reinit enemycolors if vaue changed
	if (cgx_enemyColorsModificationCount != cgx_enemyColors.modificationCount ||
		cgx_teamColorsModificationCount != cgx_teamColors.modificationCount ||
		cgx_deadBodyDarkenModificationCount != cgx_deadBodyDarken.modificationCount) {
		cgx_enemyColorsModificationCount = cgx_enemyColors.modificationCount;
		cgx_teamColorsModificationCount = cgx_teamColors.modificationCount;
		cgx_deadBodyDarkenModificationCount = cgx_deadBodyDarken.modificationCount;		

		CGX_SetSkinColorsAll();	
		D_Printf(("^6CG_UpdateCvars value changed\n"));
	}
	
	//track fps change & validate
	if (cgx_fps_modificationCount != com_maxfps.modificationCount) {
		cgx_fps_modificationCount = com_maxfps.modificationCount;		

		CGX_AutoNetworkSettings();
	}
	//track shared config change
	if (cgx_sharedConfigModificationCount != cgx_sharedConfig.modificationCount) {	
		cgx_sharedConfigModificationCount = cgx_sharedConfig.modificationCount;

		CGX_SaveSharedConfig(qtrue);
	}

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
	}
}


int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}


int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}


void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CGX_AddToConsole(text);

	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	CGX_NomipEnd();//if anywhere was NomipStart then end it due to error

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Cvar_Set("cgx_last_error", va("0 %s", text));

	trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

#endif



/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	trap_S_RegisterSound( item->pickup_sound );

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	char	name[MAX_QPATH];
	const char	*soundName;	

	if ( cgs.timelimit || cg_buildScript.integer ) {	// should we always load this?
		cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav" );
		cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav" );
		cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav" );
	}

	if ( cgs.fraglimit || cg_buildScript.integer ) {
		cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav" );
		cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav" );
		cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav" );
	}

//	if ( cgs.gametype == GT_TOURNAMENT || cg_buildScript.integer ) {
//  We always need this since a warmup can be enabled in any game mode
		cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav" );
		cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav" );
		cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav" );
		cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav" );
		cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav" );
//	}

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav" );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav" );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav" );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav" );
	}

	cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav" );
	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav" );
	cgs.media.wearOffSound = trap_S_RegisterSound( "sound/items/wearoff.wav" );
	cgs.media.useNothingSound = trap_S_RegisterSound( "sound/items/use_nothing.wav" );
	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav" );
	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav" );
	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav" );
	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav" );

	cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav" );
	cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav" );
	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav" );

	cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav" );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav" );
	cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav");

	cgs.media.hitSound = trap_S_RegisterSound( "sound/feedback/hit.wav" );

	cgs.media.hitSounds[0] = trap_S_RegisterSound("sound/feedback/hit25.wav");
	cgs.media.hitSounds[1] = trap_S_RegisterSound("sound/feedback/hit50.wav");
	cgs.media.hitSounds[2] = trap_S_RegisterSound("sound/feedback/hit75.wav");
	cgs.media.hitSounds[3] = trap_S_RegisterSound("sound/feedback/hit100.wav");

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav" );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav" );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav" );
	cgs.media.humiliationSound = trap_S_RegisterSound( "sound/feedback/humiliation.wav" );

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav");
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav");
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav");

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav");
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav");
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav");

	cgs.media.jumpPadSound = trap_S_RegisterSound ("sound/world/jumppad.wav" );

	for (i=0 ; i<4 ; i++) {
		Com_sprintf (name, sizeof(name), "sound/player/footsteps/step%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/boot%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_BOOT][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/flesh%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_FLESH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/mech%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_MECH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/energy%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_ENERGY][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/splash%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name);

		Com_sprintf (name, sizeof(name), "sound/player/footsteps/clank%i.wav", i+1);
		cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name);
	}

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName );
	}

	// FIXME: only needed with item
	cgs.media.flightSound = trap_S_RegisterSound( "sound/items/flight.wav" );
	cgs.media.medkitSound = trap_S_RegisterSound ("sound/items/use_medkit.wav");
	cgs.media.quadSound = trap_S_RegisterSound("sound/items/damage3.wav");
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav");
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav");
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav");
	cgs.media.sfx_railg = trap_S_RegisterSound ("sound/weapons/railgun/railgf1a.wav");
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav");
	cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav");

	if (cgx_killBeep.integer)
		cgs.media.killBeep = trap_S_RegisterSound (va("sound/feedback/impact%i.wav", cgx_killBeep.integer % 8));
}


//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		*fold = "";
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();
	
	CGX_LoadWorldMap();

	CGX_NomipStart();

	//D_Printf(("precache status bar pics\n"));
	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	cgs.media.botSkillShaders[0] = trap_R_RegisterShader( "menu/art/skill1.tga" );
	cgs.media.botSkillShaders[1] = trap_R_RegisterShader( "menu/art/skill2.tga" );
	cgs.media.botSkillShaders[2] = trap_R_RegisterShader( "menu/art/skill3.tga" );
	cgs.media.botSkillShaders[3] = trap_R_RegisterShader( "menu/art/skill4.tga" );
	cgs.media.botSkillShaders[4] = trap_R_RegisterShader( "menu/art/skill5.tga" );

	cgs.media.viewBloodShader = trap_R_RegisterShader( "viewBloodBlend" );

	cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );

	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	//cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "shotgunSmokePuff" );
	cgs.media.plasmaBallShader = trap_R_RegisterShader( "sprites/plasma1" );
	cgs.media.bloodTrailShader = trap_R_RegisterShader( "bloodTrail" );
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

	cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShader( "icons/noammo" );

	// powerup shaders
	cgs.media.quadShader = trap_R_RegisterShader("powerups/quad" );
	cgs.media.quadWeaponShader = trap_R_RegisterShader("powerups/quadWeapon" );
	cgs.media.battleSuitShader = trap_R_RegisterShader("powerups/battleSuit" );
	cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon" );
	cgs.media.invisShader = trap_R_RegisterShader("powerups/invisibility" );
	cgs.media.regenShader = trap_R_RegisterShader("powerups/regen" );
	cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff" );

	if (cg.q3version == 11) {
		// update icon references, some games register it even not in ctf mode
		gitem_t *item;
		item = BG_FindItemForPowerup(PW_REDFLAG);
		item->icon = "xm/icons/iconf_red1";
		item = BG_FindItemForPowerup(PW_BLUEFLAG);
		item->icon = "xm/icons/iconf_blu1";
		fold = "xm/";
	}

	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );

		for (i = 3; i--; ) {
			cgs.media.redFlagShader[i] = trap_R_RegisterShaderNoMip(va("%sicons/iconf_red%i", fold, i + 1));
			cgs.media.blueFlagShader[i] = trap_R_RegisterShaderNoMip(va("%sicons/iconf_blu%i", fold, i + 1));
		}
	}

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
		cgs.media.redQuadShader = trap_R_RegisterShader("powerups/blueflag" );
		cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
	}

#if CGX_FREEZE
	if ( cgs.gametype == GT_TEAM ) {
		cgs.media.freezeShader = trap_R_RegisterShader( "freezeShader" );
		cgs.media.freezeMarkShader = trap_R_RegisterShader( "freezeMarkShader" );
	}
#endif //freeze

	cgs.media.armorModel = trap_R_RegisterModel( "models/powerups/armor/armor_yel.md3" );
	cgs.media.armorIcon  = trap_R_RegisterShaderNoMip( "icons/iconr_yellow" );

	cgs.media.machinegunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/m_shell.md3" );
	cgs.media.shotgunBrassModel = trap_R_RegisterModel( "models/weapons2/shells/s_shell.md3" );

	cgs.media.gibAbdomen = trap_R_RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = trap_R_RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = trap_R_RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = trap_R_RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = trap_R_RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = trap_R_RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = trap_R_RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = trap_R_RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = trap_R_RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = trap_R_RegisterModel( "models/gibs/brain.md3" );
	
	cgs.media.balloonShader = trap_R_RegisterShader( "sprites/balloon3" );

	cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );

	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalGauntlet = trap_R_RegisterShaderNoMip( "medal_gauntlet" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );
	//D_Printf(("only register the items that the server says we need\n"));
	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}
	//D_Printf(("wall marks\n"));
	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );
	cgs.media.holeMarkShader = trap_R_RegisterShader( "gfx/damage/hole_lg_mrk" );
	cgs.media.energyMarkShader = trap_R_RegisterShader( "gfx/damage/plasma_mrk" );
	cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
	cgs.media.wakeMarkShader = trap_R_RegisterShader( "wake" );
	cgs.media.bloodMarkShader = trap_R_RegisterShader( "bloodMark" );
	//D_Printf(("register the inline models\n"));
	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char	name[10];
		vec3_t			mins, maxs;
		int				j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
		D_Printf(("^3%s ", name));
	}
	D_Printf(("\n"));
	//D_Printf(("register all the server specified models\n"));
	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
		D_Printf(("^6cgs.gameModels %i %s\n", i, modelName));		
	}

	CGX_NomipEnd();
}

/*
===================
CG_RegisterClients

===================
*/
static void CG_RegisterClients( void ) {
	int		i;
	
	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_LoadingClient( i );
		CG_NewClientInfo( i );
	}
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}


/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence ) {
	const char	*s;	

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );
	
	memset( &vScreen, 0, sizeof( vScreen ) );
	memset( &hud, 0, sizeof( hud ) );
	memset( &xhud, 0, sizeof( xhud ));

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader = trap_R_RegisterShader( "gfx/2d/bigchars" );
	if (!(cgs.media.charsetShader32 = trap_R_RegisterShaderNoMip("gfx/2d/bigchars_32")))
		cgs.media.charsetShader32 = cgs.media.charsetShader;
	cgs.media.whiteShader = trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB	= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_MACHINEGUN;	

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	//X-MOD: reset clientnum and client oldteam, then init enemymodels and vScreen

	cg.clientNum = -1;	
	cg.oldTeam = -1;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	CGX_Init_enemyModels();
	CGX_Init_vScreen();
	CGX_ResetModelCache();

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();

	// load the new map
	CGX_LoadCollisionMap();
	
	cg.loading = qtrue;		// force players to load instead of defer

	CG_LoadingString( "sounds" );
	D_Printf(("CG_RegisterSounds\n"));
	CG_RegisterSounds();
	D_Printf(("CG_RegisterGraphics\n"));
	CG_RegisterGraphics();
	D_Printf(("CG_RegisterClients\n"));
	CG_RegisterClients();		// if low on memory, some clients will be deferred

	cg.loading = qfalse;	// future players will be deferred
	D_Printf(("CG_InitLocalEntities\n"));
	CG_InitLocalEntities();
	D_Printf(("CG_InitMarkPolys\n"));
	CG_InitMarkPolys();

	// remove the last loading update
	cg.infoScreenText[0] = 0;
	D_Printf(("CG_SetConfigValues\n"));
	// Make sure we have update values (scores)
	CG_SetConfigValues();
	D_Printf(("CG_StartMusic\n"));
	CG_StartMusic();

	CG_LoadingString( "" );	

	//x-mod: fix redirects via activeAction
	{
		char buf[MAX_INFO_STRING];
		trap_Cvar_VariableStringBuffer("activeAction", buf, sizeof(buf));
		if (*buf && stristr(buf, "connect"))
			trap_Cvar_Set("activeAction", "");
	}
}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {		
	char cgx_last_error[MAX_QPATH], var[MAX_QPATH];

	D_Printf(("CG_Shutdown\n"));

	trap_Cvar_VariableStringBuffer("cgx_last_error", cgx_last_error, sizeof(cgx_last_error));
	D_Printf(("cgx_last_error %s\n", cgx_last_error));

	if (cgx_last_error[0] == '1') {// error during loading collision map
		CGX_GenerateMapBat("");
	} else if (cgx_last_error[0] == '2') { // error during loadworld map
		if (cgx_maploadingfix.integer)
			trap_Cvar_Set("r_vertexLight", "0");
		CGX_IncreaseHunkmegs(CGX_MINHUNKMEGS); // prolly hunkmegs		
	}

	trap_Cvar_Set("pmove_fixed", "0");
	trap_Cvar_Set("pmove_msec", "8");
	trap_Cvar_Set("pmove_accurate", "0");

	if (stats.needprint)
		CG_statsWindowPrint();

	trap_Cvar_VariableStringBuffer("cgx_com_maxfps", var, sizeof var);
	if (*var) {
		D_Printf(("Restore com_maxfps %s\n", var));
		trap_Cvar_Set("com_maxfps", var);
		trap_Cvar_Set("cgx_com_maxfps", "");
	}

	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


