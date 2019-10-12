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

brain

==============================================================================
*/

#include "g_local.h"
#include "m_brain.h"


static int	sound_chest_open;
static int	sound_tentacles_extend;
static int	sound_tentacles_retract;
static int	sound_death;
static int	sound_idle1;
static int	sound_idle2;
static int	sound_idle3;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_sight;
static int	sound_search;
static int	sound_melee1;
static int	sound_melee2;
static int	sound_melee3;

enum {
	brain_move_stand = 1,
	brain_move_idle,
	brain_move_walk1,
	brain_move_defense,
	brain_move_pain3,
	brain_move_pain2,
	brain_move_pain1,
	brain_move_duck,
	brain_move_death2,
	brain_move_death1,
	brain_move_attack1,
	brain_move_attack2,
	brain_move_run
};

void brain_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void brain_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


void brain_run (edict_t *self);
void brain_dead (edict_t *self);

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
AutoSFP(ai_turn)

AutoSFP(brain_sight)
AutoSFP(brain_search)
AutoSFP(brain_run)
AutoSFP(brain_dead)


//
// STAND
//

mframe_t brain_frames_stand [] =
{
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,

	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,

	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr
};

void brain_stand (edict_t *self)
{
	self->monsterinfo.currentmove = brain_move_stand;
}


//
// IDLE
//

mframe_t brain_frames_idle [] =
{
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,

	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,

	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr
};

void brain_idle (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_idle3, 1, ATTN_IDLE, 0);
	self->monsterinfo.currentmove = brain_move_idle;
}


//
// WALK
//
mframe_t brain_frames_walk1 [] =
{
	SFP::ai_walk,	7,	nullptr,
	SFP::ai_walk,	2,	nullptr,
	SFP::ai_walk,	3,	nullptr,
	SFP::ai_walk,	3,	nullptr,
	SFP::ai_walk,	1,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	9,	nullptr,
	SFP::ai_walk,	-4,	nullptr,
	SFP::ai_walk,	-1,	nullptr,
	SFP::ai_walk,	2,	nullptr
};

// walk2 is FUBAR, do not use
#if 0
void brain_walk2_cycle (edict_t *self)
{
	if (random() > 0.1)
		self->monsterinfo.nextframe = FRAME_walk220;
}

mframe_t brain_frames_walk2 [] =
{
	SFP::ai_walk,	3,	nullptr,
	SFP::ai_walk,	-2,	nullptr,
	SFP::ai_walk,	-4,	nullptr,
	SFP::ai_walk,	-3,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	1,	nullptr,
	SFP::ai_walk,	12,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	-3,	nullptr,
	SFP::ai_walk,	0,	nullptr,

	SFP::ai_walk,	-2,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	1,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	10,	nullptr,		// Cycle Start

	SFP::ai_walk,	-1,	nullptr,
	SFP::ai_walk,	7,	nullptr,
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	3,	nullptr,
	SFP::ai_walk,	-3,	nullptr,
	SFP::ai_walk,	2,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	-3,	nullptr,
	SFP::ai_walk,	2,	nullptr,
	SFP::ai_walk,	0,	nullptr,

	SFP::ai_walk,	4,	brain_walk2_cycle,
	SFP::ai_walk,	-1,	nullptr,
	SFP::ai_walk,	-1,	nullptr,
	SFP::ai_walk,	-8,	nullptr,		
	SFP::ai_walk,	0,	nullptr,
	SFP::ai_walk,	1,	nullptr,
	SFP::ai_walk,	5,	nullptr,
	SFP::ai_walk,	2,	nullptr,
	SFP::ai_walk,	-1,	nullptr,
	SFP::ai_walk,	-5,	nullptr
};
mmove_t brain_move_walk2 = {FRAME_walk201, FRAME_walk240, brain_frames_walk2, nullptr};
#endif

void brain_walk (edict_t *self)
{
//	if (random() <= 0.5)
		self->monsterinfo.currentmove = brain_move_walk1;
//	else
//		self->monsterinfo.currentmove = brain_move_walk2;
}



mframe_t brain_frames_defense [] =
{
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

mframe_t brain_frames_pain3 [] =
{
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-4,	nullptr
};

mframe_t brain_frames_pain2 [] =
{
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	-2,	nullptr
};

mframe_t brain_frames_pain1 [] =
{
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	-6,	nullptr,
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
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	7,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	-1,	nullptr
};


//
// DUCK
//

void brain_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->s.maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	gi.linkentity (self);
}

