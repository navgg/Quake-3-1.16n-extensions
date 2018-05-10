// Copyright (C) 2018 NaViGaToR (322)

#include "cg_local.h"

#define ShaderRGBAFill(a,c)	((a)[0]=(c),(a)[1]=(c),(a)[2]=(c),(a)[3]=(255))

#define XMOD_ANSWER(x) { CG_Printf("^7[^1xmod^7]: ^6%s\n", x); trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND ); }

#define DARKEN_COLOR 64
#define EM_SPECT			2
#define EM_INTERMISSION		4

// instead of modes 1 2 4 6 will be 1 2 3 4
static qboolean EM_Check(int x) {
	int i = cgx_enemyModel_enabled.integer;
	i = i == 3 ? i = 4 : i == 4 ? i = 6 : i;	
	return i & x;
}

static char *known_models[] = {
	"anarki",
	"biker",
	"bitterman",
	"bones",
	"crash",
	"doom",
	"grunt",
	"hunter",
	"keel",
	"klesk",
	"lucy",
	"major",
	"mynx",
	"orbb",
	"ranger",
	"razor",
	"sarge",
	"slash",
	"sorlag",
	"tankjr",
	"uriel",
	"visor",
	"xaero"
};

void CGX_Init_vScreen(void) {	
	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	// X-MOD: init virtual screen sizes for wide screen fix
	
	if ( (cgx_wideScreenFix.integer & CGX_WFIX_SCREEN) && cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		vScreen.width = (float)cgs.glconfig.vidWidth / (float)cgs.glconfig.vidHeight * 480.0f;		 				 				
		vScreen.offsetx = (vScreen.width - 640) / 2.0f;
	} else {
		vScreen.width = 640;						
		vScreen.offsetx = 0;
	}	

	vScreen.ratiox = (float)vScreen.width / 640.0f;

	vScreen.hwidth = vScreen.width / 2;
	vScreen.hheight = SCREEN_HEIGHT / 2;

	cgs.screenXScale = (float)cgs.glconfig.vidWidth / (float)vScreen.width;
	cgs.screenYScale = (float)cgs.glconfig.vidHeight / (float)SCREEN_HEIGHT; 

	vScreen.sbheadx = 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + vScreen.offsetx;
	vScreen.sbarmorx = (370 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE) + vScreen.offsetx;
	vScreen.sbammox = (CHAR_WIDTH * 3 + TEXT_ICON_SPACE) + vScreen.offsetx;
	vScreen.sbflagx = 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE + vScreen.offsetx;
	vScreen.sbhealth = 185 + vScreen.offsetx;	

	vScreen.width48 = vScreen.width - 48;
	vScreen.width5 = vScreen.width - 5;

	D_Printf(("CGX_Init_vScreen %fx%f cgx_wideScreenFix %d\n", vScreen.width, SCREEN_HEIGHT, cgx_wideScreenFix.integer));	
}

void CGX_Init_enemyModels(void) {
	char modelStr[MAX_QPATH];
	char *slash;	

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	Q_strncpyz(modelStr, cgx_enemyModel.string, sizeof(modelStr));

	slash = strchr( modelStr, '/' );
	if ( !slash ) {
		// modelName didn not include a skin name				
		Q_strncpyz(cg.enemySkin, "", sizeof(cg.enemySkin));
	} else {		
		Q_strncpyz(cg.enemySkin, slash + 1, sizeof(cg.enemySkin));
		// truncate modelName
		*slash = 0;
	}

	Q_strncpyz(cg.enemyModel, modelStr, sizeof(cg.enemyModel));	

	D_Printf(("CGX_Init_enemyModels cg.enemyModel: %s cg.enemySkin: %s\n", cg.enemyModel, cg.enemySkin));
}

void CGX_Init_teamModels(void) {
	char modelStr[MAX_QPATH];
	char *slash;	

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	Q_strncpyz(modelStr, cgx_teamModel.string, sizeof(modelStr));

	slash = strchr( modelStr, '/' );
	if ( !slash ) {
		// modelName didn not include a skin name		
		Q_strncpyz(cg.teamSkin, "", sizeof(cg.teamSkin));
	} else {		
		Q_strncpyz(cg.teamSkin, slash + 1, sizeof(cg.teamSkin));
		// truncate modelName
		*slash = 0;
	}

	Q_strncpyz(cg.teamModel, modelStr, sizeof(cg.teamModel));	

	D_Printf(("CGX_Init_teamModels cg.teamModel: %s cg.teamSkin: %s\n", cg.teamModel, cg.teamSkin));
}

