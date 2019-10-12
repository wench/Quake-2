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

hover

==============================================================================
*/

#include "g_local.h"
#include "m_hover.h"

qboolean visible (edict_t *self, edict_t *other);

enum {
	hover_move_stand = 1,
	hover_move_stop1,
	hover_move_stop2,
	hover_move_takeoff,
	hover_move_pain3,
	hover_move_pain2,
	hover_move_pain1,
	hover_move_land,
	hover_move_forward,
	hover_move_walk,
	hover_move_run,
	hover_move_death1,
	hover_move_backward,
	hover_move_start_attack,
	hover_move_attack1,
	hover_move_end_attack
};


static int	sound_pain1;
static int	sound_pain2;
static int	sound_death1;
static int	sound_death2;
static int	sound_sight;
static int	sound_search1;
static int	sound_search2;


void hover_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void hover_search (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_search2, 1, ATTN_NORM, 0);
}

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
AutoSFP(ai_turn)

void hover_run (edict_t *self);
void hover_stand (edict_t *self);
void hover_dead (edict_t *self);
void hover_attack (edict_t *self);
void hover_reattack (edict_t *self);
void hover_fire_blaster (edict_t *self);
void hover_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

AutoSFP(hover_sight)
AutoSFP(hover_search)
AutoSFP(hover_run)
AutoSFP(hover_stand)
AutoSFP(hover_dead)
AutoSFP(hover_attack)
AutoSFP(hover_reattack)
AutoSFP(hover_fire_blaster)

mframe_t hover_frames_stand [] =
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

mframe_t hover_frames_stop1 [] =
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

mframe_t hover_frames_stop2 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_takeoff [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	5,	nullptr,
	SFP::ai_move,	-1,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-1,	nullptr,
	SFP::ai_move,	-1,	nullptr,
	SFP::ai_move,	-1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-9,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_pain3 [] =
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

mframe_t hover_frames_pain2 [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_pain1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	-8,	nullptr,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	-3,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	7,	nullptr,
	SFP::ai_move,	1,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	5,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	4,	nullptr
};

mframe_t hover_frames_land [] =
{
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_forward [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_walk [] =
{
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr,
	SFP::ai_walk,	4,	nullptr
};

mframe_t hover_frames_run [] =
{
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr,
	SFP::ai_run,	10,	nullptr
};

mframe_t hover_frames_death1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-10,nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	5,	nullptr,
	SFP::ai_move,	4,	nullptr,
	SFP::ai_move,	7,	nullptr
};

mframe_t hover_frames_backward [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t hover_frames_start_attack [] =
{
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr
};

mframe_t hover_frames_attack1 [] =
{
	SFP::ai_charge,	-10,	SFP::hover_fire_blaster,
	SFP::ai_charge,	-10,	SFP::hover_fire_blaster,
	SFP::ai_charge,	0,		SFP::hover_reattack,
};


mframe_t hover_frames_end_attack [] =
{
	SFP::ai_charge,	1,	nullptr,
	SFP::ai_charge,	1,	nullptr
};

void hover_reattack (edict_t *self)
{
	if (self->enemy->health > 0 )
		if (visible (self, self->enemy) )
			if (random() <= 0.6)		
			{
				self->monsterinfo.currentmove = hover_move_attack1;
				return;
			}
	self->monsterinfo.currentmove = hover_move_end_attack;
}


void hover_fire_blaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;

	if (self->s.frame == FRAME_attak104)
		effect = EF_HYPERBLASTER;
	else
		effect = 0;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_HOVER_BLASTER_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, 1, 1000, MZ2_HOVER_BLASTER_1, effect);
}


void hover_stand (edict_t *self)
{
		self->monsterinfo.currentmove = hover_move_stand;
}

void hover_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = hover_move_stand;
	else
		self->monsterinfo.currentmove = hover_move_run;
}

void hover_walk (edict_t *self)
{
	self->monsterinfo.currentmove = hover_move_walk;
}

void hover_start_attack (edict_t *self)
{
	self->monsterinfo.currentmove = hover_move_start_attack;
}

void hover_attack(edict_t *self)
{
	self->monsterinfo.currentmove = hover_move_attack1;
}


void hover_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 25)
	{
		if (random() < 0.5)
		{
			gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
			self->monsterinfo.currentmove = hover_move_pain3;
		}
		else
		{
			gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
			self->monsterinfo.currentmove = hover_move_pain2;
		}
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = hover_move_pain1;
	}
}

