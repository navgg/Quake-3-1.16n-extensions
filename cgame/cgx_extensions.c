// Copyright (C) 2018 NaViGaToR (322)

#include "cg_local.h"

#define XMOD_ANSWER(x) { CG_Printf("^7[^1xmod^7]: ^6%s\n", x); trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND ); }

#define DARKEN_COLOR 64
#define EM_SPECT			2
#define EM_INTERMISSION		4

#define trap_Cvar_Get(name,v) trap_Cvar_VariableStringBuffer(name, v, sizeof v)

#define dl_tempname	"{X-Mod}download.tmp"

//bright skins
#define S_PM		"pm"
#define S_BRIGHT	"bright"

//bright skin shader
#define SS_BRIGHT	"xm_bright"

//file i/o
static int CGX_FCopy(char *filename, char *dest);
static qboolean CGX_FExists(char* filename);
static int CGX_FSize(char *filename);
static int CGX_FOpenFile(char *filename, fileHandle_t *f, fsMode_t mode, int maxSize);
static int CGX_FReadAll(char *filename, char *buffer, int bufsize);
static qboolean CGX_FWriteAll(char *filename, char *buffer, int bufsize);

static void CGX_Delay( int msec ) {
	CG_Printf( "Delay for %i start...\n", msec );
	msec += trap_Milliseconds();	
	while (msec > trap_Milliseconds());
	CG_Printf( "Delay end\n" );
}

#define XM_SMALLCHAR_WIDTH  6
#define XM_BIGCHAR_WIDTH	12
#define XM_GIANT_WIDTH		24
#define	XM_CHAR_WIDTH		28
#define XM_ICON_SIZE		36

//init hud constant coords
static void CGX_Init_HUD(void) {
	hud.sbammox = CHAR_WIDTH * 3 + TEXT_ICON_SPACE + vScreen.offsetx;
	hud.sbheadx = 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + vScreen.offsetx;
	hud.sbflagx = 185 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + ICON_SIZE + vScreen.offsetx;
	hud.sbarmorx = 370 + CHAR_WIDTH * 3 + TEXT_ICON_SPACE + vScreen.offsetx;
	hud.sbammo_tx = vScreen.offsetx;
	hud.sbhealth_tx = 185 + vScreen.offsetx;
	hud.sbarmor_tx = 370 + vScreen.offsetx;

	hud.width48 = vScreen.width - 48;
	hud.width5 = vScreen.width - 5;

	if (cg_draw2D.integer == HUD_COMPACT) {
		hud.icon_size = XM_ICON_SIZE;
		hud.small_char_w = XM_SMALLCHAR_WIDTH;
		hud.big_char_w = XM_BIGCHAR_WIDTH;
		hud.giant_char_w = XM_GIANT_WIDTH;
		hud.weap_icon_s = 24;
		hud.weap_icon_s2 = 30;
		hud.weap_icon_sub = 3;
		hud.weap_text_y = 16;
//		hud.weap_y = 405;
		hud.char_width = XM_CHAR_WIDTH;
		hud.head_size = XM_ICON_SIZE;

		hud.cofs = ICON_SIZE - XM_ICON_SIZE;
		hud.sbammo_tx += hud.cofs;
		hud.sbarmor_tx += hud.cofs;
		hud.sbhealth_tx += hud.cofs;
		hud.sbflagx -= hud.cofs;

		hud.minshadow = 12;
		hud.lagometer_fw = 4;
		hud.lagometer_fh = 9;

		hud.score_yofs = BIGCHAR_HEIGHT + 4;
		hud.score_yofs_no_lagometer = ICON_SIZE / 2;
	} else {
		hud.icon_size = ICON_SIZE;
		hud.small_char_w = SMALLCHAR_WIDTH;
		hud.big_char_w = BIGCHAR_WIDTH;
		hud.giant_char_w = GIANT_WIDTH;
		hud.weap_icon_s = 32;
		hud.weap_icon_s2 = 40;
		hud.weap_icon_sub = 4;
		hud.weap_text_y = 22;
		//hud.weap_y = 380;
		hud.char_width = CHAR_WIDTH;
		hud.head_size = ICON_SIZE;

		hud.minshadow = 8;
		hud.lagometer_fw = 5;
		hud.lagometer_fh = 10;

		hud.score_yofs = BIGCHAR_HEIGHT + 8;
		hud.score_yofs_no_lagometer = ICON_SIZE / 2 + 4;
	}

	if (cg_draw2D.integer == HUD_VANILLAQ3) {
		hud.head_size *= 1.25f;
		hud.weap_y = 380;
	} else {
		int ox = 0;

		if (cg_draw2D.integer == HUD_DEFAULT) {
			ox = 7;
		} else if (cg_draw2D.integer == HUD_COMPACT) {
			ox = 14;
		}

		hud.sbarmorx += ox * 2;
		hud.sbarmor_tx += ox * 2;
		hud.sbflagx += ox;
		hud.sbhealth_tx += ox;
		hud.sbheadx += ox;

		hud.weap_y = SCREEN_HEIGHT - hud.head_size - hud.weap_icon_s2 - hud.weap_icon_sub;
	}

	hud.sby = SCREEN_HEIGHT - hud.icon_size;
	hud.sbteambg_y = SCREEN_HEIGHT - hud.head_size;

	hud.lagometer_x = vScreen.width - hud.icon_size;
	hud.lagometer_y = SCREEN_HEIGHT - hud.icon_size;
}

//init virtual screen for widescreen or 4:3
void CGX_Init_vScreen(void) {
	const float baseAspect = 0.75f; // 3/4
	float aspect;
	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	aspect = (float)cgs.glconfig.vidWidth / (float)cgs.glconfig.vidHeight;

	// X-MOD: init virtual screen sizes for wide screen fix
	
	if ( (cgx_wideScreenFix.integer & CGX_WFIX_SCREEN) && cgs.glconfig.vidWidth * SCREEN_HEIGHT > cgs.glconfig.vidHeight * SCREEN_WIDTH ) {
		vScreen.width = aspect * (float)SCREEN_HEIGHT;
		vScreen.offsetx = (vScreen.width - SCREEN_WIDTH) / 2.0f;
	} else {
		vScreen.width = SCREEN_WIDTH;						
		vScreen.offsetx = 0;
	}	

	vScreen.hwidth = vScreen.width / 2;
	vScreen.fovaspect = baseAspect * aspect;

	cgs.screenXScale = (float)cgs.glconfig.vidWidth / (float)vScreen.width;
	cgs.screenYScale = (float)cgs.glconfig.vidHeight / (float)SCREEN_HEIGHT; 

	CGX_Init_HUD();

	D_Printf(("CGX_Init_vScreen %ix%i cgx_wideScreenFix %i\n", vScreen.width, SCREEN_HEIGHT, cgx_wideScreenFix.integer));	
}

#pragma region Xmod enemy models

// instead of modes 1 2 4 6 will be 1 2 3 4
static qboolean EM_Check( int x ) {
	int i = cgx_enemyModel_enabled.integer;
	i = i == 3 ? i = 4 : i == 4 ? i = 6 : i;
	return i & x;
}

static void CGX_ParseEnemyModelSetting(char *modelDest, char *skinDest, char *cvarStr) {
	char modelStr[MAX_QPATH];
	char *slash;	

	Q_strncpyz(modelStr, cvarStr, sizeof modelStr);

	slash = strchr( modelStr, '/' );
	if ( !slash ) {
		// modelName didn not include a skin name				
		Q_strncpyz(skinDest, "", sizeof modelStr);
	} else {		
		Q_strncpyz(skinDest, slash + 1, sizeof modelStr);
		// truncate modelName
		*slash = 0;
	}

	Q_strncpyz(modelDest, modelStr, sizeof modelStr);	

	if (!*skinDest && modelDest[0] == '*')//all default
		Q_strncpyz(skinDest, "*", sizeof modelStr);
}

