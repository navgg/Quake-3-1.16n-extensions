// cg_scoreboard_osp -- draw osp scoreboard on top of the game screen
// Code from nemesis mod
#include "cg_local.h"

// screen is 640 x 480
#define OSP_SB_TOP				50			// vertical distance to start of scoreboard
#define OSP_SB_HEADER			100			// vertical distance to start of player lines (header end)

#define OSP_SB_RED_XMIN			10			// LHS limit for red team area
#define OSP_SB_RED_XMAX			310			// RHS limit for red team area
#define OSP_SB_BLUE_XMIN		330			// LHS limit for blue team area
#define OSP_SB_BLUE_XMAX		630			// RHS limit for blue team area
#define OSP_SB_FREE_XMIN		170			// LHS limit for free team area
#define OSP_SB_FREE_XMAX		470			// RHS limit for free team area
#define OSP_SB_SPEC_XMIN		10			// LHS limit for spectator area
#define OSP_SB_SPEC_XMAX		630			// RHS limit for spectator area

#define OSP_SB_TSCORE_OFFSET	0			// distance into team section to draw team score
#define OSP_SB_TSTAT_OFFSET		175			// distance into team section to draw info
#define OSP_SB_TSTAT_INSET		10			// distance from OSP_SB_TOP to draw info

#define OSP_SB_TCHAR_WIDTH		8			// width of characters used for team info 
#define OSP_SB_TCHAR_HEIGHT		12			// height of characters used for team info 

#define OSP_SB_FLAG_OFFSET		5			// distance into player line to draw flag
#define OSP_SB_MODEL_OFFSET		10			// distance into player line to draw head model
#define OSP_SB_SCORE_OFFSET		45			// distance into player line to draw score
#define OSP_SB_PING_OFFSET		80			// distance into player line to draw ping
#define OSP_SB_TIME_OFFSET		115			// distance into player line to draw time
#define OSP_SB_NAME_OFFSET		150			// distance into player line to draw name

#define OSP_SB_CHAR_WIDTH		10			// width of characters used for player 
#define OSP_SB_CHAR_HEIGHT		12			// height of characters used for player 
#define OSP_SB_NAME_MAX_CHARS	15			// maximum characters to display for player name
//#define	OSP_SB_FFA				35

#define OSP_SB_LINE_HEIGHT		20  // height of player lines
#define OSP_SB_LINE_INSET		(int)((OSP_SB_LINE_HEIGHT - OSP_SB_CHAR_HEIGHT) / 2)  // ensures centered names

void CG_DrawOSPTourneyScoreboard( void );

/*
=============
CG_DrawBorder
Draws a border of set size around given dimensions
=============
*/
void CG_DrawBorder( int x, int y, int w, int h, int size, const float *borderColor ) {

	CG_FillRect( x - size, y - size, size, h + (size * 2), borderColor );  // left
	CG_FillRect( x + w, y - size, size, h + (size * 2), borderColor );  // right
	CG_FillRect( x, y - size, w, size, borderColor );  // top
	CG_FillRect( x, y + h, w, size, borderColor );  // bottom
}

/*
=====================
CG_DrawWidthGauge
=====================
*/
void CG_DrawWidthGauge( int x, int y, int width, int height, vec4_t color, int value, qboolean reverse ) {

	int shadedWidth;

	shadedWidth = (width * value) / 100;

	trap_R_SetColor( color );

	if (!reverse) {
		CG_DrawPic( x, y, shadedWidth, height, cgs.media.whiteShader );
	}
	else {
		CG_DrawPic( x + width - shadedWidth, y, shadedWidth, height, cgs.media.whiteShader );
	}

	trap_R_SetColor( NULL );
}

