// cg_scoreboard_osp -- draw osp scoreboard on top of the game screen
// Code from nemesis mod
#include "cg_local.h"

// screen is 640 x 480
#define OSP_SB_TOP				64			// vertical distance to start of scoreboard
#define OSP_SB_HEADER			108			// vertical distance to start of player lines (header end)

#define OSP_SB_RED_XMIN			10			// LHS limit for red team area
#define OSP_SB_RED_XMAX			315			// RHS limit for red team area
#define OSP_SB_BLUE_XMIN		325			// LHS limit for blue team area
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

#define OSP_SB_FLAG_OFFSET		0			// distance into player line to draw flag
#define OSP_SB_MODEL_OFFSET		12			// distance into player line to draw head model
#define OSP_SB_SCORE_OFFSET		45			// distance into player line to draw score
#define OSP_SB_PING_OFFSET		80			// distance into player line to draw ping
#define OSP_SB_TIME_OFFSET		117			// distance into player line to draw time
#define OSP_SB_NAME_OFFSET		155			// distance into player line to draw name

#define OSP_SB_CHAR_WIDTH		10			// width of characters used for player 
#define OSP_SB_CHAR_HEIGHT		12			// height of characters used for player 
#define OSP_SB_NAME_MAX_CHARS	15			// maximum characters to display for player name
//#define	OSP_SB_FFA				35

#define OSP_SB_LINE_HEIGHT		20  // height of player lines
#define OSP_SB_LINE_INSET		(int)((OSP_SB_LINE_HEIGHT - OSP_SB_CHAR_HEIGHT) / 2)  // ensures centered names

void CG_DrawOSPTourneyScoreboard( void );

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

extern void CG_DrawFieldEx(int x, int y, int width, int value, float *rgba, int charWidth, int charHeight);