void CGX_EnemyModelCheck(void) {
	int		i;
	clientInfo_t	*ci;

	if (cg.clientNum == -1) {
		D_Printf(("^1CGX_EnemyModelCheck before clientNum init\n"));
		return;
	}

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	D_Printf(("CGX_EnemyModelCheck %i\n", cg.clientNum));	

	//change models and skins if needed or restore
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ )
		if (ci->infoValid)
			CGX_SetModelAndSkin(ci, qtrue, i);
	
	D_Printf(("CG_LoadDeferredPlayers\n"));

	CG_LoadDeferredPlayers();
}

static void CGX_ColorFromChar(char v, byte *color, clientInfo_t *info) {
	int j;

	if (v == '?' || v == '!')
		switch (info->team) {
			case TEAM_RED:  v = '1'; break;
			case TEAM_BLUE: v = '4'; break;
			case TEAM_FREE: v = '2'; break;
			default: v = '7'; break;
		} 
	else if (v == '*')
		v = '0' + random() * (ArrLen(g_color_table) - 1);
	else if (v == '.')
		v = '0' + random() * (ArrLen(g_color_table_ex) - 1);

	j = (v - '0') % ArrLen(g_color_table_ex);//  for g_color_table_ex	
	//j = ColorIndex(v);// for g_color_table
	color[0] = g_color_table_ex[j][0] * 255;
	color[1] = g_color_table_ex[j][1] * 255;
	color[2] = g_color_table_ex[j][2] * 255;
	color[4] = 255;
}

static byte CGX_RGBToGray(byte *c) {		
	switch (cgx_deadBodyDarken.integer)
	{		
		case 2:	return (0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2]) / 4; //BT709 Greyscale
		case 3: return (0.2989f * c[0] + 0.5870f * c[1] + 0.1140f * c[2]) / 4; //Y-Greyscale (PAL/NTSC)
		default: return DARKEN_COLOR; //just gray
	}
}

static void CGX_SetColorInfo(clientInfo_t *info, const char *color) {	
	int i;

	// if skin is not pm skip
	if (Q_stricmp(info->skinName, "pm"))
		return;

	if (!*color)
		color = "!!!!";
	else if (i = QX_StringToColor(color))
		color = va("%c%c%c%c", i, i, i, i);

	for (i = 0; i < 4; i++) {
		if (!color[i])
			return;

		CGX_ColorFromChar(color[i], info->colors[i], info);
		//D_Printf(("%3i %3i %3i\n", info->colors[i][0], info->colors[i][1], info->colors[i][2]));		

		if (cgx_deadBodyDarken.integer)
			ShaderRGBAFill(info->darkenColors[i], CGX_RGBToGray(info->colors[i]));
		else
			ShaderRGBACopy(info->colors[i], info->darkenColors[i]);		
	}	

	// copy rail color
	if (color[0] == '!') {
		VectorCopy(info->colorCopy, info->color);
	} else {
		for (i = 0; i < 3; i++)
			info->color[i] = (float)info->colors[0][i] / 255.0f;
	}
}

void CGX_Init_enemyAndTeamColors(void) {
	int i;
	clientInfo_t *ci;

	if (cg.oldTeam == TEAM_SPECTATOR || cgs.gametype == GT_SINGLE_PLAYER)		
		return;

	for (i = 0, ci = cgs.clientinfo; i < cgs.maxclients; i++, ci++)
		if (cg.clientNum == i)
			continue;
		else if (cgs.gametype < GT_TEAM || ci->team != cgs.clientinfo[cg.clientNum].team)
			CGX_SetColorInfo(ci, cgx_enemyColors.string);
		else
			CGX_SetColorInfo(ci, cgx_teamColors.string);
}

//restore real model and skin if needed and return result
#define IsSameModel(x) !Q_stricmp(x->modelName, x->modelNameCopy) && !Q_stricmp(x->skinName, x->skinNameCopy)
static qboolean CGX_RestoreModelAndSkin(clientInfo_t *ci, int clientNum, qboolean isDeferred) {
	qboolean isSpect = ci->team == TEAM_SPECTATOR && !EM_Check(EM_SPECT);
	qboolean isPlayer = qfalse;
	qboolean isPlayerSpect = qfalse;

	//skip emtpy models
	if (!ci->modelName[0]) {
		D_Printf(("^1SKIP %i\n", clientNum));
		return qfalse;
	}

	if (cg.clientNum >= 0) {		
		isPlayerSpect = cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && !EM_Check(EM_SPECT);
		isPlayer = cg.clientNum == clientNum;
	}

	//if disabled or its spect or it's own model or player is in spect
	if (!cgx_enemyModel_enabled.integer || isSpect || isPlayerSpect || isPlayer || 
		cg.clientIntermission) {
		if (IsSameModel(ci)) {
			D_Printf(("^3OK %i\n", clientNum));
			return qtrue;
		}

		Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));
		Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));

		Vector4Copy(ci->colorCopy, ci->color);

		ci->deferred = isDeferred;

		D_Printf(("^3Restore '%i' '%s/%s' '%s/%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy, ci->infoValid, ci->deferred));
		return qtrue;
	}

	return qfalse;
}


