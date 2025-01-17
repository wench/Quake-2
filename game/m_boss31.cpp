/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
==============================================================================

jorg

==============================================================================
*/

#include "g_local.h"
#include "m_boss31.h"

extern void SP_monster_makron (edict_t *self);
qboolean visible (edict_t *self, edict_t *other);

enum {
	jorg_move_stand = 1,
	jorg_move_run,
	jorg_move_start_walk,
	jorg_move_walk,
	jorg_move_end_walk,
	jorg_move_pain3,
	jorg_move_pain2,
	jorg_move_pain1,
	jorg_move_death,
	jorg_move_attack2,
	jorg_move_start_attack1,
	jorg_move_attack1,
	jorg_move_end_attack1
};

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_idle;
static int	sound_death;
static int	sound_search1;
static int	sound_search2;
static int	sound_search3;
static int	sound_attack1;
static int	sound_attack2;
static int	sound_firegun;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_death_hit;

void BossExplode (edict_t *self);
void MakronToss (edict_t *self);


void jorg_search (edict_t *self)
{
	float r;

	r = random();

	if (r <= 0.3)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	else if (r <= 0.6)
		gi.sound (self, CHAN_VOICE, sound_search2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_search3, 1, ATTN_NORM, 0);
}


void jorg_dead (edict_t *self);
void jorgBFG (edict_t *self);
void jorgMachineGun (edict_t *self);
void jorg_firebullet (edict_t *self);
void jorg_reattack1(edict_t *self);
void jorg_attack1(edict_t *self);
void jorg_idle(edict_t *self);
void jorg_step_left(edict_t *self);
void jorg_step_right(edict_t *self);
void jorg_death_hit(edict_t *self);
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_run)
AutoSFP(ai_move)
AutoSFP(jorg_dead)
AutoSFP(jorgBFG)
//AutoSFP(jorgMachineGun)
AutoSFP(jorg_firebullet)
AutoSFP(jorg_reattack1)
AutoSFP(jorg_attack1)
AutoSFP(jorg_idle)
AutoSFP(jorg_step_left)
AutoSFP(jorg_step_right)
AutoSFP(jorg_death_hit)//
// stand
//

mframe_t jorg_frames_stand []=
{
	SFP::ai_stand, 0, SFP::jorg_idle,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 10
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 20
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 30
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 19, nullptr,
	SFP::ai_stand, 11, SFP::jorg_step_left,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 6, nullptr,
	SFP::ai_stand, 9, SFP::jorg_step_right,
	SFP::ai_stand, 0, nullptr,		// 40
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, -2, nullptr,
	SFP::ai_stand, -17, SFP::jorg_step_left,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, -12, nullptr,		// 50
	SFP::ai_stand, -14, SFP::jorg_step_right	// 51
};

void jorg_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_NORM,0);
}

void jorg_death_hit (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_death_hit, 1, ATTN_NORM,0);
}


void jorg_step_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
}

void jorg_step_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
}


void jorg_stand (edict_t *self)
{
	self->monsterinfo.currentmove = jorg_move_stand;
}

void jorg_walk (edict_t *self)
{
		self->monsterinfo.currentmove = jorg_move_walk;
}

void jorg_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = jorg_move_stand;
	else
		self->monsterinfo.currentmove = jorg_move_run;
}

mframe_t jorg_frames_run [] =
{
	SFP::ai_run, 17,	SFP::jorg_step_left,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 8,	nullptr,
	SFP::ai_run, 10,	nullptr,
	SFP::ai_run, 33,	SFP::jorg_step_right,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 0,	nullptr,
	SFP::ai_run, 9,	nullptr,
	SFP::ai_run, 9,	nullptr,
	SFP::ai_run, 9,	nullptr
};

//
// walk
//

mframe_t jorg_frames_start_walk [] =
{
	SFP::ai_walk,	5,	nullptr,
	SFP::ai_walk,	6,	nullptr,
	SFP::ai_walk,	7,	nullptr,
	SFP::ai_walk,	9,	nullptr,
	SFP::ai_walk,	15,	nullptr
};

mframe_t jorg_frames_walk [] =
{
	SFP::ai_walk, 17,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 12,	nullptr,
	SFP::ai_walk, 8,	nullptr,
	SFP::ai_walk, 10,	nullptr,
	SFP::ai_walk, 33,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 9,	nullptr,
	SFP::ai_walk, 9,	nullptr,
	SFP::ai_walk, 9,	nullptr
};

mframe_t jorg_frames_end_walk [] =
{
	SFP::ai_walk,	11,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	-8,	nullptr
};

mframe_t jorg_frames_pain3 [] =
{
	SFP::ai_move,	-28,	nullptr,
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-3,	SFP::jorg_step_left,
	SFP::ai_move,	-9,	nullptr,
	SFP::ai_move,	0,	SFP::jorg_step_right,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-7,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	-11,	nullptr,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	10,	nullptr,
	SFP::ai_move,	11,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	10,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	10,	nullptr,
	SFP::ai_move,	7,	SFP::jorg_step_left,
	SFP::ai_move,	17,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::jorg_step_right
};