//init cg.enemyModel cg.teamModel & skin values
void CGX_Init_enemyModels(void) {
	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	CGX_ParseEnemyModelSetting(cg.enemyModel, cg.enemySkin, cgx_enemyModel.string);
	CGX_ParseEnemyModelSetting(cg.teamModel, cg.teamSkin, cgx_teamModel.string);
	D_Printf(("cg.enemyModel: %s cg.enemySkin: %s\n", cg.enemyModel, cg.enemySkin));
	D_Printf(("cg.teamModel: %s cg.teamSkin: %s\n", cg.teamModel, cg.teamSkin));
}

//checks enemy models of all clients and loads if needed
void CGX_CheckEnemyModelAll(void) {
	int		i;
	clientInfo_t	*ci;

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	if (cg.clientNum == -1) {
		D_Printf(("^CGX_CheckEnemyModelAll before clientNum init\n"));
		return;
	}

	D_Printf(("CGX_CheckEnemyModelAll %i\n", cg.clientNum));	

	//change models and skins if needed or restore
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ )
		if (ci->infoValid) {
			CGX_CheckEnemyModel(ci, qtrue, i);
			CGX_SetSkinColors(ci, i);
		}
	
	D_Printf(("CG_LoadDeferredPlayers\n"));

	CG_LoadDeferredPlayers();
}

//sets skin colors for all clients
void CGX_SetSkinColorsAll(void) {
	int i;
	clientInfo_t *ci;

	if (cgs.gametype == GT_SINGLE_PLAYER)		
		return;

	for (i = 0, ci = cgs.clientinfo; i < cgs.maxclients; i++, ci++)
		if (ci->infoValid)
			CGX_SetSkinColors(ci, i);
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

static void CGX_SetColorInfo(clientInfo_t *info, const char *color, int clientNum) {
	int i;

	if (!*color || !cgx_enemyModel_enabled.integer || cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR)
		color = "!!!!";
	else if (i = QX_StringToColor(color))
		color = va("%c%c%c%c", i, i, i, i);
		
	if (strlen(color) < 4) {
		i = (atoi( color ) + '0' + ArrLen( g_color_table_ex )) % 255;
		color = va( "%c%c%c%c", (char)i, (char)i, (char)i, (char)i );
	}

	D_Printf(("CGX_SetColorInfo %s\n", color));

	for (i = 0; i < 4; i++) {
		CGX_ColorFromChar(color[i], info->colors[i], info);
		//D_Printf(("%3i %3i %3i\n", info->colors[i][0], info->colors[i][1], info->colors[i][2]));		

		if (cgx_deadBodyDarken.integer)
			ShaderRGBFill(info->darkenColors[i], CGX_RGBToGray(info->colors[i]));
		else
			ShaderRGBCopy(info->colors[i], info->darkenColors[i]);		
	}	

	// copy rail color
	if (color[0] == '!' || cg.clientNum == clientNum) {
		VectorCopy(info->colorCopy, info->color);
	} else {
		for (i = 0; i < 3; i++)
			info->color[i] = (float)info->colors[0][i] / 255.0f;
	}
}

//sets skin color for client
void CGX_SetSkinColors(clientInfo_t *ci, int clientNum) {
	qboolean isSameTeam = qfalse;	

	if (cg.clientNum >= 0)
		isSameTeam = cgs.gametype >= GT_TEAM && cgs.clientinfo[cg.clientNum].team == ci->team;

	if (!isSameTeam)
		CGX_SetColorInfo(ci, cgx_enemyColors.string, clientNum);
	else
		CGX_SetColorInfo(ci, cgx_teamColors.string, clientNum );
}

//restore real model and skin if needed and return result
#define IsSameModel(x) !Q_stricmp(x->modelName, x->modelNameCopy) && !Q_stricmp(x->skinName, x->skinNameCopy)
static qboolean CGX_RestoreModelAndSkin(clientInfo_t *ci, qboolean isDeferred) {
	ci->customShader = 0; // reset custom shader here

	if (IsSameModel(ci))		
		return qfalse;

	Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));
	Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));

	Vector4Copy(ci->colorCopy, ci->color);

	ci->deferred = isDeferred;

	return qtrue;
}

static void CGX_SetModel(clientInfo_t *ci, char *modelName) {
	//if model not specified get from copy
	if (!modelName[0] || modelName[0] == '*')
		Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));		
	else
		Q_strncpyz(ci->modelName, modelName, sizeof(ci->modelName));
}

//cache skin existense, just in case, for less trap open files
typedef struct {
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	qboolean		exists;
} skinCache_t;

static int skinCacheNum;
static skinCache_t skinCache[MAX_CLIENTS*2];

static void CGX_PrintSkinCache() {
	int i; skinCache_t *sc;
	for (i = 0, sc = skinCache; i < skinCacheNum; i++, sc++)
		CG_Printf("%i %s %s %i\n", i, sc->modelName, sc->skinName, sc->exists);
}

//check skin existence
static qboolean CGX_IsSkinExists(const char *model, const char *skin) {
	int i; qboolean res; skinCache_t *sc;

	for (i = 0, sc = skinCache; i < skinCacheNum; i++, sc++)
		if (!Q_stricmp(model, sc->modelName) && !Q_stricmp(skin, sc->skinName))
			return sc->exists;

	res = CGX_FExists(va("models\\players\\%s\\head_%s.skin", model, skin));

	if (skinCacheNum < ArrLen(skinCache)) {
		Q_strncpyz(sc->modelName, model, MAX_QPATH);
		Q_strncpyz(sc->skinName, skin, MAX_QPATH);
		sc->exists = res;
		skinCacheNum++;
	} else {
		skinCacheNum = 0;
		memset(&skinCache, 0, sizeof skinCache);
	}

	return res;
}

//adds enntity to scene with shader
void CGX_AddRefEntityWithCustomShader(refEntity_t *ent, int eFlags) {
	if ( ent->customShader < 0 ) {
		//entity has additional shader, add entity with no shader first then with customShader
		int s = ent->customShader;
		//don't draw dead players
		if (!(eFlags & EF_DEAD) || !cgx_deadBodyDarken.integer) {
			ent->customShader = 0;
			trap_R_AddRefEntityToScene(ent);
		}
		ent->customShader = -s;
		trap_R_AddRefEntityToScene(ent);
	} else {
		trap_R_AddRefEntityToScene( ent );
	}
}

//sets any shader for model instead of skin, or adds shader on skin
static void CGX_SetCustomSkinShader(clientInfo_t *ci, char *shader, qboolean additionalShader) {
	//prepare skin
	Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof ci->skinName);
	if (Q_stricmp(ci->skinName, "default") && !CGX_IsSkinExists(ci->modelName, ci->skinName))
		Q_strncpyz(ci->skinName, "default", sizeof ci->skinName);

	//calling register shader like that is not good, but it will be called not often so it's fine
	if (r_vertexLight.integer) {
		if (!ci->customShader) {
			trap_Cvar_Set("r_vertexLight", "0");
			ci->customShader = trap_R_RegisterShaderNoMip(shader);
			trap_Cvar_Set("r_vertexLight", "1");
		}
	} else {
		ci->customShader = trap_R_RegisterShaderNoMip(shader);
	}

	//it will add another entity to scene with specified shader
	if (additionalShader)
		ci->customShader = -ci->customShader;
}

