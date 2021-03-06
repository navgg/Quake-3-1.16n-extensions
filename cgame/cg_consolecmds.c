// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"


void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}

// nemesis/OSP client statistics window 
static void CG_StatsWindowDown_f(void) {
	if (cg.snap->ps.pm_type == PM_INTERMISSION && cgx_intermissionStats.integer)
		return;

	CG_statsWindow(0);
}

static void CG_StatsWindowUp_f(void) {
	if (cg.snap->ps.pm_type == PM_INTERMISSION && cgx_intermissionStats.integer)
		return;

	CG_statsWindowFree(0);
}

static void CG_ScoresDown_f( void ) {
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_StatsWindowDown_f();
		return;
	}

	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_StatsWindowUp_f();
		return;
	}

	cg.showScores = qfalse;
	cg.scoreFadeTime = cg.time;
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CGX_SaveSharedConfig_f( void ) {
	char arg[32];
	qboolean force = qfalse;
	trap_Args(arg, 32);

	CGX_SaveSharedConfig(!Q_stricmp(arg, "ui ") ? qfalse: qtrue);	
}

static void CGX_RecordSync_f( void ) {
	char	name[MAX_QPATH];
	trap_Args( name, MAX_QPATH );
	
	if (cg_syncronousClients.integer)
		trap_SendConsoleCommand(va("record %s\n", name));
	else
		trap_SendConsoleCommand(va("g_syncronousClients 1; record %s; g_syncronousClients 0\n", name));
}

static void CGX_Xmod_f(void) {
	CGX_Xmod();
}

static void CGX_Download_f(void) {
	char	name[MAX_QPATH];
	qboolean end_load = qfalse;

	if (trap_Argc() < 2) {
		CG_Printf("usage: download <mapname>\n");
		return;
	}

	trap_Argv(1, name, MAX_QPATH );

	if (trap_Argc() > 2) {
		char	str[16];
		trap_Argv(2, str, 16);
		end_load = atoi(str) ? qtrue : qfalse;
	}

	CGX_DownloadMap(name, end_load);
}

static void CGX_Say_f( void ) {
	char message[128];
	char *res;

	trap_Args( message, 128 );

	res = CGX_CheckChatTokens( message, COLOR_GREEN );

	trap_SendClientCommand( va( "say %s\n", res ) );
}

static void CGX_SayTeam_f( void ) {
	char message[128];
	char *res;

	trap_Args( message, 128 );

	res = CGX_CheckChatTokens( message, COLOR_CYAN );

	trap_SendClientCommand( va( "say_team %s\n", res ) );
}

//fixed follownext & followprev works only in spectator team
static void CGX_Follownext_f(void) {
	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR || cg.snap->ps.pm_flags & PMF_FOLLOW) {
		char args[128];
		trap_Args(args, 128);
		CGX_SendClientCommand(va("follownext %s\n", args));
	}
}

static void CGX_Followprev_f(void) {
	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR || cg.snap->ps.pm_flags & PMF_FOLLOW) {
		char args[128];
		trap_Args(args, 128);
		CGX_SendClientCommand(va("followprev %s\n", args));
	}
}

static void CGX_Followtarget_f(void) {
	int clientNum = CG_CrosshairPlayer();
	if (clientNum == -1)
		return;

	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR || cg.snap->ps.pm_flags & PMF_FOLLOW)
		CGX_SendClientCommand(va("follow %i\n", clientNum));
}

static void CGX_Echo_f(void) {
	char message[128];
	char *res;

	trap_Args( message, 128 );

	CG_Printf("%s\n", message);
}

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
#if CGX_DEBUG
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
#endif
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "loaddefered", CG_LoadDeferredPlayers },	// spelled wrong, but not changing for demo...
	// X-MOD: save shared config command
	{ "writesharedconfig", CGX_SaveSharedConfig_f },
	{ "download", CGX_Download_f },
	{ "autorecord", CGX_RecordSync_f },	
	{ "xmod", CGX_Xmod_f },
	{ "say", CGX_Say_f },
	{ "say_team", CGX_SayTeam_f },
	{ "follownext", CGX_Follownext_f },
	{ "followprev", CGX_Followprev_f },
	{ "followtarget", CGX_Followtarget_f },
	{ "echox", CGX_Echo_f },

#if CGX_FREEZE//freeze
	{ "drop", CG_Drop_f },
#endif//freeze

	//nemesis/OSP
	{ "+stats", CG_StatsWindowDown_f },
	{ "-stats", CG_StatsWindowUp_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("kill");
	//trap_AddCommand ("say");
	//trap_AddCommand ("say_team");
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("vote");
	trap_AddCommand ("callvote");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
#if CGX_FREEZE//freeze
	trap_AddCommand( "ready" );
#endif//freeze
}
