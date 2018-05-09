// Copyright (C) 2018 NaViGaToR (322)
//
/*
=======================================================================

X-MOD CREDITS 

=======================================================================
*/


#include "ui_local.h"

static char *sodmodgaNames[] = {
	"1488mp",	
	"777",
	"Abdel Morocco",
	"Agi",
	"Alex",
	"BOND",
	"Comrade",
	"DEADPOOL",
	"Fish",
	"GRAZ1",
	"It",
	"Isidora",
	"k (Hysteria)",
	"K8 (Kate)",
	"Kokoro^No^Chizzu (Jan)",
	"kestaneci",
	"KILLER",
	"///Kit",	
	"[RU]*Leon*",
	"Mongrel",
	"MORDA",	
	"Mucha",
	"Mustang",
	"MYSECRET",
	"NaGiBaToR",
	"P",
	"Poseidon",
	"Pika Pika",
	"REDNECK (UK)",
	"Roens",
	"Scout",
	"SplinDay",
	"Stalker Mechenniy",
	"SVETLOV",
	"Tanya",
	"tvinsen",
	"Ukraine",
	"Ukrop",
	"WineUbuntu",
	"yMka",
	"Yuri_semko",
	"zazo",
};

static char *sodmodtkNames[] = {
	"^Alex^",
	"Ari",
	"Browzer",
	"BuLL",
	"Crackapark",
	"^Crash^",
	"Dani007",
	"Delincuente",
	"Dori",
	"GaSoL",
	"^Green^",
	"Julione",
	"Katatonia",
	//"Kate (K8)",
	"Killer",
	"KillerBeasT",
	"LanceloT",
	"Marciniak",
	"Manu",
	"Mike",
	"Mummy",
	"Nobody",
	//"Navigator",
	"PuLsE",
	//"Roens",
	"^Samurai^",
	"SarPiT",
	"SeLaS",
	"Suicidal Love",
	"Vicente",
	"",
	"Clan EN",
	"Clan SWE",
	"Clan Sari",
	"Clan SL",
	"Clan RU",
	"Clan LPA"
};

static char *railctfnames[] = {
	"Alex",
	"Bana$",
	"**BLEU**",
	"CATWOMAN",
	"Chumbatta!",
	"Fany",
	"Ivan Star",
	"MoonGirl",
	"Nancy",
	"NOway",
	"orbu777",
	"PHANDER",
	"Poly",	
	"Sarita",
	"Seven",
	"S@yerS",
	"simec",		
	"$lu",
	"test",
	"Trance",
	"*TRoP*FoRT*",
	"*TRoP*TaRD*",
	"WhiteGandalf",
	"waterpater",
	"",
	"Clan ESP",
	"Clan AFF",
	"Clan QWZ"
};

static char *aimnames[] = {
	"AHTOXA",
	"Alice_K",
	"Blackwidow",
	"Boones",
	"Boris Johnson",
	"^buu!",
	"CHAKO",	
	"Disha",
	"Dora",
	"ERAKONE",
	"FANTASMA",
	"Flower",
	"gogman",
	"Harry Krishna",
	".icec0re.",
	"oops?!",
	"Pash",
	"pasztet",
	"PlaYuriel^^",
	"Robin",
	"MAVX",
	"Miko",
	"Ne4istaya sila",
	"Never give up!",
	"Susel",
	"TEAM[*]KOREA",
	"Uriel",
	"wibley",
	"yetlguru",
	"Zarina",
	"Zegalon",
};

#define SODNAMES_SIZE sizeof(sodmodgaNames) / sizeof(sodmodgaNames[0])
#define SODNAMES2_SIZE sizeof(sodmodtkNames) / sizeof(sodmodtkNames[0])
#define RAILCTFNAMES_SIZE sizeof(railctfnames) / sizeof(railctfnames[0])
#define AIMNAMES_SIZE sizeof(aimnames) / sizeof(aimnames[0])

#define CGX_CREDITS_END 6726
#define DEFAULT_YSTEP 0.4f

typedef struct {
	menuframework_s	menu;

	playerInfo_t		playerinfo[10];
} creditsmenu_t;

static creditsmenu_t	s_credits2;

static float y_pos = 0;
static float y_step = DEFAULT_YSTEP;
static char oldMusicVolume[16];