static void CGX_SetAnyBrightSkin(clientInfo_t *ci) {
	if (CGX_IsSkinExists(ci->modelName, S_PM)) {
		Q_strncpyz(ci->skinName, S_PM, sizeof(ci->skinName));
	} else if (CGX_IsSkinExists(ci->modelName, S_BRIGHT)) {
		Q_strncpyz(ci->skinName, S_BRIGHT, sizeof(ci->skinName));
	} else {
		CGX_SetCustomSkinShader(ci, SS_BRIGHT, qtrue);
	}
}

static void CGX_SetSkin(clientInfo_t *ci, char *skinName) {
	ci->customShader = 0;

	if (!skinName[0]) {
		CGX_SetAnyBrightSkin(ci);
	} else if (skinName[0] == '*') {
		Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));
	} else if (skinName[0] == '!') {
		CGX_SetCustomSkinShader(ci, &skinName[1], qfalse);
	} else if (skinName[0] == '+') {
		CGX_SetCustomSkinShader(ci, &skinName[1], qtrue);
	} else if (!Q_stricmp(skinName, "fb") && !CGX_IsSkinExists(ci->modelName, skinName)) {
		CGX_SetCustomSkinShader(ci, SS_BRIGHT, qtrue);
	} else if (!Q_stricmp(skinName, "fb2") && !CGX_IsSkinExists(ci->modelName, skinName)) {
		CGX_SetCustomSkinShader(ci, "xm_fb2", qfalse);
	} else if (!Q_stricmp(skinName, "fb3") && !CGX_IsSkinExists(ci->modelName, skinName)) {
		CGX_SetCustomSkinShader(ci, "xm_fb3", qfalse);
	} else if (CGX_IsSkinExists(ci->modelName, skinName)) {
		Q_strncpyz(ci->skinName, skinName, sizeof(ci->skinName)); //set whatever specified if skin exists
	} else if (CGX_IsSkinExists(ci->modelName, ci->skinNameCopy)) {
		Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName)); //set copied backup
	} else {
		//set something
		if (cgs.gametype >= GT_TEAM) {
			if (ci->team == TEAM_BLUE)
				Q_strncpyz(ci->skinName, "blue", sizeof(ci->skinName));
			else
				Q_strncpyz(ci->skinName, "red", sizeof(ci->skinName));
		} else {
			Q_strncpyz(ci->skinName, "default", sizeof(ci->skinName));
		}

		D_Printf(("^3'%s' skin not found for model '%s'. Using '%s' skin\n", skinName, ci->modelName, ci->skinName));
	}
}

#define IsSameModel2(x, y, z) (!Q_stricmp(x->modelName, y) && !Q_stricmp(x->skinName, z)) || \
 (!*y && !Q_stricmp(x->modelName, x->modelNameCopy) && !Q_stricmp(x->skinName, S_PM))
static qboolean CGX_SetModelAndSkin(clientInfo_t *ci, qboolean isDeferred, int clientNum) {
	qboolean isSameTeam = qfalse;

	if (cg.clientNum >= 0)
		isSameTeam = cgs.gametype >= GT_TEAM && cgs.clientinfo[cg.clientNum].team == ci->team;
		
	if (!isSameTeam) {
		if (IsSameModel2(ci, cg.enemyModel, cg.enemySkin))
			return qfalse;

		CGX_SetModel(ci, cg.enemyModel);
		CGX_SetSkin(ci, cg.enemySkin);
	} else {
		if (IsSameModel2(ci, cg.teamModel, cg.teamSkin))
			return qfalse;

		CGX_SetModel(ci, cg.teamModel);
		CGX_SetSkin(ci, cg.teamSkin);
	}

	ci->deferred = isDeferred;

	return qtrue;
}

