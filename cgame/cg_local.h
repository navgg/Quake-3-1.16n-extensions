// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
#include "../game/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.


#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		8

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10

#define	DEFAULT_MODEL		"sarge"

//X-MOD: limits
#define CGX_MIN_MAXPACKETS 30
#define CGX_MAX_MAXPACKETS 125

#define CGX_MINHUNKMEGS 112
#define CGX_MAX_RATE 99999
#define CGX_MAX_FPS 333

//weather
#define CGX_GOLDENRATIO 1.618034f

#define CGX_WINTER_SNOW		1
#define CGX_WINTER_STEPS	2
#define CGX_WINTER_BREATH	4
#define CGX_WINTER_COLORS	8

#define CGX_SNOW_RANGE	256
#define CGX_SNOW_TOTAL	(MAX_MARK_POLYS / 2)
#define CGX_SNOW_TURBULENT	8

//try to register shader only if it's null
#define trap_R_RegisterShaderCGXNomip(x, s) { CGX_NomipStart(); x = trap_R_RegisterShader(s); CGX_NomipEnd(); }
#define trap_R_LazyRegisterShaderCGXNoMip(x, s) if (!x) { CGX_NomipStart(); x = trap_R_RegisterShader(s); CGX_NomipEnd(); }
#define trap_R_LazyRegisterShader(x, s) if (!x) x = trap_R_RegisterShader(s)
#define trap_S_LazyRegisterSound(x, s) if (!x) x = trap_S_RegisterSound(s);
#define trap_S_LazyStartSound(var, en, chan, path) { static sfxHandle_t var; trap_S_LazyRegisterSound(var, path); trap_S_StartSound (NULL, en, chan, var); }
#define trap_S_LazyStartSound2(var, en, chan, path) { trap_S_LazyRegisterSound(var, path); trap_S_StartSound (NULL, en, chan, var); }
//stuff
#define playWinterFootstep() { int is = rand() & 3; trap_S_LazyStartSound2(cgs.media.footsteps[ FOOTSTEP_SNOW ][ is ], es->number, CHAN_BODY, va("sound/player/footsteps/snow%i.wav", is + 1) ); }
#define playWinterLand() { trap_S_LazyStartSound2(cgs.media.snowLandSound, es->number, CHAN_AUTO, "sound/player/snow-land.wav" ); }

#if CGX_DEBUG 
#define D_Printf(x) if (cgx_debug.integer) CG_Printf x
#else
#define D_Printf(x)
#endif

// OSP Window Engine
#define MAX_WINDOW_COUNT		1
#define MAX_WINDOW_LINES		64

#define MAX_STRINGS				80
#define MAX_STRING_POOL_LENGTH	128

#define WINDOW_FONTWIDTH	8		// For non-true-type: width to scale from
#define WINDOW_FONTHEIGHT	8		// For non-true-type: height to scale from

#define WID_NONE			0x00	// General window
#define WID_STATS			0x01	// Stats (reusable due to scroll effect)
#define WID_ACC				0x02	// Accuracy window
#define WID_MOTD			0x04	// MOTD
#define	WID_SPEC			0x08	// Spectator window
#define WID_INTERM			0x10	// Intermission copyright

#define WFX_TEXTSIZING		0x0001	// Size the window based on text/font setting
#define WFX_FLASH			0x0002	// Alternate between bg and b2 every half second
#define WFX_TRUETYPE		0x0004	// Use truetype fonts for text
// These need to be last
#define WFX_FADEIN			0x0010	// Fade the window in (and back out when closing)
#define WFX_SCROLLUP		0x0020	// Scroll window up from the bottom (and back down when closing)
#define WFX_SCROLLDOWN		0x0040	// Scroll window down from the top (and back up when closing)
#define WFX_SCROLLLEFT		0x0080	// Scroll window in from the left (and back right when closing)
#define WFX_SCROLLRIGHT		0x0100	// Scroll window in from the right (and back left when closing)

#define WSTATE_COMPLETE		0x00	// Window is up with startup effects complete
#define WSTATE_START		0x01	// Window is "initializing" w/effects
#define WSTATE_SHUTDOWN		0x02	// Window is shutting down with effects
#define WSTATE_OFF			0x04	// Window is completely shutdown