void brain_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void brain_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->s.maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}
AutoSFP(brain_duck_down)
AutoSFP(brain_duck_hold)
AutoSFP(brain_duck_up)
mframe_t brain_frames_duck [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-2,	SFP::brain_duck_down,
	SFP::ai_move,	17,	SFP::brain_duck_hold,
	SFP::ai_move,	-3,	nullptr,
	SFP::ai_move,	-1,	SFP::brain_duck_up,
	SFP::ai_move,	-5,	nullptr,
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-6,	nullptr
};

void brain_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.pausetime = level.time + eta + 0.5;
	self->monsterinfo.currentmove = brain_move_duck;
}


mframe_t brain_frames_death2 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	9,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t brain_frames_death1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	9,	nullptr,
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


//
// MELEE
//

void brain_swing_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_melee1, 1, ATTN_NORM, 0);
}

void brain_hit_right (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->s.maxs[0], 8);
	if (fire_hit (self, aim, (15 + (rand() %5)), 40))
		gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}

void brain_swing_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_melee2, 1, ATTN_NORM, 0);
}

void brain_hit_left (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->s.mins[0], 8);
	if (fire_hit (self, aim, (15 + (rand() %5)), 40))
		gi.sound (self, CHAN_WEAPON, sound_melee3, 1, ATTN_NORM, 0);
}
AutoSFP(brain_swing_right)
AutoSFP(brain_hit_right)
AutoSFP(brain_swing_left)
AutoSFP(brain_hit_left)
mframe_t brain_frames_attack1 [] =
{
	SFP::ai_charge,	8,	nullptr,
	SFP::ai_charge,	3,	nullptr,
	SFP::ai_charge,	5,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	-3,	SFP::brain_swing_right,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	-5,	nullptr,
	SFP::ai_charge,	-7,	SFP::brain_hit_right,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	6,	SFP::brain_swing_left,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	2,	SFP::brain_hit_left,
	SFP::ai_charge,	-3,	nullptr,
	SFP::ai_charge,	6,	nullptr,
	SFP::ai_charge,	-1,	nullptr,
	SFP::ai_charge,	-3,	nullptr,
	SFP::ai_charge,	2,	nullptr,
	SFP::ai_charge,	-11,nullptr
};

void brain_chest_open (edict_t *self)
{
	self->spawnflags &= ~65536;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	gi.sound (self, CHAN_BODY, sound_chest_open, 1, ATTN_NORM, 0);
}

void brain_tentacle_attack (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, 0, 8);
	if (fire_hit (self, aim, (10 + (rand() %5)), -600) && skill->value > 0)
		self->spawnflags |= 65536;
	gi.sound (self, CHAN_WEAPON, sound_tentacles_retract, 1, ATTN_NORM, 0);
}

void brain_chest_closed (edict_t *self)
{
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->spawnflags & 65536)
	{
		self->spawnflags &= ~65536;
		self->monsterinfo.currentmove = brain_move_attack1;
	}
}
AutoSFP(brain_chest_open)
AutoSFP(brain_tentacle_attack)
AutoSFP(brain_chest_closed)
mframe_t brain_frames_attack2 [] =
{
	SFP::ai_charge,	5,	nullptr,
	SFP::ai_charge,	-4,	nullptr,
	SFP::ai_charge,	-4,	nullptr,
	SFP::ai_charge,	-3,	nullptr,
	SFP::ai_charge,	0,	SFP::brain_chest_open,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	13,	SFP::brain_tentacle_attack,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	2,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	-9,	SFP::brain_chest_closed,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	4,	nullptr,
	SFP::ai_charge,	3,	nullptr,
	SFP::ai_charge,	2,	nullptr,
	SFP::ai_charge,	-3,	nullptr,
	SFP::ai_charge,	-6,	nullptr
};

void brain_melee(edict_t *self)
{
	if (random() <= 0.5)
		self->monsterinfo.currentmove = brain_move_attack1;
	else
		self->monsterinfo.currentmove = brain_move_attack2;
}


//
// RUN
//

