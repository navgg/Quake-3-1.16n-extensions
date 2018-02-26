// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
/*
=======================================================================

NETWORK OPTIONS MENU

=======================================================================
*/

#include "ui_local.h"


#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"

#define ID_GRAPHICS			10
#define ID_DISPLAY			11
#define ID_SOUND			12
#define ID_NETWORK			13
#define ID_RATE				14
#define ID_PACKETS			15
#define ID_PACKETDUP		16
#define ID_AUTOADJ			17
#define ID_SNAPS			18
#define ID_DELAG			19
#define	ID_PREDICTION		20

#define ID_BACK				29

#define MAX_INFO_MESSAGES	7

static void UI_Network_StatusBar( void *self ) {	
	static const char *info_messages[MAX_INFO_MESSAGES][2] = {
		{ "Max date rate in bytes per second", "Setting lower than 5000 not recommended" },
		{ "Max packets rate per second", "Set highter if you have good PC and internet" },
		{ "Send packet duplicates or no", "If you have good connection you can turn this off" },
		{ "Set off if you know how to config rate/packets", "Set max if you have strong PC and cable internet" },
		{ "Sets amout of snaps sent from server to client", "Min - 1, Max - 999. Recommended 40" },
		{ "Sets client site delag if it's available on server", "Recommended on" },
		{ "Sets prediction method", "Optimized can increase fps if your CPU was bottleneck" }
	};

	UIX_CommonStatusBar(self, ID_RATE, MAX_INFO_MESSAGES, info_messages);
}

static const char *rate_items[] = {	
	"4000 (56K old modems)",
	"5000 (ISDN)",
	"8000 (8Kb/sec)",
	"10000 (10Kb/sec)",
	"16000 (16Kb/sec)",
	"25000 (LAN/Cable/xDSL)",
	"30000 (CPMA)",
	"50000 (Yolo/WTF?/OMG)",
	0
};

static const char *packets_items[] = {	
	"40",
	"50",
	"60",	
	"70",
	"80",
	"90",
	"100",
	0
};

static const char *autoadjustments_items[] = {
	"Off",
	"Min",
	"Medium",
	"Max",
	0
};

static const char *delag_items[] = {
	"Off",
	"If available",
	0
};

static const char *prediction_items[] = {
	"Default",
	"Optimized",
	0
};

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menutext_s		graphics;
	menutext_s		display;
	menutext_s		sound;
	menutext_s		network;

	menulist_s		rate;
	menulist_s		packets;
	menulist_s		adjustments;
	menuradiobutton_s	packetdup;
	menufield_s		snaps;

	menulist_s		delag;
	menulist_s		prediction;

	menubitmap_s	back;
} networkOptionsInfo_t;

static networkOptionsInfo_t	networkOptionsInfo;

/*
=================
Preferences2_SaveChanges
=================
*/
static void UI_NetworkOptionsMenu_SaveChanges( void ) {	
	if (!networkOptionsInfo.adjustments.curvalue)
		trap_Cvar_Set( "snaps", networkOptionsInfo.snaps.field.buffer );	
}

static void UI_NetworkOptionsMenu_CheckGrayed(void) {
	if (!networkOptionsInfo.adjustments.curvalue) {
		networkOptionsInfo.snaps.generic.flags &= ~QMF_GRAYED;
		networkOptionsInfo.packets.generic.flags &= ~QMF_GRAYED;
		networkOptionsInfo.delag.generic.flags &= ~QMF_GRAYED;
		networkOptionsInfo.prediction.generic.flags &= ~QMF_GRAYED;
	} else {
		networkOptionsInfo.snaps.generic.flags |= QMF_GRAYED;
		networkOptionsInfo.packets.generic.flags |= QMF_GRAYED;
		networkOptionsInfo.delag.generic.flags |= QMF_GRAYED;
		networkOptionsInfo.prediction.generic.flags |= QMF_GRAYED;
	}
}