static void CGX_SetModel(clientInfo_t *ci, char *modelName) {
	//save copy
	if (!ci->modelNameCopy[0])
		Q_strncpyz(ci->modelNameCopy, ci->modelName, sizeof(ci->modelName));
	if (!ci->skinNameCopy[0])
		Q_strncpyz(ci->skinNameCopy, ci->skinName, sizeof(ci->skinName));

	//if model not specified get from copy
	if (!modelName[0])
		Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));		
	else
		Q_strncpyz(ci->modelName, modelName, sizeof(ci->modelName));
}

static qboolean CGX_IsKnownModel(const char *modelName) {
	int i;
	for (i = 0; i < ArrLen(known_models); i++)
		if (Q_stricmp(modelName, known_models[i]) == 0)
			return qtrue;	

	return qfalse;	
}

static void CGX_SetPMSkin(clientInfo_t *ci) {
	if (!CGX_IsKnownModel(ci->modelName)) {
		D_Printf(("^3Warning: No PM skin for model %s\n", ci->modelName));
		Q_strncpyz(ci->modelName, DEFAULT_MODEL, sizeof(ci->modelName));
	}

	Q_strncpyz(ci->skinName, "pm", sizeof(ci->skinName));		
}

static void CGX_SetSkin(clientInfo_t *ci, char *skinName) {	
	if (!skinName[0]) //if no skin set pm
		CGX_SetPMSkin(ci);
	else if (cgs.gametype < GT_TEAM || !Q_stricmp(skinName, "pm"))
		Q_strncpyz(ci->skinName, skinName, sizeof(ci->skinName));
	// if gametype is not team\ctf or skin pm set it, otherwise red\blue will be used 
}

#undef IsSameModel
#define IsSameModel(x, y, z) (!Q_stricmp(x->modelName, y) && !Q_stricmp(x->skinName, z)) || \
 (!*y && !Q_stricmp(x->modelName, x->modelNameCopy) && !Q_stricmp(x->skinName, "pm"))
