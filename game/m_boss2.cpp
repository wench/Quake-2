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

boss2

==============================================================================
*/

#include "g_local.h"
#include "m_boss2.h"

void BossExplode (edict_t *self);

qboolean infront (edict_t *self, edict_t *other);

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_death;
static int	sound_search1;

void boss2_search (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NONE, 0);
}

void boss2_run (edict_t *self);
void boss2_stand (edict_t *self);
void boss2_dead (edict_t *self);
void boss2_attack (edict_t *self);
void boss2_attack_mg (edict_t *self);
void boss2_reattack_mg (edict_t *self);
void boss2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void Boss2Rocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, nullptr);

//1
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_1], forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	monster_fire_rocket (self, start, dir, 50, 500, MZ2_BOSS2_ROCKET_1);

//2
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_2], forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	monster_fire_rocket (self, start, dir, 50, 500, MZ2_BOSS2_ROCKET_2);

//3
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_3], forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	monster_fire_rocket (self, start, dir, 50, 500, MZ2_BOSS2_ROCKET_3);

//4
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_4], forward, right, start);
	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	monster_fire_rocket (self, start, dir, 50, 500, MZ2_BOSS2_ROCKET_4);
}	

void boss2_firebullet_right (edict_t *self)
{
	vec3_t	forward, right, target;
	vec3_t	start;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_MACHINEGUN_R1], forward, right, start);

	VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;
	VectorSubtract (target, start, forward);
	VectorNormalize (forward);

	monster_fire_bullet (self, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_BOSS2_MACHINEGUN_R1);
}	

void boss2_firebullet_left (edict_t *self)
{
	vec3_t	forward, right, target;
	vec3_t	start;
	
	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_BOSS2_MACHINEGUN_L1], forward, right, start);

	VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);

	target[2] += self->enemy->viewheight;
	VectorSubtract (target, start, forward);
	VectorNormalize (forward);

	monster_fire_bullet (self, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_BOSS2_MACHINEGUN_L1);
}	

void Boss2MachineGun (edict_t *self)
{
/*	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;

	AngleVectors (self->s.angles, forward, right, nullptr);

	flash_number = MZ2_BOSS2_MACHINEGUN_1 + (self->s.frame - FRAME_attack10);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	monster_fire_bullet (self, start, dir, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
*/
	boss2_firebullet_left(self);
	boss2_firebullet_right(self);
}	
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_run)
AutoSFP(ai_move)

mframe_t boss2_frames_stand [] =
{
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr
};

mframe_t boss2_frames_fidget [] =
{
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr
};

mframe_t boss2_frames_walk [] =
{
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr,
	SFP::ai_walk,	8,	nullptr
};


mframe_t boss2_frames_run [] =
{
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr,
	SFP::ai_run,	8,	nullptr
};
AutoSFP(boss2_attack_mg)
AutoSFP(Boss2MachineGun)
AutoSFP(boss2_reattack_mg)
mframe_t boss2_frames_attack_pre_mg [] =
{
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	SFP::boss2_attack_mg
};


// Loop this
mframe_t boss2_frames_attack_mg [] =
{
	SFP::ai_charge,	1,	SFP::Boss2MachineGun,
	SFP::ai_charge,	1,	SFP::Boss2MachineGun,
	SFP::ai_charge,	1,	SFP::Boss2MachineGun,
	SFP::ai_charge,	1,	SFP::Boss2MachineGun,
	SFP::ai_charge,	1,	SFP::Boss2MachineGun,
	SFP::ai_charge,	1,	SFP::boss2_reattack_mg
};

mframe_t boss2_frames_attack_post_mg [] =
{
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr
};
AutoSFP(Boss2Rocket)
mframe_t boss2_frames_attack_rocket [] =
{
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_move,	-20,	SFP::Boss2Rocket,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr
};

mframe_t boss2_frames_pain_heavy [] =
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
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t boss2_frames_pain_light [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};
AutoSFP(BossExplode)
mframe_t boss2_frames_death [] =
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
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::BossExplode
};

enum {
	boss2_move_stand			= 1,
	boss2_move_fidget,
	boss2_move_walk,
	boss2_move_run,
	boss2_move_attack_pre_mg,
	boss2_move_attack_mg,
	boss2_move_attack_post_mg,
	boss2_move_attack_rocket,
	boss2_move_pain_heavy,
	boss2_move_pain_light,
	boss2_move_death,
};
AutoSFP(boss2_run)
AutoSFP(boss2_dead)