/*
=================
UI_NetworkOptionsMenu_Event
=================
*/
static void UI_NetworkOptionsMenu_Event( void* ptr, int event ) {	

	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GRAPHICS:
		UI_PopMenu();
		UI_GraphicsOptionsMenu();
		break;

	case ID_DISPLAY:
		UI_PopMenu();
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		UI_PopMenu();
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		break;

	case ID_RATE:
		if( networkOptionsInfo.rate.curvalue == 0 )
			trap_Cvar_SetValue( "rate", 4000 );
		else if( networkOptionsInfo.rate.curvalue == 1 )
			trap_Cvar_SetValue( "rate", 5000 );				
		else if( networkOptionsInfo.rate.curvalue == 2 )
			trap_Cvar_SetValue( "rate", 8000 );			
		else if( networkOptionsInfo.rate.curvalue == 3 )
			trap_Cvar_SetValue( "rate", 10000 );			
		else if( networkOptionsInfo.rate.curvalue == 4 )
			trap_Cvar_SetValue( "rate", 16000 );			
		else if( networkOptionsInfo.rate.curvalue == 5 )
			trap_Cvar_SetValue( "rate", 25000 );			
		else if( networkOptionsInfo.rate.curvalue == 6 )
			trap_Cvar_SetValue( "rate", 30000 );		
		else if( networkOptionsInfo.rate.curvalue == 7 )
			trap_Cvar_SetValue( "rate", 50000 );						
		break;

	case ID_PACKETS:				
		trap_Cvar_SetValue( "cl_maxpackets", 40 + networkOptionsInfo.packets.curvalue * 10 );
		break;
	
	case ID_AUTOADJ:
		trap_Cvar_SetValue( "cg_networkAdjustments", networkOptionsInfo.adjustments.curvalue );
		UI_NetworkOptionsMenu_CheckGrayed();
		break;

	case ID_PACKETDUP:
		trap_Cvar_SetValue( "cl_packetdup", networkOptionsInfo.packetdup.curvalue );							
		break;

	case ID_DELAG:
		trap_Cvar_SetValue( "cg_delag", networkOptionsInfo.delag.curvalue );							
		break;

	case ID_PREDICTION:
		trap_Cvar_SetValue( "cg_delag_optimizePrediction", networkOptionsInfo.prediction.curvalue );							
		break;

	case ID_BACK:
		UI_NetworkOptionsMenu_SaveChanges();
		UI_PopMenu();
		break;
	}
}

/*
=================
PlayerSettings_MenuKey
=================
*/
static sfxHandle_t UI_NetworkOptionsMenu_MenuKey( int key ) {
	if( key == K_MOUSE2 || key == K_ESCAPE ) {
		UI_NetworkOptionsMenu_SaveChanges();
	}
	return Menu_DefaultKey( &networkOptionsInfo.menu, key );
}