typedef struct {
	vec4_t colorBorder;			// Window border color
	vec4_t colorBackground;		// Window fill color
	vec4_t colorBackground2;	// Window fill color2 (for flashing)
	int curX;					// Scrolling X position
	int curY;					// Scrolling Y position
	int effects;				// Window effects
	int flashMidpoint;			// Flashing transition point (in ms)
	int flashPeriod;			// Background flashing period (in ms)
	int fontHeight;				// For non-truetype font drawing
	float fontScaleX;			// Font scale factor
	float fontScaleY;			// Font scale factor
	int fontWidth;				// For non-truetype font drawing
	float h;					// Height
	int	id;						// Window ID for special handling (i.e. stats, motd, etc.)
	qboolean inuse;				// Activity flag
	int lineCount;				// Number of lines to display
	int lineHeight[MAX_WINDOW_LINES];	// Height property for each line
	char *lineText[MAX_WINDOW_LINES];	// Text info
	int targetTime;				// Time to complete any defined effect
	int state;					// Current state of the window
	int time;					// Current window time
	float w;					// Width
	float x;					// Target x-coordinate
								//    negative values will align the window from the right minus the (window width + offset(x))
	float y;					// Target y-coordinate
								//    negative values will align the window from the bottom minus the (window height + offset(y))
} cg_window_t;

typedef struct {
	qboolean fActive;
	char str[MAX_STRING_POOL_LENGTH];
} cg_string_t;

typedef struct {
	int				activeWindows[MAX_WINDOW_COUNT];	// List of active windows
	int				numActiveWindows;					// Number of active windows in use
	cg_window_t	window[MAX_WINDOW_COUNT];			// Static allocation of all windows
} cg_windowHandler_t;

// client side calculated statistics
typedef struct {
	int	weaponHits[WP_NUM_WEAPONS - 1];
	int weaponShots[WP_NUM_WEAPONS - 1];
	int weaponKills[WP_NUM_WEAPONS - 1];
	int weaponDeaths[WP_NUM_WEAPONS - 1];

	int suicides;

	int	lastWeaponHit;
	int	lastWeaponShot;

	int armorTotal;
	int armorRA;
	int armorYA;

	int healthTotal;
	int healthMH;

	int damageGiven;
	int damageReceived;

	//int flagsCaptured;
	//int flagsReturned;
	//int flagTime;
	qboolean needprint; //print stats after intermission
} clientStats_t;

//OSP end

//X-mod: known server mods
typedef enum {
	SM_UNDEFINED,
	SM_DEFAULT,
	SM_NOGHOST,
	SM_NEMESIS,
	SM_CARNAGE,
	SM_BMA,
	SM_OSP
} cgxServerMod_t;

typedef enum {
	HUD_NODRAW,
	HUD_DEFAULT,
	HUD_COMPACT,
	HUD_VANILLAQ3,
} hudType_t;

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,
	// X-Mod
	FOOTSTEP_SNOW,

	FOOTSTEP_TOTAL
} footstep_t;


//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
	lerpFrame_t		legs, torso;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	// railgun trail spawning
	vec3_t			railgunImpact;
	qboolean		railgunFlash;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				miscTime;

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002			// tumble over time, used for ejecting shells
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	int				gravity;			// z-effects weapons effects
	int				customGravity;		// z-effects weapons effects

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;		
} localEntity_t;

//======================================================================


typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;

	qboolean		isReferee;
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32
typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	vec3_t			color;
	vec3_t			color2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				powerups;		// so can display quad/flag status


	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	qboolean		deferred;

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

	//X-MOD: enemy model check, save read models to restore if anything	
	char			modelNameCopy[MAX_QPATH];
	char			skinNameCopy[MAX_QPATH];	
	// rail color copy
	vec3_t			colorCopy;

	//X-MOD: model colors for pm skins	
	byte			colors[4][3];
	byte			darkenColors[4][3];
	
	qhandle_t		customShader;

#if CGX_FREEZE
	int				breathPuffTime;
#endif
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after
 
//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction

typedef struct {
	int			clientFrame;		// incremented each frame
	
	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;

	// reward medals
	int			rewardTime;
	int			rewardCount;
	qhandle_t	rewardShader;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	// X-MOD: current client num, before init = -1;
	int			clientNum;	
	team_t		oldTeam;
	qboolean	clientIntermission;
	char		enemyModel[MAX_QPATH];
	char		enemySkin[MAX_QPATH];
	char		teamModel[MAX_QPATH];
	char		teamSkin[MAX_QPATH];

	int			q3version;
	
	int			meanPing;
	int			packetloss;
	int			packetlossTotal;
	int			rateDelayed;
	int			rateDelayedTotal;	
	
	int			connectionInterrupteds;
	// x-mod: debug info
#if CGX_DEBUG
	int			entities;
	int			predictionErrors;
	int			predictionMisses;
	int			predictionDecays;

	int			numPredicted;
	int			numPlayedBack;

	int			activeParticles;
#endif
#if CGX_UNLAGGED
	//unlagged - optimized prediction
	int			lastPredictedCommand;
	int			lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int			stateHead, stateTail;
	//unlagged - optimized prediction
	qboolean	useCalcEntityLerpPositions116; //x-mod: use default calc method or unlagged
#endif
	// OSP Window Engine
	cg_string_t			aStringPool[MAX_STRINGS];
	cg_window_t			*specWindow, *statsWindow, *intermWindow;
	cg_window_t			*windowCurrent;			// Current window to update.. a bit of a hack :p
	cg_windowHandler_t	winHandler;
} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;

	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];

	qhandle_t	armorModel;
	qhandle_t	armorIcon;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	railRingsShader;
	qhandle_t	railCoreShader;

	qhandle_t	lightningShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;

	qhandle_t	selectShader;
	qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
	qhandle_t	defaultCrosshair[NUM_CROSSHAIRS];
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	smokePuffShader;
	qhandle_t	smokePuffRageProShader;
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	plasmaBallShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;

	qhandle_t	numberShaders[11];

	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	bloodMarkShader;
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;

	qhandle_t	snowShader;

#if CGX_FREEZE
	qhandle_t	freezeShader;
	qhandle_t	freezeMarkShader;
#endif//freeze

	// z-effects
	qhandle_t	blueSpark;

	// Nemesis -  New media 
	//qhandle_t	teamIconRed;
	//qhandle_t	teamIconBlue;
	qhandle_t	scoreBarRed;
	qhandle_t	scoreBarBlue;
	// End

	// powerup shaders
	qhandle_t	quadShader;
	qhandle_t	redQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	battleWeaponShader;
	qhandle_t	hastePuffShader;

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
	qhandle_t	railExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;

	// special effects models
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;

	// sounds
	sfxHandle_t	quadSound;
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	sfxHandle_t	sfx_railg;
	sfxHandle_t	sfx_rockexp;
	sfxHandle_t	sfx_plasmaexp;
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;
	//X-MOD: pro mode sounds
	sfxHandle_t hitSounds[4];
	// ql kill beep
	sfxHandle_t killBeep;
	// snow sounds
	sfxHandle_t snowLandSound;
	
	sfxHandle_t hgrenb1aSound;
	sfxHandle_t hgrenb2aSound;

	sfxHandle_t hitSound;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

	// teamplay sounds
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running
	//X-MOD: pure server
	qboolean		isPureServer;

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];
	char			mapname_clean[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	// media
	cgMedia_t		media;

	//X-MOD: save delag and server info
	//int				sv_fps;	
	//int				sv_maxrate;
	char			gamename[MAX_QPATH];
	cgxServerMod_t	serverMod;
	qboolean		sv_floodProtect;

	//unlagged - client options
	// this will be set to the server's g_delagHitscan
	int				delagHitscan;
	//unlagged - client options
} cgs_t;

//X-Mod: virtual screen sizes
//height not used, its constant 480
typedef struct {
	int 			width;
	int 			hwidth;
	
	int				offsetx;
	float			fovaspect;
} vScreen_t;