//X-mod: filled rect draw
void CG_DrawFilledRect( float x, float y, float w, float h, float size, const float *bgcolor, const float *bordercolor ) {
	float xs;
	float ys;

	xs = size * cgs.screenXScale;
	ys = size * cgs.screenYScale;

	CG_AdjustFrom640( &x, &y, &w, &h );

	trap_R_SetColor( bgcolor );

	trap_R_DrawStretchPic( x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_SetColor( bordercolor );

	trap_R_DrawStretchPic( x, y, xs, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x + w - xs, y, xs, h, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_DrawStretchPic( x, y, w, ys, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x, y + h - ys, w, ys, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawTeamBackground2
Nemesis - Function modified quite a bit for scoreboard, team colors, hud, etc.
================
*/
void CG_DrawTeamBackground2( int x, int y, int w, int h, float alpha, int team ) {
	vec4_t		hcolor;

	hcolor[3] = alpha;
	if (team == TEAM_RED) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	}
	else if (team == TEAM_BLUE) {
		hcolor[0] = 0.01f;
		hcolor[1] = 0;
		hcolor[2] = 1;
	}
	else if (team == TEAM_FREE) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 0;
	}
	else if (cg.showScores + FADE_TIME) {
		hcolor[0] = 1;
		hcolor[1] = 1;
		hcolor[2] = 1;
	}

	trap_R_SetColor( hcolor );
	CG_FillRect( x, y, w, h, hcolor );
}

/*
==============
CG_DrawField2
Draws large numbers for status bar and powerups
==============
*/
// Nemesis - Made more precise & modified for adjustability, can change indentation, change char sizes.

// Left indentation -
// qtrue  = add to the left
// qfalse = add to the right

int CG_DrawField2( int x, int y, int width, int chrWidth, int chrHeight, int value, qboolean leftIndent ) {
	char	num[16], *ptr;
	int		l;
	int		frame;
	int		startx;

	if (width < 1) {
		return 0;
	}

	// draw number string
	if (width > 5) {
		width = 5;
	}

	switch (width) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf( num, sizeof( num ), "%i", value );
	l = strlen( num );
	if (l > width)
		l = width;

	if (!leftIndent) {
		x -= 2 + chrWidth;
	}

	startx = x;

	ptr = num;
	while (*ptr && l) {
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		CG_DrawPic( x, y, chrWidth, chrHeight, cgs.media.numberShaders[frame] );

		x += chrWidth;
		ptr++;
		l--;
	}

	return startx;
}

/*
=============
CG_DrawLabels
Draws labels for score, ping, time and name
=============
*/
void CG_DrawLabels( int x, float *color ) {
	CG_DrawStringExt( x + 35, OSP_SB_HEADER - 10, "Score", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 80, OSP_SB_HEADER - 10, "Ping", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 120, OSP_SB_HEADER - 10, "Min", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + 150, OSP_SB_HEADER - 10, "Name", color, qtrue, qtrue, 8, 10, 0 );
}


/*
=================
CG_DrawOSPClientScore
Draws player score/ping/name starting at given coordinates
=================
*/
void CG_DrawOSPClientScore( int x, int y, clientInfo_t *ci, score_t *score ) {
	char string[1024];
	float color[4];

	color[0] = color[1] = color[2] = 1.0f;
	color[3] = 1.0f;

	// just in case?
	if ( score->client < 0 || score->client >= cgs.maxclients ) {
		Com_Printf( "Bad score->client: %i\n", score->client );
		return;
	}

	// flag or bot skill
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );

/*	} else if ( ci->botSkill > 0 && ci->botSkill <= 5 && cg_drawIcons.integer ) {
		CG_DrawPic( x + OSP_SB_FLAG_OFFSET, y, 
					OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, cgs.media.botSkillShaders[ ci->botSkill - 1 ] );*/
	}

	// 2nd flag (rtf) or headmodel
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );

	} else {
		vec3_t headAngles;
		VectorClear( headAngles );
		headAngles[YAW] = 180;
		CG_DrawHead( x + OSP_SB_MODEL_OFFSET, y,
			OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, score->client, headAngles );
	}

	// score/ping/time
	if ( score->ping == -1 ) {
		Com_sprintf( string, sizeof(string), "connecting" );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET, y, string, color, 
					      qfalse, qtrue,  
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	} else {
		Com_sprintf( string, sizeof(string), "%i", score->score );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET - 10, y, string, colorYellow, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%i", score->ping );
		CG_DrawStringExt( x + OSP_SB_PING_OFFSET, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%i", score->time );
		CG_DrawStringExt( x + OSP_SB_TIME_OFFSET + 5, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );
	}

	// name
	Com_sprintf( string, sizeof(string), "%s", ci->name );
	// In ffa & 1v1 allow max name (36)
	if ( cgs.gametype < GT_TEAM ) {
		CG_DrawStringExt( x + OSP_SB_NAME_OFFSET, y, string, color, 
				      qfalse, qtrue,
					  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, OSP_SB_NAME_MAX_CHARS + 21 );
	} else {
		CG_DrawStringExt( x + OSP_SB_NAME_OFFSET, y, string, color, 
				      qfalse, qtrue,
					  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, OSP_SB_NAME_MAX_CHARS );
	}


	// highlight players in the ready state
	if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) ) {
		// Lets change "READY" to "FROZEN" for freeze mode
		if ( Q_Isfreeze(score->client) ) {
			CG_DrawStringExt( x, y + 2, "FROZE", colorYellow, 
						  qfalse, qfalse, 
						  OSP_SB_CHAR_WIDTH - 4, OSP_SB_CHAR_HEIGHT - 4, 0 );
		} else {
			CG_DrawStringExt( x + OSP_SB_FLAG_OFFSET, y + 2, "READY", colorYellow, 
						  qfalse, qfalse, 
						  OSP_SB_CHAR_WIDTH - 4, OSP_SB_CHAR_HEIGHT - 4, 0 );
		}
	}
}