void CGX_SetModelAndSkin(clientInfo_t *ci, qboolean isDeferred, int clientNum) {	
	qboolean isSameTeam = qfalse;	

	// skip emtpy clientInfo 
	if (!ci->modelName[0]) {
		D_Printf(("^1Skip '%i' '%s' '%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->infoValid, ci->deferred));
		return;
	}

	//if model was restored then exit
	if (CGX_RestoreModelAndSkin(ci, clientNum, isDeferred))
		return;

	if (cg.clientNum >= 0)
		isSameTeam = cgs.gametype >= GT_TEAM && cgs.clientinfo[cg.clientNum].team == ci->team;

	if (!isSameTeam) {		
		if (IsSameModel(ci, cg.enemyModel, cg.enemySkin)) {			
			if (cgs.gametype >= GT_TEAM)
				CGX_SetColorInfo(ci, cgx_enemyColors.string);
			D_Printf(("^2OK %i\n", clientNum));
			return;
		}

		CGX_SetModel(ci, cg.enemyModel);
		CGX_SetSkin(ci, cg.enemySkin);
		CGX_SetColorInfo(ci, cgx_enemyColors.string);
	} else {
		if (IsSameModel(ci, cg.teamModel, cg.teamSkin)) {
			if (cgs.gametype >= GT_TEAM)
				CGX_SetColorInfo(ci, cgx_teamColors.string);
			D_Printf(("^2OK %i\n", clientNum));
			return;
		}

		CGX_SetModel(ci, cg.teamModel);
		CGX_SetSkin(ci, cg.teamSkin);
		CGX_SetColorInfo(ci, cgx_teamColors.string);
	}

	ci->deferred = isDeferred;
	
	D_Printf(("^2Set '%i' '%s/%s' '%s/%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy, ci->infoValid, ci->deferred));
}

// tracking changes, only after cg.snap received
void CGX_TrackEnemyModelChanges() {
	// if it's disabled skip
	if (!cgx_enemyModel_enabled.integer)
		return;

	// track client num change
	if (cg.clientNum != cg.snap->ps.clientNum) {
		cg.clientNum = cg.snap->ps.clientNum;
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;
				
		CGX_EnemyModelCheck();
		D_Printf(("^6cg.clientNum %i\n", cg.clientNum));
	} // track team change
	else if (cg.oldTeam != cgs.clientinfo[cg.clientNum].team) {
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;

		CGX_EnemyModelCheck();
		D_Printf(("^6TEAM CHANGED!\n"));
	} //track intermission change
	else if (cg.snap->ps.pm_type == PM_INTERMISSION && !cg.clientIntermission &&
		EM_Check(EM_INTERMISSION)) {
		cg.clientIntermission = qtrue;

		CGX_EnemyModelCheck();
		D_Printf(("^6PM_INTERMISSION!\n"));
	}
}

void CGX_MapRestart() {
	D_Printf(("^1CGX_MapRestart\n"));

	cg.clientIntermission = qfalse;

	// X-MOD: send modinfo
	CGX_SendModinfo();	
	CGX_EnemyModelCheck();	
	D_Printf(("^6CGX_MapRestart\n"));
}

#define NET_Set(x, y) { CG_Printf("Auto: %s %i\n", x, y); trap_Cvar_Set(x, va("%i", y)); }
void CGX_AutoAdjustNetworkSettings(void) {
	static int info_showed = 0;

	D_Printf(("CGX_AutoAdjustNetworkSettings %i\n", cgx_networkAdjustments.integer));

	if (cgx_networkAdjustments.integer) {
		int i, minRate = 5000, minSnaps, k;
		char buf[10];		
		
		//adjust sv_fps for local game
		if (cgs.localServer) {
			if (sv_fps.integer < 40)
				sv_fps.integer = 40;
			else if (sv_fps.integer > 125)
				sv_fps.integer = 125;

			NET_Set("sv_fps", sv_fps.integer)
		
			return;
		}

		i = 0;		

		if (cgx_networkAdjustments.integer == 1) {			
			minRate = 8000;
		
			// if packets < 30 set it to 30
			if (cl_maxpackets.integer < CGX_MIN_MAXPACKETS)
				i = CGX_MIN_MAXPACKETS;
			// set it litle more than sv_fps
			if (cl_maxpackets.integer < sv_fps.integer)
				i = sv_fps.integer + 10;
		} else if (cgx_networkAdjustments.integer == 2) {
			k = 2;
			minRate = 16000;			

			// if it's something lower than 100 - adjust
			if (cl_maxpackets.integer < 100)
				while ((i = com_maxfps.integer / k++) > 60);			
		} else if (cgx_networkAdjustments.integer == 3) {
			k = 1;
			minRate = 25000;			

			// if it's already 100 skip, lower - adjust
			if (cl_maxpackets.integer < 100)
				while ((i = com_maxfps.integer / k++) > 100);

			if (i >= 100)
				i-=5;
		}

		// set packets first
		if (i > 0) {
			if (i < CGX_MIN_MAXPACKETS) i = CGX_MIN_MAXPACKETS;
			else if (i > CGX_MAX_MAXPACKETS) i = CGX_MAX_MAXPACKETS;

			NET_Set("cl_maxpackets", i)
		}

		// no sense in snaps > 30 for default quake3.exe set it to sv_fps if possible, otherwise set it to 40 
		minSnaps = sv_fps.integer > 20 ? sv_fps.integer : 40;

		// check and set snaps		
		trap_Cvar_VariableStringBuffer("snaps", buf, sizeof(buf));
		i = atoi(buf);

		if (i < minSnaps)
			NET_Set("snaps", minSnaps)

		// check and set rate
		trap_Cvar_VariableStringBuffer("rate", buf, sizeof(buf));
		i = atoi(buf);

		if (i < minRate)
			NET_Set("rate", minRate)

		if (cgx_networkAdjustments.integer == 3)
			NET_Set("cl_packetdup", 0)

		// check time nudge & send hints
		// if server delaged it's better off
		/*if (cgs.delagHitscan && cl_timeNudge.integer < 0 && !info_showed) {			
			trap_Print("^5Hint: server has hitscan delag, its nice to set cl_timeNudge 0\n");
			info_showed = 1;
		}*/ 

		if (cl_timeNudge.integer < -15 && !info_showed) {
			trap_Print("^5Hint: cl_timeNudge below -15 is quite useless, set it only if you really need it and know what you are doing\n");
			info_showed = 1;
		} else if (cl_timeNudge.integer > 0 && !info_showed) {
			trap_Print("^5Hint: cl_timeNudge above 0 gives rendering delay in milliseonds, set it only if you really need it (mostly if your connection is not stable)\n");
			info_showed = 1;
		}

		if (cg_optimizePrediction.integer && cg_predictItems.integer && info_showed <= 1) {
			trap_Print("^5Hint: if you have false item pickups (picking up armor or weapon and it's doenst count) because cg_delag_optimizePrediction is set to 1 or you have high ping then try to set cg_predictitems 0\n");
			info_showed = 2;
		}		
	}
}

void CGX_SyncServer_delagHitscan(const char * info) {
	int g_delagHitscan;
	int g_delag;
	char *g_unlaggedVersion;

	//unlagged - server options
	// we'll need this for deciding whether or not to predict weapon effects
	g_delag = atoi( Info_ValueForKey( info, "g_delag" ) );
	g_delagHitscan = atoi(Info_ValueForKey(info, "g_delagHitscan"));
	g_unlaggedVersion = Info_ValueForKey(info, "g_unlaggedVersion");

	//2 - bma 3 - nemesis
	if (cgs.delagHitscan != 2 && cgs.delagHitscan != 3)
		cgs.delagHitscan = g_delag || g_delagHitscan || (Q_stricmpn("1.2", g_unlaggedVersion, 4) == 0);

	D_Printf(("cgs.delagHitscan '%i'\n", cgs.delagHitscan));
	if (g_delagHitscan || g_unlaggedVersion[0] != '\0')
		D_Printf(("g_delagHitscan '%i' g_unlaggedVersion '%s'\n", g_delagHitscan, g_unlaggedVersion));
	//unlagged - server options	

	// get sv_fps and save for unlagged	
	D_Printf(("^3g_delag '%i'\n", cgs.delagHitscan));
}

void CGX_SyncServer_sv_fps(const char *info) {
	static int old_sv_fps = -1;
	int i;

	i = atoi(Info_ValueForKey(info, "sv_fps"));

	D_Printf(("^3sv_fps serv '%i' sv_fps client '%i' ", i, sv_fps.integer));

	sv_fps.integer = i;

	if (!sv_fps.integer) {
		char buf[4];
		// get sv_fps if server sent it
		trap_Cvar_VariableStringBuffer("sv_fps", buf, sizeof(buf));
		sv_fps.integer = atoi(buf);
		//on some servs fps coming to sv_fps client value, on some stored in server info
		//try get from server info first, then from client
		if (!sv_fps.integer)
			sv_fps.integer = 20;
	}

	D_Printf(("^3sv_fps final '%i'\n", sv_fps.integer));

	if (sv_fps.integer != old_sv_fps) {
		old_sv_fps = sv_fps.integer;
		CGX_AutoAdjustNetworkSettings();		
	}
}

//check message for special commands
#define CHECK_INTERVAL	15000 //msec
void CGX_CheckChatCommand(const char *str) {
	int i;

	i = strlen(str);

	if (i > 3 && str[i - 2] == '!' && str[i - 1] == 'v') {
		int	mins, seconds, tens;
		int	msec;
		static int last_check = -CHECK_INTERVAL;

		msec = cg.time - cgs.levelStartTime;

		if (cg.time - last_check > CHECK_INTERVAL) {
			last_check = cg.time;

			seconds = msec / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			tens = seconds / 10;
			seconds -= tens * 10;

			trap_SendConsoleCommand(va("say ^7"CGX_NAME" v"CGX_VERSION" (%i:%i%i)\n", mins, tens, seconds));
		}
	} 
}

//static void CGX_Delay( int msec ) {
//	CG_Printf( "Delay for %i start...\n", msec );
//	msec += trap_Milliseconds();	
//	while (msec > trap_Milliseconds());
//	CG_Printf( "Delay end\n" );
//}

// check for unlagged enabled\disabled for bma\nms
// send modinfo in initialsnapshot and check result here
static int cgx_modinfosend = 0;
qboolean CGX_CheckModInfo(const char *str) {
	int i;
	// if 1sec passed after sending then don't check
	if (cg.time - cgx_modinfosend > 1000)
		return qtrue;

	i = strlen(str);

	if (Q_stricmp(str, "^3Unlag:           ^5ENABLED\n") == 0) {
		D_Printf(("BMA Unlagged!\n"));

		cgs.delagHitscan = 2;
		CGX_AutoAdjustNetworkSettings();
	} else if (Q_stricmp(str, "^3Unlagged compensation: ^5ENABLED\n") == 0) {
		D_Printf(("Nemesis Unlagged!\n"));

		cgs.delagHitscan = 3;
		CGX_AutoAdjustNetworkSettings();
	} else if (Q_stricmp(str, "unknown cmd modinfo\n") == 0) {
		return qfalse;
	}

	return qtrue;
}

#if CGX_NEMESIS_COMPATIBLE
// nemesis compability info
vmCvar_t	cgx_cgame;
vmCvar_t	cgx_uinfo;
#endif

// send modinfo if gamename nemesis or bma
void CGX_SendModinfo(void) {
	const char	*info;
	char	*gamename;
	qboolean isNemesis = qfalse, isBMA = qfalse;
	static qboolean isNemesisRegistered = qfalse;

	if (cg.intermissionStarted || cgs.delagHitscan == 1)
		return;

	info = CG_ConfigString( CS_SERVERINFO );
	gamename = Info_ValueForKey(info, "gamename");

	D_Printf(("gamename %s\n", gamename));

	isNemesis = !Q_stricmp(gamename, "Nemesis");
	isBMA = gamename[0] == 'B' && gamename[1] == 'M' && gamename[2] == 'A';

#if CGX_NEMESIS_COMPATIBLE
	if (isNemesis && !isNemesisRegistered) {
		//send info about clietn to nemesis servs
		trap_Cvar_Register(&cgx_cgame, "cgame", CGX_NAME" "CGX_VERSION, CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO);		
		trap_Cvar_Register(&cgx_uinfo, "cg_uinfo", "", CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO );
		trap_Cvar_Set("cg_uinfo", va("%i %i 0", cl_timeNudge.integer, cl_maxpackets.integer));

		isNemesisRegistered = qtrue;
	};
#endif	

	if (isNemesis || isBMA) {
		cgx_modinfosend = cg.time;

		trap_SendClientCommand("modinfo");

		D_Printf(("modinfo sent\n"));
	} else {
		D_Printf(("modinfo not sent\n"));
	}
}

// X-MOD: potential fix for q3config saving problem
void CGX_SaveSharedConfig(qboolean forced) {
	if (cgx_sharedConfig.integer || forced) {
		char buf[32];
		trap_Cvar_VariableStringBuffer("version", buf, 8);

		if (Q_stricmp(buf, "Q3 1.16") != 0) {
			trap_Print(va("Version %s skip shared config save\n", buf));
			return;
		}

		trap_Cvar_VariableStringBuffer("fs_game", buf, sizeof(buf));		

		if (!buf[0]) {
			trap_Print("Saving shared config... Mod: baseq3\n");
			trap_SendConsoleCommand("writeconfig q3config.cfg\n");
		}
		else {
			trap_Print(va("Saving shared config... Mod: %s\n", buf));
			trap_SendConsoleCommand("writeconfig ..\\baseq3\\q3config.cfg\n");
		}
	}
	else {
		trap_Print("Shared config saving is disabled (cg_sharedConfig)\n");
	}
}

// generate script to open url to worldspawn to download map
void CGX_GenerateMapBat(void) {
	fileHandle_t f;

	trap_Print("Generating "CGX_MAPBAT"...");

	trap_FS_FOpenFile("..\\"CGX_MAPBAT, &f, FS_WRITE);

	if (f) {
		char *buf;		

		/*buf = va("echo \"Download .pk3 file and put in baseq3 game folder\"\r\nexplorer \""CGX_MAPURL"%s/\"\r\npause",
		cgs.mapname_clean);		*/
		buf = va("explorer \""CGX_MAPURL"%s/\"", cgs.mapname_clean);
		trap_FS_Write(buf, strlen(buf), f);

		trap_FS_FCloseFile(f);
	}
	else {
		trap_Print("WARNING: Couldn't open a file "CGX_MAPBAT);
	}
}

//save picmip value
void CGX_SavePicmip() {
	if (cgx_nomip.integer) {
		trap_Cvar_Set("cgx_r_picmip", r_picmip.string);
		trap_Cvar_Update(&cgx_r_picmip);
	}
}

void CGX_NomipStart() {
	if (cgx_nomip.integer && r_picmip.integer)
		trap_Cvar_Set("r_picmip", "0");
}

void CGX_NomipEnd() {
	if (cgx_nomip.integer && r_picmip.integer)
		trap_Cvar_Set("r_picmip", cgx_r_picmip.string);	
}

void CGX_IncreaseHunkmegs(int min) {
	char buf[8];
	trap_Cvar_VariableStringBuffer("com_hunkMegs", buf, sizeof(buf));

	if (min > atoi(buf))
		trap_Cvar_Set("com_hunkMegs", va("%i", min));
}

// load collision map with last error
void CGX_LoadCollisionMap() {	
	D_Printf(("CGX_LoadCollisionMap\n"));
	CG_LoadingString( "collision map" );

	trap_Cvar_Set("cgx_last_error", va("1 Couldn't load map: %s", cgs.mapname_clean));
	trap_CM_LoadMap( cgs.mapname );
	trap_Cvar_Set("cgx_last_error", "");	
}

//cheks saved map string
static qboolean CGX_IsRememberedMap() {
	char buf[MAX_CVAR_VALUE_STRING];
	char *s, *t;

	trap_Cvar_VariableStringBuffer("cl_fixedmaps", buf, sizeof(buf));

	for (t = s = buf; *t; t = s ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t && !Q_stricmp(t, cgs.mapname_clean)) {			
			return qtrue;
		}
	}	

	return qfalse;
}

//saves mapname to memory
static void CGX_RememberBrokenMap() {
	char buf[MAX_CVAR_VALUE_STRING];
	int i;

	if (CGX_IsRememberedMap())
		return;	

	trap_Cvar_VariableStringBuffer("cl_fixedmaps", buf, sizeof(buf));

	i = strlen(buf);
	Com_sprintf(buf + i, sizeof(buf) - i, "%s ", cgs.mapname_clean);

	trap_Cvar_Set("cl_fixedmaps", buf);
}

qboolean CGX_IsPure() {
	qboolean isPure;
	char buf[4];

	trap_Cvar_VariableStringBuffer("sv_pure", buf, sizeof(buf));
	isPure = atoi(buf);
	if (cgs.localServer && isPure && cgx_networkAdjustments.integer) {
		trap_Cvar_Set("sv_pure", "0");
		return isPure;
	} else {
		return cgs.isPureServer;
	}
}

#define IsQ3Map(x) (x[0] == 'q' || x[0] == 'Q') && x[1] == '3'
//save mapname and try load aganin with fix
void CGX_TryLoadingFix() {
	if (IsQ3Map(cgs.mapname_clean))
		return;	
	
	CGX_RememberBrokenMap();

	if (cgs.localServer)
		trap_SendConsoleCommand("vid_restart\n");
	else
		trap_SendConsoleCommand("reconnect\n");	
}

// check known maps and apply loading fix if needed
static void CGX_CheckKnownMapsForFix() {
	if (!IsQ3Map(cgs.mapname_clean) && CGX_IsRememberedMap())
		trap_Cvar_Set("cgx_fix_mapload", "1");
	else
		trap_Cvar_Set("cgx_fix_mapload", "0");

	trap_Cvar_Update(&cgx_maploadingfix);
}

// load world map with last error
void CGX_LoadWorldMap() {
	D_Printf(( "^5CGX_LoadWorldMap\n" ));
	CG_LoadingString(cgs.mapname);
	trap_Cvar_Set( "cgx_last_error", va( "2 Couldn't load world map: %s", cgs.mapname_clean ) );	

	//check if need fix
	CGX_CheckKnownMapsForFix();

	//trying to apply fix only if vertexlight is off	
	if (cgx_maploadingfix.integer && !r_vertexLight.integer) {
		// load with fix
		trap_Cvar_Set("r_vertexLight", "1");
		trap_R_LoadWorldMap(cgs.mapname);
		trap_Cvar_Set("r_vertexLight", "0");
	} else {
		// default load
		trap_R_LoadWorldMap(cgs.mapname);
	}
	trap_Cvar_Set( "cgx_last_error", "" );
}

//fix enemymodels with vertex light
//check vertex light and load client info
void CGX_LoadClientInfo( clientInfo_t *ci ) {
	CGX_NomipStart();	

	if (r_vertexLight.integer) {
		trap_Cvar_Set("r_vertexLight", "0");
		CG_LoadClientInfo(ci);
		trap_Cvar_Set("r_vertexLight", "1");
	} else {
		CG_LoadClientInfo(ci);
	}

	CGX_NomipEnd();
}

//safety open file
static int CGX_FOpenFile(char *filename, fileHandle_t *f, fsMode_t mode, int maxSize) {
	int len;
	len = trap_FS_FOpenFile( filename, f, mode );

	if ( len <= 0 ) {
		trap_Print( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return 0;
	}
	if ( len >= maxSize && maxSize != -1) {
		trap_Print( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, maxSize ) );
		trap_FS_FCloseFile( *f );
		return 0;
	}

	return len;
}

//read all and close
static int CGX_FReadAll(char *filename, char *buffer, int bufsize) {
	int len;
	fileHandle_t	f;

	if (len = CGX_FOpenFile(filename, &f, FS_READ, bufsize)) {
		trap_FS_Read(buffer, len, f);
		trap_FS_FCloseFile(f);
		buffer[len] = 0;
	}

	return len;
}

//small talk
static char* CGX_XmodTalk(char *command) {
	int i;
	i = strlen(command);
	command[i] = ' ';

	if (stristr(command, "fuck") || stristr(command, "suck") || stristr(command, "shit")) {
		return command;
	} else if (stristr(command, "hi ") || stristr(command, "hello")) {
		char *txt[] = { "hi! how are you?", "hello!", "hey" };
		return txt[rand() % ArrLen(txt)];
	} else if (stristr(command, "fine") || stristr(command, "good") || stristr(command, "ok ") || stristr(command, "awesome") || stristr(command, "great")) {
		char *txt[] = { "good", "great", "okay", "awesome" };
		return rand() % 1000 > 500 ? txt[rand() % ArrLen(txt)] : command;
	} else if (stristr(command, "you") || stristr(command, " u ") || !Q_stricmpn(command, "u ", 2)) {
		char *txt[] = { "I'm just a program", "and you?", "I'm sequience of 0 and 1", "fine" };
		return txt[rand() % ArrLen(txt)];
	} else if (stristr(command, "bye ") || stristr(command, "bb ")) {
		char *txt[] = { "bb", "see you", "bye" };
		return txt[rand() % ArrLen(txt)];
	} else if (stristr(command, "?") || stristr(command, "what") || stristr(command, "where") || stristr(command, "when") || 
		stristr(command, "how") || stristr(command, "who") || stristr(command, "which") || stristr(command, "why")) {
		char *txt[] = { "I don't know", "how do I know", "idk", "the answer is... "CGX_BP_STRING };
		return txt[rand() % ArrLen(txt)];
	}

	command[i] = 0;

	return NULL;
}

static void CGX_PrintLine(char c) {
	int i;	
	CG_Printf("^%c", c);
	for(i = 0; i < 20*3/10; i++)
		CG_Printf("^%c------------", c);
	trap_Print("\n");
}

//parse info from file
//format: cmd1 - description1\r\n
static void CGX_ShowHelp(char *filename, char *cmd) {
	char			buf[1024 * 4];
	static			qboolean exampleShown;

	//start parse command list if read succesful
	if (CGX_FReadAll(filename, buf, sizeof(buf))) {
		int i, j;
		char *s, *t;

		//shift \r\n+two tabs
		for (i = 1; i < sizeof(buf) - 1; i++) {			
			if (buf[i] == '\t' && buf[i - 1] == '\t') {
				buf[i] = buf[i - 1] = buf[i - 2] = buf[i - 3] = ' ';
				for (j = i - 4; buf[j] != 0; j++)
					buf[j] = buf[j + 4];
			}
		}

		//if no command show all 
		if (!cmd[0]) {
			//find first command in file
			s = strchr(buf, 'c');
			XMOD_ANSWER("known command list");
			CGX_PrintLine(COLOR_YELLOW);
			for (t = s; *t; t = s) {
				if (!(s = strchr(s, ' ')))
					break;
				*s++ = 0;
				//print found info
				CG_Printf("%25s", t);
				if (!(s = strchr(s, '\n')))
					break;		
				s++;
			}			
			trap_Print("\n");
			CGX_PrintLine(COLOR_YELLOW);
			XMOD_ANSWER("for detailed info: \\xmod <command>");
			//show example
			if (!exampleShown) {
				XMOD_ANSWER("example: \\xmod cg_enemy"); 
				XMOD_ANSWER("'Page Up','Page Down' - scroll 'Tab' - autocomplete\n'Up','Down' - input history");
				exampleShown = qtrue; //19 - percent
			}
		} else {// find info abt command						
			for (t = buf, i = 0; *t; t++) {
				t = stristr(t, cmd);				
				if (*(t - 1) == '\n') {			
					s = strchr(t, '-');
					//if '-' in text find next
					if (*(s + 1) != ' ' || *(s - 1) != ' ')
						s = strchr(s + 1, '-');
					//if '-' found
					if (s)		
					// if it's colorlist then paint numbers
					if (!Q_stricmpn(t, "colorlist", 8)) {						
						for (t = s; *t && *t != '\r'; t++)
							if (*t >= '0' && *t <= '7') {
								*(t - 2) = ' ';//replace ','
								*(t - 1) = Q_COLOR_ESCAPE;//replace ' '
								*(t + 1) = *t++;//replace next ' ' with num
							}
						t = s + 1;
					} else { //otherwise just put white color
						*(s) = COLOR_WHITE;
						*(s - 1) = '^';
					}	
					//make end line
					if (s = strchr(s, '\r'))
						*s = 0;
					//print found info
					XMOD_ANSWER(va("^3%s", t));
					i++;
					t = s;
				}				
			}
			
			//zero matches
			if (!i) {
				if (s = CGX_XmodTalk(cmd))
					XMOD_ANSWER(s)
				else
					XMOD_ANSWER(va("unknown cmd '%s'", cmd))
			}
		} 		
	}
}

//xmod command
void CGX_Xmod(char *command) {
	int i;

	if (!Q_stricmp(command, "e ")) {
		XMOD_ANSWER("checking enemy models...");
		CGX_EnemyModelCheck();
		return;
	} 

	i = strlen(command);
	if (i && i < 3) {
		XMOD_ANSWER("too short cmd");		
		return;
	}
	//remove emtpy space
	command[i - 1] = 0;

	if (!Q_stricmp(command, "version")) {
		XMOD_ANSWER(cgx_version.string);
	} else if (!Q_stricmp(command, "help")) {
		CGX_ShowHelp("doc\\2-comand_list.txt", "");
	} else if (stristr(command, "8ball")) {
		char *balls[] = {
			"listen to your heart",
			"listen to your intuition",
			"trust your hunches",
			"follow your instincts",
			"listen to your feelings",
		};
		XMOD_ANSWER(balls[rand() % 5]);
	} else if (!Q_stricmp(command, "coin")) {
		XMOD_ANSWER(rand() % 100 >= 50 ? "true": "false");
	} else {
		CGX_ShowHelp("doc\\2-comand_list.txt", command);
	}
}

//clears sv_hostname from ^. and returns color if ^^.. was used
char CGX_ServerNameFixInfoLoad(char *str) {
	char* c;
	char res;
	//clean with leaving color
	QX_CleanStrHostnameFix(str);	

	//check if color left
	for (c = str; *c; c++)
		if (*c == Q_COLOR_ESCAPE) {
			//return the color and clean string completly
			res = *(c + 1);
			Q_CleanStr(str);			
			return res;
		}

	return COLOR_WHITE;	
}


