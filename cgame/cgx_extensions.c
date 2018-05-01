#include "cg_local.h"

#define ShaderRGBAFill(a,c)	((a)[0]=(c),(a)[1]=(c),(a)[2]=(c),(a)[3]=(255))
#define CGX_IsPMSkin(p) ( p && *(p) == 'p' && *((p)+1) && *((p)+1) == 'm' )

#define DARKEN_COLOR 64

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

void trap_DPrint(const char *str) {
	if (cgx_debug.integer)
	trap_Print(va ("^5DEBUG: %s", str) );
}

void trap_WPrint(const char *str) {
	if (cgx_debug.integer)
	trap_Print(va ("^3WARNING: %s", str) );
}

void trap_RPrint(const char *str) {
	if (cgx_debug.integer)
	trap_Print(va ("^6REASON: %s", str) );
}

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

	trap_DPrint(va("CGX_Init_vScreen %fx%f cgx_wideScreenFix %d\n", vScreen.width, SCREEN_HEIGHT, cgx_wideScreenFix.integer));	
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
		Q_strncpyz(cg.enemySkin, "default", sizeof(cg.enemySkin));
	} else {		
		Q_strncpyz(cg.enemySkin, slash + 1, sizeof(cg.enemySkin));
		// truncate modelName
		*slash = 0;
	}

	Q_strncpyz(cg.enemyModel, modelStr, sizeof(cg.enemyModel));	

	trap_DPrint(va("CGX_Init_enemyModels cg.enemyModel: %s cg.enemySkin: %s\n", cg.enemyModel, cg.enemySkin));
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
		Q_strncpyz(cg.teamSkin, "default", sizeof(cg.teamSkin));
	} else {		
		Q_strncpyz(cg.teamSkin, slash + 1, sizeof(cg.teamSkin));
		// truncate modelName
		*slash = 0;
	}

	Q_strncpyz(cg.teamModel, modelStr, sizeof(cg.teamModel));	

	trap_DPrint(va("CGX_Init_teamModels cg.teamModel: %s cg.teamSkin: %s\n", cg.teamModel, cg.teamSkin));
}

void CGX_EnemyModelCheck(void) {
	int		i;
	clientInfo_t	*ci;

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	trap_DPrint("CGX_EnemyModelCheck\n");		

	//change models and skins if needed or restore
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ )
		if (cg.clientNum != i && ci->infoValid)
			CGX_SetModelAndSkin(ci, qtrue, i);																		
}

static void CGX_ColorFromChar(char v, byte *color, clientInfo_t *info) {
	int j;

	if (v == '?')
		switch (info->team) {
			case TEAM_RED:  v = '1'; break;
			case TEAM_BLUE: v = '4'; break;
			case TEAM_FREE: v = '2'; break;
			default: v = '7'; break;
		} 
	else if (v == '!')
		switch (info->team) {
			case TEAM_RED:  v = '1'; break;
			case TEAM_BLUE: v = '4'; break;
			case TEAM_FREE: v = '0' + (random() * 7); break;
			default: v = '7'; break;
		} 
	else if (v == '*')
		v = '0' + (random() * 7);
	else if (v == '.')
		v = '0' + (random() * 35);

	j = (v - '0') % 36;//  for g_color_table_ex
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

static void CGX_SetColorInfo(const char *color, clientInfo_t *info) {	
	int i;	

	if (cg.clientNum == -1) {
		trap_DPrint("CGX_SetColorInfo skip -1\n");
		return;
	}

	if (info->skinName[0] == '\0')
		return;

	if (color[0] == '\0')
		color = "!???";

	trap_DPrint(va("CGX_SetColorInfo %s\n", color));

	for (i = 0; i < 4; i++) {
		if (!color[i])
			return;

		CGX_ColorFromChar(color[i], info->colors[i], info);
		//trap_DPrint(va("%3i %3i %3i\n", info->colors[i][0], info->colors[i][1], info->colors[i][2]));		

		if (cgx_deadBodyDarken.integer)
			ShaderRGBAFill(info->darkenColors[i], CGX_RGBToGray(info->colors[i]));
		else
			ShaderRGBACopy(info->colors[i], info->darkenColors[i]);		
	}	

	// copy rail color
	if (color[0] == '!') {
		VectorCopy(info->colorCopy, info->color);
	}
	else {
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
			CGX_SetColorInfo(cgx_enemyColors.string, ci);
		else
			CGX_SetColorInfo(cgx_teamColors.string, ci);
}

void CGX_RestoreModelNameFromCopy(clientInfo_t *ci) {
	if (Q_stricmp(ci->modelName, ci->modelNameCopy) == 0)
		return;

	Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));	
	trap_DPrint(va("%s -> %s\n", ci->modelName, ci->modelNameCopy));	
}