mframe_t jorg_frames_pain2 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t jorg_frames_pain1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};
AutoSFP(MakronToss)
AutoSFP(BossExplode)
mframe_t jorg_frames_death1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,		// 10
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,		// 20
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,		// 30
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,		// 40
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::MakronToss,
	SFP::ai_move,	0,	SFP::BossExplode		// 50
};

mframe_t jorg_frames_attack2 []=
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::jorgBFG,		
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t jorg_frames_start_attack1 [] =
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr
};

mframe_t jorg_frames_attack1[]=
{
	SFP::ai_charge,	0,	SFP::jorg_firebullet,
	SFP::ai_charge,	0,	SFP::jorg_firebullet,
	SFP::ai_charge,	0,	SFP::jorg_firebullet,
	SFP::ai_charge,	0,	SFP::jorg_firebullet,
	SFP::ai_charge,	0,	SFP::jorg_firebullet,
	SFP::ai_charge,	0,	SFP::jorg_firebullet
};

mframe_t jorg_frames_end_attack1[]=
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};
AutoSFP(jorg_run)
mmove_t jorg_moves[] = 
{
	{ 0 },
	{FRAME_stand01, FRAME_stand51, jorg_frames_stand, nullptr},
	{FRAME_walk06, FRAME_walk19, jorg_frames_run, nullptr},
	{FRAME_walk01, FRAME_walk05, jorg_frames_start_walk, nullptr},
	{FRAME_walk06, FRAME_walk19, jorg_frames_walk, nullptr},
	{FRAME_walk20, FRAME_walk25, jorg_frames_end_walk, nullptr},
	{FRAME_pain301, FRAME_pain325, jorg_frames_pain3, SFP::jorg_run},
	{FRAME_pain201, FRAME_pain203, jorg_frames_pain2, SFP::jorg_run},
	{FRAME_pain101, FRAME_pain103, jorg_frames_pain1, SFP::jorg_run},
	{FRAME_death01, FRAME_death50, jorg_frames_death1, SFP::jorg_dead},
	{FRAME_attak201, FRAME_attak213, jorg_frames_attack2, SFP::jorg_run},
	{FRAME_attak101, FRAME_attak108, jorg_frames_start_attack1, SFP::jorg_attack1},
	{FRAME_attak109, FRAME_attak114, jorg_frames_attack1, SFP::jorg_reattack1},
	{FRAME_attak115, FRAME_attak118, jorg_frames_end_attack1, SFP::jorg_run}
};

mmove_t *jorg_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &jorg_moves[self->monsterinfo.currentmove-1];
}

void jorg_reattack1(edict_t *self)
{
	if (visible(self, self->enemy))
		if (random() < 0.9)
			self->monsterinfo.currentmove = jorg_move_attack1;
		else
		{
			self->s.sound = 0;
			self->monsterinfo.currentmove = jorg_move_end_attack1;	
		}
	else
	{
		self->s.sound = 0;
		self->monsterinfo.currentmove = jorg_move_end_attack1;	
	}
}

void jorg_attack1(edict_t *self)
{
	self->monsterinfo.currentmove = jorg_move_attack1;
}

void jorg_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;
	
	self->s.sound = 0;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames if he takes little damage
	if (damage <= 40)
		if (random()<=0.6)
			return;

	/* 
	If he's entering his attack1 or using attack1, lessen the chance of him
	going into pain
	*/
	
	if ( (self->s.frame >= FRAME_attak101) && (self->s.frame <= FRAME_attak108) )
		if (random() <= 0.005)
			return;

	if ( (self->s.frame >= FRAME_attak109) && (self->s.frame <= FRAME_attak114) )
		if (random() <= 0.00005)
			return;


	if ( (self->s.frame >= FRAME_attak201) && (self->s.frame <= FRAME_attak208) )
		if (random() <= 0.005)
			return;


	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 50)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = jorg_move_pain1;
	}
	else if (damage <= 100)
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = jorg_move_pain2;
	}
	else
	{
		if (random() <= 0.3)
		{
			gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM,0);
			self->monsterinfo.currentmove = jorg_move_pain3;
		}
	}
};

void jorgBFG (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_JORG_BFG_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM, 0);
	/*void monster_fire_bfg (edict_t *self, 
							 vec3_t start, 
							 vec3_t aimdir, 
							 int damage, 
							 int speed, 
							 int kick, 
							 float damage_radius, 
							 int flashtype)*/
	monster_fire_bfg (self, start, dir, 50, 300, 100, 200, MZ2_JORG_BFG_1);
}	