mmove_t boss2_moves[] = {
	{FRAME_stand30, FRAME_stand50, boss2_frames_stand, nullptr},
	{FRAME_stand1, FRAME_stand30, boss2_frames_fidget, nullptr},
	{FRAME_walk1, FRAME_walk20, boss2_frames_walk, nullptr},
	{FRAME_walk1, FRAME_walk20, boss2_frames_run, nullptr},
	{FRAME_attack1, FRAME_attack9, boss2_frames_attack_pre_mg, nullptr},
	{FRAME_attack10, FRAME_attack15, boss2_frames_attack_mg, nullptr},
	{FRAME_attack16, FRAME_attack19, boss2_frames_attack_post_mg, SFP::boss2_run},
	{FRAME_attack20, FRAME_attack40, boss2_frames_attack_rocket, SFP::boss2_run},
	{FRAME_pain2, FRAME_pain19, boss2_frames_pain_heavy, SFP::boss2_run},
	{FRAME_pain20, FRAME_pain23, boss2_frames_pain_light, SFP::boss2_run},
	{FRAME_death2, FRAME_death50, boss2_frames_death, SFP::boss2_dead}
};

mmove_t *boss2_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &boss2_moves[self->monsterinfo.currentmove-1];
}

void boss2_stand (edict_t *self)
{
	self->monsterinfo.currentmove = boss2_move_stand;
}

void boss2_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = boss2_move_stand;
	else
		self->monsterinfo.currentmove = boss2_move_run;
}

void boss2_walk (edict_t *self)
{
	self->monsterinfo.currentmove = boss2_move_walk;
}

void boss2_attack (edict_t *self)
{
	vec3_t	vec;
	float	range;

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);
	
	if (range <= 125)
	{
		self->monsterinfo.currentmove = boss2_move_attack_pre_mg;
	}
	else 
	{
		if (random() <= 0.6)
			self->monsterinfo.currentmove = boss2_move_attack_pre_mg;
		else
			self->monsterinfo.currentmove = boss2_move_attack_rocket;
	}
}

void boss2_attack_mg (edict_t *self)
{
	self->monsterinfo.currentmove = boss2_move_attack_mg;
}

void boss2_reattack_mg (edict_t *self)
{
	if ( infront(self, self->enemy) )
		if (random() <= 0.7)
			self->monsterinfo.currentmove = boss2_move_attack_mg;
		else
			self->monsterinfo.currentmove = boss2_move_attack_post_mg;
	else
		self->monsterinfo.currentmove = boss2_move_attack_post_mg;
}


void boss2_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
// American wanted these at no attenuation
	if (damage < 10)
	{
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NONE, 0);
		self->monsterinfo.currentmove = boss2_move_pain_light;
	}
	else if (damage < 30)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NONE, 0);
		self->monsterinfo.currentmove = boss2_move_pain_light;
	}
	else 
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NONE, 0);
		self->monsterinfo.currentmove = boss2_move_pain_heavy;
	}
}

void boss2_dead (edict_t *self)
{
	VectorSet (self->s.mins, -56, -56, 0);
	VectorSet (self->s.maxs, 56, 56, 80);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void boss2_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->count = 0;
	self->monsterinfo.currentmove = boss2_move_death;
#if 0
	int		n;

	self->s.sound = 0;
	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = boss2_move_death;
#endif
}

qboolean Boss2_CheckAttack (edict_t *self)
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
		chance = 0.8;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.8;
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

SFPEnt(pain, boss2_pain);
SFPEnt(die, boss2_die);

SFPEnt(monsterinfo.get_currentmove, boss2_get_currentmove);
SFPEnt(monsterinfo.stand, boss2_stand);
SFPEnt(monsterinfo.walk, boss2_walk);
SFPEnt(monsterinfo.attack, boss2_attack);
SFPEnt(monsterinfo.search, boss2_search);
SFPEnt(monsterinfo.checkattack, Boss2_CheckAttack);

/*QUAKED monster_boss2 (1 .5 0) (-56 -56 0) (56 56 80) Ambush Trigger_Spawn Sight
*/
void SP_monster_boss2 (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 = gi.soundindex ("bosshovr/bhvpain1.wav");
	sound_pain2 = gi.soundindex ("bosshovr/bhvpain2.wav");
	sound_pain3 = gi.soundindex ("bosshovr/bhvpain3.wav");
	sound_death = gi.soundindex ("bosshovr/bhvdeth1.wav");
	sound_search1 = gi.soundindex ("bosshovr/bhvunqv1.wav");

	self->s.sound = gi.soundindex ("bosshovr/bhvengn1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss2/tris.md2");
	VectorSet (self->s.mins, -56, -56, 0);
	VectorSet (self->s.maxs, 56, 56, 80);

	self->health = 2000;
	self->gib_health = -200;
	self->mass = 1000;

	self->flags |= FL_IMMUNE_LASER;

	self->pain = SFP::boss2_pain;
	self->die = SFP::boss2_die;

	self->monsterinfo.get_currentmove = SFP::boss2_get_currentmove;
	self->monsterinfo.stand = SFP::boss2_stand;
	self->monsterinfo.walk = SFP::boss2_walk;
	self->monsterinfo.run = SFP::boss2_run;
	self->monsterinfo.attack = SFP::boss2_attack;
	self->monsterinfo.search = SFP::boss2_search;
	self->monsterinfo.checkattack = SFP::Boss2_CheckAttack;
	gi.linkentity (self);

	self->monsterinfo.currentmove = boss2_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