void CGX_RestoreSkinNameFromCopy(clientInfo_t *ci) {
	if (Q_stricmp(ci->skinName, ci->skinNameCopy) == 0)
		return;

	Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));
	trap_DPrint(va("%s -> %s\n", ci->skinName, ci->skinNameCopy));	
}

void CGX_RestoreModelAndSkin(clientInfo_t *ci) {
	//skip emtpy models
	if (ci->modelName[0] == '\0')
		return;

	trap_DPrint("CGX_RestoreModelAndSkin\n");

	CGX_RestoreModelNameFromCopy(ci);
	CGX_RestoreSkinNameFromCopy(ci);	
}

void CGX_SaveModelAndSkinCopy(clientInfo_t *ci, char *enemyOrTeamModelName) {	
	if (ci->modelNameCopy[0] == '\0')
		Q_strncpyz(ci->modelNameCopy, ci->modelName, sizeof(ci->modelName));
	if (ci->skinNameCopy[0] == '\0')
		Q_strncpyz(ci->skinNameCopy, ci->skinName, sizeof(ci->skinName));

	trap_DPrint(va("%s -> %s\n", ci->modelName, enemyOrTeamModelName));	

	Q_strncpyz(ci->modelName, enemyOrTeamModelName, sizeof(ci->modelName));
}

static qboolean CGX_IsKnownModel(const char *modelName) {
	int i;
	for (i = 0; i < 23; i++)
		if (Q_stricmp(modelName, known_models[i]) == 0)
			return qtrue;	

	return qfalse;	
}

void CGX_SetPMSkin(clientInfo_t *ci) {
	if (!CGX_IsKnownModel(ci->modelName)) {
		trap_Print(va("Warning: No PM skin for model %s\n", ci->modelName));		
		//set sarge/pm		
		Q_strncpyz(ci->modelName, DEFAULT_MODEL, sizeof(ci->modelName));
	} else if (Q_stricmp(ci->skinName, "pm") == 0) {
		trap_DPrint(va("PM skin already set %s\n", ci->modelName));	
		return;
	}		

	trap_DPrint(va("Setting PM skin %s\n", ci->modelName));
	Q_strncpyz(ci->skinName, "pm", sizeof(ci->skinName));		
}

void CGX_SetModelAndSkin(clientInfo_t *ci, qboolean isDeferred, int clientNum) {	
	qboolean isSameTeam = qfalse;	

	// skip emtpy clientInfo 
	if (ci->modelName[0] == '\0') {
		//CG_Printf("Skip '%i' '%s' '%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->infoValid, ci->deferred);
		return;
	}

	if (!cgx_enemyModel_enabled.integer) {
		//if it's disabled maybe we need to restore models
		CGX_RestoreModelAndSkin(ci);		
		ci->deferred = isDeferred;
		return;
	}

	// some additional checks after config string modified, it calls CG_NewClientInfo again
	if (cg.clientNum != -1) {
		qboolean isSpect = cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR || ci->team == TEAM_SPECTATOR;
		isSameTeam = cgs.gametype >= GT_TEAM && cgs.clientinfo[cg.clientNum].team == ci->team;

		if (isSpect) {
			CGX_RestoreModelAndSkin(ci);
			ci->deferred = isDeferred;		
			//CG_Printf("Restore '%i' '%s' '%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->infoValid, ci->deferred);
			return;
		}
	}			

	ci->deferred = isDeferred;	

	//CG_Printf("Set '%i' '%s' '%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->infoValid, ci->deferred);

	if (!isSameTeam) {
		// if enemymodels enabled and enemy model not specified, load saved real model name and set pm skin
		if (cg.enemyModel[0] == '\0') {
			CGX_RestoreModelNameFromCopy(ci);
			CGX_SetPMSkin(ci);
			CGX_SetColorInfo(cgx_enemyColors.string, ci);
			return;
		}

		// save model name and skin copy
		CGX_SaveModelAndSkinCopy(ci, cg.enemyModel);

		// if skin not specified set pm
		if (cg.enemySkin[0] == '\0')
			CGX_SetPMSkin(ci);

		// if gametype is not team\ctf or skin pm set it, otherwise red\blue will be used
		if (cgs.gametype < GT_TEAM || Q_stricmp(cg.enemySkin, "pm") == 0)
			Q_strncpyz(ci->skinName, cg.enemySkin, sizeof(ci->skinName));

		// if skin is pm set colors
		if (Q_stricmp(ci->skinName, "pm") == 0)
			CGX_SetColorInfo(cgx_enemyColors.string, ci);
	} else {
		//copy pasteeeee

		// if teammodels enabled and team model not specified, load saved real model name and set pm skin
		if (cg.teamModel[0] == '\0') {
			CGX_RestoreModelNameFromCopy(ci);
			CGX_SetPMSkin(ci);
			CGX_SetColorInfo(cgx_teamColors.string, ci);
			return;
		}

		// save model name and skin copy
		CGX_SaveModelAndSkinCopy(ci, cg.teamModel);

		// if skin not specified set pm
		if (cg.teamSkin[0] == '\0')
			CGX_SetPMSkin(ci);

		// if gametype is not team\ctf or skin pm set it, otherwise red\blue will be used
		if (cgs.gametype < GT_TEAM || Q_stricmp(cg.teamSkin, "pm") == 0)
			Q_strncpyz(ci->skinName, cg.teamSkin, sizeof(ci->skinName));

		// if skin is pm set colors
		if (Q_stricmp(ci->skinName, "pm") == 0)
			CGX_SetColorInfo(cgx_teamColors.string, ci);
	}
}

