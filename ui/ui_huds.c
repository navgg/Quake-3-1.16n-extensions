// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2019 NaViGaToR (322)
//
/*
=======================================================================

HUDS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"	
#define ART_GO0				"menu/art/load_0"
#define ART_GO1				"menu/art/load_1"
#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"
#define ART_ARROWS			"menu/art/arrows_horz_0"
#define ART_ARROWLEFT		"menu/art/arrows_horz_left"
#define ART_ARROWRIGHT		"menu/art/arrows_horz_right"

#define INGAME_FRAME		"menu/art/addbotframe"

#define MAX_HUDS			128
#define NAMEBUFSIZE			64

#define ID_BACK				10
#define ID_GO				11
#define ID_LIST				12
#define ID_RIGHT			13
#define ID_LEFT				14

#define ARROWS_WIDTH		128
#define ARROWS_HEIGHT		48

#define BASIC_HUDS			3

extern const char *basichud_items[];

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menulist_s		list;

	menubitmap_s	arrows;
	menubitmap_s	left;
	menubitmap_s	right;
	menubitmap_s	back;
	menubitmap_s	go;

	int				numHuds;
	char			hudnames[MAX_HUDS+BASIC_HUDS][NAMEBUFSIZE];

	char			*hudlist[MAX_HUDS+BASIC_HUDS];
} huds_t;

static huds_t	s_huds;

extern void Preferences2_UpdateHudName();

static void Huds_LoadCurrent() {
	if (s_huds.list.curvalue < BASIC_HUDS) {
		trap_Cvar_Set("hud", "");
		trap_Cvar_Set("cg_draw2D", va("%i", s_huds.list.curvalue + 1));
		Preferences2_UpdateHudName();
	} else {
		trap_Cvar_Set("hud", s_huds.list.itemnames[s_huds.list.curvalue]);
		Preferences2_UpdateHudName();
	}

	if (s_huds.menu.fullscreen)
		UI_PopMenu();
}

/*
===============
Huds_MenuEvent
===============
*/
static void Huds_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GO:
		Huds_LoadCurrent();
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_LEFT:
		ScrollList_Key( &s_huds.list, K_LEFTARROW );
		break;

	case ID_RIGHT:
		ScrollList_Key( &s_huds.list, K_RIGHTARROW );
		break;
	}
}


/*
=================
UI_HudsMenu_Key
=================
*/
static sfxHandle_t UI_HudsMenu_Key( int key ) {
	if (key == K_ENTER && &s_huds.list == Menu_ItemAtCursor(&s_huds.menu)) {
		Huds_LoadCurrent();
	}

	return Menu_DefaultKey( &s_huds.menu, key );
}

static qboolean Huds_Contains(const char *hudname) {
	int i;

	for (i = 0; i < s_huds.numHuds; i++)
		if (!Q_stricmp(s_huds.hudnames[i], hudname))
			return qtrue;

	return qfalse;
}

/*
=================
Huds_BuildList
Gets hud list, based on PlayerModel_BuildList
=================
*/
static void Huds_BuildList( void ) {
	int		numdirs;
	char	dirlist[2048];
	int		dirlen;
	char*	dirptr;
	char	filelist[2048];
	int		numfiles;
	int		filelen;
	char*	fileptr;
	char	filename[NAMEBUFSIZE];
	int		i, j;

	s_huds.numHuds = 0;

	//actually getting huds in root dir gets all huds in root dir & subdirs if files in archive
	//'getting huds in subdirs' can't get huds in root dir if huds on disk
	//so we need to try getting huds in root dir then in every subdir but with check
	//if array contains hudname already (because it maybe with same names on disk and in pk3)
	//I didn't find a better way

	//get huds in root dir
	numfiles = trap_FS_GetFileList("hud", "cfg", filelist, sizeof filelist);
	fileptr  = filelist;
		
	for (j=0; j<numfiles && s_huds.numHuds < MAX_HUDS;j++,fileptr+=filelen+1)
	{
		filelen = strlen(fileptr);

		COM_StripExtension(fileptr,filename);

		Com_sprintf( s_huds.hudnames[s_huds.numHuds++],
			sizeof( s_huds.hudnames[s_huds.numHuds] ),
			"%s", filename );
	}

	//get huds in subdirs
	numdirs = trap_FS_GetFileList("hud", "/", dirlist, sizeof dirlist);
	dirptr	= dirlist;

	for (i=0; i<numdirs && s_huds.numHuds < MAX_HUDS; i++,dirptr+=dirlen+1)
	{
		dirlen = strlen(dirptr);

		if (dirlen && dirptr[dirlen-1]=='/') dirptr[dirlen-1]='\0';

		if (!strcmp(dirptr,".") || !strcmp(dirptr,"..") || !*dirptr)
			continue;
			
		// iterate all hud files in directory
		numfiles = trap_FS_GetFileList( va("hud/%s",dirptr), "cfg", filelist, sizeof filelist );
		fileptr  = filelist;
		
		for (j=0; j<numfiles && s_huds.numHuds < MAX_HUDS;j++,fileptr+=filelen+1)
		{
			char hudname[NAMEBUFSIZE];

			filelen = strlen(fileptr);

			COM_StripExtension(fileptr,filename);

			Com_sprintf( hudname, sizeof hudname, "%s/%s", dirptr, filename );

			if (!Huds_Contains(hudname))
				Q_strncpyz(s_huds.hudnames[s_huds.numHuds++], hudname, sizeof s_huds.hudnames[0]);
		}
	}
	//set basic huds defined in code
	for ( i = 0; i < BASIC_HUDS; i++ ) {
		s_huds.list.itemnames[i] = basichud_items[i];
	}
	//set huds defined in files
	for ( i = 0; i < s_huds.numHuds; i++ ) {
		s_huds.list.itemnames[i+BASIC_HUDS] = s_huds.hudnames[i];
	}
	s_huds.list.numitems = s_huds.numHuds + BASIC_HUDS;
}