/*
=================
UI_CreditMenu2_Key
=================
*/
static sfxHandle_t UI_CreditMenu2_Key( int key ) {
	if( key == K_MOUSE2 || key == K_ESCAPE || key == K_SPACE || key == K_ENTER) {
		trap_Cmd_ExecuteText(EXEC_APPEND, va("s_musicvolume %s; stopsound;", oldMusicVolume));

		UI_PopMenu();
		return 0;
	} else if ((key == K_MWHEELDOWN  || key == K_DOWNARROW || key == K_PGDN)
		&& y_pos > -CGX_CREDITS_END) {
		y_pos -= PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	} else if ((key == K_MWHEELUP  || key == K_UPARROW || key == K_PGUP)
		&& y_pos > -CGX_CREDITS_END) {
		y_pos += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	}
	return 0;
}

#define PITCH1 21
#define YAW1 30
#define YAW2 50

static qboolean p_taunt;
static qboolean p_stand;

static void UI_InitPlayerModels() {
	char			buf[MAX_QPATH];
	vec3_t			viewangles;

	p_taunt = qfalse;
	p_stand = qfalse;
	
	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[0], "keel/red" );	
	viewangles[YAW]   = 180 - YAW1;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[0], LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_BFG, qfalse );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[1], "mynx/blue" );	
	viewangles[YAW]	  = 180 - YAW1;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[1], LEGS_IDLECR, TORSO_STAND, viewangles, vec3_origin, WP_ROCKET_LAUNCHER, qfalse );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[2], "visor/default" );	
	viewangles[YAW]	  = 180 - YAW1;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[2], LEGS_IDLECR, TORSO_STAND, viewangles, vec3_origin, WP_GRENADE_LAUNCHER, qfalse );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[3], "xaero/red" );	
	viewangles[YAW]   = 270 - YAW2;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[3], LEGS_IDLECR, TORSO_STAND, viewangles, vec3_origin, WP_RAILGUN, qfalse );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[4], "sarge/default" );	
	viewangles[YAW]   = 270 - YAW2;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[4], LEGS_IDLECR, TORSO_STAND, viewangles, vec3_origin, WP_MACHINEGUN, qfalse );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[5], "slash/blue" );	
	viewangles[YAW]   = 270 - YAW2;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[5], LEGS_IDLE, TORSO_STAND, viewangles, vec3_origin, WP_SHOTGUN, qfalse );

	trap_Cvar_VariableStringBuffer( "model", buf, sizeof( buf ) );

	UIX_PlayerInfo_SetModel( &s_credits2.playerinfo[6], buf );	
	viewangles[YAW]   = 180;
	viewangles[PITCH] = PITCH1;
	viewangles[ROLL]  = 0;
	UI_PlayerInfo_SetInfo( &s_credits2.playerinfo[6], LEGS_IDLE, TORSO_STAND2, viewangles, vec3_origin, WP_GAUNTLET, qfalse );	
}

/*
=================
PlayerSettings_DrawPlayer
=================
*/
static old_uis;

static void UI_CreditMenu2_DrawPlayer(  ) {	
	int t;	

	t = uis.realtime / 2;	

	UI_DrawPlayer( 450,     0, 32*10, 56*10, &s_credits2.playerinfo[0], t );
	UI_DrawPlayer( 450-70,  0, 32*10, 56*10, &s_credits2.playerinfo[2], t );
	UI_DrawPlayer( 450,    40, 32*10, 56*10, &s_credits2.playerinfo[1], t );

	UI_DrawPlayer( -70-40,  0, 32*10, 56*10, &s_credits2.playerinfo[5], t );	
	UI_DrawPlayer(   0-40,  0, 32*10, 56*10, &s_credits2.playerinfo[4], t );	
	UI_DrawPlayer( -70-40, 40, 32*10, 56*10, &s_credits2.playerinfo[3], t );	

	if (CGX_CREDITS_END <= -y_pos && !p_taunt) {
		int i;		
		p_taunt = qtrue;
		for (i = 0; i < 7; i++) {			
			UI_ForceTorsoAnim(&s_credits2.playerinfo[i], TORSO_GESTURE);			
		}
		old_uis = uis.realtime;
	}
	if (p_taunt && old_uis + 2500 <= uis.realtime && !p_stand) {
		int i;
		p_stand = qtrue;
		for (i = 6; i < 7; i++) {		
			UI_ForceTorsoAnim(&s_credits2.playerinfo[i], TORSO_STAND2);
		}
	}
	
	UI_DrawPlayer(170, CGX_CREDITS_END + y_pos, 32 * 10, 56 * 10, &s_credits2.playerinfo[6], t);	
}