void CGX_AutoAdjustNetworkSettings(void) {
	static int info_showed = 0;

	trap_DPrint(va("CGX_AutoAdjustNetworkSettings %i\n", cgx_networkAdjustments.integer));

	if (cgx_networkAdjustments.integer && !cgs.localServer) {
		int i, minRate, minSnaps, k;
		char buf[10];		
		
		i = 0;		

		if (cgx_networkAdjustments.integer == 1) {			
			minRate = 8000;			

			// if packets < 30 set it to 30
			if (cgx_maxpackets.integer < CGX_MIN_MAXPACKETS)
				i = CGX_MIN_MAXPACKETS;
		} else if (cgx_networkAdjustments.integer == 2) {
			k = 2;
			minRate = 16000;			

			// if it's something lower than 100 - adjust
			if (cgx_maxpackets.integer < 100)
				while ((i = cgx_maxfps.integer / k++) > 60);			
		} else if (cgx_networkAdjustments.integer == 3) {
			k = 1;
			minRate = 25000;			

			// if it's already 100 skip, lower - adjust
			if (cgx_maxpackets.integer < 100)
				while ((i = cgx_maxfps.integer / k++) > 100);

			if (i >= 100)
				i--;
		}

		// set packets first
		if (i > 0) {
			if (i < CGX_MIN_MAXPACKETS) i = CGX_MIN_MAXPACKETS;
			else if (i > CGX_MAX_MAXPACKETS) i = CGX_MAX_MAXPACKETS;

			trap_Cvar_Set("cl_maxpackets", va("%i", i));
			trap_Print(va("Auto: cl_maxpackets %i\n", i));
		}

		// no sense in snaps > 30 for default quake3.exe set it to sv_fps if possible, otherwise set it to 40 
		minSnaps = cgs.minSnaps;

		// check and set snaps		
		trap_Cvar_VariableStringBuffer("snaps", buf, sizeof(buf));
		i = atoi(buf);

		if (i < minSnaps) {
			trap_Cvar_Set("snaps", va("%i", minSnaps));
			trap_Print(va("Auto: snaps %i\n", minSnaps));
		};

		// check and set rate
		trap_Cvar_VariableStringBuffer("rate", buf, sizeof(buf));
		i = atoi(buf);

		if (i < minRate) {
			trap_Cvar_Set("rate", va("%i", minRate));
			trap_Print(va("Auto: rate %i\n", minRate));
		}

		if (cgx_networkAdjustments.integer == 3) {
			// check and off packetdup
			trap_Cvar_VariableStringBuffer("cl_packetdup", buf, sizeof(buf));
			i = atoi(buf);

			if (i) {
				trap_Cvar_Set("cl_packetdup", "0");
				trap_Print("Auto: cl_packetdup 0\n");
			}
		}

		// check time nudge & send hints
		// if server delaged it's better off
		if (cgs.delagHitscan && cl_timeNudge.integer < 0 && !info_showed) {			
			trap_Print("^5Hint: server has hitscan delag, its nice to set cl_timeNudge 0\n");
			info_showed = 1;
		} 

		if (cg_optimizePrediction.integer && cg_predictItems.integer && info_showed <= 1) {			
			trap_Print("^5Hint: if you have false item pickups (picking up armor or weapon and it's doenst count) because cg_delag_optimizePrediction is set to 1 or you have high ping then try to set cg_predictitems 0\n");
			info_showed = 2;
		}				
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
		trap_DPrint("BMA Unlagged!\n");

		cgs.delagHitscan = 2;
		CGX_AutoAdjustNetworkSettings();
	} else if (Q_stricmp(str, "^3Unlagged compensation: ^5ENABLED\n") == 0) {
		trap_DPrint("Nemesis Unlagged!\n");

		cgs.delagHitscan = 3;
		CGX_AutoAdjustNetworkSettings();
	} else if (Q_stricmp(str, "unknown cmd modinfo\n") == 0) {
		return qfalse;
	}

	return qtrue;
}