/*
===============
UI_NetworkOptionsMenu_Init
===============
*/
static void UI_NetworkOptionsMenu_Init( void ) {
	int		y;
	int		rate, packets;	

	memset( &networkOptionsInfo, 0, sizeof(networkOptionsInfo) );

	UI_NetworkOptionsMenu_Cache();
	networkOptionsInfo.menu.wrapAround = qtrue;
	networkOptionsInfo.menu.fullscreen = qtrue;

	networkOptionsInfo.banner.generic.type		= MTYPE_BTEXT;
	networkOptionsInfo.banner.generic.flags		= QMF_CENTER_JUSTIFY;
	networkOptionsInfo.banner.generic.x			= 320;
	networkOptionsInfo.banner.generic.y			= 16;
	networkOptionsInfo.banner.string			= "SYSTEM SETUP";
	networkOptionsInfo.banner.color				= color_white;
	networkOptionsInfo.banner.style				= UI_CENTER;

	networkOptionsInfo.framel.generic.type		= MTYPE_BITMAP;
	networkOptionsInfo.framel.generic.name		= ART_FRAMEL;
	networkOptionsInfo.framel.generic.flags		= QMF_INACTIVE;
	networkOptionsInfo.framel.generic.x			= 0;  
	networkOptionsInfo.framel.generic.y			= 78;
	networkOptionsInfo.framel.width				= 256;
	networkOptionsInfo.framel.height			= 329;

	networkOptionsInfo.framer.generic.type		= MTYPE_BITMAP;
	networkOptionsInfo.framer.generic.name		= ART_FRAMER;
	networkOptionsInfo.framer.generic.flags		= QMF_INACTIVE;
	networkOptionsInfo.framer.generic.x			= 376;
	networkOptionsInfo.framer.generic.y			= 76;
	networkOptionsInfo.framer.width				= 256;
	networkOptionsInfo.framer.height			= 334;

	networkOptionsInfo.graphics.generic.type		= MTYPE_PTEXT;
	networkOptionsInfo.graphics.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	networkOptionsInfo.graphics.generic.id			= ID_GRAPHICS;
	networkOptionsInfo.graphics.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.graphics.generic.x			= 216;
	networkOptionsInfo.graphics.generic.y			= 240 - 2 * PROP_HEIGHT;
	networkOptionsInfo.graphics.string				= "GRAPHICS";
	networkOptionsInfo.graphics.style				= UI_RIGHT;
	networkOptionsInfo.graphics.color				= color_red;

	networkOptionsInfo.display.generic.type			= MTYPE_PTEXT;
	networkOptionsInfo.display.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	networkOptionsInfo.display.generic.id			= ID_DISPLAY;
	networkOptionsInfo.display.generic.callback		= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.display.generic.x			= 216;
	networkOptionsInfo.display.generic.y			= 240 - PROP_HEIGHT;
	networkOptionsInfo.display.string				= "DISPLAY";
	networkOptionsInfo.display.style				= UI_RIGHT;
	networkOptionsInfo.display.color				= color_red;

	networkOptionsInfo.sound.generic.type			= MTYPE_PTEXT;
	networkOptionsInfo.sound.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	networkOptionsInfo.sound.generic.id				= ID_SOUND;
	networkOptionsInfo.sound.generic.callback		= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.sound.generic.x				= 216;
	networkOptionsInfo.sound.generic.y				= 240;
	networkOptionsInfo.sound.string					= "SOUND";
	networkOptionsInfo.sound.style					= UI_RIGHT;
	networkOptionsInfo.sound.color					= color_red;

	networkOptionsInfo.network.generic.type			= MTYPE_PTEXT;
	networkOptionsInfo.network.generic.flags		= QMF_RIGHT_JUSTIFY;
	networkOptionsInfo.network.generic.id			= ID_NETWORK;
	networkOptionsInfo.network.generic.callback		= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.network.generic.x			= 216;
	networkOptionsInfo.network.generic.y			= 240 + PROP_HEIGHT;
	networkOptionsInfo.network.string				= "NETWORK";
	networkOptionsInfo.network.style				= UI_RIGHT;
	networkOptionsInfo.network.color				= color_red;

	y = 240 - 2 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.rate.generic.type		= MTYPE_SPINCONTROL;
	networkOptionsInfo.rate.generic.name		= "Data Rate:";
	networkOptionsInfo.rate.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	networkOptionsInfo.rate.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.rate.generic.id			= ID_RATE;
	networkOptionsInfo.rate.generic.x			= 400;
	networkOptionsInfo.rate.generic.y			= y;
	networkOptionsInfo.rate.itemnames			= rate_items;
	networkOptionsInfo.rate.generic.statusbar	= UI_Network_StatusBar;

	y = 240 - 0 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.packets.generic.type		= MTYPE_SPINCONTROL;
	networkOptionsInfo.packets.generic.name		= "Packets Rate:";
	networkOptionsInfo.packets.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_GRAYED;
	networkOptionsInfo.packets.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.packets.generic.id		= ID_PACKETS;
	networkOptionsInfo.packets.generic.x		= 400;
	networkOptionsInfo.packets.generic.y		= y;
	networkOptionsInfo.packets.itemnames		= packets_items;
	networkOptionsInfo.packets.generic.statusbar	= UI_Network_StatusBar;

	y = 240 - 1 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.packetdup.generic.type		= MTYPE_RADIOBUTTON;
	networkOptionsInfo.packetdup.generic.name		= "Packet Dup:";
	networkOptionsInfo.packetdup.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	networkOptionsInfo.packetdup.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.packetdup.generic.id			= ID_PACKETDUP;
	networkOptionsInfo.packetdup.generic.x			= 400;
	networkOptionsInfo.packetdup.generic.y			= y;
	networkOptionsInfo.packetdup.generic.statusbar	= UI_Network_StatusBar;

	y = 240 + 1 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.snaps.generic.type		= MTYPE_FIELD;
	networkOptionsInfo.snaps.generic.name		= "Snaps:";
	networkOptionsInfo.snaps.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY|QMF_GRAYED;
	networkOptionsInfo.snaps.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.snaps.generic.id			= ID_SNAPS;
	networkOptionsInfo.snaps.generic.x			= 400;
	networkOptionsInfo.snaps.generic.y			= y;
	networkOptionsInfo.snaps.field.widthInChars = 4;
	networkOptionsInfo.snaps.field.maxchars		= 3;
	networkOptionsInfo.snaps.generic.statusbar	= UI_Network_StatusBar;

	y = 240 + 2 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.delag.generic.type		= MTYPE_SPINCONTROL;
	networkOptionsInfo.delag.generic.name		= "Delag:";
	networkOptionsInfo.delag.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_GRAYED;
	networkOptionsInfo.delag.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.delag.generic.id			= ID_DELAG;
	networkOptionsInfo.delag.generic.x			= 400;
	networkOptionsInfo.delag.generic.y			= y;
	networkOptionsInfo.delag.itemnames			= delag_items;	
	networkOptionsInfo.delag.generic.statusbar	= UI_Network_StatusBar;

	y = 240 + 3 * (BIGCHAR_HEIGHT+2);
	networkOptionsInfo.prediction.generic.type		= MTYPE_SPINCONTROL;
	networkOptionsInfo.prediction.generic.name		= "Prediction:";
	networkOptionsInfo.prediction.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_GRAYED;
	networkOptionsInfo.prediction.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.prediction.generic.id		= ID_PREDICTION;
	networkOptionsInfo.prediction.generic.x			= 400;
	networkOptionsInfo.prediction.generic.y			= y;
	networkOptionsInfo.prediction.itemnames			= prediction_items;
	networkOptionsInfo.prediction.generic.statusbar	= UI_Network_StatusBar;

	y = 240 - 3 * (BIGCHAR_HEIGHT + 2);
	networkOptionsInfo.adjustments.generic.type = MTYPE_SPINCONTROL;
	networkOptionsInfo.adjustments.generic.name = "Auto Settings:";
	networkOptionsInfo.adjustments.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
	networkOptionsInfo.adjustments.generic.callback = UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.adjustments.generic.id = ID_AUTOADJ;
	networkOptionsInfo.adjustments.generic.x = 400;
	networkOptionsInfo.adjustments.generic.y = y;
	networkOptionsInfo.adjustments.itemnames = autoadjustments_items;
	networkOptionsInfo.adjustments.generic.statusbar	= UI_Network_StatusBar;

	networkOptionsInfo.back.generic.type		= MTYPE_BITMAP;
	networkOptionsInfo.back.generic.name		= ART_BACK0;
	networkOptionsInfo.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	networkOptionsInfo.back.generic.callback	= UI_NetworkOptionsMenu_Event;
	networkOptionsInfo.back.generic.id			= ID_BACK;
	networkOptionsInfo.back.generic.x			= 0;
	networkOptionsInfo.back.generic.y			= 480-64;
	networkOptionsInfo.back.width				= 128;
	networkOptionsInfo.back.height				= 64;
	networkOptionsInfo.back.focuspic			= ART_BACK1;

	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.banner );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.framel );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.framer );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.graphics );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.display );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.sound );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.network );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.rate );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.packets );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.packetdup );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.adjustments );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.snaps );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.delag );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.prediction );
	Menu_AddItem( &networkOptionsInfo.menu, ( void * ) &networkOptionsInfo.back );

	rate = trap_Cvar_VariableValue( "rate" );
	if( rate <= 4000 )
		networkOptionsInfo.rate.curvalue = 0;
	else if( rate <= 5000 )
		networkOptionsInfo.rate.curvalue = 1;	
	else if( rate <= 8000 )
		networkOptionsInfo.rate.curvalue = 2;
	else if( rate <= 10000 )
		networkOptionsInfo.rate.curvalue = 3;
	else if( rate <= 16000 )
		networkOptionsInfo.rate.curvalue = 4;
	else if( rate <= 25000 )
		networkOptionsInfo.rate.curvalue = 5;
	else if( rate <= 30000 )
		networkOptionsInfo.rate.curvalue = 6;
	else
		networkOptionsInfo.rate.curvalue = 7;

	packets = trap_Cvar_VariableValue( "cl_maxpackets" );
	if( packets <= 40 )
		networkOptionsInfo.packets.curvalue = 0;
	else if( packets <= 50 )
		networkOptionsInfo.packets.curvalue = 1;
	else if( packets <= 60 )
		networkOptionsInfo.packets.curvalue = 2;
	else if( packets <= 70 )
		networkOptionsInfo.packets.curvalue = 3;
	else if( packets <= 80 )
		networkOptionsInfo.packets.curvalue = 4;
	else if( packets <= 90 )
		networkOptionsInfo.packets.curvalue = 5;
	else
		networkOptionsInfo.packets.curvalue = 6;
	
	trap_Cvar_VariableStringBuffer("snaps", networkOptionsInfo.snaps.field.buffer, sizeof(networkOptionsInfo.snaps.field.buffer));
	networkOptionsInfo.packetdup.curvalue = trap_Cvar_VariableValue("cl_packetdup") != 0;
	networkOptionsInfo.delag.curvalue = trap_Cvar_VariableValue("cg_delag") != 0;	
	networkOptionsInfo.prediction.curvalue = trap_Cvar_VariableValue("cg_delag_optimizePrediction") != 0;	
	networkOptionsInfo.adjustments.curvalue = abs((int)trap_Cvar_VariableValue("cg_networkAdjustments") % 4);

	UI_NetworkOptionsMenu_CheckGrayed();
}


/*
===============
UI_NetworkOptionsMenu_Cache
===============
*/
void UI_NetworkOptionsMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}


/*
===============
UI_NetworkOptionsMenu
===============
*/
void UI_NetworkOptionsMenu( void ) {
	UI_NetworkOptionsMenu_Init();
	UI_PushMenu( &networkOptionsInfo.menu );
	Menu_SetCursorToItem( &networkOptionsInfo.menu, &networkOptionsInfo.network );
}
