#include "cg_local.h"

void trap_DPrint(const char *str) {
#if 0
	trap_Print(str);
#endif
}

void CGX_Init_vScreen(void) {	
	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );

	trap_DPrint(va("CGX_Init_vScreen cgx_wideScreenFix %d\n", cgx_wideScreenFix.integer));

	// X-MOD: init virtual screen sizes for wide screen fix

	if ( cgx_wideScreenFix.integer && cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		vScreen.width = (float)cgs.glconfig.vidWidth / (float)cgs.glconfig.vidHeight * 480.0;		 
		vScreen.height = 480;		 				
		vScreen.offsetx = vScreen.width / 8.0;
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

	trap_DPrint(va("vScreen: %fx%f\n", vScreen.width, vScreen.height));
}

void CGX_Init_enemyModels(void) {
	char modelStr[MAX_QPATH];
	char *slash;

	trap_DPrint("CGX_Init_enemyModels\n");

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

	//replace pm skins for now
	if (Q_stricmp(cg.enemySkin, "pm") == 0) {
		Q_strncpyz(cg.enemySkin, "default", sizeof(cg.enemySkin));
	}

	Q_strncpyz(cg.enemyModel, modelStr, sizeof(cg.enemyModel));	

	trap_DPrint(va("cg.enemyModel: %s\ncg.enemySkin: %s\n", cg.enemyModel, cg.enemySkin));
}

void CGX_EnemyModelCheck(void) {
	int		i;
	clientInfo_t	*ci;
	clientInfo_t	*cur;			

	//trap_DPrint("CGX_EnemyModelCheck\n");		

	cur = &cgs.clientinfo[cg.clientNum];	

	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) {
		if (cg.clientNum != i) {						
			if (cgx_enemyModel_enabled.integer == 0 || cgx_enemyModel.string[0] == '\0' || 
				(cur->team == ci->team && cur->team != TEAM_FREE) || cur->team == TEAM_SPECTATOR) {
				CGX_RestoreModelAndSkin(ci);
			} else if (Q_stricmp(ci->modelName, cg.enemyModel) != 0) {										
				CGX_SetModelAndSkin(ci);
			}													
		}
	}
}

void CGX_RestoreModelAndSkin(clientInfo_t *ci) {
	//trap_DPrint("CGX_RestoreModelAndSkin ");

	if (Q_stricmp(ci->modelName, ci->modelNameCopy) != 0) {
		trap_DPrint(va("%s -> %s\n", ci->modelName, ci->modelNameCopy));				

		Q_strncpyz(ci->modelName, ci->modelNameCopy, sizeof(ci->modelName));

		ci->infoValid = qtrue;
		ci->deferred = qtrue;
	}
	if (Q_stricmp(ci->skinName, ci->skinNameCopy) != 0) {
		trap_DPrint(va("%s -> %s\n", ci->skinName, ci->skinNameCopy));

		Q_strncpyz(ci->skinName, ci->skinNameCopy, sizeof(ci->skinName));

		ci->infoValid = qtrue;
		ci->deferred = qtrue;
	}
}

void CGX_SetModelAndSkin(clientInfo_t *ci) {
	//trap_DPrint("CGX_SetModelAndSkin ");				

	if (cgx_enemyModel_enabled.integer == 0)
		return;

	if (ci->modelNameCopy[0] == '\0')
		Q_strncpyz(ci->modelNameCopy, ci->modelName, sizeof(ci->modelName));
	if (ci->skinNameCopy[0] == '\0')
		Q_strncpyz(ci->skinNameCopy , ci->skinName , sizeof(ci->skinName ));

	if (ci->modelName[0] != '\0') {
		trap_DPrint(va("%s -> %s\n", ci->modelName, cg.enemyModel));	

		Q_strncpyz(ci->modelName, cg.enemyModel, sizeof(ci->modelName));

		if (cgs.gametype < GT_TEAM)
			Q_strncpyz(ci->skinName, cg.enemySkin, sizeof(ci->skinName));		

		ci->infoValid = qtrue;
		ci->deferred = qtrue;
	}		
}