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

GLADIATOR

==============================================================================
*/

#include "g_local.h"
#include "m_gladiator.h"


static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_gun;
static int	sound_cleaver_swing;
static int	sound_cleaver_hit;
static int	sound_cleaver_miss;
static int	sound_idle;
static int	sound_search;
static int	sound_sight;

enum {
	gladiator_move_stand = 1,
	gladiator_move_walk,
	gladiator_move_run,
	gladiator_move_attack_melee,
	gladiator_move_attack_gun,
	gladiator_move_pain,
	gladiator_move_pain_air,
	gladiator_move_death
};
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)

void gladiator_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void gladiator_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void gladiator_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}

void gladiator_cleaver_swing (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_cleaver_swing, 1, ATTN_NORM, 0);
}

mframe_t gladiator_frames_stand [] =
{
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr
};

void gladiator_stand (edict_t *self)
{
	self->monsterinfo.currentmove = gladiator_move_stand;
}


mframe_t gladiator_frames_walk [] =
{
	SFP::ai_walk, 15, nullptr,
	SFP::ai_walk, 7,  nullptr,
	SFP::ai_walk, 6,  nullptr,
	SFP::ai_walk, 5,  nullptr,
	SFP::ai_walk, 2,  nullptr,
	SFP::ai_walk, 0,  nullptr,
	SFP::ai_walk, 2,  nullptr,
	SFP::ai_walk, 8,  nullptr,
	SFP::ai_walk, 12, nullptr,
	SFP::ai_walk, 8,  nullptr,
	SFP::ai_walk, 5,  nullptr,
	SFP::ai_walk, 5,  nullptr,
	SFP::ai_walk, 2,  nullptr,
	SFP::ai_walk, 2,  nullptr,
	SFP::ai_walk, 1,  nullptr,
	SFP::ai_walk, 8,  nullptr
};

void gladiator_walk (edict_t *self)
{
	self->monsterinfo.currentmove = gladiator_move_walk;
}


mframe_t gladiator_frames_run [] =
{
	SFP::ai_run, 23,	nullptr,
	SFP::ai_run, 14,	nullptr,
	SFP::ai_run, 14,	nullptr,
	SFP::ai_run, 21,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 13,	nullptr
};

void gladiator_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = gladiator_move_stand;
	else
		self->monsterinfo.currentmove = gladiator_move_run;
}