/*
=================
CG_DrawOSPTeamScoreboard
Draw the osp team scoreboard
=================
*/
void CG_DrawOSPTeamScoreboard( void ) {
	int i, tmp;
	int redTotal, blueTotal, specTotal, highTotal;
	int redPingTotal, bluePingTotal;
	int x, y;
	int redCount, blueCount, specCount;
	char *string;
	score_t	*score;
	clientInfo_t *ci;
	int ox = vScreen.offsetx;

	redTotal = blueTotal = specTotal = 0;
	redPingTotal = bluePingTotal = 0;

	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_RED ) {
			redTotal++;
			redPingTotal += score->ping;

		} else if ( ci->team == TEAM_BLUE ) {
			blueTotal++;
			bluePingTotal += score->ping;

		} else {
			specTotal++;
		}
	}

	if ( redTotal >= blueTotal ) {
		highTotal = redTotal;
	} else {
		highTotal = blueTotal;
	}

	// red header
	CG_DrawPic( ox + 86, OSP_SB_TOP - 14, 225, 15, cgs.media.scoreBarRed );

	CG_DrawTeamBackground2( ox + OSP_SB_RED_XMIN, OSP_SB_TOP,
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.25f,
						   TEAM_RED );

	//if( cg_useTeamIcons.integer ) {
		//CG_DrawPic( ox + OSP_SB_RED_XMAX - 50, OSP_SB_TOP, 50, 50, cgs.media.teamIconRed);
	//}

	// red players
	CG_DrawTeamBackground2( ox + OSP_SB_RED_XMIN, OSP_SB_HEADER,
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   redTotal * OSP_SB_LINE_HEIGHT,
						   0.15f,
						   TEAM_RED );

	CG_DrawLabels( ox + OSP_SB_RED_XMIN, colorRed );


	// blue header
	CG_DrawPic( ox + 406, OSP_SB_TOP - 14, 225, 15, cgs.media.scoreBarBlue );

	CG_DrawTeamBackground2( ox + OSP_SB_BLUE_XMIN, OSP_SB_TOP,
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.25f,
						   TEAM_BLUE );

	//if( cg_useTeamIcons.integer ) {
		//CG_DrawPic( ox + OSP_SB_BLUE_XMAX - 50, OSP_SB_TOP, 50, 50, cgs.media.teamIconBlue);
	//}
	CG_DrawLabels( ox + OSP_SB_BLUE_XMIN, colorBlue );

	// blue players
	CG_DrawTeamBackground2( ox + OSP_SB_BLUE_XMIN, OSP_SB_HEADER,
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   blueTotal * OSP_SB_LINE_HEIGHT,
						   0.15f,
						   TEAM_BLUE );
	

	// spectators
	if ( specTotal > 0 ) {
		specTotal++;
		// added spectator string since its in OSP
		CG_DrawStringExt( ox + OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 8,
			"Spectator", colorYellow, qtrue, qtrue, 12, 12, 0 );

		CG_DrawTeamBackground2( ox + OSP_SB_SPEC_XMIN, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 20,
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.20f,
							   TEAM_SPECTATOR );
	}


	// team score
	CG_DrawField2( ox + OSP_SB_RED_XMIN + 40, OSP_SB_TOP + 2, 3, 30, 36, cg.teamScores[ 0 ], qfalse );
	CG_DrawField2( ox + OSP_SB_BLUE_XMIN + 40, OSP_SB_TOP + 2, 3, 30, 36, cg.teamScores[ 1 ], qfalse );

	// work out appropiate size of labels
	tmp = CG_DrawStrlen( "Av. Ping" ) * OSP_SB_TCHAR_WIDTH + 10;  // "Av. Ping" is longest string

	// player count
	CG_DrawStringExt( ox + OSP_SB_RED_XMIN + OSP_SB_TSTAT_OFFSET - tmp,
					  OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  "PLAYERS", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	string = va( "%i", redTotal );
	CG_DrawStringExt( ox + OSP_SB_TSTAT_OFFSET,
					  OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  string, colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );

	CG_DrawStringExt( ox + OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET - tmp,
					  OSP_SB_TOP - 10, // + OSP_SB_TSTAT_INSET,
					  "PLAYERS", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	string = va( "%i", blueTotal );
	CG_DrawStringExt( ox + OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET - 10, OSP_SB_TOP - 10,// + OSP_SB_TSTAT_INSET,
					  string, colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );

	// average pings
	CG_DrawStringExt( ox + OSP_SB_RED_XMIN + OSP_SB_TSTAT_OFFSET + 100 - tmp,
					  OSP_SB_TOP - 10, 
					  "AV.PING", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	if ( redTotal > 0 ) {
		string = va( "%i", (int)( redPingTotal / redTotal) );
		CG_DrawStringExt( ox + OSP_SB_TSTAT_OFFSET + 100,
						  OSP_SB_TOP - 10,
						  string, colorWhite, 
						  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	}

	CG_DrawStringExt( ox + OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET + 100 - tmp,
					  OSP_SB_TOP - 10,
					  "AV.PING", colorWhite, 
					  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	if ( blueTotal > 0 ) {
		string = va( "%i", (int)(bluePingTotal / blueTotal ) );
		CG_DrawStringExt( ox + OSP_SB_BLUE_XMIN + OSP_SB_TSTAT_OFFSET + 90,
						  OSP_SB_TOP - 10,
						  string, colorWhite, 
						  qtrue, qtrue, OSP_SB_TCHAR_WIDTH, OSP_SB_TCHAR_WIDTH, 0 );
	}

	// player scores
	redCount = 0;
	blueCount = 0;
	specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_RED ) {
			x = OSP_SB_RED_XMIN;
			y = OSP_SB_HEADER + ( redCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else if ( ci->team == TEAM_BLUE ) {
			x = OSP_SB_BLUE_XMIN;
			y = OSP_SB_HEADER + ( blueCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (highTotal + tmp ) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (highTotal + tmp ) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;
			}
		}

		CG_DrawOSPClientScore( ox + x, y, ci, score );
	}
}


/*
=================
CG_DrawOSPFFAScoreboard
Draw the osp free for all scoreboard
=================
*/
void CG_DrawOSPFFAScoreboard( void ) {
	int i, tmp;
	int x, y;
	int playerTotal, specTotal;
	int playerCount, specCount;
	score_t	*score;
	clientInfo_t *ci;
	int ox = vScreen.offsetx;

	playerTotal = specTotal = 0;

	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_FREE ) {
			playerTotal++;
		} else {
			specTotal++;
		}
	}

	// This looks really bad.
	CG_DrawTeamBackground2( ox + OSP_SB_FREE_XMIN, OSP_SB_TOP,
						   OSP_SB_FREE_XMAX - OSP_SB_FREE_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.33f,
						   TEAM_FREE );
	CG_DrawTeamBackground2( ox + OSP_SB_FREE_XMIN, OSP_SB_HEADER,
						   OSP_SB_FREE_XMAX - OSP_SB_FREE_XMIN,
						   playerTotal * OSP_SB_LINE_HEIGHT,
						   0.20f,
						   TEAM_FREE );
	if ( specTotal > 0 ) {
		specTotal++;
		CG_DrawTeamBackground2( ox + OSP_SB_SPEC_XMIN, OSP_SB_HEADER + ( playerTotal * OSP_SB_LINE_HEIGHT ) + 10,
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.33f,
							   TEAM_SPECTATOR );
	}

	CG_DrawLabels( ox + OSP_SB_FREE_XMIN, colorWhite );

	// player scores
	playerCount = specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team == TEAM_FREE ) {
			x = OSP_SB_FREE_XMIN;
			y = OSP_SB_HEADER + ( playerCount++ * OSP_SB_LINE_HEIGHT ) + OSP_SB_LINE_INSET;

		} else {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (playerTotal + tmp ) * OSP_SB_LINE_HEIGHT + 10 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (playerTotal + tmp ) * OSP_SB_LINE_HEIGHT + 10 ) + OSP_SB_LINE_INSET;
			}
		}

		CG_DrawOSPClientScore( ox + x, y, ci, score );
	}
}

qboolean CG_DrawNormalScoreboard( void );
/*
=================
CG_DrawOSPScoreboard
Draw the osp in-game scoreboard
=================
*/
qboolean CG_DrawOSPScoreboard( void ) {
	int		x, y, w;
	float	fade;
	float	*fadeColor;
	char	*s;

	// don't draw anything if the menu or console is up
	if ( cg_paused.integer ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	if ( cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores ) {
		return qfalse;
	}

	if ( cg.showScores || 
		 cg.predictedPlayerState.pm_type == PM_DEAD ||
		 cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		
		if ( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			return qfalse;
		}
		fade = *fadeColor;
	}

	// print killer line if not 1v1
	if (cg.killerName[0] && cgs.gametype != GT_TOURNAMENT) {
		s = va("Fragged by %s", cg.killerName );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( vScreen.width - w ) / 2;
		y = 40;
		CG_DrawBigString( x, 40, s, fade );
	}

	// draw gfx icons
	// CG_DrawPic( SB_SCORE_X + (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardScore );

	if ( cgs.gametype >= GT_TEAM ) {
		CG_DrawOSPTeamScoreboard();
	} else if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawOSPTourneyScoreboard();
	} else {
		//CG_DrawOSPFFAScoreboard();
		CG_DrawNormalScoreboard();
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 ) {
		CG_LoadDeferredPlayers();
	}

	return qtrue;
}


