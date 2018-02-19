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
#if CGX_DEBUG
	if (cgx_debug.integer)
	trap_Print(va ("^5DEBUG: %s", str) );
#endif
}

void trap_WPrint(const char *str) {
#if CGX_DEBUG
	if (cgx_debug.integer)
	trap_Print(va ("^3WARNING: %s", str) );
#endif
}

void trap_RPrint(const char *str) {
#if CGX_DEBUG
	if (cgx_debug.integer)
	trap_Print(va ("^6REASON: %s", str) );
#endif
}

void CGX_Init_vScreen(void) {	
	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	// X-MOD: init virtual screen sizes for wide screen fix

	if ( cgx_wideScreenFix.integer && cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		vScreen.width = (float)cgs.glconfig.vidWidth / (float)cgs.glconfig.vidHeight * 480.0;		 
		vScreen.height = 480;		 				
		vScreen.offsetx = (vScreen.width - 640) / 2.0;
	} else {
		vScreen.width = 640;
		vScreen.height = 480;					
		vScreen.offsetx = 0;
	}	

	vScreen.ratiox = vScreen.width / 640.0;

	vScreen.hwidth = vScreen.width / 2;
	vScreen.hheight = vScreen.height / 2;

	cgs.screenXScale = cgs.glconfig.vidWidth / vScreen.width;
	cgs.screenYScale = cgs.glconfig.vidHeight / vScreen.height; 

	trap_DPrint(va("CGX_Init_vScreen %fx%f cgx_wideScreenFix %d\n", vScreen.width, vScreen.height, cgx_wideScreenFix.integer));	
}

void CGX_Init_enemyModels(void) {
	char modelStr[MAX_QPATH];
	char *slash;	

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

	trap_DPrint("CGX_EnemyModelCheck\n");		

	//change models and skins if needed or restore
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ )
		if (cg.clientNum != i)
			CGX_SetModelAndSkin(ci);																			
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
		color = "!!!!";

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

	for (i = 0; i < 3; i++)
		info->color[i] = (float)info->colors[0][i] / 255.0f;	
}

void CGX_Init_enemyAndTeamColors(void) {
	int i;
	clientInfo_t *ci;

	if (cg.oldTeam == TEAM_SPECTATOR)
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

	trap_DPrint(va("%s -> %s\n", ci->modelName, ci->modelNameCopy));

	Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));	

	ci->infoValid = qtrue;
	ci->deferred = qtrue;
}

void CGX_RestoreSkinNameFromCopy(clientInfo_t *ci) {
	if (Q_stricmp(ci->skinName, ci->skinNameCopy) == 0)
		return;
		
	trap_DPrint(va("%s -> %s\n", ci->skinName, ci->skinNameCopy));

	Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));

	ci->infoValid = qtrue;
	ci->deferred = qtrue;		
}

void CGX_RestoreModelAndSkin(clientInfo_t *ci) {
	//skip emtpy models
	if (ci->modelName[0] == '\0')		
		return;	

	trap_DPrint("CGX_RestoreModelAndSkin\n");

	CGX_RestoreModelNameFromCopy(ci);
	CGX_RestoreSkinNameFromCopy(ci);
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
		trap_WPrint(va("No PM skin for model %s\n", ci->modelName));		
		//set sarge/pm		
		Q_strncpyz(ci->modelName, DEFAULT_MODEL, sizeof(ci->modelName));
	} else if (Q_stricmp(ci->skinName, "pm") == 0) {
		trap_DPrint(va("PM skin already set %s\n", ci->modelName));	
		return;
	}		

	trap_DPrint(va("Setting PM skin %s\n", ci->modelName));
	Q_strncpyz(ci->skinName, "pm", sizeof(ci->skinName));	

	ci->infoValid = qtrue;
	ci->deferred = qtrue;		
}

