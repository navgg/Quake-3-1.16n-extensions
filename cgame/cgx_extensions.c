// Copyright (C) 2018 NaViGaToR (322)

#include "cg_local.h"

#define XMOD_ANSWER(x) { CG_Printf("^7[^1xmod^7]: ^6%s\n", x); /*trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND )*/; }

#define DARKEN_COLOR 64
#define EM_SPECT			2
#define EM_INTERMISSION		4

#define trap_Cvar_Get(name,v) trap_Cvar_VariableStringBuffer(name, v, sizeof v)

#define dl_tempname	"{X-Mod}download.tmp"

//bright skins
#define S_PM		"pm"
#define S_BRIGHT	"bright"

//bright skin shader
#define SS_BRIGHT	"xm_fb1"

//file i/o
static int CGX_FCopy(char *filename, char *dest);
static qboolean CGX_FExists(char* filename);
static int CGX_FSize(char *filename);
static int CGX_FOpenFile(char *filename, fileHandle_t *f, fsMode_t mode, int maxSize);
static int CGX_FReadAll(char *filename, char *buffer, int bufsize);
static int CGX_FReadAllBinary(char *filename, void *buffer, int bufsize);
static qboolean CGX_FWriteAll(char *filename, char *buffer, int bufsize);

static void CGX_Delay( int msec ) {
	CG_Printf( "Delay for %i start...\n", msec );
	msec += trap_Milliseconds();	
	while (msec > trap_Milliseconds());
	CG_Printf( "Delay end\n" );
}

#pragma region hud & virtual screen

#define XM_SMALLCHAR_WIDTH  6
#define XM_BIGCHAR_WIDTH	12
#define XM_GIANT_WIDTH		24
#define	XM_CHAR_WIDTH		28 //this should be 24
#define XM_ICON_SIZE		36

static void CGX_LoadHUD(char *fileName);

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

	hud.pickupTime = 3000;

	CGX_LoadHUD(cgx_hud.string);

	if (hud.file)
		hud.type = HUD_COMPACT;
	else
		hud.type = cg_draw2D.integer;

	if (hud.type == HUD_COMPACT) {
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

	if (hud.type == HUD_VANILLAQ3) {
		hud.head_size *= 1.25f;
		hud.weap_y = 380;
	} else {
		int ox = 0;

		if (hud.type == HUD_DEFAULT) {
			ox = 7;
		} else if (hud.type == HUD_COMPACT) {
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

	hud.sbheady = SCREEN_HEIGHT - hud.head_size;
}

//init virtual screen for widescreen or 4:3
void CGX_Init_vScreen(void) {
	const float baseAspect = 0.75f; // 3/4
	float aspect;

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

	cgs.screenXScale640 = (float)cgs.glconfig.vidWidth / (float)SCREEN_WIDTH;

	CGX_Init_HUD();

	D_Printf(("CGX_Init_vScreen %ix%i cgx_wideScreenFix %i\n", vScreen.width, SCREEN_HEIGHT, cgx_wideScreenFix.integer));	
}

static void CGX_CountAlive() {
	static int last_time;
	int i;
	clientInfo_t *ci;

	if (last_time > cg.time)
		return;

	last_time = cg.time + 500;

	cgs.redAlive = 0;
	cgs.blueAlive = 0;

	if (cg.numScores)
		for ( i = 0 ; i < cg.numScores; i++ ) {
			score_t	*score = &cg.scores[i];
			ci = &cgs.clientinfo[ score->client ];

			if (ci->team == TEAM_BLUE && !Q_Isfreeze(score->client))
				cgs.blueAlive++;
			else if (ci->team == TEAM_RED && !Q_Isfreeze(score->client))
				cgs.redAlive++;
		}
	else
		for (i = 0; i < 32; i++) {
			ci = &cgs.clientinfo[ i ];

			if (ci->team == TEAM_BLUE && !Q_Isfreeze(i))
				cgs.blueAlive++;
			else if (ci->team == TEAM_RED && !Q_Isfreeze(i))
				cgs.redAlive++;
		}
}

float CGX_DrawTeamCounts(float y) {
	CGX_CountAlive();

	if (hud.file) {
		CGX_DrawPic(XH_TeamIcon_OWN, hud.icon_own);
		CGX_DrawPic(XH_TeamIcon_NME, hud.icon_nme);

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
			CGX_DrawString(XH_TeamCount_OWN, va("%i", cgs.blueAlive), 0);
			CGX_DrawString(XH_TeamCount_NME, va("%i", cgs.redAlive), 0);
		} else {
			CGX_DrawString(XH_TeamCount_OWN, va("%i", cgs.redAlive), 0);
			CGX_DrawString(XH_TeamCount_NME, va("%i", cgs.blueAlive), 0);
		}

		return y;
	} else {
		vec4_t color_b = { 0, 0, 1, 0.25 };
		vec4_t color_r = { 1, 0, 0, 0.25 };
		int x = vScreen.width;
		int w;
		char *s;

		//y -= BIGCHAR_HEIGHT + 4;
		y += 4;

		s = va( "%2i", cgs.blueAlive );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;

		CG_FillRect( x, y, w, BIGCHAR_HEIGHT + 8, color_b );
		CG_DrawBigString( x + 4, y + 4, s, 0.5F);

		s = va( "%2i", cgs.redAlive );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;

		CG_FillRect( x, y, w, BIGCHAR_HEIGHT + 8, color_r );
		CG_DrawBigString( x + 4, y + 4, s, 0.5F);

		return y + BIGCHAR_HEIGHT + 8 + 4;
	}
}

#pragma endregion

#pragma region Xmod enemy models

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
void CGX_CheckEnemyModelAll(qboolean force) {
	int		i;
	clientInfo_t	*ci;

	if (cgs.gametype == GT_SINGLE_PLAYER)
		return;

	if (!cgx_enemyModel_enabled.integer && !force)
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
		case 1: return DARKEN_COLOR; //just grey
		case 2:	return (0.2126f * c[0] + 0.7152f * c[1] + 0.0722f * c[2]) / 4; //BT709 Greyscale
		case 3: return (0.2989f * c[0] + 0.5870f * c[1] + 0.1140f * c[2]) / 4; //Y-Greyscale (PAL/NTSC)
		default: return cgx_deadBodyDarken.integer;
	}
}

static void CGX_SetColorInfo(clientInfo_t *info, const char *color, int clientNum) {
	int i;

	if (!*color || !cgx_enemyModel_enabled.integer || cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR || info->team == TEAM_SPECTATOR)
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
	} else if (!Q_stricmp(skinName, "fb1") && !CGX_IsSkinExists(ci->modelName, skinName)) {
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
		isSameTeam = cgs.clientinfo[cg.clientNum].team == ci->team && ci->team != TEAM_FREE && ci->team != TEAM_SPECTATOR;
		
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
		qboolean restoreInIntermission = cg.clientIntermission && (cgx_enemyModel_enabled.integer & EM_INTERMISSION);
		qboolean restoreInSpect = !(cgx_enemyModel_enabled.integer & EM_SPECT);
		qboolean isSpect = ci->team == TEAM_SPECTATOR && restoreInSpect;
		qboolean isPlayer = qfalse;
		qboolean isPlayerSpect = qfalse;

		if (cg.clientNum >= 0) {		
			isPlayerSpect = cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && restoreInSpect;
			isPlayer = cg.clientNum == clientNum;
		}

		if (!cgx_enemyModel_enabled.integer || isPlayer || isSpect || isPlayerSpect || restoreInIntermission) {
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
void CGX_TrackPlayerStateChanges() {
	// track client num change
	if (cg.clientNum != cg.snap->ps.clientNum) {
		cg.clientNum = cg.snap->ps.clientNum;
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;

		CGX_CheckEnemyModelAll(qfalse);
		D_Printf(("^6cg.clientNum %i\n", cg.clientNum));
	} // track team change
	else if (cg.oldTeam != cgs.clientinfo[cg.clientNum].team) {
		cg.oldTeam = cgs.clientinfo[cg.clientNum].team;

		//clear stats if it's real player switch
		if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
			memset( &stats, 0, sizeof( stats ) );

		CGX_CheckEnemyModelAll(qfalse);
		D_Printf(("^6TEAM CHANGED!\n"));
	} //track intermission change
	else if (cg.snap->ps.pm_type == PM_INTERMISSION && !cg.clientIntermission) {
		cg.clientIntermission = qtrue;

		CGX_CheckEnemyModelAll(qfalse);
		D_Printf(("^6PM_INTERMISSION!\n"));
	}
}

#pragma endregion

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
void CGX_AutoNetworkSettings(void) {
	static int info_showed = 0;

	D_Printf(("CGX_AutoNetworkSettings %i\n", cgx_networkAdjustments.integer));

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
			minRate = 10000;
		
			// if packets < 30 set it to 30
			if (cl_maxpackets.integer < CGX_MIN_MAXPACKETS)
				i = CGX_MIN_MAXPACKETS;
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
			// set it litle more than sv_fps
			if (i <= sv_fps.integer)
				i = sv_fps.integer + 10;

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

		NET_Set("cl_packetdup", cgx_networkAdjustments.integer == 1 ? 1 : 0)

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

		CGX_AutoNetworkSettings();
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
			if (cgs.gametype == GT_TEAM) {
				int g_gamemod = atoi( Info_ValueForKey( info, "g_gameMod" ) );
				cgs.countAlive = g_gamemod == 1 || g_gamemod == 3;
			}
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
			trap_Cvar_Register(&cgx_uinfo, "cg_uinfo", "", CVAR_INIT | CVAR_ROM | CVAR_TEMP | CVAR_USERINFO);

			trap_Cvar_Set("cg_uinfo", cg_uinfo);
#endif
			trap_AddCommand("modinfo");
			trap_AddCommand("clientmod");
		} else if (cgs.serverMod == SM_BMA) {
			trap_AddCommand("modinfo");
		}
	}
}

#pragma endregion

#pragma region Chat filter & check

