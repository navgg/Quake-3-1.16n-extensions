//Authors
//  Ideas: Randall "Ravenant" Piatek 
//	Awesome coding: Matt McChesney
// ported from Nemesis mod

#include "cg_local.h"

#define PLASMA_TIME		150
#define PLASMA_TIME_RND 150
#define PLASMA_RADIUS	2

#define ROCKET_TIME		150
#define ROCKET_TIME_RND	200
#define ROCKET_RADIUS	1.5f

#define LG_TIME			150
#define LG_TIME_RND		150
#define LG_RADIUS		2

#define BULLET_TIME		200
#define BULLET_TIME_RND	200
#define BULLET_RADIUS	2

#define EXPLOSION_TIME		200
#define EXPLOSION_TIME_RND	250
#define EXPLOSION_RADIUS	2

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

	float spacing = 28;
	float gravity = 0;
	float vel_rand = 20;
	int normTime = PLASMA_TIME;
	int randTime = PLASMA_TIME_RND;
	float radius = PLASMA_RADIUS;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = random() * spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	trap_R_LazyRegisterShader(cgs.media.blueSpark, "gfx/misc/bluespark");

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + normTime + random() * randTime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = radius;

		re->customShader = cgs.media.blueSpark;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 0.2f;

		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		if (gravity) {
			le->pos.trType = TR_GRAVITY;
			le->gravity = gravity;
			le->customGravity = qtrue;
		} else {
			le->pos.trType = TR_LINEAR;
		}
		le->pos.trDelta[0] = crandom()*vel_rand;
		le->pos.trDelta[1] = crandom()*vel_rand;
		le->pos.trDelta[2] = crandom()*vel_rand;

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

	float spacing = 14;
	float gravity = 0;
	float vel_rand = 40;

	int normTime = ROCKET_TIME;
	int randTime = ROCKET_TIME_RND;
	float radius = ROCKET_RADIUS;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = random() * spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + normTime + random() * randTime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = radius;
		re->customShader = cgs.media.tracerShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 0.2f;

		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		if (gravity) {
			le->pos.trType = TR_GRAVITY;
			le->gravity = gravity;
			le->customGravity = qtrue;
		} else {
			le->pos.trType = TR_LINEAR;
		}
		le->pos.trDelta[0] = crandom()*vel_rand;
		le->pos.trDelta[1] = crandom()*vel_rand;
		le->pos.trDelta[2] = crandom()*vel_rand;

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
	int i, j;
	int count =/* random() * 2*/ + 1;
	float begin = 0;
	float gravity = 10;
	int normTime = LG_TIME;
	int randTime = LG_TIME_RND;
	float normVel = 100;
	float randVel = 200;
	float radius = LG_RADIUS;

	localEntity_t	*le;
	refEntity_t		*re;

	trap_R_LazyRegisterShader(cgs.media.blueSpark, "gfx/misc/bluespark");

	//for (i = 0; i < count; i++) {
		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + normTime + random() * randTime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = radius;
		re->customShader = cgs.media.blueSpark;
		re->shaderRGBA[0] = 255;
		re->shaderRGBA[1] = 255;
		re->shaderRGBA[2] = 255;
		re->shaderRGBA[3] = 255;

		le->color[0] = re->shaderRGBA[0] / 255.0;
		le->color[1] = re->shaderRGBA[1] / 255.0;
		le->color[2] = re->shaderRGBA[2] / 255.0;
		le->color[3] = re->shaderRGBA[3] / 255.0;

		if (gravity) {
			le->pos.trType = TR_GRAVITY;
			le->gravity = gravity;
			le->customGravity = qtrue;
		} else {
			le->pos.trType = TR_LINEAR;
		}
		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		VectorCopy(dir, le->pos.trDelta);
		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] += crandom() * 0.7;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, begin, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, normVel + random() * randVel, le->pos.trDelta);
		le->pos.trDelta[2] += random() * 100;
	//}
}

/*
==================
CG_BulletSpark

Ported from z-effects
==================
*/
void CG_BulletSpark( vec3_t origin, vec3_t dir ) {
	int /*i,*/ j;
	int count = 1;
	float begin = 0;
	float gravity = 10;
	int normTime = BULLET_TIME;
	int randTime = BULLET_TIME_RND;
	float normVel = 100;
	float randVel = 200;
	float radius = BULLET_RADIUS/* + random()*/;
	localEntity_t *smoke;
	vec3_t smokeOrigin, up;

	localEntity_t	*le;
	refEntity_t		*re;

	//for (i = 0; i < count; i++) {
		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + normTime + random() * randTime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = radius;
		re->customShader = cgs.media.tracerShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		if (gravity) {
			le->pos.trType = TR_GRAVITY;
			le->gravity = gravity;
			le->customGravity = qtrue;
		} else {
			le->pos.trType = TR_LINEAR;
		}
		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		VectorCopy(dir, le->pos.trDelta);
		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] += crandom() * 0.7;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, begin, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, normVel + random() * randVel, le->pos.trDelta);
		le->pos.trDelta[2] += random() * 100;
	//}

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
	int			i, j;
	int count = 40;
	float begin = 20;
	float gravity = 50;
	int normTime = EXPLOSION_TIME;
	int randTime = EXPLOSION_TIME_RND;
	float normVel = 130;
	float randVel = 200;
	float normZVel = 100;
	float randZVel = 100;
	float radius = EXPLOSION_RADIUS;

	for (i = 0; i < count; i++) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + normTime + random() * randTime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = radius;
		re->customShader = cgs.media.tracerShader;
		re->shaderRGBA[0] = 255;
		re->shaderRGBA[1] = 255;
		re->shaderRGBA[2] = 255;
		re->shaderRGBA[3] = 255;
		le->color[0] = re->shaderRGBA[0] / 255.0;
		le->color[1] = re->shaderRGBA[1] / 255.0;
		le->color[2] = re->shaderRGBA[2] / 255.0;
		le->color[3] = re->shaderRGBA[3] / 255.0;

		if (gravity) {
			le->pos.trType = TR_GRAVITY;
			le->gravity = gravity;
			le->customGravity = qtrue;
		} else {
			le->pos.trType = TR_LINEAR;
		}
		le->pos.trTime = cg.time;
		VectorCopy( origin, le->pos.trBase );

		for (j = 0; j < 3; j++) {
			le->pos.trDelta[j] = 2 * random() - 1;
		}
		VectorNormalize(le->pos.trDelta);
		VectorMA(le->pos.trBase, begin, le->pos.trDelta, le->pos.trBase);
		VectorScale(le->pos.trDelta, normVel + random() * randVel, le->pos.trDelta);
		le->pos.trDelta[2] += normZVel + random() * randZVel;
	}
}