void CGX_SetModelAndSkin(clientInfo_t *ci) {	
	qboolean isSameTeam = qfalse;
	if (!cgx_enemyModel_enabled.integer) {
		//if it's disabled maybe we need to restore models
		CGX_RestoreModelAndSkin(ci);
		return;
	}

	// skip emtpy clientInfo 
	if (ci->modelName[0] == '\0')		
		return;	

	// some additional checks after config string modified, it calls CG_NewClientInfo again
	if (cg.clientNum != -1) {
		qboolean isSpect = cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR || ci->team == TEAM_SPECTATOR;
		isSameTeam = cgs.gametype >= GT_TEAM && cgs.clientinfo[cg.clientNum].team == ci->team;

		if (isSpect) {
			CGX_RestoreModelAndSkin(ci);
			return;
		}
	}		

	trap_DPrint(va("CGX_SetModelAndSkin %i\n", cg.clientNum));

	if (!isSameTeam) {
		// if enemymodels enabled and enemy model not specified, load saved real model name and set pm skin
		if (cg.enemyModel[0] == '\0') {
			CGX_RestoreModelNameFromCopy(ci);
			CGX_SetPMSkin(ci);
			CGX_SetColorInfo(cgx_enemyColors.string, ci);
			return;
		}

		// save model name and skin copy
		if (ci->modelNameCopy[0] == '\0')
			Q_strncpyz(ci->modelNameCopy, ci->modelName, sizeof(ci->modelName));
		if (ci->skinNameCopy[0] == '\0')
			Q_strncpyz(ci->skinNameCopy, ci->skinName, sizeof(ci->skinName));

		trap_DPrint(va("%s -> %s\n", ci->modelName, cg.enemyModel));

		Q_strncpyz(ci->modelName, cg.enemyModel, sizeof(ci->modelName));

		// if skin not specified set pm
		if (cg.enemySkin[0] == '\0')
			CGX_SetPMSkin(ci);

		// if gametype is not team\ctf or skin pm set it, otherwise red\blue will be used
		if (cgs.gametype < GT_TEAM || Q_stricmp(cg.enemySkin, "pm") == 0)
			Q_strncpyz(ci->skinName, cg.enemySkin, sizeof(ci->skinName));

		// if skin is pm set colors
		if (Q_stricmp(ci->skinName, "pm") == 0)
			CGX_SetColorInfo(cgx_enemyColors.string, ci);

		ci->infoValid = qtrue;
		ci->deferred = qtrue;
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
		if (ci->modelNameCopy[0] == '\0')
			Q_strncpyz(ci->modelNameCopy, ci->modelName, sizeof(ci->modelName));
		if (ci->skinNameCopy[0] == '\0')
			Q_strncpyz(ci->skinNameCopy, ci->skinName, sizeof(ci->skinName));

		trap_DPrint(va("%s -> %s\n", ci->modelName, cg.teamModel));

		Q_strncpyz(ci->modelName, cg.teamModel, sizeof(ci->modelName));

		// if skin not specified set pm
		if (cg.teamSkin[0] == '\0')
			CGX_SetPMSkin(ci);

		// if gametype is not team\ctf or skin pm set it, otherwise red\blue will be used
		if (cgs.gametype < GT_TEAM || Q_stricmp(cg.teamSkin, "pm") == 0)
			Q_strncpyz(ci->skinName, cg.teamSkin, sizeof(ci->skinName));

		// if skin is pm set colors
		if (Q_stricmp(ci->skinName, "pm") == 0)
			CGX_SetColorInfo(cgx_teamColors.string, ci);

		ci->infoValid = qtrue;
		ci->deferred = qtrue;
	}
}

void CGX_AutoAdjustNetworkSettings(void) {
	trap_DPrint(va("CGX_AutoAdjustNetworkSettings %i\n", cgx_networkAdjustments.integer));

	if (cgx_networkAdjustments.integer) {
		int i, minRate, minSnaps;
		char buf[10];		

		if (cgx_networkAdjustments.integer == 1) {
			minRate = 8000;
			minSnaps = 4444;
		} else {
			minRate = 30000;
			minSnaps = 8888;
		}

		i = cgs.sv_fps > 0 ? cgs.sv_fps : minSnaps;
		trap_Cvar_Set("snaps", va("%i", i));
		trap_WPrint(va("Auto: snaps %i\n", i));

		trap_Cvar_VariableStringBuffer("rate", buf, sizeof(buf));

		i = atoi(buf);

		if (i < minRate) {
			trap_Cvar_Set("rate", va("%i", minRate));
			trap_WPrint(va("Auto: rate %i\n", minRate));
		}
	}		
}
