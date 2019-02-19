//Authors
//  Ideas: Randall "Ravenant" Piatek 
//	Awesome coding: Matt McChesney
// ported from Nemesis mod
// modified for X-mod by 322

#include "cg_local.h"

#if !CGX_Z_EFFECTS
void CG_ParticlePlasmaTrail( centity_t *cent, vec3_t start, vec3_t end ) { }
void CG_ParticleSparkTrail( vec3_t start, vec3_t end ) { }
void CG_LightningSpark( vec3_t origin, vec3_t dir ) { }
void CG_BulletSpark( vec3_t origin, vec3_t dir ) { }
void CG_ParticleExplosionZE( vec3_t origin ) { }
#else

#define PLASMA_TIME		150
#define PLASMA_TIME_RND 100
#define PLASMA_RADIUS	2.0f
#define PLASMA_VEL		14.0f
#define PLASMA_SPACING	30.0f

#define ROCKET_TIME		150
#define ROCKET_TIME_RND	150
#define ROCKET_RADIUS	1.5f
#define ROCKET_VEL		40.0f
#define ROCKET_SPACING	14.0f

#define LG_TIME			150
#define LG_TIME_RND		150
#define LG_RADIUS		2.0f
#define LG_VEL			100.0f
#define LG_VEL_RND		200.0f
#define LG_GRAVITY		10

#define BULLET_TIME		200
#define BULLET_TIME_RND	200
#define BULLET_RADIUS	2.0f
#define BULLET_VEL		100.0f
#define BULLET_VEL_RND	200.0f
#define BULLET_GRAVITY	10

#define EXPLOSION_TIME		200
#define EXPLOSION_TIME_RND	250
#define EXPLOSION_RADIUS	2.0f
#define EXPLOSION_VEL		130.0f
#define EXPLOSION_VEL_RND	200.0f
#define EXPLOSION_Z_VEL		100.0f
#define EXPLOSION_Z_VEL_RND	100.0f
#define EXPLOSION_GRAVITY	50
#define EXPLOSION_COUNT		38

/*
==================
CG_ParticlePlasmaTrail

Ported from z-effects
==================
*/
void CG_ParticlePlasmaTrail( centity_t *cent, vec3_t start, vec3_t end ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	float		i;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = random() * PLASMA_SPACING;
	VectorMA( move, i, vec, move );

	VectorScale (vec, PLASMA_SPACING, vec);

	trap_R_LazyRegisterShaderCGXNoMip(cgs.media.blueSpark, "gfx/misc/bluespark");

	for ( ; i < len; i += PLASMA_SPACING ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + PLASMA_TIME + rand() % PLASMA_TIME_RND;
		le->lifeRate = 1.0f / ( le->endTime - le->startTime );

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = PLASMA_RADIUS;

		re->customShader = cgs.media.blueSpark;

		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );

		le->pos.trType = TR_LINEAR;

		le->pos.trDelta[0] = crandom()*PLASMA_VEL;
		le->pos.trDelta[1] = random()*PLASMA_VEL/4;
		le->pos.trDelta[2] = crandom()*PLASMA_VEL;

		VectorAdd (move, vec, move);
	}
}

/*
==================
CG_ParticleSparkTrail

Rocket spark trails
Ported from z-effects
==================
*/
void CG_ParticleSparkTrail( vec3_t start, vec3_t end ) {
	vec3_t		move;
	vec3_t		vec;
	float		len;
	float		i;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = random() * ROCKET_SPACING;
	VectorMA( move, i, vec, move );

	VectorScale (vec, ROCKET_SPACING, vec);

	for ( ; i < len; i += ROCKET_SPACING ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + ROCKET_TIME + rand() % ROCKET_TIME_RND;
		le->lifeRate = 1.0f / ( le->endTime - le->startTime );

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = ROCKET_RADIUS;
		re->customShader = cgs.media.tracerShader;

		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );

		le->pos.trType = TR_LINEAR;
		
		le->pos.trDelta[0] = crandom()*ROCKET_VEL;
		le->pos.trDelta[1] = crandom()*ROCKET_VEL/4;
		le->pos.trDelta[2] = crandom()*ROCKET_VEL;

		VectorAdd (move, vec, move);
	}
}

/*
==================
CG_LightningSpark

Ported from z-effects
==================
*/
void CG_LightningSpark( vec3_t origin, vec3_t dir ) {
	int j;

	localEntity_t	*le;
	refEntity_t		*re;

	if ( trap_CM_PointContents(origin, 0) & CONTENTS_WATER )
		return;

	trap_R_LazyRegisterShaderCGXNoMip(cgs.media.blueSpark, "gfx/misc/bluespark");

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + LG_TIME + rand() % LG_TIME_RND;
		le->lifeRate = 1.0f / ( le->endTime - le->startTime );

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = LG_RADIUS;
		re->customShader = cgs.media.blueSpark;

		le->pos.trType = TR_GRAVITY;
		le->gravity = LG_GRAVITY;
		le->customGravity = qtrue;
		
		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		VectorCopy(dir, le->pos.trDelta);
		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] += crandom() * 0.7;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, 0.0f, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, LG_VEL + random() * LG_VEL_RND, le->pos.trDelta);
		le->pos.trDelta[2] += random() * 100;
}

/*
==================
CG_BulletSpark

Ported from z-effects
==================
*/
void CG_BulletSpark( vec3_t origin, vec3_t dir ) {
	int j;

	localEntity_t *smoke;
	vec3_t smokeOrigin, up;

	localEntity_t	*le;
	refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + BULLET_TIME + rand() % BULLET_TIME_RND;
		le->lifeRate = 1.0f / ( le->endTime - le->startTime );

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = BULLET_RADIUS;
		re->customShader = cgs.media.tracerShader;

		le->pos.trType = TR_GRAVITY;
		le->gravity = BULLET_GRAVITY;
		le->customGravity = qtrue;

		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		VectorCopy(dir, le->pos.trDelta);
		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] += crandom() * 0.7;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, 0.0f, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, BULLET_VEL + random() * BULLET_VEL_RND, le->pos.trDelta);
		le->pos.trDelta[2] += random() * 100;

	VectorMA(origin, 4 + random() * 4, dir, smokeOrigin);
	VectorSet(up, 0, 0, random() * 12);

	smoke = CG_SmokePuff(smokeOrigin, up, 10, 1, 1, 1, 0.6f, 700, cg.time, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_MOVE_SCALE_FADE;
}

/*
==================
CG_ParticleExplosionZE

Particle explosion
Ported from z-effects
==================
*/
void CG_ParticleExplosionZE( vec3_t origin ) {
	int	i, j;
	int endTime;

	if ( trap_CM_PointContents(origin, 0) & CONTENTS_WATER )
		return;

	endTime = cg.time + EXPLOSION_TIME;

	for (i = 0; i < EXPLOSION_COUNT; i++) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = endTime + rand() % EXPLOSION_TIME_RND;
		le->lifeRate = 1.0f / ( le->endTime - le->startTime );

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = EXPLOSION_RADIUS;
		re->customShader = cgs.media.tracerShader;

		le->pos.trType = TR_GRAVITY;
		le->gravity = EXPLOSION_GRAVITY;
		le->customGravity = qtrue;

		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] = 2 * random() - 1;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, 20.0f, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, EXPLOSION_VEL + random() * EXPLOSION_VEL_RND, le->pos.trDelta);
		le->pos.trDelta[2] += EXPLOSION_Z_VEL + random() * EXPLOSION_Z_VEL_RND;
	}
}

#endif
