// Copyright (C) 2018 NaViGaToR (322)

#include "ui_local.h"

//gets picmip and save its value
int UIX_GetPicmip() {	
	static int val = -1;
	if (val == -1) {
		char buf[4];
		trap_Cvar_VariableStringBuffer("r_picmip", buf, sizeof(buf));		
		val = atoi(buf);
		//save for cg_nomip
		if (uix_nomip.integer && val)
			trap_Cvar_Set("cgx_r_picmip", buf);
	}
	return val;
}

static void UIX_NomipStart() {	
	if (uix_nomip.integer && UIX_GetPicmip())
		trap_Cvar_Set("r_picmip", "0");
}

static void UIX_NomipEnd() {		
	if (uix_nomip.integer && UIX_GetPicmip()) {
		char buf[4];
		trap_Cvar_VariableStringBuffer("cgx_r_picmip", buf, sizeof(buf));		
		trap_Cvar_Set("r_picmip", buf);
	}
}

static void UIX_PlayerInfo_SetModel( playerInfo_t *pi, const char *model ) {
	UIX_NomipStart();
	UI_PlayerInfo_SetModel(pi, model);
	UIX_NomipEnd();
}

static void UIX_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
	UIX_NomipStart();
	UI_PlayerInfo_SetWeapon(pi, weaponNum);
	UIX_NomipEnd();
}