//checks and sets enemymodel or restores real
void CGX_CheckEnemyModel(clientInfo_t *ci, qboolean isDeferred, int clientNum) {	
	// skip emtpy clientInfo 
	if (!ci->modelName[0] || !ci->skinName[0]) {
		D_Printf(("^7Skip '%i' '%s' '%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->infoValid, ci->deferred));
	} else {
		qboolean isSpect = ci->team == TEAM_SPECTATOR && !EM_Check(EM_SPECT);
		qboolean isPlayer = qfalse;
		qboolean isPlayerSpect = qfalse;

		if (cg.clientNum >= 0) {		
			isPlayerSpect = cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && !EM_Check(EM_SPECT);
			isPlayer = cg.clientNum == clientNum;
		}

		if (!cgx_enemyModel_enabled.integer || isSpect || isPlayerSpect || isPlayer || cg.clientIntermission) {
			if (CGX_RestoreModelAndSkin(ci, isDeferred)) {
				D_Printf(("^3Restore '%i' '%s/%s' '%s/%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy, ci->infoValid, ci->deferred));				
			} else {
				D_Printf(("^3OK %i\n", clientNum));
			}
		} else {
			if (CGX_SetModelAndSkin(ci, isDeferred, clientNum)) {
				D_Printf(("^2Set '%i' '%s/%s' '%s/%s' '%i' '%i'\n", clientNum, ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy, ci->infoValid, ci->deferred));
			} else {
				D_Printf(("^2OK %i\n", clientNum));
			}
		}
	}
}

// tracking changes, only after cg.snap received
void CGX_TrackEnemyModelChanges() {
	// track client num change
	if (cg.clientNum != cg.snap->ps.clientNum) {
		cg.clientNum = cg.snap->ps.clientNum;
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;

		if (cgx_enemyModel_enabled.integer)
		CGX_CheckEnemyModelAll();
		D_Printf(("^6cg.clientNum %i\n", cg.clientNum));
	} // track team change
	else if (cg.oldTeam != cgs.clientinfo[cg.clientNum].team) {
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;

		//clear stats if it's real player switch
		if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
			memset( &stats, 0, sizeof( stats ) );

		if (cgx_enemyModel_enabled.integer)
		CGX_CheckEnemyModelAll();
		D_Printf(("^6TEAM CHANGED!\n"));
	} //track intermission change
	else if (cg.snap->ps.pm_type == PM_INTERMISSION && !cg.clientIntermission &&
		EM_Check(EM_INTERMISSION)) {
		cg.clientIntermission = qtrue;

		if (cgx_enemyModel_enabled.integer)
		CGX_CheckEnemyModelAll();
		D_Printf(("^6PM_INTERMISSION!\n"));
	}
}

#pragma endregion

//extended map restart
void CGX_MapRestart() {
	D_Printf(("^1CGX_MapRestart\n"));

	cg.clientIntermission = qfalse;

	// X-MOD: send modinfo
	CGX_SendModinfo(qfalse);
	if (cgx_enemyModel_enabled.integer)
		CGX_CheckEnemyModelAll();

	//nemesis/osp reset stats
	memset( &stats, 0, sizeof( stats ) );

	CG_statsWindowPrint();
	CG_statsWindowFree( 0 );

	D_Printf(("^6CGX_MapRestart\n"));
}

//send cmd with interval on servers with sv_floodprotect 1
void CGX_SendClientCommand(char *command) {
	static int cmd_time;

	if (cgs.sv_floodProtect) {
		if (cg.time < cmd_time)
			return;

		cmd_time = cg.time + 1000;
	}

	trap_SendClientCommand(command);
}

#pragma region auto network settings

static qboolean CGX_ValidateFPS(void) {
	if (!com_maxfps.integer) {
		trap_Cvar_Set("com_maxfps", "125");		
		return qfalse;
	} else if (com_maxfps.integer > CGX_MAX_FPS) {
		com_maxfps.integer = CGX_MAX_FPS;		
		CG_Printf("Max com_maxfps is %i\n", CGX_MAX_FPS);
		trap_Cvar_Set("com_maxfps", va("%i", CGX_MAX_FPS));		
		return qfalse;
	}

	return qtrue;
}

#define NET_Set(x, y) { CG_Printf("Auto: %s %i\n", x, y); trap_Cvar_Set(x, va("%i", y)); }
static void CGX_Auto_sv_fps(void) {
	char buf[10];
	int i;
	trap_Cvar_Get("sv_fps", buf);
	i = atoi(buf);

	if (i < 40)
		NET_Set("sv_fps", 40)
	else if (i > 125)
		NET_Set("sv_fps", 125)

	sv_fps.integer = i;
}

//validate and adjust client network settings
void CGX_AutoAdjustNetworkSettings(void) {
	static int info_showed = 0;

	D_Printf(("CGX_AutoAdjustNetworkSettings %i\n", cgx_networkAdjustments.integer));

	if (cgx_networkAdjustments.integer) {
		int i, minRate, minSnaps, k;
		char buf[10];
		
		//adjust sv_fps for local game
		if (cgs.localServer) {
			CGX_Auto_sv_fps();
			return;
		}

		if (!CGX_ValidateFPS())
			return;

		i = 0;		

		if (cgx_networkAdjustments.integer == 1) {			
			minRate = 8000;
		
			// if packets < 30 set it to 30
			if (cl_maxpackets.integer < CGX_MIN_MAXPACKETS)
				i = CGX_MIN_MAXPACKETS;
			// set it litle more than sv_fps
			if (cl_maxpackets.integer <= sv_fps.integer)
				i = sv_fps.integer + 10;
		} else if (cgx_networkAdjustments.integer == 2) {
			k = 2;
			minRate = 16000;			

			// if it's something lower than 100 - adjust
			if (cl_maxpackets.integer < 100)
				while ((i = com_maxfps.integer / k++) > 60);			
		} else {
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
		trap_Cvar_Get("snaps", buf);
		i = atoi(buf);

		if (i < minSnaps)
			NET_Set("snaps", minSnaps)

		// check and set rate
		trap_Cvar_Get("rate", buf);
		i = atoi(buf);
		
		if (i < minRate)
			NET_Set("rate", minRate)
		else if (i > CGX_MAX_RATE) // no point in more than 25k rate, just for beautiful adjust in playerlist
			NET_Set("rate", CGX_MAX_RATE)

		if (cgx_networkAdjustments.integer == 3)
			NET_Set("cl_packetdup", 0)

		// check time nudge & send hints
		// if server delaged it's better off
		/*if (cgs.delagHitscan && cl_timeNudge.integer < 0 && !info_showed) {			
			trap_Print("^5Hint: server has hitscan delag, its nice to set cl_timeNudge 0\n");
			info_showed = 1;
		}*/ 

		//if (cl_timeNudge.integer < -15 && !info_showed) {
		//	trap_Print("^5Hint: cl_timeNudge below -15 is quite useless, set it only if you really need it and know what you are doing\n");
		//	info_showed = 1;
		//} else if (cl_timeNudge.integer > 0 && !info_showed) {
		//	trap_Print("^5Hint: cl_timeNudge above 0 gives rendering delay in milliseonds, set it only if you really need it (mostly if your connection is not stable)\n");
		//	info_showed = 1;
		//}

/*		if (cg_optimizePrediction.integer && cg_predictItems.integer && info_showed <= 1) {
			trap_Print("^5Hint: if you have false item pickups (picking up armor or weapon and it's doenst count) because cg_delag_optimizePrediction is set to 1 or you have high ping then try to set cg_predictitems 0\n");
			info_showed = 2;
		}	*/	
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
}

void CGX_SyncServer_sv_fps(const char *info) {
	static int old_sv_fps = -1;

	if (!sv_fps.integer || old_sv_fps != sv_fps.integer) {
		int i = atoi(Info_ValueForKey(info, "sv_fps"));

		D_Printf(("^3sv_fps serv '%i' sv_fps client '%i' ", i, sv_fps.integer));

		if (!i) {
			char buf[4];
			// get sv_fps if server sent it
			trap_Cvar_Get("sv_fps", buf);
			i = atoi(buf);
			//on some servs fps coming to sv_fps client value, on some stored in server info
			//try get from server info first, then from client
			if (!i)
				i = 20;
		}

		old_sv_fps = sv_fps.integer = i;

		D_Printf(("^3sv_fps final '%i'\n", sv_fps.integer));

		CGX_AutoAdjustNetworkSettings();
	}
}

void CGX_SyncServerParams(const char *info) {
	CGX_SyncServer_sv_fps( info );

	if (cgs.serverMod == SM_UNDEFINED) {
		CGX_SyncServer_delagHitscan( info );

		cgs.sv_floodProtect = atoi(Info_ValueForKey(info, "sv_floodProtect"));

		Q_strncpyz( cgs.gamename, Info_ValueForKey( info, "gamename" ), sizeof( cgs.gamename ) );

		if (!Q_stricmp( cgs.gamename, "NoGhost" )) {
			cgs.serverMod = SM_NOGHOST;
			//cgs.isFreezTag = atoi( Info_ValueForKey( info, "g_gameMod" ) ) == 1;
		} else if (!Q_stricmp(cgs.gamename, "Nemesis")) {
			cgs.serverMod = SM_NEMESIS;
		} else if (!Q_stricmp(cgs.gamename, "Carnage")) {
			cgs.serverMod = SM_CARNAGE;
		} else if (cgs.gamename[0] == 'B' && cgs.gamename[1] == 'M' && cgs.gamename[2] == 'A') {
			cgs.serverMod = SM_BMA;
		} else if (!Q_stricmp( cgs.gamename, "osp" )) {
			cgs.serverMod = SM_OSP;
		} else {
			cgs.serverMod = SM_DEFAULT;
			return;
		}

		// X-Mod: add commands just for autocomplete
		if (cgs.serverMod > SM_NOGHOST) {
			trap_AddCommand("players");
		} else if (cgs.serverMod == SM_NOGHOST) {
			//for ref info receiving in scores
			static vmCvar_t	mod_build;
			trap_Cvar_Register(&mod_build, "Mod_Build", "155XM", CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO);

			trap_AddCommand("playerlist");
			trap_AddCommand("setref");
		}
		if (cgs.serverMod >= SM_NOGHOST) {
			trap_AddCommand("help");
			trap_AddCommand("stats");
			trap_AddCommand("ref");
		}
		if (cgs.serverMod == SM_NEMESIS || cgs.serverMod == SM_CARNAGE) {
#if CGX_NEMESIS_COMPATIBLE
			// nemesis compability info
			static vmCvar_t	cgx_cgame, cgx_uinfo;
			const char *cg_uinfo = va("%i %i 0", cl_timeNudge.integer, cl_maxpackets.integer);

			//send info about client to nemesis servs
			trap_Cvar_Register(&cgx_cgame, "cgame", CGX_FULLVER, CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO);		
			trap_Cvar_Register(&cgx_uinfo, "cg_uinfo", cg_uinfo, CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO);
#endif
			trap_AddCommand("modinfo");
			trap_AddCommand("clientmod");
		} else if (cgs.serverMod == SM_BMA) {
			trap_AddCommand("modinfo");
		}
	}
}

#pragma endregion

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

			trap_SendConsoleCommand(va("say ^7"CGX_FULLVER" (%i:%i%i)\n", mins, tens, seconds));
		}
	} 
}

//fiter chat
void CGX_ChatFilter(char *str) {
	char *c;

	//fix \r bug in chat
	for (c = str; *c; c++)
		if (*c == '\r') *c = '.';
}

#pragma region Chat Tokens

// helpers for chat tokens
static char *team_names[] = { "Free", "Red", "Blue", "Spectator" };
static char team_cols[] = { COLOR_YELLOW, COLOR_RED, COLOR_BLUE, COLOR_WHITE };
static int intlen( int i ) {
	int l = 0;
	do { i /= 10; l++; } while (i);
	return l;
}
static char col_i( int i ) {
	if (i >= 100) return COLOR_WHITE;
	if (i >= 66) return COLOR_GREEN;
	if (i >= 33) return COLOR_YELLOW;
	return COLOR_RED;
}
static char col_i2( int i ) {
	if (i >= 100) return COLOR_WHITE;
	return COLOR_YELLOW;
}

#define cgx_token_fmt(x,len) \
*c = '%'; *(c + 1) = 's'; \
pos = c - res; \
res = va( res, x ); \
c = res + pos + len - 1;
#define cgx_add_i(x) Com_sprintf(val, sizeof val, "%i^%c", x, chatcol); cgx_token_fmt(val, intlen(x) + 2); //2 - chatcol len
#define cgx_add_s(x) Com_sprintf(val, sizeof val, "%s^%c", x, chatcol); cgx_token_fmt(val, strlen(x) + 2);
#define cgx_add_powerup(x) Com_sprintf(val, sizeof val, "%i^%c ", x, chatcol); cgx_token_fmt(val, intlen(x) + 2 + 1);
#define cgx_add_col_hp(x) Com_sprintf(val, sizeof val, "^%c%i^%c", col_i(x), x, chatcol); cgx_token_fmt(val, intlen(x) + 2 + 2); //2 - col_i len
#define cgx_add_col_armor(x) Com_sprintf(val, sizeof val, "^%c%i^%c", col_i2(x), x, chatcol); cgx_token_fmt(val, intlen(x) + 2 + 2);
#define cgx_add_col_team(c,x) Com_sprintf(val, sizeof val, "^%c%s^%c", c, x, chatcol); cgx_token_fmt(val, strlen(x) + 2 + 2);

//check string for chat tokens and replace them with info if necessary
char *CGX_CheckChatTokens( char *message, char chatcol ) {
	char *res, *c;

	//if following someone skip checks
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return message;

	//process tokens
	for (res = c = message; *c; c++) {
		if (*c != '#')
			continue;
		if (!*(c + 1) || !*(c + 2))
			break;
		if (!Q_isalpha( *(c + 2) )) {
			int pos;
			char val[128];
			playerState_t *ps = &cg.snap->ps;
			clientInfo_t *ci = &cgs.clientinfo[ps->clientNum];			

			switch (*(c + 1)) {
				case 'h': cgx_add_i( ps->stats[STAT_HEALTH] ); break;
				case 'a': cgx_add_i( ps->stats[STAT_ARMOR] ); break;
				case 'H': cgx_add_col_hp( ps->stats[STAT_HEALTH] ); break;
				case 'A': cgx_add_col_armor( ps->stats[STAT_ARMOR] ); break;
				case 'n': cgx_add_s( ci->name ); break;
				case 's': cgx_add_i( ci->score ); break;
				case 't': cgx_add_s( team_names[ci->team] ); break;
				case 'T': cgx_add_col_team( team_cols[ci->team], team_names[ci->team] ); break;
				case 'i': cgx_add_s( bg_itemlist[cg.itemPickup].pickup_name ); break;
				case 'x': cgx_add_s( cgs.clientinfo[CG_CrosshairPlayer()].name ); break;
				case 'l': cgx_add_s( cgs.clientinfo[CG_LastAttacker()].name ); break;
				case 'k': cgx_add_s( cg.killerName ); break;
				case 'u':
				case 'U': {
					char p[1024];
					int i;
					qboolean colorful = Q_isupper( *(c + 1) );

					for (p[0] = 0, i = 0; i < PW_NUM_POWERUPS; i++)
						if (ci->powerups & (1 << i)) {
							int pl = strlen( p );

							if (colorful) {
								char col;
								switch (i) {
								case PW_BLUEFLAG:
								case PW_INVIS: col = COLOR_BLUE; break;
								case PW_REDFLAG:
								case PW_REGEN: col = COLOR_RED; break;
								case PW_QUAD: col = COLOR_CYAN; break;
								case PW_HASTE:
								case PW_BATTLESUIT: col = COLOR_YELLOW; break;
								case PW_FLIGHT: col = COLOR_MAGENTA; break;
								default: col = chatcol; break; }

								Com_sprintf( p + pl, sizeof p - pl, "^%c%s^%c, ", col, bg_itemlist[27 + i].pickup_name, chatcol );
							}
							else {
								Com_sprintf( p + pl, sizeof p - pl, "%s, ", bg_itemlist[27 + i].pickup_name );
							}
						}

					i = strlen( p );

					if (!i) {
						cgx_add_s( "" )
					} else {
						p[i - 2] = 0;
						cgx_add_s( p )
					}
				} break;
				case 'L': {
					char *p = (char*)CG_ConfigString( CS_LOCATIONS + ci->location );
					if (!p || !*p)
						p = "unknown";

					cgx_add_s( p );
				} break;
			}
		}
	}

	return res;
}

#pragma endregion

#pragma region Modinfo

// check for unlagged enabled\disabled for bma\nms
// send modinfo in initialsnapshot and check result here
#define SEND_MODINFO_TIME	30000
static int cgx_modinfosend = 0;
qboolean CGX_CheckModInfo(const char *str) {
	int i;
	// if some time passed after sending then don't check
	if (cg.time - cgx_modinfosend > SEND_MODINFO_TIME)
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

// send modinfo if gamename nemesis or bma
void CGX_SendModinfo(qboolean force) {
	if (cg.intermissionStarted || cgs.delagHitscan == 1 || cg.time - cgx_modinfosend <= SEND_MODINFO_TIME)
		return;

	D_Printf(("gamename %s\n", cgs.gamename));

	if (cgs.serverMod == SM_NEMESIS || cgs.serverMod == SM_BMA || force) {
		cgx_modinfosend = cg.time;

		trap_SendClientCommand("modinfo");

		D_Printf(("modinfo sent\n"));
	} else {
		D_Printf(("modinfo not sent\n"));
	}
}

#pragma endregion

// X-MOD: potential fix for q3config saving problem
void CGX_SaveSharedConfig(qboolean forced) {
	if (cgx_sharedConfig.integer || forced) {
		char buf[32];
		trap_Cvar_Get("version", buf);

		if (!stristr(buf, "1.16")) {
			trap_Print(va("Version %s skip shared config save\n", buf));
			return;
		}

		trap_Cvar_Get("fs_game", buf);		

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

static char* CGX_GetQuakeStartPars(char *map, char *fs_game, char* custom) {
	char *q3pars;

	if (cgs.localServer) {
		if (cgs.gametype == GT_SINGLE_PLAYER)
			q3pars = va("+set sv_pure 0 +set fs_game \"%s\" %s +spmap %s", fs_game, custom, map);
		else {
			char bot_enable[256];
			trap_Cvar_Get("bot_enable", bot_enable);
			//if botenable off restart with its off
			if (!atoi(bot_enable) && !*custom)
				custom = "+set bot_enable 0";
			q3pars = va("+set sv_pure 0 +set fs_game \"%s\" %s +map %s", fs_game, custom, map);
		}
	} else {
		char servaddr[128];
		trap_Cvar_Get("cl_currentServerAddress", servaddr);

		q3pars = *servaddr ? va("+connect %s", servaddr) : "";
	}

	return q3pars;
}

//read bat, insert params and save
void CGX_GenerateMapBat2(char *map) {
	char txt[1024*3];
	char *mod = "baseq3";
	int res;

	if (!cgx_dl_tobaseq3.integer) {
		char fs_game[256];
		trap_Cvar_Get("fs_game", fs_game);
		mod = *fs_game ? fs_game : "baseq3";		
	}

	if (res = CGX_FReadAll("win\\download_map.bat", txt, sizeof txt)) {
		char page[1024];
		char *script;

		Com_sprintf(page, sizeof page, cgx_dl_page.string, map);

		/*set "map=%s"
		set "mod=%s"
		set "tmp=%s"
		set "host=%s"
		set "page=%s"
		set "pars=%s"*/
		script = va(txt, map, mod, dl_tempname,
			cgx_dl_host.string, page, CGX_GetQuakeStartPars(map, mod, "+set bot_enable 0"));

		if (res = CGX_FWriteAll("..\\"CGX_MAPBAT, script, strlen(script))) {
			char	path[MAX_INFO_VALUE];
			trap_Cvar_Get("fs_basepath", path);

			trap_Print(CGX_MAPBAT" generated successfully\n");
			XMOD_ANSWER(va("Open folder %s", path));
			XMOD_ANSWER("And start ^2"CGX_MAPBAT);
		}
	}
}

// generate script to open url to worldspawn to download map
void CGX_GenerateMapBat(char *map) {
	fileHandle_t f;
	char	path[MAX_INFO_VALUE];
	trap_Cvar_Get("fs_basepath", path);

	trap_Print("Generating "CGX_MAPBAT"...\n");

	trap_FS_FOpenFile("..\\"CGX_MAPBAT, &f, FS_WRITE);

	if (f) {
		char *buf;
		qboolean answer = qfalse;

		if (!*map)
			map = cgs.mapname_clean;
		else
			answer = qtrue;

		buf = va("explorer \""CGX_MAPURL"%s/\"\ndel \"%%~f0\"", map);
		trap_FS_Write(buf, strlen(buf), f);

		trap_FS_FCloseFile(f);

		trap_Print(CGX_MAPBAT" generated successfully\n");

		if (answer) {			
			XMOD_ANSWER(va("Open folder %s", path));
			XMOD_ANSWER("And start ^2"CGX_MAPBAT);
		}
	}
	else {
		trap_Print("^1Couldn't open a file "CGX_MAPBAT"\n");
	}
}

#ifdef CGX_WIN

//#define	CGX_DLURL		"http://ws.q3df.org/maps/download/"
//for URLDownloadToFile
//#include <urlmon.h>
//#pragma comment(lib, "urlmon.lib")
//static void CGX_DownloadSync(char *name) {
//	char *url, *filePath;
//	char basepath[1024];
//	int res;
//
//	trap_Cvar_Get("fs_basepath", basepath);
//
//	url = va(CGX_DLURL"%s/", name);
//	filePath = va("%s\\%s\\map-%s.pk3", basepath, "baseq3", name);
//
//	CG_Printf("Attempting to download %s\nTo: %s\n", name, filePath);
//	CG_Printf("From: %s\n", CGX_DLURL);
//
//	if (res = URLDownloadToFile(NULL, url, filePath, 0, NULL)) {
//		CG_Printf("^1Error during download %s\n", name);
//	} else {
//		XMOD_ANSWER(va("Map %s dowloaded successfuly", name));
//	}
//}

#endif

//download map and load after
void CGX_DownloadMap(char *name, qboolean end_load) {	
	if (CGX_FExists(va("maps\\%s.bsp", name))) {
		XMOD_ANSWER(va("You have map %s already", name));
		return;
	}

#ifdef CGX_WIN
#else
	CGX_GenerateMapBat2(name);
#endif
}

#pragma region cg_nomip

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

#pragma endregion

#pragma region map lodading fix

void CGX_IncreaseHunkmegs(int min) {
	char buf[8];
	trap_Cvar_Get("com_hunkMegs", buf);

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
	char buf[MAX_INFO_VALUE];
	char *s, *t;

	trap_Cvar_Get("cl_fixedmaps", buf);

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
	char buf[MAX_INFO_VALUE];
	int i;

	if (CGX_IsRememberedMap())
		return;	

	trap_Cvar_Get("cl_fixedmaps", buf);

	i = strlen(buf);
	Com_sprintf(buf + i, sizeof(buf) - i, "%s ", cgs.mapname_clean);

	trap_Cvar_Set("cl_fixedmaps", buf);
}

qboolean CGX_IsPure() {
	qboolean isPure;
	char buf[4];

	trap_Cvar_Get("sv_pure", buf);
	isPure = atoi(buf);
	if (cgs.localServer && isPure && cgx_networkAdjustments.integer) {
		trap_Cvar_Set("sv_pure", "0");
		return isPure;
	} else {
		return cgs.isPureServer;
	}
}

#define IsQ3Map(x) ((x[0] == 'q' || x[0] == 'Q') && x[1] == '3')
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

#pragma endregion

#pragma region Xmod command

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
	char			buf[1024 * 8];
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
				CG_Printf("%-25s", t);
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
				if (!(t = stristr(t, cmd)))
					break;
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

static void CGX_Pk3list_f(void) {
	trap_SendConsoleCommand("dir . pk3\n");
}

static char CGX_RandChar() {
	char res;
	do res = 32 + rand() % 58;
	while (res == '%' || (res >= 39 && res <= 63));
	return res;
}

#define rc CGX_RandChar()
static void CGX_RageQuit_f(void) {
	trap_SendConsoleCommand(va("say %c%c%c%c%c%c%c%c%c%c%c%c%c!!!; wait 300; disconnet; quit;\n", rc, rc, rc, rc, rc, rc, rc, rc, rc, rc, rc, rc, rc));
}
#undef rc

//method to reload effects, used to call reload from UI
static void CGX_ReloadEffects() {
	char buf[32];
	// get this cvar this way, in case if menu changed it and send command
	trap_Cvar_Get("cg_weaponEffects", buf);

	if (atoi(buf) & WE_LG32)
		trap_R_RegisterShaderOrNomip(cgs.media.lightningShader, "lightningBoltNew")
	else
		trap_R_RegisterShaderOrNomip(cgs.media.lightningShader, "lightningBolt")
}

#define help_file "doc\\2-comand_list.txt"
static void CGX_PrintModelCache();
//xmod command
void CGX_Xmod() {
	char command[MAX_QPATH], arg[MAX_QPATH];
	int i, argc = trap_Argc();

	if (argc < 2) {
		Q_strncpyz(command, "help", MAX_QPATH);
	} else {
		trap_Argv(1, command, MAX_QPATH);
		if (argc > 2) trap_Argv(2, arg, MAX_QPATH);
	}

	if (!Q_stricmp(command, "e")) {
		XMOD_ANSWER("checking enemy models...");
		CGX_CheckEnemyModelAll();
		return;
	} 

	i = strlen(command);
	if (i && i < 3) {
		XMOD_ANSWER("too short cmd");		
		return;
	}

#if CGX_DEBUG
	if (!Q_stricmp(command, "float")) {
		float f;
		int p = abs(atoi(arg));
		i = 1;
		while (p--) i *= 10;
		for (f = 0.0f; f <= 0.001f * i; f += 0.00001f * i)
			CG_Printf("%f\n", f);
	} else if (!Q_stricmp(command, "crandom")) {
		for (i = 0; i < 100; i++)
			CG_Printf("%1.2f ", crandom());
		CG_Printf("\n");
		return;
	} else
	if (!Q_stricmp(command, "eFlags")) {
		centity_t *cent = &cg_entities[cg.clientNum];

		CG_Printf("%i\n%i\n", cent->currentState.eFlags, cg.snap->ps.eFlags);
	} else if (!Q_stricmp(command, "stats")) {
		for (i = 0; i < MAX_STATS; i++)
			CG_Printf("%i ", cg.snap->ps.stats[i]);
		CG_Printf("\n");
	} else if (!Q_stricmp(command, "pers")) {
		for (i = 0; i < MAX_PERSISTANT; i++)
			CG_Printf("%i ", cg.snap->ps.persistant[i]);
		CG_Printf("\n");
	} else if (!Q_stricmp(command, "powerups")) {
		for (i = 0; i < MAX_POWERUPS; i++)
			CG_Printf("%i ", cg.snap->ps.powerups[i]);
		CG_Printf("\n");
	} else if (!Q_stricmp(command, "ammo")) {
		for (i = 0; i < MAX_WEAPONS; i++)
			CG_Printf("%i ", cg.snap->ps.ammo[i]);
		CG_Printf("\n");
	} else if (!Q_stricmp(command, "css")) {
		for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
			const char *cs = CG_ConfigString(i);
			if (*cs) CG_Printf("%i %s\n", i, cs);
		}
	} else if (!Q_stricmp(command, "ccss")) {
		for (i = 0; i < MAX_CLIENTS; i++) {
			const char *cs = CG_ConfigString(CS_PLAYERS + i);
			if (*cs) CG_Printf("%i %s\n", i, cs);
		}
	} else 
#endif

	if (!Q_stricmp(command, "version")) {
		XMOD_ANSWER(CGX_FULLVER" "CGX_DATE);
	} else if (!Q_stricmp(command, "help")) {
		CGX_ShowHelp(help_file, "");
	} else if (!Q_stricmp(command, "freemem")) {
		CG_Printf("%i Mb\n", trap_MemoryRemaining() / 1024 / 1024);
	} else if (!Q_stricmp(command, "models")) {
		CGX_PrintModelCache();
	} else if (!Q_stricmp(command, "skins")) {
		CGX_PrintSkinCache();
	} else if (!Q_stricmp(command, "clients")) {
		clientInfo_t *ci;
		char clname[MAX_QPATH];
		for (i = 0, ci = cgs.clientinfo; i < MAX_CLIENTS; i++, ci++)
			if (*ci->modelName && *ci->skinName) {
				Q_strncpyz(clname, ci->name, sizeof clname);
				CG_Printf("%3i %20s %s/%s %s/%s\n", i, Q_CleanStr(clname), ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy);
			}
	} else if (!Q_stricmp(command, "modinfo")) {
		CGX_SendModinfo(qtrue);
	} else if (!Q_stricmp(command, "reload")) {
		if (!Q_stricmp(arg, "effects"))
			CGX_ReloadEffects();
	} else if (stristr(command, "pk3")) {
		CGX_Pk3list_f();
	} else if (!Q_stricmp(command, "ragequit")) {
		CGX_RageQuit_f();
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
		CGX_ShowHelp(help_file, command);
	}
}

#pragma endregion

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

#pragma region Xmod I/O

//safety open file, -1 unlim size
static int CGX_FOpenFile(char *filename, fileHandle_t *f, fsMode_t mode, int maxSize) {
	int len;
	len = trap_FS_FOpenFile( filename, f, mode );

	if ( len <= 0 ) {
		trap_Print( va( S_COLOR_YELLOW "File not found: %s\n", filename ) );
		return 0;
	}
	if ( len >= maxSize && maxSize != -1) {
		trap_Print( va( S_COLOR_RED "File too large: %s is %i, max allowed is %i", filename, len, maxSize ) );
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

//wirte all and close
static qboolean CGX_FWriteAll(char *filename, char *buffer, int bufsize) {
	fileHandle_t f;

	trap_FS_FOpenFile(filename, &f, FS_WRITE);

	if (f) {
		trap_FS_Write(buffer, bufsize, f);
		trap_FS_FCloseFile(f);
		return qtrue;
	}

	return qfalse;
}

//get file size
static int CGX_FSize(char* filename) {
	fileHandle_t f;
	int size;
	if (size = CGX_FOpenFile(filename, &f, FS_READ, -1)) {
		trap_FS_FCloseFile(f);
		return size;
	}

	return 0;
}

//check if file exists
static qboolean CGX_FExists(char* filename) {
	fileHandle_t f;
	if (trap_FS_FOpenFile(filename, &f, FS_READ) <= 0) return qfalse;
	trap_FS_FCloseFile(f);
	return qtrue;
}

//copy file
static int CGX_FCopy(char *filename, char *dest) {
	int len;
	fileHandle_t	f1, f2;

	if (len = CGX_FOpenFile(filename, &f1, FS_READ, -1)) {
		char			buf[1024 * 8];
		int read = 0;
		int bytesleft = 0;

		trap_FS_FOpenFile(dest, &f2, FS_WRITE);

		if (!f2) {
			CG_Printf("^1Couldn't open %s to write", dest);
			return 0;
		}

		bytesleft = len;

		while (bytesleft > 0) {
			read = sizeof buf;

			if (read > bytesleft)
				read = bytesleft;

			trap_FS_Read(buf, read, f1);
			trap_FS_Write(buf, read, f2);

			bytesleft -= read;
		}

		trap_FS_FCloseFile(f1);
		trap_FS_FCloseFile(f2);
	}

	return len;
}

#pragma endregion

#pragma region Model loading and caching

typedef struct {
#if CGX_DEBUG
	int				size;
#endif

	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;

	qhandle_t		modelIcon;

	animation_t		animations[MAX_ANIMATIONS];

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
} modelInfo_t;

static modelInfo_t modelCache[MAX_CLIENTS];

static int modelCacheNum;

void CGX_ResetModelCache() {
	if (!cgx_modelCache.integer) return;

	memset(&modelCache, 0, sizeof modelCache);
	modelCacheNum = 0;

	D_Printf(("CGX_ResetModelCache %i\n", sizeof modelCache));
}

static void CGX_PrintModelCache() {
	int i;

	if (!cgx_modelCache.integer) {
		CG_Printf("Model cache is disabled\n");
		return;
	}

	for (i = 0; i < modelCacheNum; i++) {
		modelInfo_t *mi = &modelCache[i];

#if CGX_DEBUG
		CG_Printf("%i %s %s %i b\n", i, mi->modelName, mi->skinName, mi->size);
#else
		CG_Printf("%i %s %s\n", i, mi->modelName, mi->skinName);
#endif
	}

	CG_Printf("Freemem: %i Mb\n", trap_MemoryRemaining() / 1024 / 1024);
}

static void CGX_SaveModelToCache(modelInfo_t *to, clientInfo_t *from) {
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	to->legsModel = from->legsModel;
	to->legsSkin = from->legsSkin;
	to->torsoModel = from->torsoModel;
	to->torsoSkin = from->torsoSkin;
	to->headModel = from->headModel;
	to->headSkin = from->headSkin;
	to->modelIcon = from->modelIcon;

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

static void CGX_LoadModelFromCache(clientInfo_t *to, modelInfo_t *from) {
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;

	to->legsModel = from->legsModel;
	to->legsSkin = from->legsSkin;
	to->torsoModel = from->torsoModel;
	to->torsoSkin = from->torsoSkin;
	to->headModel = from->headModel;
	to->headSkin = from->headSkin;
	to->modelIcon = from->modelIcon;

	memcpy( to->animations, from->animations, sizeof( to->animations ) );
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );

	to->deferred = qfalse;
}

//sort z-a by modelName
static int QDECL CacheModel_Compare(const void *arg1, const void *arg2) {
	modelInfo_t* t1 = (modelInfo_t *)arg1;
	modelInfo_t* t2 = (modelInfo_t *)arg2;

	return Q_stricmp(t2->modelName, t1->modelName);
}

#if CGX_DEBUG
static void CGX_CacheModel(clientInfo_t *ci, int size) {
#else
static void CGX_CacheModel(clientInfo_t *ci) {
#endif
	int i;
	modelInfo_t *mi;

	if (!cgx_modelCache.integer) return;

	if (modelCacheNum == ArrLen(modelCache)) {
		D_Printf(("Model cache is full\n"));
		return;
	}

	for (i = 0, mi = modelCache; i < modelCacheNum; i++, mi++) {
		if (!Q_stricmp(ci->skinName, mi->skinName) && !Q_stricmp(ci->modelName, mi->modelName)) {
			D_Printf(("Already cached %s/%s\n", mi->modelName, mi->skinName));
			return;
		}
	}
	
	// if it's not cached, cache here
#if CGX_DEBUG
	mi->size = size;
#endif
	Q_strncpyz(mi->skinName, ci->skinName, sizeof ci->skinName);
	Q_strncpyz(mi->modelName, ci->modelName, sizeof ci->modelName);

	CGX_SaveModelToCache(mi, ci);
	modelCacheNum++;
	D_Printf(("CGX_CacheModel %s/%s %i\n", mi->modelName, mi->skinName, modelCacheNum));

	//qsort(modelCache, modelCacheNum, sizeof (modelInfo_t), CacheModel_Compare);
}

static qboolean CGX_TrySkinLoad(clientInfo_t *ci) {
	int i;
	modelInfo_t *mi;

	for (i = modelCacheNum, mi = modelCache; i--; mi++)
		if (!Q_stricmp(ci->modelName, mi->modelName)) {
			D_Printf(("^5Skin load %i %s/%s\n", i, mi->modelName, mi->skinName));
			CGX_LoadClientInfo(ci);
			return qtrue;
		}

	return qfalse;
}

qboolean CGX_TryLoadModelFromCache(clientInfo_t *ci, qboolean tryAny, qboolean trySkinLoads) {
	int i;
	modelInfo_t *mi;

	if (!cgx_modelCache.integer) return qfalse;

	D_Printf(("CGX_TryLoadModelFromCache %s/%s ", ci->modelName, ci->skinName));

	for (i = modelCacheNum, mi = modelCache; i--; mi++)
		if (!Q_stricmp(ci->skinName, mi->skinName) &&
			!Q_stricmp(ci->modelName, mi->modelName)) {

			CGX_LoadModelFromCache(ci, mi);
			D_Printf(("^2Success %i\n", i));
			return qtrue;
		}

	if (tryAny) {
		int mem = trap_MemoryRemaining();
		qboolean lowMem = mem < LOW_MEMORY;
		
		if (trySkinLoads) {
			D_Printf(("trySkinLoads "));
			//if have some mem try skin load, it takes 3-5kb
			if (mem > LOW_MEMORY / 5 && CGX_TrySkinLoad(ci))
				return qtrue;
			D_Printf(("^3NoModel... "));
		}

		for (i = modelCacheNum, mi = modelCache; i--; mi++)
			if (cgs.gametype < GT_TEAM || !Q_stricmp(ci->skinName, mi->skinName)) {
				if (cgx_enemyModel_enabled.integer && !Q_stricmp(mi->modelName, cg.enemyModel))
					continue;
			
				D_Printf(("^4Got any %i %s/%s\n", i, mi->modelName, mi->skinName));

				CGX_LoadModelFromCache(ci, mi);
#if !CGX_DEBUG
				if (lowMem && trySkinLoads)
					CG_Printf("Memory is low. For %s/%s loaded %s/%s from cache.\n", ci->modelName, ci->skinName, mi->modelName, mi->skinName);
#endif
				return qtrue;
			}

		// try skin loads before full loads if we didn't try yet
		if (!trySkinLoads) {
			D_Printf(("trySkinLoads 2! "));
			if (mem > LOW_MEMORY / 5 && CGX_TrySkinLoad(ci))
				return qtrue;
			D_Printf(("^3NoModel... "));
		}

		//if we are on low mem and have something in cache try to load only skin for first model
		if (lowMem && modelCacheNum > 1) {
			//get first not enemy model
			for (i = modelCacheNum, mi = modelCache; i--; mi++)
				if (!cgx_enemyModel_enabled.integer || Q_stricmp(mi->modelName, cg.enemyModel))
					break;

			if (cgs.gametype >= GT_TEAM) {
				D_Printf(("^1Last model skin %i %s => %s \n", i, ci->modelName, mi->modelName));
#if !CGX_DEBUG
				CG_Printf("^3Memory is low. For %s loading %s.\n", ci->modelName, mi->modelName);
#endif
				Q_strncpyz(ci->modelName, mi->modelName, sizeof ci->modelName);
				Q_strncpyz(ci->modelNameCopy, mi->modelName, sizeof ci->modelName);
			} else {
				D_Printf(("^1Last model skin %i %s/%s => %s/%s \n", i, ci->modelName, ci->skinName, mi->modelName, mi->skinName));
#if !CGX_DEBUG
				CG_Printf("^3Memory is low. For %s/%s loading %s/%s.\n", ci->modelName, ci->skinName, mi->modelName, mi->skinName);
#endif
				Q_strncpyz(ci->modelName, mi->modelName, sizeof ci->modelName);
				Q_strncpyz(ci->modelNameCopy, mi->modelName, sizeof ci->modelName);
				Q_strncpyz(ci->skinName, mi->skinName, sizeof ci->skinName);
				Q_strncpyz(ci->skinNameCopy, mi->skinName, sizeof ci->skinName);
			}

			CGX_LoadClientInfo(ci);

			return qtrue;
		} 

		D_Printf(("^1FULL LOAD\n"));

		CGX_LoadClientInfo(ci);

		return qtrue;
	}

	D_Printf(("NoModel %i\n", i));
	return qfalse;
}

//fix enemymodels with vertex light
//check vertex light and load client info
//cache models
void CGX_LoadClientInfo( clientInfo_t *ci ) {
#if CGX_DEBUG
	int size = trap_MemoryRemaining();
#endif
	CGX_NomipStart();
	
	if (r_vertexLight.integer) {
		trap_Cvar_Set("r_vertexLight", "0");
		CG_LoadClientInfo(ci);
		trap_Cvar_Set("r_vertexLight", "1");
	} else {
		CG_LoadClientInfo(ci);
	}
	
	CGX_NomipEnd();
#if CGX_DEBUG
	CGX_CacheModel(ci, size - trap_MemoryRemaining());
#else
	CGX_CacheModel(ci);
#endif
}

#pragma endregion