void jorg_firebullet_right (edict_t *self)
{
	vec3_t	forward, right, target;
	vec3_t	start;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_JORG_MACHINEGUN_R1], forward, right, start);

	VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;
	VectorSubtract (target, start, forward);
	VectorNormalize (forward);

	monster_fire_bullet (self, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_JORG_MACHINEGUN_R1);
}	

void jorg_firebullet_left (edict_t *self)
{
	vec3_t	forward, right, target;
	vec3_t	start;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_JORG_MACHINEGUN_L1], forward, right, start);

	VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;
	VectorSubtract (target, start, forward);
	VectorNormalize (forward);

	monster_fire_bullet (self, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_JORG_MACHINEGUN_L1);
}	

void jorg_firebullet (edict_t *self)
{
	jorg_firebullet_left(self);
	jorg_firebullet_right(self);
};

void jorg_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	
	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	if (random() <= 0.75)
	{
		gi.sound (self, CHAN_VOICE, sound_attack1, 1, ATTN_NORM,0);
		self->s.sound = gi.soundindex ("boss3/w_loop.wav");
		self->monsterinfo.currentmove = jorg_move_start_attack1;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_attack2, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = jorg_move_attack2;
	}
}

void jorg_dead (edict_t *self)
{
#if 0
	edict_t	*tempent;
	/*
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	*/
	
	// Jorg is on modelindex2. Do not clear him.
	VectorSet (self->mins, -60, -60, 0);
	VectorSet (self->maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->nextthink = 0;
	gi.linkentity (self);

	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->killtarget = self->killtarget;
	tempent->target = self->target;
	tempent->activator = self->enemy;
	self->killtarget = 0;
	self->target = 0;
	SP_monster_makron (tempent);
#endif
}


void jorg_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->s.sound = 0;
	self->count = 0;
	self->monsterinfo.currentmove = jorg_move_death;
}

qboolean Jorg_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	vec3_t	temp;
	float	chance;
	trace_t	tr;
	qboolean	enemy_infront;
	int			enemy_range;
	float		enemy_yaw;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, nullptr, nullptr, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);

	self->ideal_yaw = enemy_yaw;


	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
// missile attack
	if (!self->monsterinfo.attack)
		return false;
		
	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (enemy_range == RANGE_FAR)
		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.2;
	}
	else
	{
		return false;
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2*random();
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}

SFPEnt(pain, jorg_pain);
SFPEnt(die, jorg_die);
SFPEnt(monsterinfo.get_currentmove, jorg_get_currentmove);
SFPEnt(monsterinfo.stand, jorg_stand);
SFPEnt(monsterinfo.walk, jorg_walk);

SFPEnt(monsterinfo.attack, jorg_attack);
SFPEnt(monsterinfo.search, jorg_search);
SFPEnt(monsterinfo.checkattack, Jorg_CheckAttack);
void MakronPrecache (void);

/*QUAKED monster_jorg (1 .5 0) (-80 -80 0) (90 90 140) Ambush Trigger_Spawn Sight
*/
void SP_monster_jorg (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 = gi.soundindex ("boss3/bs3pain1.wav");
	sound_pain2 = gi.soundindex ("boss3/bs3pain2.wav");
	sound_pain3 = gi.soundindex ("boss3/bs3pain3.wav");
	sound_death = gi.soundindex ("boss3/bs3deth1.wav");
	sound_attack1 = gi.soundindex ("boss3/bs3atck1.wav");
	sound_attack2 = gi.soundindex ("boss3/bs3atck2.wav");
	sound_search1 = gi.soundindex ("boss3/bs3srch1.wav");
	sound_search2 = gi.soundindex ("boss3/bs3srch2.wav");
	sound_search3 = gi.soundindex ("boss3/bs3srch3.wav");
	sound_idle = gi.soundindex ("boss3/bs3idle1.wav");
	sound_step_left = gi.soundindex ("boss3/step1.wav");
	sound_step_right = gi.soundindex ("boss3/step2.wav");
	sound_firegun = gi.soundindex ("boss3/xfire.wav");
	sound_death_hit = gi.soundindex ("boss3/d_hit.wav");

	MakronPrecache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	self->s.modelindex2 = gi.modelindex ("models/monsters/boss3/jorg/tris.md2");
	VectorSet (self->s.mins, -80, -80, 0);
	VectorSet (self->s.maxs, 80, 80, 140);

	self->health = 3000;
	self->gib_health = -2000;
	self->mass = 1000;

	self->pain = SFP::jorg_pain;
	self->die = SFP::jorg_die;
	self->monsterinfo.get_currentmove = SFP::jorg_get_currentmove;
	self->monsterinfo.stand = SFP::jorg_stand;
	self->monsterinfo.walk = SFP::jorg_walk;
	self->monsterinfo.run = SFP::jorg_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = SFP::jorg_attack;
	self->monsterinfo.search = SFP::jorg_search;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = nullptr;
	self->monsterinfo.checkattack = SFP::Jorg_CheckAttack;
	gi.linkentity (self);
	
	self->monsterinfo.currentmove = jorg_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