/*
===============
UI_CreditMenu2_Draw
===============
*/

#define DRAW_W(x) UI_DrawProportionalString( 320, y, x, UI_CENTER|UI_SMALLFONT, color_white );
#define DRAW_R(x) UI_DrawProportionalString( 320, y, x, UI_CENTER|UI_SMALLFONT, color_red );
#define STEP_Y() y += PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
#define STEP_Y1() y += 1.65 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;

#define DRAW_WSTEP(x) DRAW_W(x) STEP_Y()
#define DRAW_WSTEP1(x) DRAW_W(x) STEP_Y1()
#define DRAW_RSTEP(x) DRAW_R(x) STEP_Y()
#define DRAW_RSTEP1(x) DRAW_R(x) STEP_Y1()

static void UI_CreditMenu2_Draw( void ) {
	int		y;
	int		i;

	y_pos -= y_step;

	if (y_pos < -CGX_CREDITS_END)
		y_pos = -CGX_CREDITS_END;
	else if (y_pos > SCREEN_HEIGHT)
		y_pos = SCREEN_HEIGHT;
	
	if (uis.debug)
		UI_DrawString( 320, 0, va("%i %i %i", (int)y_pos, CGX_CREDITS_END, uis.realtime), UI_CENTER|UI_SMALLFONT, color_red );

	UI_CreditMenu2_DrawPlayer();

	y = y_pos;	

	DRAW_RSTEP1("Quake III "CGX_NAME)
	DRAW_WSTEP1("Programming")
	DRAW_WSTEP1("NaViGaToR (322)")	
	DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_RSTEP1("Thanks to")
	DRAW_WSTEP1("Sodmod.ga community")
	DRAW_WSTEP1("")

	for (i = 0; i < SODNAMES_SIZE; i++) {		
		DRAW_WSTEP1(sodmodgaNames[i])
	}

	DRAW_WSTEP1("")
	DRAW_WSTEP1("Old Sodmod.tk community")
	DRAW_WSTEP1("")

	for (i = 0; i < SODNAMES2_SIZE; i++) {
		DRAW_WSTEP1(sodmodtkNames[i])
	}

	DRAW_WSTEP1("")
	DRAW_WSTEP1("Esp CTF & CCCP CTF Servers")		
	DRAW_WSTEP1("")	

	for (i = 0; i < RAILCTFNAMES_SIZE; i++) {
		DRAW_WSTEP1(railctfnames[i])
	}

	DRAW_WSTEP1("")
	DRAW_WSTEP1("I can't aim server")
	DRAW_WSTEP1("")

	for (i = 0; i < AIMNAMES_SIZE; i++) {
		DRAW_WSTEP1(aimnames[i])
	}

	DRAW_WSTEP1("")
	DRAW_WSTEP1("All people who host servers")	
	DRAW_WSTEP1("And made good stuff for game")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("Cyrus & Wonkey")
	DRAW_WSTEP1("Gpe community")
	DRAW_WSTEP1("Huxx & Bma team")
	DRAW_WSTEP1("///hmn")
	DRAW_WSTEP1("maverick")
	DRAW_WSTEP1("Para & q3pwnz community")
	DRAW_WSTEP1("q3nightmare")
	DRAW_WSTEP1("Clan PAR")
	DRAW_WSTEP1("And other")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("All UnnamedPlayers")
	DRAW_WSTEP1("And GRACZs")
	DRAW_WSTEP1("And other people who I may forgot")
	DRAW_WSTEP1("But they love quake")

	DRAW_WSTEP1("")
	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_RSTEP1("Special thanks to")
	DRAW_WSTEP1("Michal")
	DRAW_WSTEP1("for /rate 10000 /snaps 40")
	DRAW_WSTEP1("Bana$")
	DRAW_WSTEP1("for /cg_enemyModel and /cg_enemyColors")
	DRAW_WSTEP1("Leon")
	DRAW_WSTEP1("for showing SoDMoD")
	DRAW_WSTEP1("///Qbz")
	DRAW_WSTEP1("for being programmer")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("")
	DRAW_WSTEP1("Neil 'haste' Toronto")	
	DRAW_WSTEP1("for Unlagged 2.01")
	DRAW_WSTEP1("Challenge World & The ProMode Team")	
	DRAW_WSTEP1("for CPMA")
	DRAW_WSTEP1("Id Software")
	DRAW_WSTEP1("for Awesome Game")
	DRAW_WSTEP1("Sonic Mayhem")
	DRAW_WSTEP1("for Awesome Music")
	/*DRAW_WSTEP1("")
	DRAW_WSTEP1("")*/
	//DRAW_RSTEP1("Unlagged 2.01")	
	//DRAW_WSTEP1("Neil 'haste' Toronto")	

	//DRAW_WSTEP1("")
	//DRAW_RSTEP1("CPMA")
	//DRAW_WSTEP1("Challenge World")
	//DRAW_WSTEP1("The ProMode Team")

	//DRAW_WSTEP1("")
	//DRAW_RSTEP1("id Software")
	
	//DRAW_WSTEP1("Programming:")	
	//DRAW_WSTEP1("John Carmack")
	//DRAW_WSTEP1("John Cash")
	//
	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("Art:")
	//DRAW_WSTEP1("Adrian Carmack")
	//DRAW_WSTEP1("Kevin Cloud")
	//DRAW_WSTEP1("Paul Steed")
	//DRAW_WSTEP1("Kenneth Scott")

	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("Game Designer:")
	//DRAW_WSTEP1("Graeme Devine")

	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("Level Design:")
	//DRAW_WSTEP1("Tim Willits")
	//DRAW_WSTEP1("Christian Antkow")
	//DRAW_WSTEP1("Paul Jaquays")

	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("CEO:")
	//DRAW_WSTEP1("Todd Hollenshead")

	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("Director of Business Development:")
	//DRAW_WSTEP1("Katherine Anna Kang")

	//DRAW_WSTEP1("")
	//DRAW_WSTEP1("Biz Assist and id Mom:")
	//DRAW_WSTEP1("Donna Jackson")

	y += 380;	

	UI_DrawProportionalString( 320, y, "QUAKE III ARENA", UI_CENTER|UI_GIANTFONT|UI_PULSE, color_red );	

	//y += 1.65 * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	//UI_DrawString( 320, y, "To order: 1-800-idgames     www.quake3arena.com     www.idsoftware.com", UI_CENTER|UI_SMALLFONT, color_red );
	//y += SMALLCHAR_HEIGHT;
	//UI_DrawString( 320, y, "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved", UI_CENTER|UI_SMALLFONT, color_red );
}