//X-Mod: hud constant coords storage
typedef struct {
	int				sbheadx;
	int				sbarmorx;
	int				sbammox;
	int				sbflagx;
	int				sbhealth_tx;

	int				sby;//screen_height - icon_size

	int				sbteambg_y;//team background

	int				sbarmor_tx;
	int				sbammo_tx;

	int				width48;
	int				width5;

	int				small_char_w;
	int				big_char_w;
	int				giant_char_w;

	int				icon_size;
	int				head_size;//head icon
	int				weap_icon_s;//weapon icon size
	int				weap_icon_s2;
	int				weap_icon_sub;
	int				weap_y;
	int				weap_text_y;

	int				char_width;

	int				lagometer_x;
	int				lagometer_y;

	int				cofs;//compact hud offset

	int				score_yofs;
	int				score_yofs_no_lagometer;

	int				minshadow;
	int				lagometer_fw;
	int				lagometer_fh;
} xhud_t;

//==============================================================================

extern  vScreen_t		vScreen;
extern  xhud_t			hud;

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

//osp stats
extern	clientStats_t	stats;

extern  vmCvar_t		cgx_wideScreenFix;
extern	vmCvar_t		cgx_enemyModel;
extern	vmCvar_t		cgx_enemyModel_enabled;
extern	vmCvar_t		cgx_enemyColors;
extern	vmCvar_t		cgx_teamModel;
//extern	vmCvar_t		cgx_teamModel_enabled;
extern	vmCvar_t		cgx_teamColors;
extern	vmCvar_t		cgx_deadBodyDarken;
extern	vmCvar_t		cgx_defaultWeapon;
extern	vmCvar_t		cgx_chatSound;
extern	vmCvar_t		cgx_noTaunt;
extern	vmCvar_t		cgx_centerPrintAlpha;
extern	vmCvar_t		cgx_crosshairColor;
extern	vmCvar_t		cgx_drawSpeed;
extern	vmCvar_t		cgx_hitsounds;
extern	vmCvar_t		cgx_networkAdjustments;
extern	vmCvar_t		cgx_drawScoreBox;
extern	vmCvar_t		cgx_scoreboard;
extern	vmCvar_t		cgx_drawAccuracy;
extern	vmCvar_t		cgx_weaponEffects;
extern	vmCvar_t		cgx_nomip;
extern	vmCvar_t		cgx_sharedConfig;
extern	vmCvar_t		cgx_chatFilter;
extern	vmCvar_t		cgx_killBeep;
extern	vmCvar_t		cgx_winterEffects;
extern	vmCvar_t		cgx_modelCache;
extern	vmCvar_t		cgx_intermissionStats;

extern	vmCvar_t		com_maxfps;
extern	vmCvar_t		cl_maxpackets;

extern	vmCvar_t		cgx_debug;

//some temp info
extern	vmCvar_t		cgx_version;
extern	vmCvar_t		cgx_last_error;

extern	vmCvar_t		cgx_maploadingfix;
extern	vmCvar_t		cgx_r_picmip;

extern	vmCvar_t		r_vertexLight;
extern	vmCvar_t		r_picmip;

extern	vmCvar_t		cgx_downloadedbytes;
extern	vmCvar_t		cgx_totalbytes;
extern	vmCvar_t		cgx_downloadname;
extern	vmCvar_t		cgx_dl_host;
extern	vmCvar_t		cgx_dl_page;
extern	vmCvar_t		cgx_dl_dynb;
extern	vmCvar_t		cgx_dl_tobaseq3;

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
#if CGX_DEBUG
extern	vmCvar_t		cg_drawSnapshot;
#endif
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
#if CGX_DEBUG
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
#endif
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
#if CGX_DEBUG
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_footsteps;
#endif
extern	vmCvar_t		cg_showmiss;

extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_autoswitch;
#if CGX_DEBUG
extern	vmCvar_t		cg_ignore;
#endif
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_syncronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
#if CGX_DEBUG
extern	vmCvar_t		cg_stats;
#endif
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
//1.32
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
extern	vmCvar_t		pmove_accurate;

//extern	pmove_t			cg_pmove;

//unlagged - client options
extern	vmCvar_t		cg_delag;
extern	vmCvar_t		cg_cmdTimeNudge;
#if CGX_UNLAGGED
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cg_optimizePrediction;
extern	vmCvar_t		cg_delag_interp32;
#endif
extern	vmCvar_t		cl_timeNudge;
extern	vmCvar_t		sv_fps;
#if CGX_DEBUG
extern	vmCvar_t		cg_debugDelag;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_latentSnaps;
extern	vmCvar_t		cg_latentCmds;
extern	vmCvar_t		cg_plOut;
#endif
//unlagged - client options