void hover_deadthink (edict_t *self)
{
	if (!self->groundentity && level.time < self->timestamp)
	{
		self->nextthink = level.time + FRAMETIME;
		return;
	}
	BecomeExplosion1(self);
}
AutoSFP(hover_deadthink)
void hover_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->think = SFP::hover_deadthink;
	self->nextthink = level.time + FRAMETIME;
	self->timestamp = level.time + 15;
	gi.linkentity (self);
}

void hover_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_death2, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = hover_move_death1;
}
mmove_t	hover_moves[] = {
	{FRAME_stand01, FRAME_stand30, hover_frames_stand, nullptr},
	{FRAME_stop101, FRAME_stop109, hover_frames_stop1, nullptr},
	{FRAME_stop201, FRAME_stop208, hover_frames_stop2, nullptr},
	{FRAME_takeof01, FRAME_takeof30, hover_frames_takeoff, nullptr},
	{FRAME_pain301, FRAME_pain309, hover_frames_pain3, SFP::hover_run},
	{FRAME_pain201, FRAME_pain212, hover_frames_pain2, SFP::hover_run},
	{FRAME_pain101, FRAME_pain128, hover_frames_pain1, SFP::hover_run},
	{FRAME_land01, FRAME_land01, hover_frames_land, nullptr},
	{FRAME_forwrd01, FRAME_forwrd35, hover_frames_forward, nullptr},
	{FRAME_forwrd01, FRAME_forwrd35, hover_frames_walk, nullptr},
	{FRAME_forwrd01, FRAME_forwrd35, hover_frames_run, nullptr},
	{FRAME_death101, FRAME_death111, hover_frames_death1, SFP::hover_dead},
	{FRAME_backwd01, FRAME_backwd24, hover_frames_backward, nullptr},
	{FRAME_attak101, FRAME_attak103, hover_frames_start_attack, SFP::hover_attack},
	{FRAME_attak104, FRAME_attak106, hover_frames_attack1, nullptr},
	{FRAME_attak101, FRAME_attak103, hover_frames_start_attack, SFP::hover_attack},
	{FRAME_attak107, FRAME_attak108, hover_frames_end_attack,SFP::hover_run}
};

mmove_t * hover_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &hover_moves[self->monsterinfo.currentmove-1];
}

SFPEnt(pain, hover_pain)
SFPEnt(die, hover_die)

SFPEnt(monsterinfo.walk, hover_walk)
//	SFPEnt(monsterinfo.dodge, hover_dodge)
SFPEnt(monsterinfo.attack, hover_start_attack)
SFPEnt(monsterinfo.get_currentmove, hover_get_currentmove)

/*QUAKED monster_hover (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_hover (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 = gi.soundindex ("hover/hovpain1.wav");	
	sound_pain2 = gi.soundindex ("hover/hovpain2.wav");	
	sound_death1 = gi.soundindex ("hover/hovdeth1.wav");	
	sound_death2 = gi.soundindex ("hover/hovdeth2.wav");	
	sound_sight = gi.soundindex ("hover/hovsght1.wav");	
	sound_search1 = gi.soundindex ("hover/hovsrch1.wav");	
	sound_search2 = gi.soundindex ("hover/hovsrch2.wav");	

	gi.soundindex ("hover/hovatck1.wav");	

	self->s.sound = gi.soundindex ("hover/hovidle1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/hover/tris.md2");
	VectorSet (self->s.mins, -24, -24, -24);
	VectorSet (self->s.maxs, 24, 24, 32);

	self->health = 240;
	self->gib_health = -100;
	self->mass = 150;

	self->pain = SFP::hover_pain;
	self->die = SFP::hover_die;

	self->monsterinfo.stand = SFP::hover_stand;
	self->monsterinfo.walk = SFP::hover_walk;
	self->monsterinfo.run = SFP::hover_run;
//	self->monsterinfo.dodge = SFP::hover_dodge;
	self->monsterinfo.attack = SFP::hover_start_attack;
	self->monsterinfo.sight = SFP::hover_sight;
	self->monsterinfo.search = SFP::hover_search;
	self->monsterinfo.get_currentmove = SFP::hover_get_currentmove;

	gi.linkentity (self);

	self->monsterinfo.currentmove = hover_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