/*
=================
CG_DrawOSPTourneyPlayerScore
Draws player name, head, score, ping, time, wins/losses
=================
*/
void CG_DrawOSPTourneyPlayerScore(clientInfo_t *ci, score_t *score, qboolean isLeft, score_t *oppScore) {
	int x, y;
	int tmp, headSize;
	int split;  // what to divide cg.time by to give time intervals
	int loopSpeed;  // animation (head turn) loop speed in seconds
	int frames;  // total number of frames in the sequence
	int midPoint;  // frame at which animation switch from left->right to right->left
	vec3_t headAngles;
	char nameString[MAX_STRING_TOKENS];
	int ox = vScreen.offsetx;

	// head - size and movement speed is based on difference in scores
	if (oppScore == NULL) {
		headSize = 32;
		loopSpeed = 16;
	} else {
		int scoreDiff = score->score - oppScore->score;
		if (scoreDiff >= 4) {
			headSize = 48;
			loopSpeed = 8;
		} else if (scoreDiff >= 2) {
			headSize = 40;
			loopSpeed = 12;
		} else if (scoreDiff >= -1) {
			headSize = 32;
			loopSpeed = 16;
		} else if (scoreDiff >= -3) {
			headSize = 24;
			loopSpeed = 20;
		} else {
			headSize = 16;
			loopSpeed = 24;
		}
	}

	if (isLeft) {
		x = OSP_SB_RED_XMIN + 26 - (headSize/2);
		y = OSP_SB_TOP + (48 - headSize)/2;
	} else {
		x = OSP_SB_BLUE_XMAX - 26 - (headSize/2);
		y = OSP_SB_TOP + (48 - headSize)/2;
	}
	VectorClear( headAngles );

	split = 10;
	frames = (loopSpeed * 1000) / split;
	midPoint = (int)(frames / 2);

	tmp = (int)(cg.time / split);  // how many time intervals
	tmp = (tmp % frames) + 1;  // which frame we are in (1 to frames)

	if (tmp > midPoint) {
		// moving right to left
		headAngles[YAW] = 270 - (int)(180.0f/midPoint * (tmp-midPoint));
	} else {
		// moving left to right
		headAngles[YAW] = 90 + (int)(180.0f/midPoint * tmp);
	}

	CG_DrawHead(ox + x, y, headSize, headSize, score->client, headAngles);

	if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client)) {
		CG_DrawStringExt(ox + x, y + (int)(headSize/2) - 4, "READY", colorYellow, qfalse, qfalse, 8, 8, 0);
	}

	// name
	if (isLeft) {
		x = OSP_SB_RED_XMIN + 50 + 2;
		y = OSP_SB_TOP + 2;
		Q_strncpyz(nameString, ci->name, sizeof(nameString));
	} else {
		// FIXME: we should right align this name, but it is a pain working out the printable
		// characters as ^ mess up %17s alignment.

		x = OSP_SB_BLUE_XMIN + 2 + 60 + 2;
		y = OSP_SB_TOP + 2;
		Q_strncpyz(nameString, ci->name, sizeof(nameString));
	}
	CG_DrawStringExt(ox + x, y, nameString, colorWhite, qfalse, qtrue, 10, 12, 18);

	// wins/loses and ping
	if (isLeft) {
		x = OSP_SB_RED_XMIN + 50 + 2;
		y = OSP_SB_TOP + 20;
	} else {
		x = OSP_SB_BLUE_XMIN + 2 + 60 + 2;
		y = OSP_SB_TOP + 20;
	}
	CG_DrawStringExt(ox + x, y, "^7Win   Loss  Ping  Time", colorWhite, qfalse, qtrue, 8, 8, 50);
	CG_DrawStringExt(ox + x, y + 10 + 2,
		va("^2%3i    ^1%3i   ^7%3i   %3i", ci->wins, ci->losses, score->ping, score->time),
		colorWhite, qfalse, qtrue, 8, 8, 50 );

	// score
	if (isLeft) {
		// if double figures or negative, adjust left
		if ((int)(score->score / 10) || score->score < 0) {
			CG_DrawField2(ox + OSP_SB_RED_XMAX - 2 - 30, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
		} else {
			CG_DrawField2(ox + OSP_SB_RED_XMAX - 2, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
		}
	} else {
		CG_DrawField2(ox + OSP_SB_BLUE_XMIN + 2 + 30 + 2, OSP_SB_TOP + 2 + 5, 3, 30, 36, score->score, qfalse);
	}
}


/*
=================
CG_DrawOSPTourneySpectatorScore
Draws player name, ping, wins/losses
=================
*/
void CG_DrawOSPTourneySpectatorScore(int x, int y, clientInfo_t *ci, score_t *score) {
	char nameString[MAX_STRING_TOKENS];

	if (score->ping == -1) {
		CG_DrawStringExt(x, y, "connecting", colorYellow, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 12);
	} else {
		CG_DrawStringExt(x, y, va("%2i/%-2i", ci->wins, ci->losses), colorYellow, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 5);
		CG_DrawStringExt(x + 55, y, va("%-3i", score->ping), colorWhite, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 5);
	}

	Q_strncpyz(nameString, ci->name, sizeof(nameString));
	CG_DrawStringExt(x + 90, y, nameString, colorWhite, qfalse, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 18);
}


/*
=================
CG_DrawOSPTourneyScoreboard
Draw the custom 1v1 scoreboard
=================
*/
void CG_DrawOSPTourneyScoreboard( void ) {
	int i, tmp;
	int x, y;
	int specTotal, specCount;
	score_t *score, *s1, *s2;
	clientInfo_t *ci;
	vec4_t hcolor;
	int ox = vScreen.offsetx;

	s1 = NULL;
	s2 = NULL;
	specTotal = 0;
	for (i = 0 ; i < cg.numScores ; i++) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[score->client];

		if (ci->team == TEAM_FREE) {
			if (!s1) {
				s1 = &cg.scores[i];
			} else if (!s2) {
				s2 = &cg.scores[i];
			}

		} else if (ci->team == TEAM_SPECTATOR) {
			specTotal++;
		}
	}

	// yellow backing
	hcolor[0] = 0.75f;
	hcolor[1] = 0.75f;
	hcolor[2] = 0.75f;
	hcolor[3] = 0.2f;

	if (s1) {
		// player one layout
		trap_R_SetColor( hcolor );
		CG_FillRect(
			ox + OSP_SB_RED_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP, 
			hcolor);
		hcolor[3] = 0.4f;
		CG_DrawBorder(
			ox + OSP_SB_RED_XMIN,
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor);

		ci = &cgs.clientinfo[s1->client];
		CG_DrawOSPTourneyPlayerScore(ci, s1, qtrue, s2);
	}

	if (s2) {
		// player two layout
		hcolor[3] = 0.2f;
		CG_FillRect(
			ox + OSP_SB_BLUE_XMIN,
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			hcolor);
		hcolor[3] = 0.4f;
		CG_DrawBorder(
			ox + OSP_SB_BLUE_XMIN,
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor);

		ci = &cgs.clientinfo[s2->client];
		CG_DrawOSPTourneyPlayerScore(ci, s2, qfalse, s1);
	}

	// spectator layout
	if ( specTotal > 0 ) {
		specTotal++;

		CG_DrawStringExt( ox + OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + OSP_SB_LINE_HEIGHT + 8,
			"Spectator", colorYellow, qtrue, qtrue, 12, 12, 0 );

		CG_DrawTeamBackground2( ox + OSP_SB_SPEC_XMIN, OSP_SB_HEADER + OSP_SB_LINE_HEIGHT + 20,
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.20f,
							   TEAM_SPECTATOR );
	}

	specCount = 0;
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( ci->team != TEAM_FREE ) {
			tmp = (int)( ( specCount++ ) / 2 );

			if ( (specCount % 2) == 0 ) {
				x = OSP_SB_BLUE_XMIN;
				y = OSP_SB_HEADER + ( (tmp+1) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;

			} else {
				x = OSP_SB_RED_XMIN;
				y = OSP_SB_HEADER + ( (tmp+1) * OSP_SB_LINE_HEIGHT + 20 ) + OSP_SB_LINE_INSET;
			}

			CG_DrawOSPTourneySpectatorScore(ox + x, y, ci, score);
		}
	}
}