/*
===============
UI_CreditMenu2
===============
*/
void UI_CreditMenu2( void ) {
	float fps;
	memset( &s_credits2, 0 ,sizeof(s_credits2) );

	s_credits2.menu.draw = UI_CreditMenu2_Draw;
	s_credits2.menu.key = UI_CreditMenu2_Key;
	s_credits2.menu.fullscreen = qtrue;

	UI_InitPlayerModels();

	UI_PushMenu ( &s_credits2.menu );
	// set start pos
	y_pos = SCREEN_HEIGHT;	
	// calc y_step based on fps
	fps = trap_Cvar_VariableValue("com_maxfps");

	if (fps)
		y_step = 125.0f / fps * DEFAULT_YSTEP;
	
	trap_Cvar_VariableStringBuffer( "s_musicvolume", oldMusicVolume, sizeof( oldMusicVolume ) );
	// set a little music volume if it's off and restore it on menu pop
	if (!trap_Cvar_VariableValue("s_musicvolume"))
		trap_Cmd_ExecuteText(EXEC_APPEND, "s_musicvolume 0.25;");

	//trap_Cmd_ExecuteText(EXEC_APPEND, "music music/fla22k_02;");
	trap_Cmd_ExecuteText(EXEC_APPEND, "music music/sonic1;");

}