// send modinfo if gamename nemesis or bma
void CGX_SendModinfo(void) {
	const char	*info;
	char	*gamename;

	if (cg.intermissionStarted || cgs.delagHitscan == 1)
		return;

	info = CG_ConfigString( CS_SERVERINFO );
	gamename = Info_ValueForKey(info, "gamename");

	trap_DPrint(va("gamename %s\n", gamename));

	if (Q_stricmp(gamename, "Nemesis") == 0 ||
		(gamename[0] == 'B' && gamename[1] == 'M' && gamename[2] == 'A')) {

		cgx_modinfosend = cg.time;

		trap_SendClientCommand("modinfo");

		trap_DPrint("modinfo sent\n");
	} else {
		trap_DPrint("modinfo not sent\n");
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

		if (buf[0] == '\0') {
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

static void CGX_IncreaseHunkmegs(int min) {
	char buf[8];
	trap_Cvar_VariableStringBuffer("com_hunkMegs", buf, sizeof(buf));

	if (min > atoi(buf))
		trap_Cvar_Set("com_hunkMegs", va("%i", min));
}

qboolean CGX_IsVertexLight() {
	char buf[4];
	static int vertexLight = -1;
	if (vertexLight == -1) {
		trap_Cvar_VariableStringBuffer("r_vertexLight", buf, sizeof(buf));
		vertexLight = atoi(buf);
	}
	return vertexLight;
}

// load collision map with last error
static void CGX_LoadCollisionMap() {	
	trap_DPrint("CGX_LoadCollisionMap\n");
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
		if (*t && !Q_stricmp(t, cgs.mapname_clean))
			return qtrue;		
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

//save mapname and try load aganin with fix
static void CGX_TryLoadingFix() {
	CGX_RememberBrokenMap();	

	if (cgs.localServer)
		trap_SendConsoleCommand("vid_restart\n");
	else 
		trap_SendConsoleCommand("reconnect\n");
}

// check known maps and apply loading fix if needed
static void CGX_CheckKnownMapsForFix() {
	char buf[MAX_INFO_STRING];
	char *s;

	if (cgs.mapname_clean[0] != 'q' &&
		cgs.mapname_clean[1] != '3' &&
		CGX_IsRememberedMap()) {		
		trap_Cvar_Set("cgx_fix_mapload", "1");
	} else {
		trap_Cvar_Set("cgx_fix_mapload", "0");
	}

	trap_Cvar_Update(&cgx_maploadingfix);
}

// load world map with last error
static void CGX_LoadWorldMap() {
	trap_DPrint( "CGX_LoadWorldMap\n" );
	CG_LoadingString(cgs.mapname);
	trap_Cvar_Set( "cgx_last_error", va( "2 Couldn't load world map: %s", cgs.mapname_clean ) );	

	//check if need fix
	CGX_CheckKnownMapsForFix();

	//trying to apply fix only if vertexlight is off	
	if (cgx_maploadingfix.integer && !CGX_IsVertexLight()) {
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
static void CGX_LoadClientInfo( clientInfo_t *ci ) {
	if (CGX_IsVertexLight()) {
		trap_Cvar_Set("r_vertexLight", "0");
		CG_LoadClientInfo(ci);		// if low on memory, some clients will be deferred
		trap_Cvar_Set("r_vertexLight", "1");
	} else {
		CG_LoadClientInfo(ci);		// if low on memory, some clients will be deferred
	}
}