//OSP windows
//cg_window.c

// String buffer handling
void CG_initStrings( void ) {
	int i;

	for (i = 0; i<MAX_STRINGS; i++) {
		cg.aStringPool[i].fActive = qfalse;
		cg.aStringPool[i].str[0] = 0;
	}
}

// Windowing system setup
void CG_windowInit( void ) {
	int i;

	cg.winHandler.numActiveWindows = 0;
	for (i = 0; i<MAX_WINDOW_COUNT; i++) {
		cg.winHandler.window[i].inuse = qfalse;
	}
	CG_initStrings();
}

qboolean CG_addString( cg_window_t *w, char *buf ) {
	int i;

	// Check if we're reusing the current buf
	if (w->lineText[w->lineCount] != NULL) {
		for (i = 0; i<MAX_STRINGS; i++) {
			if (!cg.aStringPool[i].fActive) continue;

			if (w->lineText[w->lineCount] == (char *)&cg.aStringPool[i].str) {
				w->lineCount++;
				cg.aStringPool[i].fActive = qtrue;
				strcpy( cg.aStringPool[i].str, buf );

				return(qtrue);
			}
		}
	}

	for (i = 0; i<MAX_STRINGS; i++) {
		if (!cg.aStringPool[i].fActive) {
			cg.aStringPool[i].fActive = qtrue;
			strcpy( cg.aStringPool[i].str, buf );
			w->lineText[w->lineCount++] = (char *)&cg.aStringPool[i].str;

			return(qtrue);
		}
	}

	return(qfalse);
}