mframe_t brain_frames_run [] =
{
	SFP::ai_run,	9,	nullptr,
	SFP::ai_run,	2,	nullptr,
	SFP::ai_run,	3,	nullptr,
	SFP::ai_run,	3,	nullptr,
	SFP::ai_run,	1,	nullptr,
	SFP::ai_run,	0,	nullptr,
	SFP::ai_run,	0,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	-4,	nullptr,
	SFP::ai_run,	-1,	nullptr,
	SFP::ai_run,	2,	nullptr
};
AutoSFP(brain_stand)
mmove_t brain_moves[] = {
	{FRAME_stand01, FRAME_stand30, brain_frames_stand, nullptr},
	{FRAME_stand31, FRAME_stand60, brain_frames_idle, SFP::brain_stand},
	{FRAME_walk101, FRAME_walk111, brain_frames_walk1, nullptr},
	{FRAME_defens01, FRAME_defens08, brain_frames_defense, nullptr},
	{FRAME_pain301, FRAME_pain306, brain_frames_pain3, SFP::brain_run},
	{FRAME_pain201, FRAME_pain208, brain_frames_pain2, SFP::brain_run},
	{FRAME_pain101, FRAME_pain121, brain_frames_pain1, SFP::brain_run},
	{FRAME_duck01, FRAME_duck08, brain_frames_duck, SFP::brain_run},
	{FRAME_death201, FRAME_death205, brain_frames_death2, SFP::brain_dead},
	{FRAME_death101, FRAME_death118, brain_frames_death1, SFP::brain_dead},
	{FRAME_attak101, FRAME_attak118, brain_frames_attack1, SFP::brain_run},
	{FRAME_attak201, FRAME_attak217, brain_frames_attack2, SFP::brain_run},
	{FRAME_walk101, FRAME_walk111, brain_frames_run, nullptr}
};

mmove_t * brain_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &brain_moves[self->monsterinfo.currentmove-1];
}

void brain_run (edict_t *self)
{
	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = brain_move_stand;
	else
		self->monsterinfo.currentmove = brain_move_run;
}


void brain_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	r = random();
	if (r < 0.33)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = brain_move_pain1;
	}
	else if (r < 0.66)
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = brain_move_pain2;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = brain_move_pain3;
	}
}

void brain_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}



void brain_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	self->s.effects = 0;
	self->monsterinfo.power_armor_type = POWER_ARMOR_NONE;

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

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	if (random() <= 0.5)
		self->monsterinfo.currentmove = brain_move_death1;
	else
		self->monsterinfo.currentmove = brain_move_death2;
}

	SFPEnt(pain, brain_pain);
	SFPEnt(die, brain_die);

	SFPEnt(monsterinfo.walk, brain_walk);
	SFPEnt(monsterinfo.dodge, brain_dodge);
//	SFPEnt(monsterinfo.attack, brain_attack);
	SFPEnt(monsterinfo.melee, brain_melee);
	SFPEnt(monsterinfo.idle, brain_idle);
	SFPEnt(monsterinfo.get_currentmove, brain_get_currentmove);
/*QUAKED monster_brain (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_brain (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_chest_open = gi.soundindex ("brain/brnatck1.wav");
	sound_tentacles_extend = gi.soundindex ("brain/brnatck2.wav");
	sound_tentacles_retract = gi.soundindex ("brain/brnatck3.wav");
	sound_death = gi.soundindex ("brain/brndeth1.wav");
	sound_idle1 = gi.soundindex ("brain/brnidle1.wav");
	sound_idle2 = gi.soundindex ("brain/brnidle2.wav");
	sound_idle3 = gi.soundindex ("brain/brnlens1.wav");
	sound_pain1 = gi.soundindex ("brain/brnpain1.wav");
	sound_pain2 = gi.soundindex ("brain/brnpain2.wav");
	sound_sight = gi.soundindex ("brain/brnsght1.wav");
	sound_search = gi.soundindex ("brain/brnsrch1.wav");
	sound_melee1 = gi.soundindex ("brain/melee1.wav");
	sound_melee2 = gi.soundindex ("brain/melee2.wav");
	sound_melee3 = gi.soundindex ("brain/melee3.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/brain/tris.md2");
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, 32);

	self->health = 300;
	self->gib_health = -150;
	self->mass = 400;

	self->pain = SFP::brain_pain;
	self->die = SFP::brain_die;

	self->monsterinfo.stand = SFP::brain_stand;
	self->monsterinfo.walk = SFP::brain_walk;
	self->monsterinfo.run = SFP::brain_run;
	self->monsterinfo.dodge = SFP::brain_dodge;
//	self->monsterinfo.attack = brain_attack;
	self->monsterinfo.melee = SFP::brain_melee;
	self->monsterinfo.sight = SFP::brain_sight;
	self->monsterinfo.search = SFP::brain_search;
	self->monsterinfo.idle = SFP::brain_idle;
	self->monsterinfo.get_currentmove = SFP::brain_get_currentmove;

	self->monsterinfo.power_armor_type = POWER_ARMOR_SCREEN;
	self->monsterinfo.power_armor_power = 100;

	gi.linkentity (self);

	self->monsterinfo.currentmove = brain_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