/*
=============
CG_DrawLabels
Draws labels for score, ping, time and name
=============
*/
void CG_DrawLabels( int x, float *color ) {
#define OSP_SB_LABEL_HEIGHT 14
	//CG_DrawStringExt( x, OSP_SB_HEADER + 2 + 1 - OSP_SB_LABEL_HEIGHT, "Id", color, qtrue, qtrue, 6, 8, 0 );
	CG_DrawStringExt( x + 35, OSP_SB_HEADER + 2 - OSP_SB_LABEL_HEIGHT, "Score", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + OSP_SB_PING_OFFSET, OSP_SB_HEADER + 2 - OSP_SB_LABEL_HEIGHT, "Ping", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + OSP_SB_TIME_OFFSET, OSP_SB_HEADER + 2 - OSP_SB_LABEL_HEIGHT, "Time", color, qtrue, qtrue, 8, 10, 0 );
	CG_DrawStringExt( x + OSP_SB_NAME_OFFSET, OSP_SB_HEADER + 2 - OSP_SB_LABEL_HEIGHT, "Name", color, qtrue, qtrue, 8, 10, 0 );
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

	//x-mod: highlight player
	if (score->client == cg.snap->ps.clientNum && cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
		float	hcolor[4] = { 0.7, 0.7, 0.7, 0.25 };
		CG_FillRect( x, y - 4, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, OSP_SB_LINE_HEIGHT, hcolor );
	}

	// flag or bot skill
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_FLAG_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );

	} else if ( ci->botSkill > 0 && ci->botSkill <= 5 && cg_drawIcons.integer ) {
		CG_DrawPic( x + OSP_SB_FLAG_OFFSET, y, 
					OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, cgs.media.botSkillShaders[ ci->botSkill - 1 ] );
	}

	// 2nd flag (rtf) or headmodel
	if ( ci->powerups & ( 1 << PW_REDFLAG ) && ci->team == TEAM_RED ) {
		CG_DrawFlagModel( x + OSP_SB_MODEL_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_RED );

	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) && ci->team == TEAM_BLUE ) {
		CG_DrawFlagModel( x + OSP_SB_MODEL_OFFSET, y, 
						  OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, TEAM_BLUE );
	} else {
		vec3_t headAngles;
		VectorClear( headAngles );
		headAngles[YAW] = 180;
		CG_DrawHead( x + OSP_SB_MODEL_OFFSET, y,
			OSP_SB_CHAR_HEIGHT, OSP_SB_CHAR_HEIGHT, score->client, headAngles );
	}

	if ( score->isReferee ) {
		CG_DrawStringExt( x + OSP_SB_MODEL_OFFSET + OSP_SB_CHAR_HEIGHT, y, "^2R", color,
			qfalse, qtrue,
			OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );
	}

	//x-mod: player's id
	{
		vec4_t idcolor = { 1, 1, 1, 0.66 };
		int char_h = OSP_SB_CHAR_HEIGHT * 16 / 12 / 2;
		int char_w = OSP_SB_CHAR_HEIGHT / 2;
		Com_sprintf(string, sizeof(string), "%i", score->client);
		CG_DrawStringExt(x + OSP_SB_FLAG_OFFSET + (2 - CG_DrawStrlen(string)) * char_w / 2, 
			y + (OSP_SB_CHAR_HEIGHT - char_h) / 2, 
			string, idcolor, qfalse, qfalse,
			char_w, char_h, 2);
	}

	// score/ping/time
	if ( score->ping == -1 ) {
		Com_sprintf( string, sizeof(string), "connecting" );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET, y, string, color, 
					      qfalse, qtrue,  
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	} else {
		Com_sprintf( string, sizeof(string), "%4i", score->score );
		CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET - 9, y, string, colorYellow, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%3i", score->ping );
		CG_DrawStringExt( x + OSP_SB_PING_OFFSET + 3, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		Com_sprintf( string, sizeof(string), "%3i", score->time );
		CG_DrawStringExt( x + OSP_SB_TIME_OFFSET + 3, y, string, color, 
					      qfalse, qtrue,
						  OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

		//Com_sprintf( string, sizeof(string),
		//	"%3i ^7%3i ^7%3i", score->score, score->ping, score->time);

		//CG_DrawStringExt( x + OSP_SB_SCORE_OFFSET, y, string, colorYellow, 
		//			qfalse, qtrue,
		//			OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );
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
#if CGX_FREEZE
		//if ( Q_Isfreeze(score->client) ) {
		//	CG_DrawStringExt( x, y + 2, "FROZE", colorYellow, 
		//				  qfalse, qfalse, 
		//				  OSP_SB_CHAR_WIDTH - 4, OSP_SB_CHAR_HEIGHT - 4, 0 );
		//} else
		//x-mod: let's use deferred icon
		if ( !Q_Isfreeze( score->client ) )
#endif
		{
			CG_DrawStringExt( x + 2, y + 2, "READY", colorYellow, 
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
	//CG_DrawPic( ox + 86, OSP_SB_TOP - 14, 225, 15, cgs.media.scoreBarRed );

	CG_DrawTeamBackground2( ox + OSP_SB_RED_XMIN, OSP_SB_TOP,
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.2f,
						   TEAM_RED );

	// red players
	CG_DrawTeamBackground2( ox + OSP_SB_RED_XMIN, OSP_SB_HEADER - OSP_SB_LABEL_HEIGHT,
						   OSP_SB_RED_XMAX - OSP_SB_RED_XMIN,
						   redTotal * OSP_SB_LINE_HEIGHT + OSP_SB_LABEL_HEIGHT,
						   0.15f,
						   TEAM_RED );

	CG_DrawLabels( ox + OSP_SB_RED_XMIN, colorRed );

	CG_DrawTeamBackground2( ox + OSP_SB_BLUE_XMIN, OSP_SB_TOP,
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   OSP_SB_HEADER - OSP_SB_TOP,
						   0.2f,
						   TEAM_BLUE );

	CG_DrawLabels( ox + OSP_SB_BLUE_XMIN, colorBlue );

	// blue players
	CG_DrawTeamBackground2( ox + OSP_SB_BLUE_XMIN, OSP_SB_HEADER - OSP_SB_LABEL_HEIGHT,
						   OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
						   blueTotal * OSP_SB_LINE_HEIGHT + OSP_SB_LABEL_HEIGHT,
						   0.15f,
						   TEAM_BLUE );
	

	// spectators
	if ( specTotal > 0 ) {
		specTotal++;
		// added spectator string since its in OSP
		CG_DrawStringExt( ox + OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 8 - 2,
			"Spectator", colorYellow, qtrue, qtrue, 12, 12, 0 );

		CG_DrawTeamBackground2( ox + OSP_SB_SPEC_XMIN, OSP_SB_HEADER + ( highTotal * OSP_SB_LINE_HEIGHT ) + 20,
							   OSP_SB_SPEC_XMAX - OSP_SB_SPEC_XMIN,
							   (int)(specTotal / 2) * OSP_SB_LINE_HEIGHT,
							   0.20f,
							   TEAM_SPECTATOR );
	}

#define OSB_HEADER_PADX 3
	// team score
	CG_DrawFieldEx( ox + OSP_SB_RED_XMAX - 20 * 5 - OSB_HEADER_PADX, OSP_SB_TOP + 2, 5, cg.teamScores[ 0 ], colorWhite, 20, 26 );
	CG_DrawFieldEx( ox + OSP_SB_BLUE_XMIN + OSB_HEADER_PADX, OSP_SB_TOP + 2, -5, cg.teamScores[ 1 ], colorWhite, 20, 26 );

	// work out appropiate size of labels
#define OSP_SB_COUNT_OFFS 2
	// player count
	y = OSP_SB_TOP + OSP_SB_COUNT_OFFS - 1;// + OSP_SB_TSTAT_INSET,
	x = OSP_SB_RED_XMIN + ox + OSB_HEADER_PADX;
	string = va("Players %3i", redTotal);
	CG_DrawStringExt( x, y, string, colorWhite, 
					  qtrue, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	y += OSP_SB_CHAR_HEIGHT + 2;
	string = va("Av.Ping %3i", redTotal ? redPingTotal / redTotal : 0);
	CG_DrawStringExt( x, y, string, colorWhite, 
					  qtrue, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	y = OSP_SB_TOP + OSP_SB_COUNT_OFFS - 1;// + OSP_SB_TSTAT_INSET,
	x = OSP_SB_BLUE_XMAX + ox - OSB_HEADER_PADX - CG_DrawStrlen( "Players %3i" ) * OSP_SB_CHAR_WIDTH;
	string = va("Players %3i", blueTotal);
	CG_DrawStringExt( x, y, string, colorWhite, 
					  qtrue, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

	y += OSP_SB_CHAR_HEIGHT + 2;
	string = va("Av.Ping %3i", blueTotal ? bluePingTotal / blueTotal : 0);
	CG_DrawStringExt( x, y, string, colorWhite, 
					  qtrue, qtrue, OSP_SB_CHAR_WIDTH, OSP_SB_CHAR_HEIGHT, 0 );

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
	if ( cg.warmup && !cg.showScores && cg.predictedPlayerState.pm_type != PM_INTERMISSION) {
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
		y = OSP_SB_TOP - BIGCHAR_HEIGHT - 4;
		CG_DrawBigString( x, y, s, fade );
	}

	// draw gfx icons
	// CG_DrawPic( SB_SCORE_X + (SB_RATING_WIDTH / 2), y, 64, 32, cgs.media.scoreboardScore );

	if ( cgs.gametype >= GT_TEAM ) {
		CG_DrawOSPTeamScoreboard();
	} else if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawOSPTourneyScoreboard();
	} else {
		CG_DrawOSPFFAScoreboard();
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
	//if (oppScore == NULL) {
	//	headSize = 32;
	//	loopSpeed = 16;
	//} else {
	//	int scoreDiff = score->score - oppScore->score;
	//	if (scoreDiff >= 4) {
	//		headSize = 48;
	//		loopSpeed = 8;
	//	} else if (scoreDiff >= 2) {
	//		headSize = 40;
	//		loopSpeed = 12;
	//	} else if (scoreDiff >= -1) {
	//		headSize = 32;
	//		loopSpeed = 16;
	//	} else if (scoreDiff >= -3) {
	//		headSize = 24;
	//		loopSpeed = 20;
	//	} else {
	//		headSize = 16;
	//		loopSpeed = 24;
	//	}
	//}
	//x-mod: static
	headSize = 36;
	loopSpeed = 16;

	if (isLeft) {
		x = OSP_SB_RED_XMIN + 26 - (headSize/2);
		y = OSP_SB_TOP + (OSP_SB_HEADER - OSP_SB_TOP - headSize)/2;
	} else {
		x = OSP_SB_BLUE_XMAX - 26 - (headSize/2);
		y = OSP_SB_TOP + (OSP_SB_HEADER - OSP_SB_TOP - headSize)/2;
	}
	VectorClear( headAngles );

	//split = 10;
	//frames = (loopSpeed * 1000) / split;
	//midPoint = (int)(frames / 2);

	//tmp = (int)(cg.time / split);  // how many time intervals
	//tmp = (tmp % frames) + 1;  // which frame we are in (1 to frames)

	//if (tmp > midPoint) {
	//	// moving right to left
	//	headAngles[YAW] = 270 - (int)(180.0f/midPoint * (tmp-midPoint));
	//} else {
	//	// moving left to right
	//	headAngles[YAW] = 90 + (int)(180.0f/midPoint * tmp);
	//}

	//x-mod: static
	headAngles[YAW] = 180;

	CG_DrawHead(ox + x, y, headSize, headSize, score->client, headAngles);

	if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client)) {
		CG_DrawStringExt(ox + x, y + (int)(headSize/2) - 4, "READY", colorYellow, qfalse, qfalse, 8, 8, 0);
	}

	Q_strncpyz(nameString, ci->name, sizeof(nameString));

	// name
	if (isLeft) {
		x = OSP_SB_RED_XMIN + 50 + 2;
		y = OSP_SB_TOP + 4;
	} else {
		// FIXME: we should right align this name, but it is a pain working out the printable
		// characters as ^ mess up %17s alignment.
		// x-mod: fixed.
		int w = CG_DrawStrlen(nameString);
		if (w > 18)
			w = 18;
		x = OSP_SB_BLUE_XMIN + 60 + (18 - w) * 10;
		y = OSP_SB_TOP + 4;
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

	y = OSP_SB_TOP + 2 + 2;

	// score
	if (isLeft) {
		CG_DrawFieldEx(ox + OSP_SB_RED_XMAX - 4 - 30 * 3, y, 3, score->score, colorWhite, 30, 36);
	} else {
		CG_DrawFieldEx(ox + OSP_SB_BLUE_XMIN + 4, y, -3, score->score, colorWhite, 30, 36);
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
	vec4_t hcolor, hcolor2;
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
	hcolor[0] = 0.45f;
	hcolor[1] = 0.45f;
	hcolor[2] = 0.45f;
	hcolor[3] = 0.2f;
	
	VectorCopy(colorBlack, hcolor2);
	hcolor2[3] = 0.7f;

	if (s1) {
		// player one layout
		trap_R_SetColor( hcolor );
		CG_FillRect(
			ox + OSP_SB_RED_XMIN, 
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP, 
			hcolor);
		CG_DrawBorder(
			ox + OSP_SB_RED_XMIN,
			OSP_SB_TOP, 
			OSP_SB_RED_XMAX - OSP_SB_RED_XMIN, 
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor2);

		ci = &cgs.clientinfo[s1->client];
		CG_DrawOSPTourneyPlayerScore(ci, s1, qtrue, s2);
	}

	if (s2) {
		// player two layout
		CG_FillRect(
			ox + OSP_SB_BLUE_XMIN,
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			hcolor);
		CG_DrawBorder(
			ox + OSP_SB_BLUE_XMIN,
			OSP_SB_TOP, 
			OSP_SB_BLUE_XMAX - OSP_SB_BLUE_XMIN,
			OSP_SB_HEADER - OSP_SB_TOP,
			1,
			hcolor2);

		ci = &cgs.clientinfo[s2->client];
		CG_DrawOSPTourneyPlayerScore(ci, s2, qfalse, s1);
	}

	// spectator layout
	if ( specTotal > 0 ) {
		specTotal++;

		CG_DrawStringExt( ox + OSP_SB_SPEC_XMIN + 256, OSP_SB_HEADER + OSP_SB_LINE_HEIGHT + 8 - 2,
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