void CG_printWindow( char *str ) {
	int pos = 0, pos2 = 0;
	char buf[MAX_STRING_CHARS];
	cg_window_t *w = cg.windowCurrent;

	if (w == NULL) return;

	// Silly logic for a strict format
	Q_strncpyz( buf, str, MAX_STRING_CHARS );
	while (buf[pos] > 0 && w->lineCount < MAX_WINDOW_LINES) {
		if (buf[pos] == '\n') {
			if (pos2 == pos) {
				if (!CG_addString( w, " " )) {
					return;
				}
			}
			else {
				buf[pos] = 0;
				if (!CG_addString( w, buf + pos2 )) {
					return;
				}
			}
			pos2 = ++pos;
			continue;
		}
		pos++;
	}

	if (pos2 < pos) {
		CG_addString( w, buf + pos2 );
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
		CG_Printf( "%s\n", str );
}

// Window stuct "constructor" with some common defaults
void CG_windowReset( cg_window_t *w, int fx, int startupLength ) {
	vec4_t colorGeneralBorder = { 0, 0, 0, 0.75f };
	vec4_t colorGeneralFill = { 0.2f, 0.2f, 0.2f, 0.6f };

	w->effects = fx;
	w->fontScaleX = 0.25;
	w->fontScaleY = 0.25;
	w->flashPeriod = 1000;
	w->flashMidpoint = w->flashPeriod / 2;
	w->id = WID_NONE;
	w->inuse = qtrue;
	w->lineCount = 0;
	w->state = (fx >= WFX_FADEIN) ? WSTATE_START : WSTATE_COMPLETE;
	w->targetTime = (startupLength > 0) ? startupLength : 0;
	w->time = trap_Milliseconds();
	w->x = 0;
	w->y = 0;

	memcpy( &w->colorBorder, &colorGeneralBorder, sizeof( vec4_t ) );
	memcpy( &w->colorBackground, &colorGeneralFill, sizeof( vec4_t ) );
}

// Reserve a window
cg_window_t *CG_windowAlloc( int fx, int startupLength ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	if (wh->numActiveWindows == MAX_WINDOW_COUNT) return(NULL);

	for (i = 0; i<MAX_WINDOW_COUNT; i++) {
		w = &wh->window[i];
		if (w->inuse == qfalse) {
			CG_windowReset( w, fx, startupLength );
			wh->activeWindows[wh->numActiveWindows++] = i;
			return(w);
		}
	}

	// Fail if we're a full airplane
	return(NULL);
}

// Set the window width and height based on the windows text/font parameters
void CG_windowNormalizeOnText( cg_window_t *w ) {
	int i, tmp;

	if (w == NULL) return;

	w->w = 0;
	w->h = 0;

	w->fontWidth = w->fontScaleX * WINDOW_FONTWIDTH;
	w->fontHeight = w->fontScaleY * WINDOW_FONTHEIGHT;

	for (i = 0; i<w->lineCount; i++) {
		tmp = CG_DrawStrlen( (char*)w->lineText[i] ) * w->fontWidth;

		if (tmp > w->w) {
			w->w = tmp;
		}
	}

	for (i = 0; i<w->lineCount; i++) {
		w->lineHeight[i] = w->fontHeight;
		w->h += w->lineHeight[i] + 3;
	}

	// Border + margins
	w->w += 10;
	w->h += 3;

	// Set up bottom alignment
	if (w->x < 0) w->x += 640 - w->w;
	if (w->y < 0) w->y += 480 - w->h;
}



void CG_removeStrings( cg_window_t *w ) {
	int i, j;

	for (i = 0; i<w->lineCount; i++) {
		char *ref = w->lineText[i];

		for (j = 0; j<MAX_STRINGS; j++) {
			if (!cg.aStringPool[j].fActive) continue;

			if (ref == (char *)&cg.aStringPool[j].str) {
				w->lineText[i] = NULL;
				cg.aStringPool[j].fActive = qfalse;
				cg.aStringPool[j].str[0] = 0;

				break;
			}
		}
	}
}

// Free up a window reservation
void CG_windowFree( cg_window_t *w ) {
	int i, j;
	cg_windowHandler_t *wh = &cg.winHandler;

	if (w == NULL) return;

	if (w->effects >= WFX_FADEIN && w->state != WSTATE_OFF && w->inuse == qtrue) {
		w->state = WSTATE_SHUTDOWN;
		w->time = trap_Milliseconds();
		return;
	}

	for (i = 0; i<wh->numActiveWindows; i++) {
		if (w == &wh->window[wh->activeWindows[i]]) {
			for (j = i; j<wh->numActiveWindows; j++) {
				if (j + 1 < wh->numActiveWindows) {
					wh->activeWindows[j] = wh->activeWindows[j + 1];
				}
			}

			w->id = WID_NONE;
			w->inuse = qfalse;
			w->state = WSTATE_OFF;

			CG_removeStrings( w );

			wh->numActiveWindows--;

			break;
		}
	}
}

void CG_windowCleanup( void ) {
	int i;
	cg_window_t *w;
	cg_windowHandler_t *wh = &cg.winHandler;

	for (i = 0; i<wh->numActiveWindows; i++) {
		w = &wh->window[wh->activeWindows[i]];
		if (!w->inuse || w->state == WSTATE_OFF) {
			CG_windowFree( w );
			i--;
		}
	}
}

// Main window-drawing handler
void CG_windowDraw( void ) {
	int h, x, y, i, j, milli, t_offset, tmp;
	cg_window_t *w;
	qboolean fCleanup = qfalse;
	vec4_t *bg;
	vec4_t textColor, borderColor, bgColor;

	if (cg.winHandler.numActiveWindows == 0) {
		return;
	}

	milli = trap_Milliseconds();
	memcpy( textColor, colorWhite, sizeof( vec4_t ) );

	for (i = 0; i<cg.winHandler.numActiveWindows; i++) {
		w = &cg.winHandler.window[cg.winHandler.activeWindows[i]];

		if (!w->inuse || w->state == WSTATE_OFF) {
			fCleanup = qtrue;
			continue;
		}

		if (w->effects & WFX_TEXTSIZING) {
			CG_windowNormalizeOnText( w );
			w->effects &= ~WFX_TEXTSIZING;
		}

		bg = ((w->effects & WFX_FLASH) && (milli % w->flashPeriod) > w->flashMidpoint) ? &w->colorBackground2 : &w->colorBackground;

		h = w->h;
		x = w->x;
		y = w->y;
		t_offset = milli - w->time;
		textColor[3] = 1.0f;
		memcpy( &borderColor, w->colorBorder, sizeof( vec4_t ) );
		memcpy( &bgColor, bg, sizeof( vec4_t ) );

		tmp = w->targetTime - t_offset;

		// Window started - FIXME: Are calculations correct?
		if (w->state == WSTATE_START) {
			// Scroll up
			if (w->effects & WFX_SCROLLUP) {
				if (tmp > 0) {
					y += (480 - y) * tmp / w->targetTime;
				}
				else {
					w->state = WSTATE_COMPLETE;
				}

				w->curY = y;
			}
			// Scroll down
			if (w->effects & WFX_SCROLLDOWN) {
				if (tmp > 0) {
					y -= (0 + y) * tmp / w->targetTime;
				}
				else {
					w->state = WSTATE_COMPLETE;
				}

				w->curY = y;
			}
			// Scroll right
			if (w->effects & WFX_SCROLLRIGHT) {
				if (tmp > 0) {
					x -= (640 - x) * tmp / w->targetTime;
				}
				else {
					w->state = WSTATE_COMPLETE;
				}

				w->curX = x;
			}
			// Scroll left
			if (w->effects & WFX_SCROLLLEFT) {
				if (tmp > 0) {
					x += (640 - x) * tmp / w->targetTime;
				}
				else {
					w->state = WSTATE_COMPLETE;
				}

				w->curX = x;
			}
			// Fade in
			if (w->effects & WFX_FADEIN) {
				if (tmp > 0) {
					textColor[3] = (float)((float)t_offset / (float)w->targetTime);
				}
				else {
					w->state = WSTATE_COMPLETE;
				}
			}
			// Window Shutdown FIXME: Are calculations correct?
		}
		else if (w->state == WSTATE_SHUTDOWN) {
			// Fade in
			if (w->effects & WFX_FADEIN) {
				if (tmp > 0) {
					textColor[3] -= (float)((float)t_offset / (float)w->targetTime);
				}
				else {
					textColor[3] = 0.0f;
					w->state = WSTATE_OFF;
				}
			}
			// Scroll up
			if (w->effects & WFX_SCROLLUP) {
				if (tmp > 0) y = w->curY - (480 + w->y) * t_offset / w->targetTime;
				if (tmp < 0 || y >= 480) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			// Scroll right
			if (w->effects & WFX_SCROLLRIGHT) {
				if (tmp > 0) x = w->curX + (640 - w->x) * t_offset / w->targetTime;
				if (tmp < 0 || x >= 640) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
			// Scroll left
			if (w->effects & WFX_SCROLLLEFT) {
				if (tmp > 0) x = w->curX - (640 - w->x) * t_offset / w->targetTime;
				if (tmp < 0 || x >= 640) {
					w->state = WSTATE_OFF;
					fCleanup = qtrue;
					continue;
				}
			}
		}

		borderColor[3] *= textColor[3];
		bgColor[3] *= textColor[3];

		CG_DrawFilledRect( x, y, w->w, h, 1.25f, bgColor, borderColor );

		for (j = w->lineCount - 1; j >= 0; j--) {
			h -= (w->lineHeight[j] + 3);

			CG_DrawStringExt( x + 5, y + h, (char*)w->lineText[j], textColor, qfalse, qtrue, w->fontWidth, w->fontHeight, 0 );
		}
	}

	if (fCleanup) {
		CG_windowCleanup();
	}
}

//stats windows

void CG_statsWindow( void ) {
	int weaponCount;
	int kills, deaths, score;
	//float efficiency;
	char *weaponName, *damageGivenStr;
	const char		*line = "^7.............................................";
	qboolean interMission = cg.snap->ps.pm_type == PM_INTERMISSION;

	int i, j, w;
	cg_window_t *sw;

	//window already open
	if (cg.statsWindow)
		return;

	if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && interMission)
		return;

	sw = CG_windowAlloc( WFX_TEXTSIZING | (interMission ? WFX_FADEIN : 0), 350 );

	cg.statsWindow = sw;
	if (sw == NULL) return;

	// work out how many weapons to display for positioning
	weaponCount = 0;
	for (i = WP_GAUNTLET; i <= WP_BFG; i++) {
		if (stats.weaponShots[i] != 0 || stats.weaponKills[i] != 0 || stats.weaponDeaths[i] != 0) {
			weaponCount++;
		}
	}

	// Window specific
	sw->state = WSTATE_START;
	sw->id = WID_ACC;
	sw->fontScaleX = 1.0f;
	sw->fontScaleY = 1.0f;
	sw->x = 8;

	sw->flashPeriod = 700;
	sw->flashMidpoint = sw->flashPeriod * 0.5f;

	// gather stats
	kills = 0;
	deaths = 0;
	for (i = WP_GAUNTLET; i <= WP_BFG; i++) {
		kills += stats.weaponKills[i];
		deaths += stats.weaponDeaths[i];
	}

	score = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].client == cg.clientNum) {
			score = cg.scores[i].score;  //kills - stats.suicides;
			break;
		}
	}

	if (kills == 0)
		j = 0;
	else
		j = kills * 1000 / (kills + deaths + stats.suicides);

	w = j / 10;

	cg.windowCurrent = sw;

	if (interMission && cgs.serverMod != SM_NOGHOST)
		CG_Printf( "\n", line );

	CG_printWindow( "   ^3Score   ^3Kills  ^3Deaths   ^3SelfK      ^3Eff " );
	CG_printWindow( va( "^7%8i%8i%8i%8i  %5i.%i%%", score, kills, deaths, stats.suicides,
		w, j - w * 10 ) );

	//if (cgs.gametype == GT_CTF) {
	//	sw->y -= 20;

	//	CG_printWindow( "    ^3FCap    FRet   FTime           FCRatio" );
	//	CG_printWindow( va("^7%8i%8i%8i\n\n", stats.flagsCaptured, stats.flagsReturned, stats.flagTime) );
	//}
	//else {
	//	CG_printWindow( "\n\n" );
	//}	

	CG_printWindow( va( "\n^3Weapon           ^3Acc    ^3   Hits    ^3   K  ^3   D\n%s", line ) );

	for (i = WP_GAUNTLET; i <= WP_BFG; i++) {
		if (stats.weaponShots[i] == 0 && stats.weaponKills[i] == 0 && stats.weaponDeaths[i] == 0) {
			continue;
		}

		switch (i) {
		case WP_GAUNTLET: weaponName = "Gauntlet"; break;
		case WP_MACHINEGUN: weaponName = "Machinegun"; break;
		case WP_SHOTGUN: weaponName = "Shotgun"; break;
		case WP_GRENADE_LAUNCHER: weaponName = "Grenade L."; break;
		case WP_ROCKET_LAUNCHER: weaponName = "Rocket L."; break;
		case WP_LIGHTNING: weaponName = "Lightning Gun"; break;
		case WP_RAILGUN: weaponName = "Railgun"; break;
		case WP_PLASMAGUN: weaponName = "Plasma Gun"; break;
		case WP_BFG: weaponName = "BFG"; break;
		default: weaponName = "Unknown"; break;
		}

		if (stats.weaponHits[i] == 0)
			j = 0;
		else
			j = stats.weaponHits[i] * 1000 / stats.weaponShots[i];

		w = j / 10;

		CG_printWindow( va( "^7%-13s  ^5%3i.%i%%   ^7%4i/%-4i  ^7%4i  ^7%4i",
			weaponName,
			w, j - w * 10,
			stats.weaponHits[i],
			stats.weaponShots[i],
			stats.weaponKills[i],
			stats.weaponDeaths[i] ) );
	}

	// fix high damage given
	j = stats.damageGiven;
	if (j >= 1000000)
		damageGivenStr = va( "%5ik", j / 1000 );
	else
		damageGivenStr = va( "%6i", j );

	CG_printWindow( va( "\n^3Damage Done   ^7%6s   ^3Armor  ^7%5i   ^3^7%-2i^1R ^7%-2i^3Y^7",
		damageGivenStr,
		stats.armorTotal,
		stats.armorRA,
		stats.armorYA ) );

	CG_printWindow( va( "^3Damage Taken  ^7%6i  ^3Health  ^7%5i ^3      ^7%-2i^5M^7",
		stats.damageReceived,
		stats.healthTotal,
		stats.healthMH ) );

	sw->y = hud.sbteambg_y - (sw->lineCount) * (sw->fontScaleY * WINDOW_FONTHEIGHT + 3) - 8;

	if (interMission) {
		sw->y += hud.head_size;
		CG_Printf("\n", line);
	}
}