//unlagged - cg_unlagged.c
void CG_PredictWeaponEffects( centity_t *cent );
void CG_AddBoundingBox( centity_t *cent );
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max );
//unlagged - cg_unlagged.c

#if CGX_FREEZE
extern	vmCvar_t	cg_enableBreath;

void CG_Drop_f( void );
void CG_BodyObituary( entityState_t *ent, char *targetName );
qboolean Q_Isfreeze( int clientNum );
void CG_AddGib( localEntity_t *le );
#endif//freeze

// cg_scoreboard_osps.c
qboolean CG_DrawOSPScoreboard( void );

// cg_osp.c
void CG_DrawBorder( int x, int y, int w, int h, int size, const float *borderColor );
void CG_DrawWidthGauge( int x, int y, int width, int height, vec4_t color, int value, qboolean reverse );
void CG_windowDraw( void );
void CG_statsWindow( int weffects );
void CG_statsWindowFree( int weffects );
void CG_statsWindowPrint( void );

void CGX_WeaponAccCheck();
void CGX_UpdateItemPickupStats(entityState_t *es, gitem_t *item);
void CGX_UpdateDamageStats(playerState_t *ps, playerState_t *ops);
void CGX_UpdateKillsDeathsStats(int mod, int target, int attacker);

// cg_zeffects.c
void CG_ParticlePlasmaTrail( centity_t *cent, vec3_t start, vec3_t end );
void CG_ParticleSparkTrail( vec3_t start, vec3_t end );
void CG_LightningSpark( vec3_t origin, vec3_t dir );
void CG_BulletSpark( vec3_t origin, vec3_t dir );
void CG_ParticleExplosionZE( vec3_t origin );

//
// cgx_extensions.c
//

void CG_LoadClientInfo( clientInfo_t *ci );
void CGX_SendClientCommand ( char *command );

void CGX_ResetModelCache();
qboolean CGX_TryLoadModelFromCache(clientInfo_t *ci, qboolean tryAny, qboolean trySkinLoads);

qboolean CGX_IsPure();
qboolean CGX_CheckModInfo(const char *str);
void CGX_CheckChatCommand(const char *str);
void CGX_AutoAdjustNetworkSettings(void);
void CGX_CheckEnemyModelAll(qboolean force);
void CGX_GenerateMapBat(char *map);
void CGX_IncreaseHunkmegs(int min);
void CGX_AddRefEntityWithCustomShader(refEntity_t *ent, int eFlags);
void CGX_SetSkinColors(clientInfo_t *ci, int clientNum);
void CGX_SetSkinColorsAll(void);
void CGX_Init_enemyModels(void);
void CGX_Init_vScreen(void);
void CGX_LoadClientInfo(clientInfo_t *ci );
void CGX_LoadCollisionMap();
void CGX_LoadWorldMap();
void CGX_MapRestart();
void CGX_NomipEnd();
void CGX_NomipStart();
void CGX_SavePicmip();
void CGX_SaveSharedConfig(qboolean forced);
void CGX_SendModinfo(qboolean force);
char CGX_ServerNameFixInfoLoad(char *str);
void CGX_CheckEnemyModel(clientInfo_t *ci, qboolean isDeferred, int clientNum);
void CGX_SyncServerParams(const char *info);
void CGX_TrackPlayerStateChanges();
void CGX_TryLoadingFix();
void CGX_Xmod();
void CGX_DownloadMap(char *name, qboolean end_load);
void CGX_ChatFilter(char *str);
char *CGX_CheckChatTokens(char *message, char chatcol);

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string, 
				   float charWidth, float charHeight, const float *modulate );


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawBigString2( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor2( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );


//
// cg_draw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );


//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_PredictPlayerState32( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );



//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
void CG_ShotgunFire( entityState_t *es );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent );
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader, 
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary );

//
// cg_particles.c
//

void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p, 
				   const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
void CG_GibPlayer( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
//unlagged - early transitioning
void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawScoreboard( void );
void CG_DrawTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( void );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h, 
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
void		trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
