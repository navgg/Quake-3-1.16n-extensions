// some osp draw methods and window engine, modified client stats calc

#include "cg_local.h"

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
}

// Window stuct "constructor" with some common defaults
void CG_windowReset( cg_window_t *w, int fx, int startupLength ) {
	vec4_t colorGeneralBorder = { 0, 0, 0, 0.75f };
	vec4_t colorGeneralFill = { 0.2f, 0.2f, 0.2f, 0.7f };

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

// stats windows init
void CG_statsWindow( int weffects ) {
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

	sw = CG_windowAlloc( WFX_TEXTSIZING | weffects, 350 );

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

		if (stats.weaponShots[i] == 0)
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

		stats.needprint = qtrue;
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

//X-Mod: print to console
void CG_statsWindowPrint( void ) {
	cg_window_t *w;
	int i;
	
	if (!cg.statsWindow)
		CG_statsWindow(0);

	w = cg.statsWindow;

	if (!w) return;

	CG_Printf("\n");
	for (i = 0; i < w->lineCount; i++)
		CG_Printf("%s\n", w->lineText[i]);
	CG_Printf("\n");

	CG_statsWindowFree(0);
}

//X-Mod: refactored and improved stats gathering

/*
=========================
CGX_WeaponAccCheck
==========================
*/
void CGX_WeaponAccCheck( void ) {
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

#if CGX_DEBUG && 0
#define Dmg_Printf D_Printf
#else
#define Dmg_Printf(x)
#endif
void CGX_UpdateDamageStats( playerState_t *ps, playerState_t *ops ) {
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;

	if (ps->persistant[PERS_TEAM] != ops->persistant[PERS_TEAM])
		return;

	if (ps->persistant[PERS_HITS] > ops->persistant[PERS_HITS]) {
		int given = ps->persistant[PERS_HITS] - ops->persistant[PERS_HITS];
		if (given > ops->stats[STAT_HEALTH])
			given = ops->stats[STAT_HEALTH];
		Dmg_Printf( ("Damaged ^3%i (%i)\n", given, ps->persistant[PERS_HITS] - ops->persistant[PERS_HITS]) );
		stats.damageGiven += given;
	}

	if (ps->stats[STAT_HEALTH] < ops->stats[STAT_HEALTH]) {
		int received = ops->stats[STAT_HEALTH] - ps->stats[STAT_HEALTH];

		if (!(received == 1 && ps->stats[STAT_HEALTH] >= 100)) {
			if (received > ops->stats[STAT_HEALTH])
				received = ops->stats[STAT_HEALTH];
			if (received < 0)
				received = 0;
			Dmg_Printf( ("Health -^3%i (%i)\n", received, ops->stats[STAT_HEALTH] - ps->stats[STAT_HEALTH]) );
			stats.damageReceived += received;
		}
	} else if (ps->stats[STAT_HEALTH] > ops->stats[STAT_HEALTH] && ops->stats[STAT_HEALTH] > 0) {
		Dmg_Printf( ("Helath +^3%i\n", ps->stats[STAT_HEALTH] - ops->stats[STAT_HEALTH]) );
		stats.healthTotal += ps->stats[STAT_HEALTH] - ops->stats[STAT_HEALTH];
	}

	if (ps->stats[STAT_ARMOR] < ops->stats[STAT_ARMOR]) {
		int received = ops->stats[STAT_ARMOR] - ps->stats[STAT_ARMOR];

		if (!(received == 1 && ps->stats[STAT_ARMOR] >= 100)) {
			Dmg_Printf( ("Armor -^3%i\n", received) );
			stats.damageReceived += received;
		}
	} else if (ps->stats[STAT_ARMOR] > ops->stats[STAT_ARMOR] && ops->stats[STAT_HEALTH] > 0) {
		Dmg_Printf( ("Armor +^3%i\n", ps->stats[STAT_ARMOR] - ops->stats[STAT_ARMOR]) );
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