void CG_statsWindowFree( int weffects ) {
	if (!cg.statsWindow)
		return;

	if (weffects)
		cg.statsWindow->effects = weffects;

	CG_windowFree( cg.statsWindow );
	cg.statsWindow = NULL;
}

/*
=========================
CG_WeaponAccCheck
==========================
*/
void CG_WeaponAccCheck( void ) {
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;

	if (stats.lastWeaponHit != cg.snap->ps.persistant[PERS_ACCURACY_HITS]) {
		stats.weaponHits[cg.snap->ps.weapon] += cg.snap->ps.persistant[PERS_ACCURACY_HITS] - stats.lastWeaponHit;
	}

	stats.lastWeaponHit = cg.snap->ps.persistant[PERS_ACCURACY_HITS];

	if (stats.lastWeaponShot != cg.snap->ps.persistant[PERS_ACCURACY_SHOTS]) {
		stats.weaponShots[cg.snap->ps.weapon] += cg.snap->ps.persistant[PERS_ACCURACY_SHOTS] - stats.lastWeaponShot;
	}

	stats.lastWeaponShot = cg.snap->ps.persistant[PERS_ACCURACY_SHOTS];
}

void CGX_UpdateItemPickupStats( entityState_t *es, gitem_t *item ) {
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;

	if (es->clientNum == cg.clientNum) {
		// ok, we just picked something up

		if (!strcmp( item->classname, "item_armor_combat" ))
			stats.armorYA++;
		else if (!strcmp( item->classname, "item_armor_body" ))
			stats.armorRA++;
		else if (!strcmp( item->classname, "item_health_mega" ))
			stats.healthMH++;
	}
}