//check message for special commands
#define CHECK_INTERVAL	15000 //msec
void CGX_CheckChatCommand(const char *str) {
	int i;

	i = strlen(str);

	if (i > 3 && str[i - 2] == '!' && str[i - 1] == 'v') {
		int	mins, seconds, tens, msec;
		static int last_check;

		if (cg.time > last_check) {
			last_check = cg.time + CHECK_INTERVAL;

			msec = cg.time - cgs.levelStartTime;
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

	if (cgx_chatFilter.integer & 1)
	//fix \r bug in chat
	for (c = str; *c; c++)
		if (*c == '\r') *c = '.';
}

//check chat string
void CGX_CheckChat(const char *str, qboolean tchat) {
	if (!tchat)
		CGX_CheckChatCommand(str);

	CGX_ChatFilter((char *)str);
}

#pragma endregion

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
// send modinfo and check result here
#define SEND_MODINFO_TIME	30000
static int cgx_modinfosend = 0;
qboolean CGX_CheckModInfo(const char *str) {
	int i;
	// if some time passed after sending then don't check
	if (cg.time > cgx_modinfosend)
		return qtrue;

	i = strlen(str);

	if (Q_stricmp(str, "^3Unlag:           ^5ENABLED\n") == 0) {
		D_Printf(("BMA Unlagged!\n"));

		cgs.delagHitscan = 2;
		CGX_AutoNetworkSettings();
	} else if (Q_stricmp(str, "^3Unlagged compensation: ^5ENABLED\n") == 0) {
		D_Printf(("Nemesis Unlagged!\n"));

		cgs.delagHitscan = 3;
		CGX_AutoNetworkSettings();
	} else if (Q_stricmp(str, "unknown cmd modinfo\n") == 0) {
		D_Printf(("unknwn cmd\n"));
		return qfalse;
	}

	return qtrue;
}

// send modinfo if gamename nemesis or bma
void CGX_SendModinfo(qboolean force) {
	if (cgx_modinfosend <= cg.time || cg.snap->ps.pm_type == PM_INTERMISSION)
		return;

	D_Printf(("gamename %s\n", cgs.gamename));

	if (cgs.serverMod == SM_NEMESIS || cgs.serverMod == SM_BMA || force) {
		cgx_modinfosend = cg.time + SEND_MODINFO_TIME;

		trap_SendClientCommand("modinfo");

		D_Printf(("modinfo sent\n"));
	} else {
		D_Printf(("modinfo not sent\n"));
	}
}

#pragma endregion

#pragma region extended commands & fixes

//calling method after receiving first snapshot
void CGX_InitialSnapshot() {
	CGX_SendModinfo(qfalse);
}

//extended map restart
void CGX_MapRestart() {
	D_Printf(("^1CGX_MapRestart\n"));

	cg.clientIntermission = qfalse;

	// X-MOD: send modinfo
	CGX_SendModinfo(qfalse);

	CGX_CheckEnemyModelAll(qfalse);

	if (stats.needprint)
		CG_statsWindowPrint();
	else
		CG_statsWindowFree(0);

	//nemesis/osp reset stats
	memset(&stats, 0, sizeof(stats));

	D_Printf(("^6CGX_MapRestart\n"));
}

//send cmd with interval on servers with sv_floodprotect 1, in intermission skip
void CGX_SendClientCommand(char *command) {
	static int cmd_time;

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
		return;

	if (cgs.sv_floodProtect) {
		if (cg.time < cmd_time)
			return;

		cmd_time = cg.time + 1000;
	}

	trap_SendClientCommand(command);
}

// X-MOD: potential fix for q3config saving problem
void CGX_SaveSharedConfig(qboolean forced) {
	if (cg.q3version > 16) {
		CG_Printf("Shared config saving not supported by current q3 version");
		return;
	}

	if (cgx_sharedConfig.integer || forced) {
		char buf[32];

		trap_Cvar_Get("fs_game", buf);

		if (!buf[0]) {
			CG_Printf("Saving shared config... Mod: baseq3\n");
			trap_SendConsoleCommand("writeconfig q3config.cfg\n");
		}
		else {
			CG_Printf("Saving shared config... Mod: %s\n", buf);
			trap_SendConsoleCommand("writeconfig ..\\baseq3\\q3config.cfg\n");
		}
	}
	else {
		CG_Printf("Shared config saving is disabled (cg_sharedConfig)\n");
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

#pragma endregion

#pragma region map download

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

	if (res = CGX_FReadAll("xm\\win\\download_map.bat", txt, sizeof txt)) {
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

			CG_Printf(CGX_MAPBAT" generated successfully\n");
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

	CG_Printf("Generating "CGX_MAPBAT"...\n");

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

		CG_Printf(CGX_MAPBAT" generated successfully\n");

		if (answer) {			
			XMOD_ANSWER(va("Open folder %s", path));
			XMOD_ANSWER("And start ^2"CGX_MAPBAT);
		}
	}
	else {
		CG_Printf("^1Couldn't open a file "CGX_MAPBAT"\n");
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

#pragma endregion

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

//qboolean CGX_IsPure() {
//	qboolean isPure;
//	char buf[4];
//
//	trap_Cvar_Get("sv_pure", buf);
//	isPure = atoi(buf);
//	if (cgs.localServer && isPure && cgx_networkAdjustments.integer) {
//		trap_Cvar_Set("sv_pure", "0");
//		return isPure;
//	} else {
//		return cgs.isPureServer;
//	}
//}

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
	if (stristr(command, "fuck") || stristr(command, "suck") || stristr(command, "shit")) {
		return command;
	} else if (stristr(command, "hi ") || stristr(command, "hello")) {
		char *txt[] = { "hi, %s! how are you?", "hello!", "hey", "hi, %s!" };
		int num = rand() % ArrLen(txt);
		if (stristr(txt[num], "%s")) {
			char username[MAX_QPATH];
			trap_Cvar_Get(rand() % 1000 > 500 ? "username" : "name", username);
			return va(txt[num], username);
		}
		return txt[num];
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
	} else if (stristr(command, "haha") || stristr(command, "hehe") || stristr(command, "hehe") || stristr(command, "hhhh")) {
		char *txt[] = { "haha", "hehe", "hhhh", ":)" };
		return rand() % 1000 > 500 ? txt[rand() % ArrLen(txt)] : command;
	} else if (!Q_stricmp(command, ":) ") || !Q_stricmp(command, ":D ") || !Q_stricmp(command, "xD ")) {
		return command;
	}

	return NULL;
}

static void CGX_PrintLine(char c) {
	int i;	
	CG_Printf("^%c", c);
	for(i = 0; i < 20*3/10; i++)
		CG_Printf("^%c------------", c);
	CG_Printf("\n");
}

// find cmd with params and description, or list all
static int CGX_FindCmds(char *filename, char *cmd) {
	int	i = 0;
	char buf[1024 * 8];
	char *c, *d, *p, *e;
	char *ncmd;

	if (!CGX_FReadAll(filename, buf, sizeof buf))
		return 0;

	//start parse command list if read succesful
	e = buf;

	//list all
	if (!*cmd) {
_next_cmd:
		for (c = e; *c; c++) {
			if (*c == '\n' && Q_isalpha(*(c + 1)))
				break;
		}
		if (!*c) {
			CG_Printf("\n");
			return i;
		}
		c++;
		p = strchr(c, ' ');
		if (!*p) {
			CG_Printf("\n");
			return i;
		}
		*p = 0;
		e = p + 1;

		i++;
		CG_Printf("%-25s", c);
		goto _next_cmd;
	}
	
	ncmd = va("\n%s", cmd);

_find_next_cmd:
	//find cmd
	c = stristr(e, ncmd);

	if (!*c)
		return i;

	c++;

	//find params
	for (p = c + 1; *p; p++) {
		if (*p == ' ') {
			if (*(p + 1) == '-' && *(p + 2) == ' ') {
				p = "^"; //hack to remove one space

				d = c + 1;
			} else {
				*p = 0;
				p++;

				d = p + 1;
			}
			break;
		}
	}

	//find descr
	for (; *d; d++)
		if (*d == '-' && *(d - 1) == ' ')
			break;

	if (!*d)
		return i;

	*(d - 1) = 0;
	d++;
	d++;

	//find end of descr
	for (e = d + 1; *e; e++)
		if (*e == '\n' && (Q_isalpha(*(e + 1)) || *(e + 1) == '\r'))
			break;

	if (!*e)
		return i;

	*(e - 1) = 0;

	//colorize colorlist
	if (!Q_stricmp(c, "colorlist")) {
		char *x;
		for (x = d; *x; x++) {
			if (*x == ' ' && *(x + 1) == ' ') {
				*x = '^';
				*(x + 1) = *(x + 2);
			}
		}
	}

	i++;
	CG_Printf("^3%s ^2%s ^7- ^5%s\n", c, p, d);
	goto _find_next_cmd;
}

//parse info from file
#define help_file1 "doc\\2-comand_list.txt"
#define help_file2 "doc\\3-unlagged_commands.txt"
static void CGX_ShowHelp(char *cmd) {
	static qboolean exampleShown;
	
	//if no command show all 
	if (!*cmd) {
		//parse command list from file
		XMOD_ANSWER("known command list");
		CGX_PrintLine(COLOR_YELLOW);
		CG_Printf("%-25s", "cg_delag");
		CGX_FindCmds(help_file1, "");
		CGX_PrintLine(COLOR_YELLOW);
		XMOD_ANSWER("for detailed info type: \\xmod <command>");
		//show example
		if (!exampleShown) {
			XMOD_ANSWER("example: \\xmod cg_enemy"); 
			XMOD_ANSWER("'Page Up' 'Page Down' - scroll, 'Tab' - autocomplete");
			XMOD_ANSWER("'Up' 'Down' - input history");
			exampleShown = qtrue;
		}
	} else {
		//find info abt command in files
		int i = 0;

		i += CGX_FindCmds(help_file1, cmd);
		i += CGX_FindCmds(help_file2, cmd);

		//zero matches
		if (!i) {
			char str[128]; char *s;
			trap_Args(str, sizeof str);
			
			if (s = CGX_XmodTalk(str))
				XMOD_ANSWER(s)
			else
				XMOD_ANSWER(va("unknown cmd '%s'", cmd))
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

//weird bug when desktop has 60hz and game trying to set 144hz, picture becomes darker (with r_overbrightbits 1)
//toggling gamma fixing it
//another bug it's not always sets 144hz, vid_restart helps
static void CGX_GammaFix() {
	char buf[4];

	trap_Cvar_Get("r_gamma", buf);
	trap_SendConsoleCommand(va("r_gamma 0;r_gamma %s\n", buf));
}

static void CGX_PrintClients() {
	int i;  clientInfo_t *ci; char clname[MAX_QPATH];
	for (i = 0, ci = cgs.clientinfo; i < MAX_CLIENTS; i++, ci++)
		if (*ci->modelName && *ci->skinName) {
			Q_strncpyz(clname, ci->name, sizeof clname);
			CG_Printf("%3i %20s %s/%s %s/%s\n", i, Q_CleanStr(clname), ci->modelName, ci->skinName, ci->modelNameCopy, ci->skinNameCopy);
		}
}

//method to reload effects, used to call reload from UI
static void CGX_ReloadEffects() {
	char buf[32];
	// get cvar this way, in case if menu changed it and sent command
	trap_Cvar_Get("cg_weaponEffects", buf);

	if (atoi(buf) & WE_LG32)
		trap_R_RegisterShaderCGXNomip(cgs.media.lightningShader, "lightningBoltNew")
	else
		trap_R_RegisterShaderCGXNomip(cgs.media.lightningShader, "lightningBolt")
}

#define cmd_is(x) !Q_stricmp(command, x)
#define arg_is(x) !Q_stricmp(arg, x)
static void CGX_PrintModelCache();
static void CGX_PrintFonts();
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

	if (cmd_is("e")) {
		XMOD_ANSWER("checking enemy models...");
		CGX_CheckEnemyModelAll(qtrue);
		return;
	} 

	i = strlen(command);
	if (i && i < 2) {
		XMOD_ANSWER("too short cmd");		
		return;
	}

#if CGX_DEBUG
	if (cmd_is("float")) {
		float f;
		int p = abs(atoi(arg));
		i = 1;
		while (p--) i *= 10;
		for (f = 0.0f; f <= 0.001f * i; f += 0.00001f * i)
			CG_Printf("%f\n", f);
	} else if (cmd_is("crandom")) {
		for (i = 0; i < 100; i++)
			CG_Printf("%1.2f ", crandom());
		CG_Printf("\n");
		return;
	} else
	if (cmd_is("eFlags")) {
		centity_t *cent = &cg_entities[cg.clientNum];

		CG_Printf("%i\n%i\n", cent->currentState.eFlags, cg.snap->ps.eFlags);
	} else if (cmd_is("stats")) {
		for (i = 0; i < MAX_STATS; i++)
			CG_Printf("%i ", cg.snap->ps.stats[i]);
		CG_Printf("\n");
	} else if (cmd_is("pers")) {
		for (i = 0; i < MAX_PERSISTANT; i++)
			CG_Printf("%i ", cg.snap->ps.persistant[i]);
		CG_Printf("\n");
	} else if (cmd_is("powerups")) {
		for (i = 0; i < MAX_POWERUPS; i++)
			CG_Printf("%i ", cg.snap->ps.powerups[i]);
		CG_Printf("\n");
	} else if (cmd_is("ammo")) {
		for (i = 0; i < MAX_WEAPONS; i++)
			CG_Printf("%i ", cg.snap->ps.ammo[i]);
		CG_Printf("\n");
	} else if (cmd_is("css")) {
		for (i = 0; i < MAX_CONFIGSTRINGS; i++) {
			const char *cs = CG_ConfigString(i);
			if (*cs) CG_Printf("%i %s\n", i, cs);
		}
	} else if (cmd_is("ccss")) {
		for (i = 0; i < MAX_CLIENTS; i++) {
			const char *cs = CG_ConfigString(CS_PLAYERS + i);
			if (*cs) CG_Printf("%i %s\n", i, cs);
		}
	} else 
#endif

	if (cmd_is("version")) {
		XMOD_ANSWER(CGX_FULLVER" "CGX_DATE);
	} else if (cmd_is("help")) {
		CGX_ShowHelp("");
	} else if (cmd_is("freemem")) {
		CG_Printf("%i Mb\n", trap_MemoryRemaining() / 1024 / 1024);
	} else if (cmd_is("models")) {
		CGX_PrintModelCache();
	} else if (cmd_is("skins")) {
		CGX_PrintSkinCache();
	} else if (cmd_is("clients")) {
		CGX_PrintClients();
	} else if (cmd_is("fonts")) {
		CGX_PrintFonts();
	} else if (cmd_is("modinfo")) {
		CGX_SendModinfo(qtrue);
	} else if (cmd_is("reload")) {
		if (arg_is("effects"))
			CGX_ReloadEffects();
		else if (arg_is("hud"))
			CGX_LoadHUD(cgx_hud.string);
		else 
			CG_Printf("usage: \\xmod reload <effects|hud>\n");
	} else if (cmd_is("stats")) {
		CG_statsWindowPrint();
	} else if (cmd_is("gammafix")) {
		CGX_GammaFix();
	} else if (stristr(command, "pk3")) {
		CGX_Pk3list_f();
	} else if (cmd_is("ragequit")) {
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
	} else if (cmd_is("coin")) {
		XMOD_ANSWER(rand() % 100 >= 50 ? "true": "false");
	} else {
		CGX_ShowHelp(command);
	}
}

#pragma endregion

#pragma region Xmod I/O

//safety open file, -1 unlim size
static int CGX_FOpenFile(char *filename, fileHandle_t *f, fsMode_t mode, int maxSize) {
	int len;
	len = trap_FS_FOpenFile( filename, f, mode );

	if ( len <= 0 ) {
		CG_Printf( S_COLOR_YELLOW "File not found: %s\n", filename );
		return 0;
	}
	if ( len > maxSize && maxSize != -1) {
		CG_Printf( S_COLOR_RED "File too large: %s is %i, max allowed is %i\n", filename, len, maxSize );
		trap_FS_FCloseFile( *f );
		return 0;
	}

	return len;
}

//read all binary data
static int CGX_FReadAllBinary(char *filename, void *buffer, int bufsize) {
	int len;
	fileHandle_t	f;

	if (len = CGX_FOpenFile(filename, &f, FS_READ, bufsize)) {
		trap_FS_Read(buffer, len, f);
		trap_FS_FCloseFile(f);
	}

	return len;
}

//read all text and close
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
		qboolean showLowMem = qfalse;
		static int show_time = 0;
		
		if (trySkinLoads) {
			D_Printf(("trySkinLoads "));
			//if have some mem try skin load, it takes 3-5kb
			if (mem > LOW_MEMORY / 5 && CGX_TrySkinLoad(ci))
				return qtrue;
			D_Printf(("^3NoModel... "));
		}

		if (lowMem && trySkinLoads && show_time < cg.time) {
			showLowMem = qtrue;
			show_time = cg.time + 60 * 1000;
		}

		for (i = modelCacheNum, mi = modelCache; i--; mi++)
			if (cgs.gametype < GT_TEAM || !Q_stricmp(ci->skinName, mi->skinName)) {
				if (cgx_enemyModel_enabled.integer && !Q_stricmp(mi->modelName, cg.enemyModel))
					continue;
			
				D_Printf(("^4Got any %i %s/%s\n", i, mi->modelName, mi->skinName));

				CGX_LoadModelFromCache(ci, mi);
#if !CGX_DEBUG
				if (showLowMem)
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

#pragma region Xmod HUD

//x-mod hud
//inspired by aftershock, aftershock-xe & revolution quake source codes

//just for info about this trap_R_SetColor( NULL ) on function exits - it's needed.
//because if not doing this then when calling UI, it could start using
//previously set color in cgame, and no way to reset it in UI.

//TODO (not important): 
//e+ outline
//color fading

static float	stored_scalex;
static float	stored_scaley;
static int		stored_3dicons;

xhudElem_t *fi; //font info

#define XH_Set3DIcons(x) stored_3dicons = cg_draw3dIcons.integer; cg_draw3dIcons.integer = x;
#define XH_Restore3DIcons() cg_draw3dIcons.integer = stored_3dicons;

#define XH_SetFont(e) fi = e
#define XH_RestoreFont()

#define XH_SetScale640() stored_scaley = cgs.screenYScale; stored_scalex = cgs.screenXScale; cgs.screenXScale = 1; cgs.screenYScale = 1;
#define XH_RestoreScale() cgs.screenXScale = stored_scalex; cgs.screenYScale = stored_scaley;

#define HUD_SYNTAX_CPMA 1
#define HUD_SYNTAX_XP 2 << 0
#define HUD_SYNTAX_AS 2 << 1
//#define HUD_SYNTAX_NMS 2 << 3

//textstyle
#define TS_SHADOW 1
#define TS_FORCECOLOR 2
#define TS_LITE 4
#define TS_OUTLINE 8

#define XH_WEAPLIST_PAD 4 //weapon list pad

static vmCvar_t cgx_hud_TEColors; //T E color values: "2417" FFA BLUE RED SPECT
static vmCvar_t	cgx_hud_font16threshold; //font 16 threshold in pixels

extern void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
extern void CG_Draw3DModelColor(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles, byte *color, qhandle_t shader);

static int XH_IntLen(int i) {
	int r = 0;
	if (i < 0) { i *= -1; r++; }
    if (i >= 1000) return r + 4;
    if (i >= 100) return r + 3;
    if (i >= 10) return r + 2;
    return r + 1;
}

//anchors
#define XA_TOP 1
#define XA_RIGHT 2
#define XA_BOT 4
#define XA_LEFT 8

#define AdjX640(x) x *= cgs.screenXScale640
#define AdjY640(y) y *= cgs.screenYScale

float FromX640(float x) { return x * cgs.screenXScale640; }
float FromY640(float y) { return y * cgs.screenYScale; }

float ToX640(float x) { return x / cgs.screenXScale640; }
float ToY640(float y) { return y / cgs.screenYScale; }

static void XH_FillRect( float x, float y, float w, float h, const float *color ) {
	trap_R_SetColor( color );

	if (w < 0) { w = -w; x -= w; }
	if (h < 0) { h = -h; y -= h; }

	trap_R_DrawStretchPic( x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

static void XH_DrawRect(float x, float y, float w, float h, float lineWidth, const float *color) {
	float xs = FromX640(lineWidth);
	float ys = FromY640(lineWidth);

	trap_R_SetColor(color);

	if (w < 0) { w = -w; x -= w; }
	if (h < 0) { h = -h; y -= h; }

	trap_R_DrawStretchPic(x, y, xs, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x + w - xs, y, xs, h, 0, 0, 0, 0, cgs.media.whiteShader);

	trap_R_DrawStretchPic(x, y, w, ys, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_DrawStretchPic(x, y + h - ys, w, ys, 0, 0, 0, 0, cgs.media.whiteShader);

	trap_R_SetColor(NULL);
}

static char *xhudElemNames[] = {
	"!DEFAULT",
	"AmmoMessage",
	"AttackerIcon",
	"AttackerName",
	"Chat1",
	"Chat2",
	"Chat3",
	"Chat4",
	"Chat5",
	"Chat6",
	"Chat7",
	"Chat8",
	"Console",
	"FlagStatus_NME",
	"FlagStatus_OWN",
	"FollowMessage",
	"FPS",
	"FragMessage",
	"GameTime",
	"GameType",
	"ItemPickup",
	"ItemPickupIcon",
	//"KeyDown_Back",
	//"KeyDown_Crouch",
	//"KeyDown_Forward",
	//"KeyDown_Jump",
	//"KeyDown_Left",
	//"KeyDown_Right",
	//"KeyUp_Back",
	//"KeyUp_Crouch",
	//"KeyUp_Forward",
	//"KeyUp_Jump",
	//"KeyUp_Left",
	//"KeyUp_Right",
	//"LocalTime",
	//"MultiView",
	"NetGraph",
	"NetGraphPing",
	"PlayerSpeed",
	"PlayerAccuracy",
	"StatusBar_Flag",
	"PowerUp1_Icon",
	"PowerUp2_Icon",
	"PowerUp3_Icon",
	"PowerUp4_Icon",
	"PowerUp5_Icon",
	"PowerUp6_Icon",
	"PowerUp7_Icon",
	"PowerUp8_Icon",
	"PowerUp1_Time",
	"PowerUp2_Time",
	"PowerUp3_Time",
	"PowerUp4_Time",
	"PowerUp5_Time",
	"PowerUp6_Time",
	"PowerUp7_Time",
	"PowerUp8_Time",
	"RankMessage",
	"Score_Limit",
	"Score_NME",
	"Score_OWN",
	"SpecMessage",
	"StatusBar_ArmorCount",
	"StatusBar_ArmorIcon",
	"StatusBar_AmmoCount",
	"StatusBar_AmmoIcon",
	"StatusBar_HealthCount",
	"StatusBar_HealthIcon",
	"TargetName",
	"TargetStatus",
	"TeamCount_NME",
	"TeamCount_OWN",
	"TeamIcon_NME",
	"TeamIcon_OWN",
	"Team1",
	"Team2",
	"Team3",
	"Team4",
	"Team5",
	"Team6",
	"Team7",
	"Team8",
	"VoteMessageArena", //VoteMessageArena added just to skip unknown element errors
	"VoteMessageWorld",
	"WarmupInfo",
	"WeaponList",
	"PreDecorate",
	"PostDecorate",
	"StatusBar_HealthBar",
	"StatusBar_ArmorBar",
	"StatusBar_AmmoBar",
};

//element redirects to CPMA syntax
struct {
	const char	*name;
	int	elNum;
} xhudExtraElems[] =  {
	//aftershock
	{ "ItemPickupName", XH_ItemPickup },
	//{ "ItemPickupTime", 0 },
	{ "VoteMessage", XH_VoteMessageWorld },
	{ "TeamOverlay1", XH_Team1 },
	{ "TeamOverlay2", XH_Team2 },
	{ "TeamOverlay3", XH_Team3 },
	{ "TeamOverlay4", XH_Team4 },
	{ "TeamOverlay5", XH_Team5 },
	{ "TeamOverlay6", XH_Team6 },
	{ "TeamOverlay7", XH_Team7 },
	{ "TeamOverlay8", XH_Team8 },
#if HUD_SYNTAX_NMS
	//nms
	{ "AMMO_MESSAGE", XH_AmmoMessage },
	{ "ARMOR", XH_StatusBar_ArmorCount },
	{ "ARMOR_BAR", XH_StatusBar_ArmorBar },
	{ "ATTACKER_NAME", XH_AttackerName },
	//{ "CENTERPRINT",  },
	{ "CHAT_1", XH_Chat1 },
	{ "CHAT_2", XH_Chat2 },
	{ "CHAT_3", XH_Chat3 },
	{ "CHAT_4", XH_Chat4 },
	{ "CHAT_5", XH_Chat5 },
	{ "CHAT_6", XH_Chat6 },
	{ "CHAT_7", XH_Chat7 },
	{ "CHAT_8", XH_Chat8 },
	//{ "CONNECTION_INTERRUPTED",  },
	//{ "COUNTDOWN",  },
	{ "CROSSHAIR_NAME", XH_TargetName },
	//{ "ENEMY_COUNT",  },
	//{ "FOLLOW_MESSAGE_1",  },
	//{ "FOLLOW_MESSAGE_2",  },
	//{ "FPS",  },
	//{ "GAMETYPE",  },
	{ "HEALTH", XH_StatusBar_HealthCount },
	{ "HEALTH_BAR", XH_StatusBar_HealthBar },
	//{ "HOLDABLE_ICON",  },
	//{ "NAME",  },
	{ "LAGOMETER", XH_NetGraph },
	{ "PICKUP_ICON", XH_ItemPickupIcon },
	{ "PICKUP_NAME", XH_ItemPickup },
	{ "POWERUP_ICON_1", XH_PowerUp1_Icon },
	{ "POWERUP_ICON_2", XH_PowerUp2_Icon },
	{ "POWERUP_TIMER_1", XH_PowerUp1_Time },
	{ "POWERUP_TIMER_2", XH_PowerUp2_Time },
	//{ "SCORE_LIMIT",  },
	{ "SCORE_TEAM", XH_Score_OWN },
	{ "SCORE_ENEMY", XH_Score_NME },
	{ "SPEED", XH_PlayerSpeed },
	//{ "SPEED_BAR",  },
	{ "TEAMOVERLAY_1", XH_Team1 },
	{ "TEAMOVERLAY_2", XH_Team2 },
	{ "TEAMOVERLAY_3", XH_Team3 },
	{ "TEAMOVERLAY_4", XH_Team4 },
	{ "TEAMOVERLAY_5", XH_Team5 },
	{ "TEAMOVERLAY_6", XH_Team6 },
	{ "TEAMOVERLAY_7", XH_Team7 },
	{ "TEAMOVERLAY_8", XH_Team8 },
	//{ "TEAM_COUNT",  },
	{ "TIMER", XH_GameTime },
	//{ "VERSUS_MESSAGE",  },
	{ "VOTE_MESSAGE", XH_VoteMessageWorld },
	//{ "WAITING_FOR_PLAYERS",  },
	{ "WEAPON_AMMO", XH_StatusBar_AmmoCount },
	//{ "WEAPON_AMMO_ICON", XH_StatusBar_AmmoIcon },
	{ "WEAPON_ICON", XH_StatusBar_AmmoIcon },
	{ "WEAPON_LIST", XH_WeaponList }
#endif
};

//pool for strings
//#define CGX_POOLSIZE 1024 * 2
//
//static char	memoryPool[CGX_POOLSIZE];
//static int	allocPoint = 0, outOfMemory = qfalse;
//
//void *CGX_Alloc(int size) 
//	char	*p;
//
//	if (allocPoint + size > CGX_POOLSIZE) {
//		outOfMemory = qtrue;
//		return NULL;
//	}
//
//	p = &memoryPool[allocPoint];
//
//	allocPoint += (size + 31) & ~31;
//
//	return p;
//}

#define XH_ELEM_POOL_START XH_MAX_STATIC_HUD_ELEMS
#define XH_ELEM_POOL_END XH_MAX_STATIC_HUD_ELEMS + XH_ELEM_POOL

xhudElem_t xhud[XH_TOTAL_ELEMS]; //hud elements array

static int elemsPoolNum = XH_ELEM_POOL_START;

//allocating element in pool
static xhudElem_t *XH_AllocElem(xhudElemType_t type) {
	xhudElem_t *res;

	if (type < XH_MAX_STATIC_HUD_ELEMS) {
		res = &xhud[type];
	} else {
		if (elemsPoolNum == XH_ELEM_POOL_END)
			elemsPoolNum = XH_ELEM_POOL_START;

		res = &xhud[elemsPoolNum++];
	}

	if (type > 0)
		memcpy(res, &xhud[0], sizeof xhud[0]);

	res->type = type;

	if (type == XH_StatusBar_HealthIcon)
		res->flags |= XF_PLAYER_HEAD;

	return res;
}

static cw_info_t cwi_pool[8]; //holds char width ratio & size_x for different fonts

//register character width file and predefine character width ratio & size_x values
static cw_info_t *XH_Register_CW(qhandle_t hFont, char *fontName) {
	int i, j;
	char cw[256], xp[520];
	cw_info_t *cwi;
	char *fontFile;
	float w;
	int registerFontType; //1 - cpma 2 - xp

	if (!hFont)
		return NULL;

	for (i = 0, cwi = cwi_pool; i < ArrLen(cwi_pool); i++, cwi++)
		if (cwi->font == hFont) {
			//CG_Printf("XH_Register_CW fonud %s %i\n", fontName, i);
			return cwi;
		}

	fontFile = va("fonts/%s.cw", fontName);
	if (CGX_FExists(fontFile)) {
		registerFontType = 1;
	} else {
		fontFile = va("fonts/%s.font", fontName);
		if (CGX_FExists(fontFile)) {
			registerFontType = 2;
		} else {
			return NULL;
		}
	}

	for (i = 0, cwi = cwi_pool; i < ArrLen(cwi_pool); i++, cwi++)
		if (!cwi->font)
			if (registerFontType == 1) {
				if (CGX_FReadAllBinary(fontFile, cw, sizeof cw)) {
					Q_strncpyz(cwi->fontName, fontName, sizeof cwi->fontName);
					cwi->font = hFont;
					cwi->avg_cwr = 0;
					for (j = 0; j < ArrLen(cw); j++) {
						float f = cw[j];
						cwi->cwr[j] = f / 32.0f;
						cwi->szx[j] = f / 512.0f;

						if (j < ArrLen(cw) / 2)
							cwi->avg_cwr += cwi->cwr[j];
					}
					cwi->avg_cwr = cwi->avg_cwr / (ArrLen(cw) / 2);
					//CG_Printf("XH_Register_CW registered %s %i\n", fontName, i);
					break;
				} else {
					//unexpected error
					return NULL;
				}
			} else if (registerFontType == 2) { //works not exatly like in e+, but fine
				if (CGX_FReadAllBinary(fontFile, xp, sizeof xp)) {
					Q_strncpyz(cwi->fontName, fontName, sizeof cwi->fontName);
					cwi->font = hFont;
					cwi->xp = qtrue;
					cwi->avg_cwr = 0;
					for (j = 0; j < ArrLen(cw); j++) {
						float f = (float)xp[j * 2 + 8];
						//float f2 = (float)xp[j * 2 + 9]; //for what this field - no idea
						if (f >= 16) //space fix
							f = 12;
						cwi->cwr[j] = (16.0f - f) / 16.0f;
						cwi->szx[j] = (f) / 512.0f;

						if (j < ArrLen(cw) / 2)
							cwi->avg_cwr += cwi->cwr[j];
					}
					cwi->avg_cwr = cwi->avg_cwr / (ArrLen(cw) / 2);
					//CG_Printf("XH_Register_CW registered %s %i\n", fontName, i);
					break;
				} else {
					//unexpected error
					return NULL;
				}
			}

	if (i == ArrLen(cwi_pool)) {
		CG_Printf("XH_Register_CW pool reached max value\n");
		return NULL;
	}

	return cwi;
}

static void CGX_PrintFonts() {
	int i; cw_info_t *cwi;

	for (i = 0, cwi = cwi_pool; i < ArrLen(cwi_pool); i++, cwi++)
		if (*cwi->fontName)
			CG_Printf("%i %s\n", i, cwi->fontName);
}

static void XH_Clear() {
	memset(&xhud, 0, sizeof xhud);
	memset(&cwi_pool, 0, sizeof cwi_pool);

	elemsPoolNum = XH_ELEM_POOL_START;

	hud.file = qfalse;

	hud.chatHeight = hud.chatPos = hud.chatLastPos = 0;
	hud.conHeight = hud.conPos = hud.conLastPos = 0;
	hud.powerupsMaxNum = 0;
	hud.overlayMaxNum = 0;
	hud.oe = hud.ot = -1;

	trap_Cvar_Set("con_notifytime", "3");
}

#if HUD_SYNTAX_NMS
static void XH_DrawBorder(xhudElem_t *e) {
	if (e->flags & XF_BORDER)
		XH_DrawRect(e->x, e->y, e->w, e->h, e->border_w, e->bordercolor);
}
#else
#define XH_DrawBorder(x)
#endif

//based on UI_DrawHandlePic
static void XH_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w = -w;
		x -= w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h = -h;
		y -= h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

//this method is brainfuck
static void XH_DrawBarEx(float x, float y, float w, float h, int value, int maxValue, const float *color, xhudElem_t *e) {
	qhandle_t shader = e->img;
	qboolean reverse = e->textalign == 'R';
	qboolean vertical = e->flags & XF_VERTICALBAR;
	float s0, s1, t0, t1;
	float fw, fh, rw, rh, r;
	vec4_t finalColor;

	r = (float)value / maxValue;

	if (r > 1.0f)
		r = 1.0f;

	if( w < 0 ) {	// flip about vertical
		w = -w;
		x -= w;
		s0 = 1;
		s1 = 0;
	} else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h = -h;
		y -= h;
		t0 = 1;
		t1 = 0;
	} else {
		t0 = 0;
		t1 = 1;
	}

	fw = w; //final width & height
	fh = h;

	if (vertical) {
		fh *= r;

		rh = fh / h;
		rw = fw / w;

		if (!reverse)
			y += h - fh;

		if (y < h * 2) {
			t0 *= rh;
			t1 *= rh;
		} else {
			t0 = 1 - (1 - t0) * rh;
			t1 = 1 - (1 - t1) * rh;
		}
	} else {
		fw *= r;

		rh = fh / h;
		rw = fw / w;

		if (reverse)
			x += w - fw;

		t0 *= rh;
		t1 *= rh;
	}

	s0 *= rw;
	s1 *= rw;

	if (e->color[3]) {
		color = e->color;
	} else if (e->alpha) {
		VectorCopy(color, finalColor);
		finalColor[3] = e->alpha;
		color = finalColor;
	} else {
		VectorCopy(color, finalColor);
		finalColor[3] = 0.7f;
		color = finalColor;
	}

	if (!shader) {
		shader = cgs.media.whiteShader;
#if 0
		if (shadow) {
			vec4_t shadowColor = { 0, 0, 0, 0 };
			shadowColor[3] = color[3];
			trap_R_SetColor(shadowColor);
			trap_R_DrawStretchPic(x + 1.0f, y + 1.0f, fw, fh, s0, t0, s1, t1, shader);
		}
#endif
	}

	trap_R_SetColor(color);
	trap_R_DrawStretchPic(x, y, fw, fh, s0, t0, s1, t1, shader);
	trap_R_SetColor(NULL);
}

static void XH_DrawBar(xhudElem_t *e, int value, int maxValue, const float *color) {
	if (e->flags & XF_DOUBLEBAR) {
		int halfMax = maxValue / 2;
		float hh = e->h / 2;
		float pad = hud.bar_pad;

		if (value > 0)
			XH_DrawBarEx(e->x, e->y, e->w, hh - pad,
				value, halfMax, color, e);
		if (value > halfMax)
			XH_DrawBarEx(e->x, e->y + hh + pad, e->w, hh - pad,
				value - halfMax, halfMax, color, e);
	} else {
		XH_DrawBarEx(e->x, e->y, e->w, e->h, value, maxValue, color, e);
	}

	XH_DrawBorder(e);
}

void CGX_DrawBar(int elNum, int value, int maxValue, const float *color) {
	int i;

	if (value <= 0 || maxValue <= 0)
		return;

	if (elNum >= XH_StatusBar_HealthBar && elNum <= XH_StatusBar_AmmoBar) {
		xhudElem_t *e = &xhud[XH_ELEM_POOL_START];
		for (i = XH_ELEM_POOL_START; i < elemsPoolNum; i++, e++) {
			if (e->type == elNum && e->inuse) {
				if (e->flags & XF_PARAM_1) {
					int halfMax = maxValue / 2;
					if (value > halfMax)
						XH_DrawBar(e, value - halfMax, halfMax, color);
				} else if (e->flags & XF_PARAM_2) {
					int halfMax = maxValue / 2;
					if (value > 0)
						XH_DrawBar(e, value, maxValue / 2, color);
				} else {
					XH_DrawBar(e, value, maxValue, color);
				}
			}
		}
	}
}

static void XH_DrawHead(xhudElem_t *e, int clientNum, vec_t *angles) {
	trap_R_SetColor(NULL);
	XH_SetScale640();
	CG_DrawHead(e->x, e->y, e->w, e->h, clientNum, angles);
	XH_RestoreScale();
}

static void XH_Draw3DModelColor(xhudElem_t *e, vec3_t offset, vec3_t angles, const float *rgba) {
	byte color[4];
	vec3_t mins, maxs, origin;
	float len;
	int i;

	trap_R_ModelBounds( e->model, mins, maxs );

	origin[2] = -0.5f * ( mins[2] + maxs[2] );
	origin[1] = 0.5f * ( mins[1] + maxs[1] );

	len = 0.7 * ( maxs[2] - mins[2] );		
	origin[0] = len / 0.268f;

	VectorAdd( origin, offset, origin );

	for (i = 4; i--; )
		color[i] = rgba[i] * 0xFF;

	XH_Set3DIcons(1);
	XH_SetScale640();
	CG_Draw3DModelColor(e->x, e->y, e->w, e->h, e->model, e->img, origin, angles, color, 0);
	XH_RestoreScale();
	XH_Restore3DIcons();
}

void CGX_DrawHead(int elNum, int clientNum, vec_t *angles) {
	xhudElem_t *e = &xhud[elNum];
	
	if (!e->inuse)
		return;

	XH_DrawHead(e, clientNum, angles);
	XH_DrawBorder(e);
}

void CGX_DrawFlagModel(int elNum, int team) {
	xhudElem_t *e = &xhud[elNum];

	if (!e->inuse)
		return;

	XH_SetScale640();
	CG_DrawFlagModel(e->x, e->y, e->w, e->h, team);
	XH_RestoreScale();
}

static void XH_CalcAnglesRotation(xhudElem_t *e) {
	if (e->angles[3] < 0)
		e->angles[YAW] = (cg.time & 2047) * -e->angles[3] / 2048.0f;
	else if (e->angles[3] > 0)
		e->angles[YAW] = e->angles_initial[YAW] + e->angles[3] * sin(cg.time / 1000.0);
}

void CGX_Draw3DModel(int elNum, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles) {
	xhudElem_t *e = &xhud[elNum];

	if (!e->inuse)
		return;

	if (e->flags & XF_ANGLES) {
		VectorCopy(e->angles, angles);
		XH_CalcAnglesRotation(e);
	}

	XH_SetScale640();
	XH_Set3DIcons(cg_draw3dIcons.integer || e->flags & XF_DRAW3D);
	CG_Draw3DModel(e->x, e->y, e->w, e->h, model, skin, origin, angles);
	XH_Restore3DIcons();
	XH_RestoreScale();
}

extern const float drawchar_coords[];

static float XH_DrawChar_Handle( float xx, float yy, float ww, float hh, qhandle_t curfont, char ch ) {
	int row, col;
	float frow, fcol;
	float size_y, size_x;

	if ( ch == ' ' ) {
		if (fi->cwi != NULL && !(fi->flags & XF_MONOSPACE))
			ww *= fi->cwi->cwr[ch];

		return ww;
	}

	row = ch>>4;
	col = ch&15;

	frow = drawchar_coords[row];
	fcol = drawchar_coords[col];
	size_y = 0.0625;

	if (fi->cwi != NULL) {
		//cpma font letters adjusted to left of 32x32

		size_x = fi->cwi->szx[ch];

		if (fi->flags & XF_MONOSPACE) {
			//adjust to center if cpma font
			if (!fi->cwi->xp) {
				float wo = ww * (1 - fi->cwi->cwr[ch]);
				float xo = wo / 2;

				trap_R_DrawStretchPic( xx + xo, yy, ww - wo, hh,
					fcol, frow, 
					fcol + size_x, frow + size_y, 
					curfont );

				return ww;
			}
		}

		ww *= fi->cwi->cwr[ch];

		//xp font letters in the middle of 32x32
		if (fi->cwi->xp) {
			frow -= 1.0f / 512.0f;//xp font's looks not very well vertically aligned without it...
			if (frow < 0)
				frow = 0;

			fcol += size_x;
			size_x = 0.0625 - size_x * 2;
		}
	} else {
		size_x = 0.0625;
	}

	trap_R_DrawStretchPic( xx, yy, ww, hh,
					fcol, frow, 
					fcol + size_x, frow + size_y, 
					curfont );

	return ww;
}

static float XH_DrawChar_FieldFont( float xx, float yy, float ww, float hh, qhandle_t curfont, char ch ) {
	int frame;

	frame = ch >= '0' && ch <= '9' ? ch - '0' : STAT_MINUS;
	trap_R_DrawStretchPic( xx, yy, ww, hh, 0, 0, 1, 1, cgs.media.numberShaders[frame] );

	return ww;
}

static float(*XH_DrawChar)(float x, float y, float w, float h, qhandle_t curfont, char c) = &XH_DrawChar_Handle;

static void XH_DrawStringExt( float x, float y, const char *string, const float *setColor, 
		int textstyle, float shadowSize, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			cnt;
	float xx, yy, ww, hh;
	qboolean forceColor = textstyle & TS_FORCECOLOR;
	qhandle_t curfont;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	ww = charWidth;
	hh = charHeight;

	if (charHeight < cgx_hud_font16threshold.integer && charWidth < cgx_hud_font16threshold.integer) {
		curfont = fi->font;
	} else {
		curfont = fi->font32;
	}

	// draw the drop shadow
	if (shadowSize) {
		float sw, sh, sb;
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor( color );
		s = string;
		xx = x + shadowSize;
		yy = y + shadowSize;
		cnt = 0;
		while ( *s && cnt < maxChars) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			cnt++;
			xx += XH_DrawChar( xx, yy, ww, hh, curfont, *s );
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	yy = y;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				trap_R_SetColor( color );
			}
			s += 2;
			continue;
		}
		xx += XH_DrawChar( xx, yy, ww, hh, curfont, *s );
		cnt++;
		s++;
	}

	trap_R_SetColor( NULL );
}

static int XH_DrawStrWidth(xhudElem_t *e, const char *str) {
	const char *s = str;
	int width = 0;

	if (e->cwi == NULL || e->flags & XF_MONOSPACE) {
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
			} else {
				width += e->font_w;
				s++;
			}
		}
	} else {
		while ( *s ) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
			} else {
				width += e->font_w * e->cwi->cwr[*s];
				s++;
			}
		}
	}

	return width;
}

static float XH_GetAlignedXDrawStr(xhudElem_t *e, const char *s) {
	switch (e->textalign) {
		default:
		case 'L': return e->x;
		case 'R': return e->x + e->w - XH_DrawStrWidth(e, s);
		case 'C': return e->x + (e->w - XH_DrawStrWidth(e, s)) / 2;
	}
}

static float XH_GetAlignedYDrawStr(xhudElem_t *e) {
	switch (e->valign) {
		default:
		case 'T': return e->y;
		case 'B': return e->y + e->h - e->font_h;
		case 'C': return e->y + (e->h - e->font_h) / 2;
	}
}

//some weird fill logic
static qboolean XH_DrawFill(xhudElem_t *e) {
	if (!e->bgcolor[3])
		return qfalse;

	if (e->flags & XF_FILL) {
		if (e->flags & XF_E_COLOR || e->flags & XF_T_COLOR) {
			vec4_t fill_c;
			VectorCopy(e->color, fill_c);
			fill_c[3] = e->bgcolor[3];
			XH_FillRect(e->x, e->y, e->w, e->h, fill_c);

			return qtrue;
		}

		XH_FillRect(e->x, e->y, e->w, e->h, e->bgcolor);

		return qtrue;
	}

	return qfalse;
}

static void XH_DrawString(xhudElem_t *e, const char *s, const float *rgba) {
	int x, y;
	qboolean f;
	vec4_t col;

	if (XH_DrawFill(e))
		rgba = colorWhite;

	x = XH_GetAlignedXDrawStr(e, s);
	y = XH_GetAlignedYDrawStr(e);

	XH_SetFont(e);

	if (e->flags & XF_FIELDFONT) {
		XH_DrawChar = &XH_DrawChar_FieldFont;
		XH_DrawStringExt( x, y, s, rgba, e->textstyle, e->shadowsize, e->font_w, e->font_h, 0 );
		XH_DrawChar = &XH_DrawChar_Handle;
	} else {
		XH_DrawStringExt(x, y, s, rgba, e->textstyle, e->shadowsize, e->font_w, e->font_h, 0);
	}

	XH_RestoreFont();
}

void CGX_DrawString( int elNum, const char *s, float alpha ) {
	xhudElem_t *e = &xhud[elNum];
	vec4_t color;
	float *col;

	if (!e->inuse)
		return;

	if (alpha) {
		Vector4Copy(e->color, color);
		color[3] = alpha;
		col = color;
	} else {
		col = e->color;
	}

	XH_DrawString(e, s, col);
	XH_DrawBorder(e);
}

#if 0
float *CGX_FadeColor( int startMsec, int totalMsec, float *rgba ) {
	static vec4_t		color;
	int			t;

	if ( startMsec == 0 ) {
		return NULL;
	}

	t = cg.time - startMsec;

	if ( t >= totalMsec ) {
		return NULL;
	}

	// fade out
	if ( totalMsec - t < FADE_TIME ) {
		color[3] = ( totalMsec - t ) * 1.0/FADE_TIME;
	} else {
		color[3] = rgba[3];
	}
	VectorCopy(rgba, color);

	return color;
}

void CGX_DrawStringRgba(int elNum, const char *s, float *rgba) {
	xhudElem_t *e = &xhud[elNum];
	float *col;

	if (!e->inuse)
		return;

	if (rgba) {
		col = rgba;
	} else {
		col = e->color;
	}

	XH_DrawString(e, s, col);
	XH_DrawBorder(e);
}
#endif

extern void CG_DrawStatusBarHead( float x, float y, float w, float h ) ;

static void XH_DrawStatusBarHead(xhudElem_t *e) {
	//if have image or model then, reset flag
	if (e->img || e->model) {
		e->flags &= ~(XF_PLAYER_HEAD);
		return;
	}

	trap_R_SetColor(NULL);

	XH_SetScale640();
	XH_Set3DIcons(cg_draw3dIcons.integer || e->flags & XF_DRAW3D);

	if (e->flags & XF_ANGLES) {
		XH_CalcAnglesRotation(e);

		CG_DrawHead(e->x, e->y, e->w, e->h, cg.snap->ps.clientNum, e->angles);
	} else {
		CG_DrawStatusBarHead(e->x, e->y, e->w, e->h);
	}

	XH_Restore3DIcons();
	XH_RestoreScale();
}

static void XH_DrawPic(xhudElem_t *e, qhandle_t hShader) {
	if (e->flags & XF_PLAYER_HEAD) {
		XH_DrawStatusBarHead(e);
	} else if (e->model) {
		XH_Draw3DModelColor(e, e->offset, e->angles, e->color);
	} else  {
		XH_DrawFill(e);

		if (!hShader)
			hShader = e->img;

		if (hShader) {
			trap_R_SetColor(e->color);
			XH_DrawHandlePic(e->x, e->y, e->w, e->h, hShader);
			trap_R_SetColor(NULL);
		}
	}
}

void CGX_DrawFlagStatus(int elNum, int flag) {
	xhudElem_t *e = &xhud[elNum];
	qhandle_t shader;
	qhandle_t *flagShader;
	int flagStatus;

	if (flag == PW_REDFLAG) {
		flagStatus = cgs.redflag;
		flagShader = (qhandle_t*)&cgs.media.redFlagShader;
	} else if (flag == PW_BLUEFLAG) {
		flagStatus = cgs.blueflag;
		flagShader = (qhandle_t*)&cgs.media.blueFlagShader;
	}

	//pm flag, cpma style, looks weird
#if 0
	if (e->color[3]) {
		static qhandle_t pmFlagShader[3];
		trap_R_LazyRegisterShader(pmFlagShader[flagStatus], va("xm/icons/flag_pm%i.tga", flagStatus));
		shader = pmFlagShader[flagStatus];
	} else
#endif
		shader = flagShader[flagStatus];

	trap_R_SetColor(NULL);
	XH_DrawHandlePic(e->x, e->y, e->w, e->h, shader);

	XH_DrawBorder(e);
}

void CGX_DrawPic( int elNum, qhandle_t hShader ) {
	xhudElem_t *e = &xhud[elNum];

	if (!e->inuse)
		return;

	XH_DrawPic(e, hShader);

	XH_DrawBorder(e);
}

void CGX_DrawLagometer(int elNum, float *ax, float *ay, float *aw, float *ah, qhandle_t hShader) {
	xhudElem_t *e = &xhud[elNum];

	if (!e->inuse) {
		*aw = 0;
		return;
	}

	*ax = e->x; *ay = e->y; *aw = e->w; *ah = e->h;

	CGX_DrawPic(elNum, hShader);

	//it's needed
	*ax -= 0.5f;
	*aw -= 0.5f;
}

void CGX_DrawField(int elNum, int value, const float *rgba) {
	xhudElem_t *e = &xhud[elNum];
	char num[16];

	if (!e->inuse)
		return;

	if (value > 999)
		value = 999;
	else if (value < -99)
		value = -99;

	Com_sprintf (num, sizeof(num), "%i", value);

	XH_DrawString(e, num, rgba);

	XH_DrawBorder(e);
}

void CGX_DrawWeaponSelect(void) {
	int bits, count;
	int	i;
	float x, y, h, w;
	int xstart;
	float *color;
	float pady, padx;
	float text_pad_top;
	float icon_pad_top;
	float icon_w, icon_h;
	int limits;
	qboolean horizontal;
	qboolean fill;
	xhudElem_t *e = &xhud[XH_WeaponList];

	color = e->time ?
		//CGX_FadeColor(cg.weaponSelectTime, e->time, e->fade) :
		CG_FadeColor(cg.weaponSelectTime, e->time) :
		colorWhite;

	if (!color)
		return;

	// showing weapon select clears pickup item display, but not the blend blob
	// in xhud not needed
	//cg.itemPickupTime = 0;

	bits = cg.snap->ps.stats[STAT_WEAPONS];

	fill = e->flags & XF_FILL;

	w = e->w;
	h = e->h;

	horizontal = e->textalign == 'C';

	padx = hud.weplist_padx;
	pady = hud.weplist_pady;
	icon_w = icon_h = e->font_h > e->font_w ? e->font_w : e->font_h;
	text_pad_top = (h - e->font_h) / 2;
	icon_pad_top = (h - icon_h) / 2;

	if (horizontal) {
		// count the number of weapons owned
		count = 0;
		for (i = 1; i < WP_NUM_WEAPONS - 1; i++)
			if (bits & (1 << i) || (fill && cg.snap->ps.ammo[i] > 0))
				count++;

		xstart = (hud.screen_width - count * (w + padx)) / 2;
		limits = hud.screen_width - w * 2 - padx;
	} else {
		xstart = e->x;
		limits = hud.screen_height - h * 2 - pady;
	}

	y = e->y;
	x = xstart;

	XH_SetFont(e);

	for (i = 1; i < WP_NUM_WEAPONS - 1; i++) {
		int ammo;
		int hasWeapon = (bits & (1 << i));
		float *wcolor;
		int icon_y;

		if (!hasWeapon && !(fill && cg.snap->ps.ammo[i] > 0))
			continue;

		ammo = cg.snap->ps.ammo[i];

		CG_RegisterWeapon(i);

		wcolor = i == cg.weaponSelect ?
			e->color :
			e->bgcolor;

		XH_FillRect(x, y, w, h, wcolor);

		//if (e->flags & XF_BORDER)
		//	XH_DrawRect(x, y, w, h, e->border_w, e->bordercolor);

		icon_y = y + icon_pad_top;

		// draw weapon icon
		XH_DrawHandlePic(x, icon_y, icon_w, icon_h, cg_weapons[i].weaponIcon);

		// draw selection marker
		//if (i == cg.weaponSelect) {
		//	CG_DrawPic(x - pad, y - pad, szpad, szpad, cgs.media.selectShader);
		//}

		// no ammo cross on top
		if (!ammo || !hasWeapon) {
			XH_DrawHandlePic(x, icon_y, icon_w, icon_h, cgs.media.noammoShader);
		}

		else if ((unsigned int)ammo > 999 && i != WP_GAUNTLET)
			ammo = 999;

		if (ammo >= 0) {
			char *s = va("%i", ammo);
			int xx;
			//if (horizontal)
			//	xx = x + (icon_w + w - XH_DrawStrWidth(e, s)) / 2;
			//else
				xx = x + icon_w;
			XH_DrawStringExt(xx, y + text_pad_top, s, color, TS_FORCECOLOR, e->shadowsize, e->font_w, e->font_h, 3);
		}

		if (horizontal) {
			if ((int)x > limits) {
				x = xstart;
				y += h + pady;
			} else {
				x += w + padx;
			}
		} else {
			if ((int)y > limits) {
				y = e->y;
				x += w + padx;
			} else {
				y += h + pady;
			}
		}
	}

	XH_RestoreFont();

	//TODO: style selected name maybe

	// draw the selected name
	//if ( cg_weapons[ cg.weaponSelect ].item ) {
	//	name = cg_weapons[ cg.weaponSelect ].item->pickup_name;
	//	if ( name ) {
	//		w = CG_DrawStrlen( name ) * hud.big_char_w;
	//		x = ( vScreen.width - w ) / 2;
	//		CG_DrawBigStringColor2(x, y - hud.weap_text_y, name, color);
	//	}
	//}
}

extern int numSortedTeamPlayers;

void CGX_DrawTeamOverlay(int pwidth, int lwidth) {
	int x, w, xx;
	float yy, fwa;
	int i, j, len;
	const char *p;
	vec4_t hcolor;
	char st[16];
	clientInfo_t *ci;

	qboolean right;

	if (cg.scoreBoardShowing)
		return;

	if (numSortedTeamPlayers > hud.overlayMaxNum)
		numSortedTeamPlayers = hud.overlayMaxNum;

	//TODO: add settings for max player name len, max location len (not important)

	for (i = 0; i < numSortedTeamPlayers; i++) {
		xhudElem_t *e = &xhud[XH_Team1 + i];
		if (!e->inuse)
			continue;

		fwa = e->flags & XF_MONOSPACE || e->cwi == NULL ? e->font_w : e->cwi->avg_cwr * e->font_w;

		ci = cgs.clientinfo + sortedTeamPlayers[i];

		hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

		right = e->textalign == 'R';

		w = (pwidth + lwidth + 3 + 3) * fwa + e->font_w * 5;

		yy = XH_GetAlignedYDrawStr(e);

		if ( right ) {
			x = hud.screen_width - w;
		} else {
			x = e->x;
		}

		if (e->img)
			XH_DrawHandlePic(x, yy, w, e->h, e->img);
		else
			XH_FillRect(x, yy, w, e->h, e->bgcolor);

		xx = x + e->font_w;

		XH_SetFont(e);

		XH_DrawStringExt( xx, yy,
			ci->name, hcolor, 0, e->shadowsize,
			e->font_w, e->font_h, TEAM_OVERLAY_MAXNAME_WIDTH);

		if (lwidth) {
			p = CG_ConfigString(CS_LOCATIONS + ci->location);
			if (!p || !*p)
				p = "unknown";

			xx = x + e->font_w * 2 + fwa * pwidth;
			XH_DrawStringExt( xx, yy,
				p, hcolor, 0, e->shadowsize, e->font_w, e->font_h,
				TEAM_OVERLAY_MAXLOCATION_WIDTH);
		}

		CG_GetColorForHealth( ci->health, ci->armor, hcolor );

		// draw hp
		Com_sprintf (st, sizeof(st), "%3i", ci->health);

		xx = x + w - fwa * 6 - e->font_w * 2;

		XH_DrawStringExt( xx, yy,
			st, hcolor, 0, e->shadowsize,
			e->font_w, e->font_h, 0 );

		// draw weapon icon
		xx += fwa * 3;

		if ( cg_weapons[ci->curWeapon].weaponIcon ) {
			XH_DrawHandlePic( xx, yy, e->font_w, e->font_h, 
				cg_weapons[ci->curWeapon].weaponIcon );
		} else {
			XH_DrawHandlePic( xx, yy, e->font_w, e->font_h, 
				cgs.media.deferShader );
		}

		// draw armor
		Com_sprintf (st, sizeof(st), "%3i", ci->armor);

		xx += e->font_w;

		XH_DrawStringExt( xx, yy,
			st, hcolor, 0, e->shadowsize,
			e->font_w, e->font_h, 0 );

		// Draw powerup icons
		if (right) {
			xx = x;
		} else {
			xx = x + w - e->font_w;
		}
		for (j = 0; j < PW_NUM_POWERUPS; j++) {
#if CGX_FREEZE//freeze
			if ( Q_Isfreeze( ci - cgs.clientinfo ) ) {
				XH_DrawHandlePic( xx, yy, e->font_w, e->font_h, cgs.media.noammoShader );
				break;
			}
#endif//freeze
			if (ci->powerups & (1 << j)) {
				gitem_t	*item;

				item = BG_FindItemForPowerup( j );

				XH_DrawHandlePic( xx, yy, e->font_w, e->font_h, 
					trap_R_RegisterShader( item->icon ) );
				if (right) {
					xx -= e->font_w;
				} else {
					xx += e->font_w;
				}
			}
		}

		XH_RestoreFont();
	}
}

void CGX_DrawChat( void ) {
	if (hud.chatHeight <= 0)
		return;

	if (hud.chatLastPos != hud.chatPos) {
		int i = hud.chatLastPos % hud.chatHeight;
		if (cg.time - hud.chatMsgTimes[i] > xhud[XH_Chat1 + i].time) {
			hud.chatLastPos++;
		}

		for (i = hud.chatLastPos; i < hud.chatPos; i++) {
			xhudElem_t *e = &xhud[XH_Chat1 + hud.chatPos - i - 1];
			//float *color = e->flags & XF_FADE ? CGX_FadeColor(hud.conMsgTimes[index], e->time, e->fade) : colorWhite;
			//qboolean forceColor = e->textstyle & TS_FORCECOLOR;
			//float *color = forceColor ? e->color : colorWhite;
			XH_SetFont(e);
			XH_DrawStringExt(e->x, e->y, 
				hud.chatMsgs[i % hud.chatHeight], 
				colorWhite, e->textstyle, e->shadowsize,
				e->font_w, e->font_h, 0);
			XH_RestoreFont();
		}
	}
}

void CGX_DrawConsole( void ) {
	int i;

	if (hud.conHeight <= 0)
		return;

	if (hud.conLastPos != hud.conPos) {
		xhudElem_t *con = &xhud[XH_Console];
		//qboolean fade = con->flags & XF_FADE;
		//qboolean forceColor = con->textstyle & TS_FORCECOLOR;
		//float *color = forceColor ? con->color : colorWhite;

		if (cg.time - hud.conMsgTimes[hud.conLastPos % hud.conHeight] > con->time) {
			hud.conLastPos++;
		}

		XH_SetFont(con);
		for (i = hud.conPos - 1; i >= hud.conLastPos; i--) {
			int index = i % hud.conHeight;
			char *s = hud.conMsgs[index];
			//float *color = fade ? CGX_FadeColor(hud.conMsgTimes[index], con->time, con->fade) : colorWhite;
			int x = XH_GetAlignedXDrawStr(con, s);

			XH_DrawStringExt(x, con->y + (i - hud.conLastPos)*con->font_h,
				s, colorWhite, con->textstyle, con->shadowsize,
				con->font_w, con->font_h, 0);
		}
		XH_RestoreFont();

		XH_DrawBorder(con);
	}
}

static void XH_GetSpecialColors() {
	int i;
	char t, e;
	clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	xhudElem_t *el;
	float *col;
	char *teColors = cgx_hud_TEColors.string;

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		if (cgs.gametype >= GT_TEAM) {
			t = teColors[2];
			e = teColors[1];
		} else {
			t = teColors[3];
			e = teColors[3];
		}
	} else if (cgx_enemyModel_enabled.integer && cgs.gametype < GT_TEAM) {
		t = cgx_teamColors.string[2] ? 
			cgx_teamColors.string[2] :
			teColors[(ArrLen(teColors) - 1) - ci->team % ArrLen(teColors)];

		e = cgx_enemyColors.string[2] ? 
			cgx_enemyColors.string[2] :
			teColors[ci->team % ArrLen(teColors)];
	} else {
		i = ci->team % ArrLen(teColors);
		t = teColors[(ArrLen(teColors) - 1) - i];
		e = teColors[i];
	}

	if (hud.oe != e) {
		hud.oe = e;
		i = (e - '0') % ArrLen(g_color_table_ex);
		col = g_color_table_ex[i];
		for (i = 0, el = xhud; i < XH_TOTAL_ELEMS; i++, el++) {
			if (el->flags & XF_E_COLOR)
				Vector4Copy(col, el->color);
			if (el->flags & XF_E_BGCOLOR)
				Vector4Copy(col, el->bgcolor);
		}
	}
	
	if (hud.ot != t) {
		hud.ot = t;
		i = (t - '0') % ArrLen(g_color_table_ex);
		col = g_color_table_ex[i];
		for (i = 0, el = xhud; i < XH_TOTAL_ELEMS; i++, el++) {
			if (el->flags & XF_T_COLOR)
				Vector4Copy(col, el->color);
			if (el->flags & XF_T_BGCOLOR)
				Vector4Copy(col, el->bgcolor);
		}
	}
}

static void XH_GetTeamIcons() {
	static int oldTeam = -1;
	static int oldClientNum = -1;
	int clientNum = cg.snap->ps.clientNum;
	int i, team = cgs.clientinfo[clientNum].team;

	if (!cgs.countAlive || (oldTeam == team && oldClientNum == clientNum))
		return;

	oldTeam = team;
	oldClientNum = clientNum;

	if (team == TEAM_SPECTATOR) {
		team = TEAM_RED;

		for (i = 0; i < MAX_CLIENTS; i++) {
			clientInfo_t *ci = &cgs.clientinfo[i];

			if (ci->team == team && ci->team != TEAM_SPECTATOR && ci->modelIcon) {
				hud.icon_own = ci->modelIcon;
				break;
			}
		}

		if (i == MAX_CLIENTS)
			hud.icon_own = trap_R_RegisterShaderNoMip(va("models/players/%s/icon_%s.tga", DEFAULT_MODEL, "red"));
	} else {
		hud.icon_own = cgs.clientinfo[clientNum].modelIcon;
	}

	for (i = 0; i < MAX_CLIENTS; i++) {
		clientInfo_t *ci = &cgs.clientinfo[i];

		if (ci->team != team && ci->team != TEAM_SPECTATOR && ci->modelIcon) {
			hud.icon_nme = ci->modelIcon;
			break;
		}
	}

	if (i == MAX_CLIENTS)
		hud.icon_nme = trap_R_RegisterShaderNoMip(va("models/players/%s/icon_%s.tga", DEFAULT_MODEL, team == TEAM_RED ? "blue" : "red"));
}

void CGX_DrawHUDLayer(int layerNum) {
	if (!hud.file)
		return;

	if (layerNum < 0) {
		XH_GetSpecialColors();
		XH_GetTeamIcons();

		CGX_DrawConsole();
		CGX_DrawChat();
	} else {
		int i = XH_ELEM_POOL_START;
		xhudElem_t *e = &xhud[XH_ELEM_POOL_START];
		for (; i < elemsPoolNum; i++, e++)
			if (e->type == layerNum && e->inuse)
				XH_DrawPic(e, 0);
	}
}

static void XH_Error(char *err) {
	CG_Printf("^3xhud: line %i, %s\n", COM_GetCurrentParseLine(), err);
}

#define xhud_err(x) XH_Error(x)

extern char *COM_PeekParse( char **data_p );

static qboolean NextToken_IsNum(char **data_p) {
	char *t = COM_PeekParse(data_p);
	return (*t >= '0' && *t <= '9') || (*t == '-' && *(t + 1) >= '0' && *(t + 1) <= '9');
}

static qboolean NextToken_IsBracket(char **data_p) {
	char *t = COM_PeekParse(data_p);
	return !Q_stricmp(t, "{");
}

static char* Parse_Str(char **data_p) {
	char *y = COM_Parse(data_p);
	if (!*y) {
		xhud_err("unexpected end of file");
		return NULL;
	} else {
		return y;
	}
}

static float Parse_Float(char **data_p) {
	char *s = Parse_Str(data_p);
	return *s ? atof(s) : 0;
}

static int Parse_Int(char **data_p) {
	char *s = Parse_Str(data_p);
	return *s ? atoi(s) : 0;
}

static void Parse_Flag(xhudElem_t *e, int flag, char **data_p) {
	e->flags |= flag; 
	
	if (NextToken_IsNum(data_p) && !Parse_Int(data_p)) 
		e->flags &= ~(flag);
}

static void Parse_Rect(xhudElem_t *e, char **data_p) {
	e->x = Parse_Float(data_p);
	e->y = Parse_Float(data_p);
	e->w = Parse_Float(data_p);
	e->h = Parse_Float(data_p);
}

static void Parse_ColorArgs(xhudElem_t *e, float *color, char **data_p, int flag_t, int flag_e) {
	e->flags &= ~(flag_e | flag_t);

	if (NextToken_IsNum(data_p)) {
		color[0] = Parse_Float(data_p);
		color[1] = Parse_Float(data_p);
		color[2] = Parse_Float(data_p);
		color[3] = Parse_Float(data_p);
	} else {
		char *t = Parse_Str(data_p);
		char c = toupper(*t);

		if (c == 'T') {
			e->flags |= flag_t;
		} else if (c == 'E') {
			e->flags |= flag_e;
		} else {
			int i = ColorIndex(QX_StringToColor(t));
			Vector4Copy(g_color_table_ex[i], e->color);
		}
	}
}

static void Parse_Bgcolor(xhudElem_t *e, char **data_p) {
	Parse_ColorArgs(e, e->bgcolor, data_p, XF_T_BGCOLOR, XF_E_BGCOLOR);
}

static void Parse_Color(xhudElem_t *e, char **data_p) {
	Parse_ColorArgs(e, e->color, data_p, XF_T_COLOR, XF_E_COLOR);
}

static void Parse_FontSize(xhudElem_t *e, char **data_p) {
	e->font_w = Parse_Int(data_p);

	if (NextToken_IsNum(data_p)) {
		e->font_h = Parse_Int(data_p);
	} else {
		e->font_h = e->font_w;
	}
}

#define CGX_MAX_FONTSIZE 1050000 //big files crashes quake3

//check file size and register shader
static qhandle_t XH_Register_FontShader(char *fontFile) {
	int fsize;
	if ((fsize = CGX_FSize(fontFile)) <= CGX_MAX_FONTSIZE)
		return trap_R_RegisterShaderNoMip(fontFile);

	CG_Printf("^3%s file size is too large %i, max allowed %i bytes, 1024x1024 pixels.\n", fontFile, fsize, CGX_MAX_FONTSIZE);
	return 0;
}

//register font 16 & 32 shaders, .cw or .font file if it's exists
static void XH_Register_Font(xhudElem_t *e, char *fontName) {
	char *fontFile, *fontFile32;
	qhandle_t fontShader;

	e->cwi = NULL;
	e->font = cgs.media.charsetShader;
	e->font32 = cgs.media.charsetShader32;
	e->flags &= ~(XF_FIELDFONT);

	if (!Q_stricmp(fontName, "$default"))
		return;

	fontFile = va("fonts/%s.tga", fontName);

	if (!CGX_FExists(fontFile)) {
		if (Q_stricmp(fontName, "numbers"))
			fontFile = va("fonts/%s_16.tga", fontName);
	}

	if (!CGX_FExists(fontFile)) {
		if (!Q_stricmp(fontName, "idblock") || !Q_stricmp(fontName, "numbers")) {
			e->flags |= XF_FIELDFONT;
		} else {
			xhud_err(va("font %s not found", fontName));
		}
		return;
	}

	fontShader = XH_Register_FontShader(fontFile);

	if (fontShader) {
		e->font = e->font32 = fontShader;

		e->cwi = XH_Register_CW(e->font, fontName);

		fontFile32 = va("fonts/%s_32.tga", fontName);

		if (CGX_FExists(fontFile32))
			if (fontShader = XH_Register_FontShader(fontFile32))
				e->font32 = fontShader;
	}
}

//take registered font and register lite font with it's name if possible, e+ stuff
static void XH_Register_Lite_Font(xhudElem_t *e) {
	int i; cw_info_t *cwi;
	char fontName[MAX_QPATH];

	for (i = 0, cwi = cwi_pool; i < ArrLen(cwi_pool); i++, cwi++)
		if (cwi->font == e->font) {
			Com_sprintf(fontName, sizeof fontName, "%s_l", cwi->fontName);
			XH_Register_Font(e, fontName);
			return;
		}
}

static void Parse_Font(xhudElem_t *e, char **data_p) {
	char *fontName = Parse_Str(data_p);
	XH_Register_Font(e, fontName);
}

static void Parse_TextAlign(xhudElem_t *e, char **data_p) {
	char *t = Parse_Str(data_p);
	e->textalign = toupper(*t);
}

static void Parse_Time(xhudElem_t *e, char **data_p) {
	e->time = Parse_Int(data_p);
}

static void Parse_Image(xhudElem_t *e, char **data_p) {
	char *name = Parse_Str(data_p);

	if (stristr(name, ".skin")) {
		e->img = trap_R_RegisterSkin(name);
	} else if (!Q_stricmp(name, "$lagometer")) {
		e->img = cgs.media.lagometerShader;
	} else if (!Q_stricmp(name, "none")) {
		e->img = 0;
	} else {
		e->img = trap_R_RegisterShaderNoMip(name);
	}
}

static void Parse_Model(xhudElem_t *e, char **data_p) {
	char *name = Parse_Str(data_p);
	e->model = trap_R_RegisterModel(name);
}

static void Parse_Angles(xhudElem_t *e, char **data_p) {
	e->angles[0] = Parse_Float(data_p);
	e->angles[1] = Parse_Float(data_p);
	e->angles[2] = Parse_Float(data_p);
	e->angles[3] = NextToken_IsNum(data_p) ? Parse_Float(data_p) : 0;
	e->flags |= XF_ANGLES;
	VectorCopy(e->angles, e->angles_initial);
}

static void Parse_Offset(xhudElem_t *e, char **data_p) {
	e->offset[0] = Parse_Float(data_p);
	e->offset[1] = Parse_Float(data_p);
	e->offset[2] = Parse_Float(data_p);
	e->flags |= XF_OFFSET;
}

static void Parse_Alignv(xhudElem_t *e, char **data_p) {
	char *t = Parse_Str(data_p);
	e->valign = toupper(*t);
}

static void Parse_TextStyle(xhudElem_t *e, char **data_p) {
	e->textstyle = Parse_Int(data_p);
}

static void Parse_Fade(xhudElem_t *e, char **data_p) {
	Parse_Float(data_p);
	Parse_Float(data_p);
	Parse_Float(data_p);
	Parse_Float(data_p);
	//e->fade[0] = Parse_Float(data_p);
	//e->fade[1] = Parse_Float(data_p);
	//e->fade[2] = Parse_Float(data_p);
	//e->fade[3] = Parse_Float(data_p);
	//e->flags |= XF_FADE;
}

static void Parse_Fill(xhudElem_t *e, char **data_p) {
	Parse_Flag(e, XF_FILL, data_p);
}

static void Parse_Monospace(xhudElem_t *e, char **data_p) {
	Parse_Flag(e, XF_MONOSPACE, data_p);
}

static void Parse_Doublebar(xhudElem_t *e, char **data_p) {
	Parse_Flag(e, XF_DOUBLEBAR, data_p);
}

static void Parse_Verticalbar(xhudElem_t *e, char **data_p) {
	Parse_Flag(e, XF_VERTICALBAR, data_p);
}

//xp
static void Parse_Anchors(xhudElem_t *e, char **data_p) {
	e->anchors = Parse_Int(data_p);
}

static void Parse_Param(xhudElem_t *e, char **data_p) {
	int param = Parse_Int(data_p);
	if (param == 1)
		e->flags |= XF_PARAM_1;
	else if (param == 2)
		e->flags |= XF_PARAM_2;
}

static void Parse_Draw3D(xhudElem_t *e, char **data_p) {
	Parse_Flag(e, XF_DRAW3D, data_p);
}

//my
static void Parse_Alpha(xhudElem_t *e, char **data_p) {
	e->alpha = Parse_Float(data_p);
}

static void Parse_ShadowSize(xhudElem_t *e, char **data_p) {
	e->shadowsize = Parse_Float(data_p);
}
#if HUD_SYNTAX_NMS
static void Parse_Border(xhudElem_t *e, char **data_p) {
	e->border_w = Parse_Float(data_p);
	if (NextToken_IsNum(data_p)) {//nms has 4 border widths, don't think it's important..
		e->border_w = Parse_Float(data_p);
		e->border_w = Parse_Float(data_p);
		e->border_w = Parse_Float(data_p);
	}
	if (e->border_w)
		e->flags |= XF_BORDER;
	else
		e->flags &= ~(XF_BORDER);
}

static void Parse_BorderColor(xhudElem_t *e, char **data_p) {
	Parse_ColorArgs(e, e->bordercolor, data_p, 0, 0);
}

//nms
static void Parse_FontHorizontal(xhudElem_t *e, char **data_p) {
	char *t = Parse_Str(data_p);

	if (!Q_stricmp(t, "CENTER"))
		e->textalign = 'C';
	else if (!Q_stricmp(t, "RIGHT"))
		e->textalign = 'R';
	else
		e->textalign = 'L';
}

static void Parse_FontVertical(xhudElem_t *e, char **data_p) {
	char *t = Parse_Str(data_p);

	if (!Q_stricmp(t, "MIDDLE"))
		e->valign = 'C';
	else if (!Q_stricmp(t, "BOTTOM"))
		e->valign = 'B';
	else
		e->valign = 'T';
}

static void Parse_Extra(xhudElem_t *e, char **data_p) {
	int i = Parse_Int(data_p);

	if (i & 1) e->flags |= XF_EXTRA_1;
	if (i & 2) e->flags |= XF_EXTRA_2;
}
#endif

struct {
	const char	*name;
	void 		(*parseFunc)(xhudElem_t *e, char **arg);
} xhudElemProps[] = {
	//cpma
	{ "rect", Parse_Rect },
	{ "bgcolor", Parse_Bgcolor },
	{ "color", Parse_Color },
	{ "image", Parse_Image },
	{ "model", Parse_Model },
	{ "fontsize", Parse_FontSize },
	{ "textalign", Parse_TextAlign },
	{ "textstyle", Parse_TextStyle },
	{ "time", Parse_Time },
	{ "font", Parse_Font },
	{ "fade", Parse_Fade },
	{ "fill", Parse_Fill },
	{ "monospace", Parse_Monospace },
	{ "doublebar", Parse_Doublebar },
	{ "verticalbar", Parse_Verticalbar },
	{ "angles", Parse_Angles },
	{ "offset", Parse_Offset },
	{ "alignv", Parse_Alignv },
	//xp
	{ "anchors", Parse_Anchors },
	{ "draw3d", Parse_Draw3D },
	{ "param", Parse_Param },
	//my
	{ "alpha", Parse_Alpha },
	{ "shadowsize", Parse_ShadowSize },
	//nms
#if HUD_SYNTAX_NMS
	{ "border", Parse_Border },
	{ "borderColor", Parse_BorderColor },
	{ "fontHorizontal", Parse_FontHorizontal },
	{ "fontVertical", Parse_FontVertical },
	{ "fontShadow", Parse_ShadowSize },
	{ "backgroundColor", Parse_Bgcolor },
	{ "extra", Parse_Extra }
#endif
};

static qboolean NextElement(char **t, char **data_p) {
	*t = COM_Parse(data_p); 
	return **t != 0;
}

static qboolean NextToken(char **t, char **data_p) {
	*t = COM_Parse(data_p);
	if (!**t) {
		xhud_err("unexpected end of file");
		return qfalse;
	}
	return qtrue;
}

static void SkipElem(char **data_p) {
	char *t;
	while (NextToken(&t, data_p))
		if (!Q_stricmp(t, "}"))
			break;
}

static qboolean ParseProperties(int elNum, char **data_p) {
	xhudElem_t *el;
	char *token;
	int i, k = 0;

	if (!NextToken(&token, data_p))
		return qfalse;

	if (Q_stricmp(token, "{")) { 
		xhud_err("{ is missing"); 
		SkipElem(data_p);
		return qfalse;
	}

	el = XH_AllocElem(elNum);

	while (NextToken(&token, data_p)) {
		if (!Q_stricmp(token, "}"))
			break;

		for (i = 0; i < ArrLen(xhudElemProps); i++)
			if (!Q_stricmp(token, xhudElemProps[i].name)) {
				xhudElemProps[i].parseFunc(el, data_p);
				el->inuse = qtrue;
				break;
			}

		if (i == ArrLen(xhudElemProps)) {
			xhud_err(va("unknown property %s", token));
			//skip property
			do {
				char *t = COM_PeekParse(data_p);
				if (Q_isalpha(*t) || !Q_stricmp(t, "}"))
					break;
			} while (NextToken(&token, data_p));
		}
	}

	return qtrue;
}

static int XH_Parse(char *file, int *syntax) {
	char	buf[16 * 1024];
	char	*token;
	char	*p;
	int		i, k;
	int		totalParsed = 0;

	if (!CGX_FReadAll(file, buf, sizeof buf))
		return 0;

	//prepare buffer, replace ; and #
	for (p = buf; *p; p++) {
		if (*p == ';')
			*p = ' ';
		else if (*p == '#')
			while (*p && *p != '\n')
				*p++ = ' ';
	}

	p = buf;

	COM_BeginParseSession();

	*syntax |= HUD_SYNTAX_CPMA;

	while (NextElement(&token, &p)) {
		//CG_Printf("%i %s\n", COM_GetCurrentParseLine(), token);

		//check basic known elements
		for (i = 0; i < ArrLen(xhudElemNames); i++)
			if (!Q_stricmp(token, xhudElemNames[i])) {
				if (ParseProperties(i, &p))
					totalParsed++;
				break;
			}

		if (i < ArrLen(xhudElemNames))
			continue;

		//check extra known elements
		for (i = 0; i < ArrLen(xhudExtraElems); i++)
			if (!Q_stricmp(token, xhudExtraElems[i].name)) {
				if (ParseProperties(xhudExtraElems[i].elNum, &p))
					totalParsed++;
#if HUD_SYNTAX_NMS
				if (i >= 10) *syntax |= HUD_SYNTAX_NMS;
				else 
#endif
				*syntax |= HUD_SYNTAX_AS;

				break;
			}

		//if we checked all known elements
		if (i == ArrLen(xhudExtraElems)) {
			if (!Q_stricmp(token, "{")) {
				xhud_err("found { withount element name specified");
				SkipElem(&p);
			} else if (!Q_stricmp(token, "}")) {
				xhud_err("found } before {");
			} else {
				xhud_err(va("unknown element %s", token));
				if (NextToken_IsBracket(&p))
					SkipElem(&p);
			}
		}
	}

	return totalParsed;
}

//#define XH_Set_Rect(e, xx, yy, ww, hh) e->x = xx; e->y = yy; e->w = ww; e->h = hh;
//#define XH_Set_Color(e, r, g, b, a) e->color[0] = r; e->color[1] = g; e->color[2] = b; e->color[3] = a;
//#define XH_Set_Font(e, fnt16, fnt32, w, h) e->font = fnt16; e->font32 = fnt32; e->font_w = w; e->font_h = h;
//#define XH_Set_TextStyle(e, textStyle, textAlign) e->textstyle = textStyle; e->textalign = textAlign;
//#define XH_Set_Inuse(e, use) e->inuse = use;

static int Vector4Compare( const vec4_t v1, const vec4_t v2 ) {
	return !(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2] || v1[3] != v2[3]);
}

static void XH_WeaponListWidthHeight(xhudElem_t *e, float *w, float *h) {
	if (e->type != XH_WeaponList)
		return;
	if (e->textalign == 'C')
		*w = (e->w + XH_WEAPLIST_PAD) * 8;
	else
		*h = (e->h + XH_WEAPLIST_PAD) * 8;
}

static qboolean XH_Intersect(xhudElem_t *a, xhudElem_t *b) {
	float aw = a->w, ah = a->h;
	float bw = b->w, bh = b->h;
	XH_WeaponListWidthHeight(a, &aw, &ah);
	XH_WeaponListWidthHeight(b, &bw, &bh);
	if (b->x < a->x + aw && a->x < b->x + bw && b->y < a->y + ah)
		return a->y < b->y + bh;
	else
		return qfalse;
}

//loads hud from file
static void CGX_LoadHUD(char *fileName) {
	int		i;
	int		totalParsed;
	int		syntax = 0;
	char	fileNameCopy[2048];

	if (!*fileName) {
		if (hud.file)
			XH_Clear();
		return;
	}

	if (!strchr(fileName, '.'))
		fileName = va("%s.cfg", fileName);

	if (!CGX_FExists(fileName) && Q_stricmpn(fileName, "hud/", 4))
		fileName = va("hud/%s", fileName);

	if (!CGX_FSize(fileName)) {
		XH_Clear();
		return;
	}

	if (!cgs.media.charsetShader32) {
		if (!(cgs.media.charsetShader32 = trap_R_RegisterShaderNoMip("gfx/2d/bigchars_32")))
			cgs.media.charsetShader32 = cgs.media.charsetShader;
		trap_Cvar_Register(&cgx_hud_TEColors, "cgx_hud_TEColors", "2417" , CVAR_TEMP);
		trap_Cvar_Register(&cgx_hud_font16threshold, "cgx_hud_font16threshold", "24" , CVAR_TEMP);
	}

	trap_Cvar_Update(&cgx_hud_TEColors);
	trap_Cvar_Update(&cgx_hud_font16threshold);

	XH_Clear();

	if (ArrLen(xhudElemNames) != XH_MAX_STATIC_HUD_ELEMS + XH_MAX_POOLED_HUD_ELEMTYPES) {
		CG_Error("xhudElemNames count is different from xhudElemType_t count\n");
	}

	{//some default values
		Vector4Copy(colorWhite, xhud[0].color);
		xhud[0].font_h = 16;
		xhud[0].font_w = 16;
		xhud[0].textalign = 'L';
		xhud[0].font = cgs.media.charsetShader;
		xhud[0].font32 = cgs.media.charsetShader32;
		xhud[0].cwi = NULL;
		xhud[0].shadowsize = 1.0f;
		//Vector4Copy(colorWhite, xhud[0].fade);
#if HUD_SYNTAX_NMS
		xhud[0].border_w = 0.5f;
		Vector4Copy(colorMagenta, xhud[0].bordercolor);
		//xhud[0].flags |= XF_BORDER;
#endif
	}
	
	//other calls of va method will modify file name, so saving copy here
	Q_strncpyz(fileNameCopy, fileName, sizeof fileNameCopy);

	totalParsed = XH_Parse(fileName, &syntax);

	//adjustments after parse
	if (syntax)
	{
		const float xr = (float)SCREEN_WIDTH / vScreen.width;
		const float ox = FromX640((vScreen.width - SCREEN_WIDTH) / 2.0f * xr);
		xhudElem_t *wl = &xhud[XH_WeaponList];

		hud.file = qtrue;

		hud.screen_width = FromX640(SCREEN_WIDTH);
		hud.screen_height = FromY640(SCREEN_HEIGHT);
		hud.weplist_padx = FromX640(XH_WEAPLIST_PAD);
		hud.weplist_pady = FromY640(XH_WEAPLIST_PAD);
		hud.bar_pad = FromY640(3);

		//check for open arena weapon list style (where width and height is total values)
		if (wl->w > SCREEN_WIDTH / 8 || wl->h > SCREEN_HEIGHT / 8) {
			//horizontal
			if (wl->h < wl->w)
				wl->textalign = 'C';
			if (wl->w > SCREEN_WIDTH / 8)
				wl->w /= 8;
			if (wl->h > SCREEN_HEIGHT / 8)
				wl->h /= 8;
		}

		//stick PlayerAccuracy, because most of huds will not have it
		if (!xhud[XH_PlayerAccuracy].inuse) {
			xhudElem_t *pa = &xhud[XH_PlayerAccuracy];
			int k = 0;
			pa->y = 4;
			pa->w = XM_BIGCHAR_WIDTH * 10;
			pa->h = BIGCHAR_HEIGHT;
			pa->x = SCREEN_WIDTH - pa->w;
			for (i = 0; i < XH_TOTAL_ELEMS && k < 10000; i++, k++) {
				xhudElem_t *e = &xhud[i];
				if (e->inuse && XH_Intersect(e, pa)) {
					int h = e->type == XH_WeaponList ? e->h * 8 : e->h;
					pa->y = e->y + h + 4;
					i = 0;
				}
			}
			if (pa->y < SCREEN_HEIGHT - BIGCHAR_HEIGHT) {
				pa->inuse = qtrue;
				pa->font = cgs.media.charsetShader;
				pa->font32 = cgs.media.charsetShader32;
				pa->font_w = XM_BIGCHAR_WIDTH;
				pa->font_h = XM_BIGCHAR_WIDTH;
				pa->anchors = XA_RIGHT;
				pa->flags = 0;
				Vector4Copy(colorWhite, pa->color);
				pa->textalign = 'R';
				pa->shadowsize = 1.0f;
				pa->textstyle = TS_SHADOW;
			}
		}

		//final preparations
		for (i = 0; i < ArrLen(xhud); i++) {
			xhudElem_t *e = &xhud[i];

			if (!(e->textstyle & TS_SHADOW))
				e->shadowsize = 0;
			if (e->textstyle & TS_LITE)
				XH_Register_Lite_Font(e);
			//vertical align for team overlay
			if (!e->valign)
				e->valign = e->type >= XH_Team1 && e->type <= XH_Team8 ? 'C' : 'T';

			//adjust coordinates & sizes
			{
				AdjX640(e->x);
				AdjX640(e->w);

				AdjY640(e->y);
				AdjY640(e->h);

				AdjX640(e->font_w);
				AdjY640(e->font_h);
				
				AdjY640(e->shadowsize);
			}

			//adjust anchors
			if (cgx_wideScreenFix.integer & CGX_WFIX_SCREEN) {
				if (e->anchors) {
					float x = e->x, y = e->y, w = e->w, h = e->h;

					e->w = w * xr;
					e->font_w *= xr;
					e->x = x * xr + ox;

					if (e->anchors & XA_LEFT && e->anchors & XA_RIGHT) {
						e->w = w;
						e->x = x;
					} else {
						if (e->anchors & XA_LEFT) {
							e->x = x * xr;
						} else if (e->anchors & XA_RIGHT) {
							e->x = x * xr + ox * 2;
						}
					}
				}
			}
		}

		if (!(xhud[XH_StatusBar_AmmoIcon].flags & XF_ANGLES)) {
			xhud[XH_StatusBar_AmmoIcon].angles[3] = 20;
			xhud[XH_StatusBar_AmmoIcon].angles_initial[YAW] = 90;
			xhud[XH_StatusBar_AmmoIcon].flags |= XF_ANGLES;
		}

		if (!(xhud[XH_StatusBar_ArmorIcon].flags & XF_ANGLES)) {
			xhud[XH_StatusBar_ArmorIcon].angles[3] = -360;
			xhud[XH_StatusBar_ArmorIcon].flags |= XF_ANGLES;
		}

		for (i = XH_Team1; i <= XH_Team8; i++) {
			if (xhud[i].inuse)
				hud.overlayMaxNum++;
		}

		for (i = XH_Chat1; i <= XH_Chat8; i++) {
			if (xhud[i].inuse)
				hud.chatHeight++;
			else
				break;
		}

		for (i = 0; i <= XH_PowerUp8_Icon - XH_PowerUp1_Icon; i++) {
			xhudElem_t *icon = &xhud[XH_PowerUp1_Icon + i];
			xhudElem_t *time = &xhud[XH_PowerUp1_Time + i];

			//remove any E T colors from powerup icons
			icon->flags &= ~(XF_E_COLOR | XF_T_COLOR);
			Vector4Copy(colorWhite, icon->color);

			if (icon->inuse && time->inuse)
				hud.powerupsMaxNum++;
			else
				break;
		}

		if (xhud[XH_Console].inuse) {
			hud.conHeight = 3;

			trap_Cvar_Set("con_notifytime", "-1");
		}

		{//pickup time
			xhudElem_t *pIcon = &xhud[XH_ItemPickupIcon];
			xhudElem_t *pName = &xhud[XH_ItemPickup];
			if (pIcon->inuse || pName->inuse)
				hud.pickupTime = pIcon->time > pName->time ? pIcon->time : pName->time;
		}

		if (syntax & HUD_SYNTAX_AS) {
			//CG_Printf("Aftershock syntax found\n");
			if (Vector4Compare(xhud[XH_WeaponList].color, colorWhite))
				xhud[XH_WeaponList].color[3] = 0.25f;

			for (i = XH_Team1; i <= XH_Team8; i++)
				if (xhud[i].x > SCREEN_WIDTH / 2)
					xhud[i].textalign = 'R';
		}
#if HUD_SYNTAX_NMS
		else if (syntax & HUD_SYNTAX_NMS) {
#define NMS_WEP_PAD 5
			xhudElem_t *wl = &xhud[XH_WeaponList];
			hud.weplist_padx = FromX640(NMS_WEP_PAD);
			hud.weplist_pady = FromY640(NMS_WEP_PAD);

			//CG_Printf("Nms syntax found\n");
			//nms
			if (wl->flags & XF_EXTRA_1) {
				wl->textalign = 'C';
			} else {
				wl->y = wl->y - (wl->h * 8 + NMS_WEP_PAD * 8) / 2;
				wl->x = wl->x - wl->w / 2;
			}

			if (!wl->time)
				wl->time = WEAPON_SELECT_TIME;

			for (i = XH_Chat1; i <= XH_Chat8; i++)
				if (!xhud[i].time)
					xhud[i].time = 3000;

			for (i = XH_Team1; i <= XH_Team8; i++)
				if (xhud[i].x > SCREEN_WIDTH / 2)
					xhud[i].textalign = 'R';
		}
#endif
	}

	CG_Printf("loaded %s, elements %i\n", fileNameCopy, totalParsed);
	//TODO: sort pooled elems
}

qboolean CGX_AddToChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;

	if (hud.chatHeight <= 0)
		return qfalse;

	//print to console for history
	trap_Print(va("%s\n", str));

	len = 0;

	p = hud.chatMsgs[hud.chatPos % hud.chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			hud.chatMsgTimes[hud.chatPos % hud.chatHeight] = cg.time;

			hud.chatPos++;
			p = hud.chatMsgs[hud.chatPos % hud.chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p = *str++;
		if (*p == '\n')
			*p = ' ';
		p++;

		len++;
	}
	*p = 0;

	hud.chatMsgTimes[hud.chatPos % hud.chatHeight] = cg.time;
	hud.chatPos++;

	if (hud.chatPos - hud.chatLastPos > hud.chatHeight)
		hud.chatLastPos = hud.chatPos - hud.chatHeight;

	return qtrue;
}

void CGX_AddToConsole( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;

	if (hud.conHeight <= 0)
		return;

	len = 0;

	p = hud.conMsgs[hud.conPos % hud.conHeight];
	*p = 0;

	lastcolor = '7';

	ls = NULL;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			hud.conMsgTimes[hud.conPos % hud.conHeight] = cg.time;

			hud.conPos++;
			p = hud.conMsgs[hud.conPos % hud.conHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = NULL;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p = *str++;
		if (*p == '\n')
			*p = ' ';
		p++;

		len++;
	}
	*p = 0;

	hud.conMsgTimes[hud.conPos % hud.conHeight] = cg.time;
	hud.conPos++;

	if (hud.conPos - hud.conLastPos > hud.conHeight)
		hud.conLastPos = hud.conPos - hud.conHeight;
}

#pragma endregion