static void Huds_SelectCurrent( void ) {
	char hudname[NAMEBUFSIZE];
	menulist_s *l = &s_huds.list;
	int i;

	trap_Cvar_VariableStringBuffer("hud", hudname, sizeof hudname);

	if (*hudname) {
		for (i = 0; i < s_huds.numHuds; i++)
			if (!Q_stricmpn(hudname, s_huds.hudnames[i], sizeof hudname)) {
				//set curvalue
				l->oldvalue = l->curvalue = i + BASIC_HUDS;
				//set proper column
				if( l->columns > 1 && l->curvalue >= l->height * l->columns) {
					int c = (l->curvalue / l->height + 1) * l->height;
					l->top = c - (l->columns * l->height);
				}
				return;
			}
	} else {
		l->oldvalue = l->curvalue = abs((int)trap_Cvar_VariableValue("cg_draw2D") - 1) % BASIC_HUDS;
	}
}

/*
===============
Huds_MenuInit
===============
*/
static void Huds_MenuInit( qboolean fullscreen ) {
	int arr_y = 400;
	int btn_y = 480-64;
	int btn_x1 = 0, btn_x2 = 640;

	memset( &s_huds, 0 ,sizeof(huds_t) );
	s_huds.menu.key = UI_HudsMenu_Key;

	Huds_Cache();

	s_huds.menu.fullscreen = fullscreen;
	s_huds.menu.wrapAround = qtrue;

	s_huds.banner.generic.type	= MTYPE_BTEXT;
	s_huds.banner.generic.x		= 320;
	s_huds.banner.generic.y		= 16;
	s_huds.banner.string		= "HUDS";
	s_huds.banner.color			= color_white;
	s_huds.banner.style			= UI_CENTER;

	if (!fullscreen) {
		s_huds.framel.generic.type		= MTYPE_BITMAP;
		s_huds.framel.generic.flags		= QMF_INACTIVE;
		s_huds.framel.generic.name		= INGAME_FRAME;
		s_huds.framel.generic.x			= 320-233;//142;
		s_huds.framel.generic.y			= 240-166;//118;
		s_huds.framel.width				= 466;//359;
		s_huds.framel.height			= 332;//256;

		arr_y -= 40;
		btn_y -= 120;
		btn_x1 += 190;
		btn_x2 -= 190;
	} else {
		s_huds.framel.generic.type = MTYPE_BITMAP;
		s_huds.framel.generic.name = ART_FRAMEL;
		s_huds.framel.generic.flags = QMF_INACTIVE;
		s_huds.framel.generic.x = 0;
		s_huds.framel.generic.y = 78;
		s_huds.framel.width = 256;
		s_huds.framel.height = 329;

		s_huds.framer.generic.type = MTYPE_BITMAP;
		s_huds.framer.generic.name = ART_FRAMER;
		s_huds.framer.generic.flags = QMF_INACTIVE;
		s_huds.framer.generic.x = 376;
		s_huds.framer.generic.y = 76;
		s_huds.framer.width = 256;
		s_huds.framer.height = 334;
	}

	s_huds.arrows.generic.type	= MTYPE_BITMAP;
	s_huds.arrows.generic.name	= ART_ARROWS;
	s_huds.arrows.generic.flags	= QMF_INACTIVE;
	s_huds.arrows.generic.x		= 320-ARROWS_WIDTH/2;
	s_huds.arrows.generic.y		= arr_y;
	s_huds.arrows.width			= ARROWS_WIDTH;
	s_huds.arrows.height		= ARROWS_HEIGHT;

	s_huds.left.generic.type		= MTYPE_BITMAP;
	s_huds.left.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_huds.left.generic.x			= 320-ARROWS_WIDTH/2;
	s_huds.left.generic.y			= arr_y;
	s_huds.left.generic.id			= ID_LEFT;
	s_huds.left.generic.callback	= Huds_MenuEvent;
	s_huds.left.width				= ARROWS_WIDTH/2;
	s_huds.left.height				= ARROWS_HEIGHT;
	s_huds.left.focuspic			= ART_ARROWLEFT;

	s_huds.right.generic.type		= MTYPE_BITMAP;
	s_huds.right.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_MOUSEONLY;
	s_huds.right.generic.x			= 320;
	s_huds.right.generic.y			= arr_y;
	s_huds.right.generic.id			= ID_RIGHT;
	s_huds.right.generic.callback	= Huds_MenuEvent;
	s_huds.right.width				= ARROWS_WIDTH/2;
	s_huds.right.height				= ARROWS_HEIGHT;
	s_huds.right.focuspic			= ART_ARROWRIGHT;

	s_huds.back.generic.type		= MTYPE_BITMAP;
	s_huds.back.generic.name		= ART_BACK0;
	s_huds.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_huds.back.generic.id			= ID_BACK;
	s_huds.back.generic.callback	= Huds_MenuEvent;
	s_huds.back.generic.x			= btn_x1;
	s_huds.back.generic.y			= btn_y;
	s_huds.back.width				= 128;
	s_huds.back.height				= 64;
	s_huds.back.focuspic			= ART_BACK1;

	s_huds.go.generic.type			= MTYPE_BITMAP;
	s_huds.go.generic.name			= ART_GO0;
	s_huds.go.generic.flags			= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_huds.go.generic.id			= ID_GO;
	s_huds.go.generic.callback		= Huds_MenuEvent;
	s_huds.go.generic.x				= btn_x2;
	s_huds.go.generic.y				= btn_y;
	s_huds.go.width					= 128;
	s_huds.go.height				= 64;
	s_huds.go.focuspic				= ART_GO1;

	s_huds.list.generic.type		= MTYPE_SCROLLLIST;
	s_huds.list.generic.flags		= QMF_PULSEIFFOCUS;
	s_huds.list.generic.callback	= Huds_MenuEvent;
	s_huds.list.generic.id			= ID_LIST;
	s_huds.list.generic.x			= fullscreen ? 118 : 180;
	s_huds.list.generic.y			= 130;
	s_huds.list.width				= 16;
	s_huds.list.height				= fullscreen ? 14 : 10;
	s_huds.list.numitems			= 0;
	s_huds.list.itemnames			= (const char **)s_huds.hudlist;
	s_huds.list.columns				= fullscreen ? 3 : 2;

	Huds_BuildList();

	Menu_AddItem( &s_huds.menu, &s_huds.banner );
	Menu_AddItem( &s_huds.menu, &s_huds.framel );
	if (fullscreen)
	Menu_AddItem( &s_huds.menu, &s_huds.framer );
	Menu_AddItem( &s_huds.menu, &s_huds.list );
	Menu_AddItem( &s_huds.menu, &s_huds.arrows );
	Menu_AddItem( &s_huds.menu, &s_huds.left );
	Menu_AddItem( &s_huds.menu, &s_huds.right );
	Menu_AddItem( &s_huds.menu, &s_huds.back );
	Menu_AddItem( &s_huds.menu, &s_huds.go );

	Huds_SelectCurrent();
}

/*
=================
Huds_Cache
=================
*/
void Huds_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_GO0 );
	trap_R_RegisterShaderNoMip( ART_GO1 );
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
	trap_R_RegisterShaderNoMip( ART_ARROWS );
	trap_R_RegisterShaderNoMip( ART_ARROWLEFT );
	trap_R_RegisterShaderNoMip( ART_ARROWRIGHT );
}

/*
===============
UI_HudsMenu
===============
*/
void UI_HudsMenu( qboolean fullscreen ) {
	Huds_MenuInit(fullscreen);
	UI_PushMenu( &s_huds.menu );
}