void CGX_UpdateDamageStats( playerState_t *ps, playerState_t *ops ) {
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;

	if (ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM])
		return;
	
	if (ps->persistant[PERS_HITS] > ops->persistant[PERS_HITS]) {
		int given = ps->persistant[PERS_HITS] - ops->persistant[PERS_HITS];
		if (given > ops->stats[STAT_HEALTH])
			given = ops->stats[STAT_HEALTH];
		D_Printf( ("Damaged ^3%i (%i)\n", given, ps->persistant[PERS_HITS] - ops->persistant[PERS_HITS]) );
		stats.damageGiven += given;
	}

	if (ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH]) {
		int received = ops->stats[STAT_HEALTH] - ps->stats[STAT_HEALTH];

		if (!(received == 1 && ps->stats[STAT_HEALTH] >= 100)) {
			if (received > ops->stats[STAT_HEALTH])
				received = ops->stats[STAT_HEALTH];
			if (received < 0)
				received = 0;
			D_Printf( ("Health -^3%i (%i)\n", received, ops->stats[STAT_HEALTH] - ps->stats[STAT_HEALTH]) );
			stats.damageReceived += received;
		}
	} else if (ps->stats[STAT_HEALTH] > ops->stats[STAT_HEALTH] && ops->stats[STAT_HEALTH] > 0) {
		D_Printf( ("Helath +^3%i\n", ps->stats[STAT_HEALTH] - ops->stats[STAT_HEALTH]) );
		stats.healthTotal += ps->stats[STAT_HEALTH] - ops->stats[STAT_HEALTH];
	}

	if (ps->stats[STAT_ARMOR] < ops->stats[STAT_ARMOR]) {
		int received = ops->stats[STAT_ARMOR] - ps->stats[STAT_ARMOR];

		if (!(received == 1 && ps->stats[STAT_ARMOR] >= 100)) {
			D_Printf( ("Armor -^3%i\n", received) );
			stats.damageReceived += received;
		}
	} else if (ps->stats[STAT_ARMOR] > ops->stats[STAT_ARMOR] && ops->stats[STAT_HEALTH] > 0) {
		D_Printf( ("Armor +^3%i\n", ps->stats[STAT_ARMOR] - ops->stats[STAT_ARMOR]) );
		stats.armorTotal += ps->stats[STAT_ARMOR] - ops->stats[STAT_ARMOR];
	}
}


#define WP_KillsDeaths(w) if (target == cg.clientNum) { stats.weaponDeaths[w]++; } else if (attacker == cg.clientNum) { stats.weaponKills[w]++; }

void CGX_UpdateKillsDeathsStats( int mod, int target, int attacker ) {
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;

	if ((attacker == ENTITYNUM_WORLD || attacker == target) && target == cg.clientNum)
		stats.suicides++;
	else if (attacker != ENTITYNUM_WORLD)
	switch (mod) {
	case MOD_GAUNTLET:
		WP_KillsDeaths( WP_GAUNTLET )
		break;
	case MOD_MACHINEGUN:
		WP_KillsDeaths( WP_MACHINEGUN )
		break;
	case MOD_SHOTGUN:
		WP_KillsDeaths( WP_SHOTGUN )
		break;
	case MOD_GRENADE:
	case MOD_GRENADE_SPLASH:
		WP_KillsDeaths( WP_GRENADE_LAUNCHER )
		break;
	case MOD_ROCKET:
	case MOD_ROCKET_SPLASH:
		WP_KillsDeaths( WP_ROCKET_LAUNCHER )
		break;
	case MOD_PLASMA:
	case MOD_PLASMA_SPLASH:
		WP_KillsDeaths( WP_PLASMAGUN )
		break;
	case MOD_RAILGUN:
		WP_KillsDeaths( WP_RAILGUN )
		break;
	case MOD_LIGHTNING:
		WP_KillsDeaths( WP_LIGHTNING )
		break;
	case MOD_BFG:
	case MOD_BFG_SPLASH:
		WP_KillsDeaths( WP_BFG )
		break;
	case MOD_TELEFRAG:
		break;
	}
}