void GaldiatorMelee (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->s.mins[0], -4);
	if (fire_hit (self, aim, (20 + (rand() %5)), 300))
		gi.sound (self, CHAN_AUTO, sound_cleaver_hit, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_cleaver_miss, 1, ATTN_NORM, 0);
}
AutoSFP(gladiator_cleaver_swing)
AutoSFP(GaldiatorMelee)
mframe_t gladiator_frames_attack_melee [] =
{
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::gladiator_cleaver_swing,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GaldiatorMelee,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::gladiator_cleaver_swing,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GaldiatorMelee,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

void gladiator_melee(edict_t *self)
{
	self->monsterinfo.currentmove = gladiator_move_attack_melee;
}


void GladiatorGun (edict_t *self)
{
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GLADIATOR_RAILGUN_1], forward, right, start);

	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	monster_fire_railgun (self, start, dir, 50, 100, MZ2_GLADIATOR_RAILGUN_1);
}
AutoSFP(GladiatorGun)
mframe_t gladiator_frames_attack_gun [] =
{
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GladiatorGun,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

void gladiator_attack(edict_t *self)
{
	float	range;
	vec3_t	v;

	// a small safe zone
	VectorSubtract (self->s.origin, self->enemy->s.origin, v);
	range = VectorLength(v);
	if (range <= (MELEE_DISTANCE + 32))
		return;

	// charge up the railgun
	gi.sound (self, CHAN_WEAPON, sound_gun, 1, ATTN_NORM, 0);
	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;
	self->monsterinfo.currentmove = gladiator_move_attack_gun;
}


mframe_t gladiator_frames_pain [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t gladiator_frames_pain_air [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

void gladiator_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
	{
		if ((self->velocity[2] > 100) && (self->monsterinfo.currentmove == gladiator_move_pain))
			self->monsterinfo.currentmove = gladiator_move_pain_air;
		return;
	}

	self->pain_debounce_time = level.time + 3;

	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (self->velocity[2] > 100)
		self->monsterinfo.currentmove = gladiator_move_pain_air;
	else
		self->monsterinfo.currentmove = gladiator_move_pain;
	
}


void gladiator_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t gladiator_frames_death [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

void gladiator_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

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
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = gladiator_move_death;
}

AutoSFP(gladiator_run)
AutoSFP(gladiator_dead)
mmove_t gladiator_moves[] = {
	{FRAME_stand1, FRAME_stand7, gladiator_frames_stand, nullptr},
	{FRAME_walk1, FRAME_walk16, gladiator_frames_walk, nullptr},
	{FRAME_run1, FRAME_run6, gladiator_frames_run, nullptr},
	{FRAME_melee1, FRAME_melee17, gladiator_frames_attack_melee, SFP::gladiator_run},
	{FRAME_attack1, FRAME_attack9, gladiator_frames_attack_gun, SFP::gladiator_run},
	{FRAME_pain1, FRAME_pain6, gladiator_frames_pain, SFP::gladiator_run},
	{FRAME_painup1, FRAME_painup7, gladiator_frames_pain_air, SFP::gladiator_run},
	{FRAME_death1, FRAME_death22, gladiator_frames_death, SFP::gladiator_dead}
};

mmove_t * gladiator_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &gladiator_moves[self->monsterinfo.currentmove-1];
}

SFPEnt(pain, gladiator_pain)
SFPEnt(die, gladiator_die)

SFPEnt(monsterinfo.stand, gladiator_stand)
SFPEnt(monsterinfo.walk, gladiator_walk)
SFPEnt(monsterinfo.attack, gladiator_attack)
SFPEnt(monsterinfo.melee, gladiator_melee)
SFPEnt(monsterinfo.sight, gladiator_sight)
SFPEnt(monsterinfo.idle, gladiator_idle)
SFPEnt(monsterinfo.search, gladiator_search)
SFPEnt(monsterinfo.get_currentmove, gladiator_get_currentmove)

/*QUAKED monster_gladiator (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
*/
void SP_monster_gladiator (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}


	sound_pain1 = gi.soundindex ("gladiator/pain.wav");	
	sound_pain2 = gi.soundindex ("gladiator/gldpain2.wav");	
	sound_die = gi.soundindex ("gladiator/glddeth2.wav");	
	sound_gun = gi.soundindex ("gladiator/railgun.wav");
	sound_cleaver_swing = gi.soundindex ("gladiator/melee1.wav");
	sound_cleaver_hit = gi.soundindex ("gladiator/melee2.wav");
	sound_cleaver_miss = gi.soundindex ("gladiator/melee3.wav");
	sound_idle = gi.soundindex ("gladiator/gldidle1.wav");
	sound_search = gi.soundindex ("gladiator/gldsrch1.wav");
	sound_sight = gi.soundindex ("gladiator/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/gladiatr/tris.md2");
	VectorSet (self->s.mins, -32, -32, -24);
	VectorSet (self->s.maxs, 32, 32, 64);

	self->health = 400;
	self->gib_health = -175;
	self->mass = 400;

	self->pain = SFP::gladiator_pain;
	self->die = SFP::gladiator_die;

	self->monsterinfo.stand = SFP::gladiator_stand;
	self->monsterinfo.walk = SFP::gladiator_walk;
	self->monsterinfo.run = SFP::gladiator_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = SFP::gladiator_attack;
	self->monsterinfo.melee = SFP::gladiator_melee;
	self->monsterinfo.sight = SFP::gladiator_sight;
	self->monsterinfo.idle = SFP::gladiator_idle;
	self->monsterinfo.search = SFP::gladiator_search;
	self->monsterinfo.get_currentmove = SFP::gladiator_get_currentmove;

	gi.linkentity (self);
	self->monsterinfo.currentmove = gladiator_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
