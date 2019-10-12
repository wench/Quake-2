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

FLIPPER

==============================================================================
*/

#include "g_local.h"
#include "m_flipper.h"


static int	sound_chomp;
static int	sound_attack;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_death;
static int	sound_idle;
static int	sound_search;
static int	sound_sight;

enum {
	flipper_move_stand = 1,
	flipper_move_run_loop,
	flipper_move_run_start,
	flipper_move_walk,
	flipper_move_start_run,
	flipper_move_pain2,
	flipper_move_pain1,
	flipper_move_attack,
	flipper_move_death
};

void flipper_stand (edict_t *self);
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
mframe_t flipper_frames_stand [] =
{
	SFP::ai_stand, 0, nullptr
};
	

void flipper_stand (edict_t *self)
{
		self->monsterinfo.currentmove = flipper_move_stand;
}

#define FLIPPER_RUN_SPEED	24

mframe_t flipper_frames_run [] =
{
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,	// 6
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,	// 10

	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,	// 20

	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr,
	SFP::ai_run, FLIPPER_RUN_SPEED, nullptr		// 29
};

void flipper_run_loop (edict_t *self)
{
	self->monsterinfo.currentmove = flipper_move_run_loop;
}

mframe_t flipper_frames_run_start [] =
{
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr
};

void flipper_run (edict_t *self)
{
	self->monsterinfo.currentmove = flipper_move_run_start;
}

/* Standard Swimming */ 
mframe_t flipper_frames_walk [] =
{
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 4, nullptr
};

void flipper_walk (edict_t *self)
{
	self->monsterinfo.currentmove = flipper_move_walk;
}
AutoSFP(flipper_run)
mframe_t flipper_frames_start_run [] =
{
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, nullptr,
	SFP::ai_run, 8, SFP::flipper_run
};

void flipper_start_run (edict_t *self)
{
	self->monsterinfo.currentmove = flipper_move_start_run;
}

mframe_t flipper_frames_pain2 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0,	nullptr,
	SFP::ai_move, 0,	nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t flipper_frames_pain1 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0,	nullptr,
	SFP::ai_move, 0,	nullptr,
	SFP::ai_move, 0, nullptr
};

void flipper_bite (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, 0, 0);
	fire_hit (self, aim, 5, 0);
}

void flipper_preattack (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_chomp, 1, ATTN_NORM, 0);
}
AutoSFP(flipper_preattack)
AutoSFP(flipper_bite)


mframe_t flipper_frames_attack [] =
{
	SFP::ai_charge, 0,	SFP::flipper_preattack,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::flipper_bite,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::flipper_bite,
	SFP::ai_charge, 0,	nullptr
};

void flipper_melee(edict_t *self)
{
	self->monsterinfo.currentmove = flipper_move_attack;
}

void flipper_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = (rand() + 1) % 2;
	if (n == 0)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = flipper_move_pain1;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = flipper_move_pain2;
	}
}

void flipper_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t flipper_frames_death [] =
{
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,

	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,

	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,

	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,

	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,

	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr
};
AutoSFP(flipper_run_loop)
AutoSFP(flipper_dead)
mmove_t flipper_moves[] = {
	{FRAME_flphor01, FRAME_flphor01, flipper_frames_stand, nullptr},
	{FRAME_flpver06, FRAME_flpver29, flipper_frames_run, nullptr},
	{FRAME_flpver01, FRAME_flpver06, flipper_frames_run_start, SFP::flipper_run_loop},
	{FRAME_flphor01, FRAME_flphor24, flipper_frames_walk, nullptr},
	{FRAME_flphor01, FRAME_flphor05, flipper_frames_start_run, nullptr},
	{FRAME_flppn101, FRAME_flppn105, flipper_frames_pain2, SFP::flipper_run},
	{FRAME_flppn201, FRAME_flppn205, flipper_frames_pain1, SFP::flipper_run},
	{FRAME_flpbit01, FRAME_flpbit20, flipper_frames_attack, SFP::flipper_run},
	{FRAME_flpdth01, FRAME_flpdth56, flipper_frames_death, SFP::flipper_dead},
};

mmove_t * flipper_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &flipper_moves[self->monsterinfo.currentmove-1];
}

void flipper_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void flipper_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = flipper_move_death;
}
SFPEnt(pain, flipper_pain)
SFPEnt(die, flipper_die)

SFPEnt(monsterinfo.stand, flipper_stand)
SFPEnt(monsterinfo.walk, flipper_walk)
SFPEnt(monsterinfo.run, flipper_start_run)
SFPEnt(monsterinfo.melee, flipper_melee)
SFPEnt(monsterinfo.sight, flipper_sight)
SFPEnt(monsterinfo.get_currentmove, flipper_get_currentmove)
/*QUAKED monster_flipper (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_flipper (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1		= gi.soundindex ("flipper/flppain1.wav");	
	sound_pain2		= gi.soundindex ("flipper/flppain2.wav");	
	sound_death		= gi.soundindex ("flipper/flpdeth1.wav");	
	sound_chomp		= gi.soundindex ("flipper/flpatck1.wav");
	sound_attack	= gi.soundindex ("flipper/flpatck2.wav");
	sound_idle		= gi.soundindex ("flipper/flpidle1.wav");
	sound_search	= gi.soundindex ("flipper/flpsrch1.wav");
	sound_sight		= gi.soundindex ("flipper/flpsght1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/flipper/tris.md2");
	VectorSet (self->s.mins, -16, -16, 0);
	VectorSet (self->s.maxs, 16, 16, 32);

	self->health = 50;
	self->gib_health = -30;
	self->mass = 100;

	self->pain = SFP::flipper_pain;
	self->die = SFP::flipper_die;

	self->monsterinfo.stand = SFP::flipper_stand;
	self->monsterinfo.walk = SFP::flipper_walk;
	self->monsterinfo.run = SFP::flipper_start_run;
	self->monsterinfo.melee = SFP::flipper_melee;
	self->monsterinfo.sight = SFP::flipper_sight;
	self->monsterinfo.get_currentmove = SFP::flipper_get_currentmove;

	gi.linkentity (self);

	self->monsterinfo.currentmove = flipper_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	swimmonster_start (self);
}
