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

chick

==============================================================================
*/

#include "g_local.h"
#include "m_chick.h"

qboolean visible (edict_t *self, edict_t *other);

void chick_stand (edict_t *self);
void chick_run (edict_t *self);
void chick_reslash(edict_t *self);
void chick_rerocket(edict_t *self);
void chick_attack1(edict_t *self);

static int	sound_missile_prelaunch;
static int	sound_missile_launch;
static int	sound_melee_swing;
static int	sound_melee_hit;
static int	sound_missile_reload;
static int	sound_death1;
static int	sound_death2;
static int	sound_fall_down;
static int	sound_idle1;
static int	sound_idle2;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_sight;
static int	sound_search;

enum {
	chick_move_fidget = 1,
	chick_move_stand,
	chick_move_start_run,
	chick_move_run,
	chick_move_walk,
	chick_move_pain1,
	chick_move_pain2,
	chick_move_pain3,
	chick_move_death2,
	chick_move_death1,
	chick_move_duck,
	chick_move_start_attack1,
	chick_move_attack1,
	chick_move_end_attack1,
	chick_move_slash,
	chick_move_end_slash,
	chick_move_start_slash,
};


void ChickMoan (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
}
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
AutoSFP(ChickMoan)
mframe_t chick_frames_fidget [] =
{
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  SFP::ChickMoan,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr,
	SFP::ai_stand, 0,  nullptr
};

void chick_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.3)
		self->monsterinfo.currentmove = chick_move_fidget;
}
AutoSFP(chick_fidget)
mframe_t chick_frames_stand [] =
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
	SFP::ai_stand, 0, SFP::chick_fidget,

};

void chick_stand (edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_stand;
}

mframe_t chick_frames_start_run [] =
{
	SFP::ai_run, 1,  nullptr,
	SFP::ai_run, 0,  nullptr,
	SFP::ai_run, 0,	 nullptr,
	SFP::ai_run, -1, nullptr, 
	SFP::ai_run, -1, nullptr, 
	SFP::ai_run, 0,  nullptr,
	SFP::ai_run, 1,  nullptr,
	SFP::ai_run, 3,  nullptr,
	SFP::ai_run, 6,	 nullptr,
	SFP::ai_run, 3,	 nullptr
};

mframe_t chick_frames_run [] =
{
	SFP::ai_run, 6,	nullptr,
	SFP::ai_run, 8,  nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 5,  nullptr,
	SFP::ai_run, 7,  nullptr,
	SFP::ai_run, 4,  nullptr,
	SFP::ai_run, 11, nullptr,
	SFP::ai_run, 5,  nullptr,
	SFP::ai_run, 9,  nullptr,
	SFP::ai_run, 7,  nullptr

};


mframe_t chick_frames_walk [] =
{
	SFP::ai_walk, 6,	 nullptr,
	SFP::ai_walk, 8,  nullptr,
	SFP::ai_walk, 13, nullptr,
	SFP::ai_walk, 5,  nullptr,
	SFP::ai_walk, 7,  nullptr,
	SFP::ai_walk, 4,  nullptr,
	SFP::ai_walk, 11, nullptr,
	SFP::ai_walk, 5,  nullptr,
	SFP::ai_walk, 9,  nullptr,
	SFP::ai_walk, 7,  nullptr
};


void chick_walk (edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_walk;
}

void chick_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = chick_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == chick_move_walk ||
		self->monsterinfo.currentmove == chick_move_start_run)
	{
		self->monsterinfo.currentmove = chick_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = chick_move_start_run;
	}
}

mframe_t chick_frames_pain1 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t chick_frames_pain2 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t chick_frames_pain3 [] =
{
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, -6,	nullptr,
	SFP::ai_move, 3,		nullptr,
	SFP::ai_move, 11,	nullptr,
	SFP::ai_move, 3,		nullptr,
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, 4,		nullptr,
	SFP::ai_move, 1,		nullptr,
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, -3,	nullptr,
	SFP::ai_move, -4,	nullptr,
	SFP::ai_move, 5,		nullptr,
	SFP::ai_move, 7,		nullptr,
	SFP::ai_move, -2,	nullptr,
	SFP::ai_move, 3,		nullptr,
	SFP::ai_move, -5,	nullptr,
	SFP::ai_move, -2,	nullptr,
	SFP::ai_move, -8,	nullptr,
	SFP::ai_move, 2,		nullptr
};

void chick_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	r = random();
	if (r < 0.33)
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else if (r < 0.66)
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = chick_move_pain1;
	else if (damage <= 25)
		self->monsterinfo.currentmove = chick_move_pain2;
	else
		self->monsterinfo.currentmove = chick_move_pain3;
}

void chick_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, 0);
	VectorSet (self->s.maxs, 16, 16, 16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t chick_frames_death2 [] =
{
	SFP::ai_move, -6, nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -5, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, 1,  nullptr,
	SFP::ai_move, 10, nullptr,
	SFP::ai_move, 2,  nullptr,
	SFP::ai_move, 3,  nullptr,
	SFP::ai_move, 1,  nullptr,
	SFP::ai_move, 2, nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 3,  nullptr,
	SFP::ai_move, 3,  nullptr,
	SFP::ai_move, 1,  nullptr,
	SFP::ai_move, -3,  nullptr,
	SFP::ai_move, -5, nullptr,
	SFP::ai_move, 4, nullptr,
	SFP::ai_move, 15, nullptr,
	SFP::ai_move, 14, nullptr,
	SFP::ai_move, 1, nullptr
};

mframe_t chick_frames_death1 [] =
{
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, -7, nullptr,
	SFP::ai_move, 4,  nullptr,
	SFP::ai_move, 11, nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr
	
};

void chick_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	n = rand() % 2;
	if (n == 0)
	{
		self->monsterinfo.currentmove = chick_move_death1;
		gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	}
	else
	{
		self->monsterinfo.currentmove = chick_move_death2;
		gi.sound (self, CHAN_VOICE, sound_death2, 1, ATTN_NORM, 0);
	}
}


void chick_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->s.maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void chick_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void chick_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->s.maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}
AutoSFP(chick_duck_down)
AutoSFP(chick_duck_hold)
AutoSFP(chick_duck_up)
mframe_t chick_frames_duck [] =
{
	SFP::ai_move, 0, SFP::chick_duck_down,
	SFP::ai_move, 1, nullptr,
	SFP::ai_move, 4, SFP::chick_duck_hold,
	SFP::ai_move, -4,  nullptr,
	SFP::ai_move, -5,  SFP::chick_duck_up,
	SFP::ai_move, 3, nullptr,
	SFP::ai_move, 1,  nullptr
};

void chick_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = chick_move_duck;
}

void ChickSlash (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->s.mins[0], 10);
	gi.sound (self, CHAN_WEAPON, sound_melee_swing, 1, ATTN_NORM, 0);
	fire_hit (self, aim, (10 + (rand() %6)), 100);
}


void ChickRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_CHICK_ROCKET_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, 50, 500, MZ2_CHICK_ROCKET_1);
}	

void Chick_PreAttack1 (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_missile_prelaunch, 1, ATTN_NORM, 0);
}

void ChickReload (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_missile_reload, 1, ATTN_NORM, 0);
}

AutoSFP(Chick_PreAttack1)
AutoSFP(chick_attack1)

mframe_t chick_frames_start_attack1 [] =
{
	SFP::ai_charge, 0,	SFP::Chick_PreAttack1,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 4,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, -3,  nullptr,
	SFP::ai_charge, 3,	nullptr,
	SFP::ai_charge, 5,	nullptr,
	SFP::ai_charge, 7,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::chick_attack1
};

AutoSFP(ChickRocket)
AutoSFP(ChickReload)
AutoSFP(chick_rerocket)
mframe_t chick_frames_attack1 [] =
{
	SFP::ai_charge, 19,	SFP::ChickRocket,
	SFP::ai_charge, -6,	nullptr,
	SFP::ai_charge, -5,	nullptr,
	SFP::ai_charge, -2,	nullptr,
	SFP::ai_charge, -7,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, 10,	SFP::ChickReload,
	SFP::ai_charge, 4,	nullptr,
	SFP::ai_charge, 5,	nullptr,
	SFP::ai_charge, 6,	nullptr,
	SFP::ai_charge, 6,	nullptr,
	SFP::ai_charge, 4,	nullptr,
	SFP::ai_charge, 3,	SFP::chick_rerocket

};

mframe_t chick_frames_end_attack1 [] =
{
	SFP::ai_charge, -3,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, -6,	nullptr,
	SFP::ai_charge, -4,	nullptr,
	SFP::ai_charge, -2,  nullptr
};

void chick_rerocket(edict_t *self)
{
	if (self->enemy->health > 0)
	{
		if (range (self, self->enemy) > RANGE_MELEE)
			if ( visible (self, self->enemy) )
				if (random() <= 0.6)
				{
					self->monsterinfo.currentmove = chick_move_attack1;
					return;
				}
	}	
	self->monsterinfo.currentmove = chick_move_end_attack1;
}

void chick_attack1(edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_attack1;
}
AutoSFP(ChickSlash)
AutoSFP(chick_reslash)
mframe_t chick_frames_slash [] =
{
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, 7,	SFP::ChickSlash,
	SFP::ai_charge, -7,	nullptr,
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, -1,	nullptr,
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, -2,	SFP::chick_reslash
};

mframe_t chick_frames_end_slash [] =
{
	SFP::ai_charge, -6,	nullptr,
	SFP::ai_charge, -1,	nullptr,
	SFP::ai_charge, -6,	nullptr,
	SFP::ai_charge, 0,	nullptr
};


void chick_reslash(edict_t *self)
{
	if (self->enemy->health > 0)
	{
		if (range (self, self->enemy) == RANGE_MELEE)
			if (random() <= 0.9)
			{				
				self->monsterinfo.currentmove = chick_move_slash;
				return;
			}
			else
			{
				self->monsterinfo.currentmove = chick_move_end_slash;
				return;
			}
	}
	self->monsterinfo.currentmove = chick_move_end_slash;
}

void chick_slash(edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_slash;
}


mframe_t chick_frames_start_slash [] =
{	
	SFP::ai_charge, 1,	nullptr,
	SFP::ai_charge, 8,	nullptr,
	SFP::ai_charge, 3,	nullptr
};
AutoSFP(chick_stand)
AutoSFP(chick_run)
AutoSFP(chick_dead)
AutoSFP(chick_slash)

mmove_t chick_moves[] = {
	{FRAME_stand201, FRAME_stand230, chick_frames_fidget, SFP::chick_stand},
	{FRAME_stand101, FRAME_stand130, chick_frames_stand, nullptr},
	{FRAME_walk01, FRAME_walk10, chick_frames_start_run, SFP::chick_run},
	{FRAME_walk11, FRAME_walk20, chick_frames_run, nullptr},
	{FRAME_walk11, FRAME_walk20, chick_frames_walk, nullptr},
	{FRAME_pain101, FRAME_pain105, chick_frames_pain1, SFP::chick_run},
	{FRAME_pain201, FRAME_pain205, chick_frames_pain2, SFP::chick_run},
	{FRAME_pain301, FRAME_pain321, chick_frames_pain3, SFP::chick_run},
	{FRAME_death201, FRAME_death223, chick_frames_death2, SFP::chick_dead},
	{FRAME_death101, FRAME_death112, chick_frames_death1, SFP::chick_dead},
	{FRAME_duck01, FRAME_duck07, chick_frames_duck, SFP::chick_run},
	{FRAME_attak101, FRAME_attak113, chick_frames_start_attack1, nullptr},
	{FRAME_attak114, FRAME_attak127, chick_frames_attack1, nullptr},
	{FRAME_attak128, FRAME_attak132, chick_frames_end_attack1, SFP::chick_run},
	{FRAME_attak204, FRAME_attak212, chick_frames_slash, nullptr},
	{FRAME_attak213, FRAME_attak216, chick_frames_end_slash, SFP::chick_run},
	{FRAME_attak201, FRAME_attak203, chick_frames_start_slash, SFP::chick_slash},
};

mmove_t * chick_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &chick_moves[self->monsterinfo.currentmove-1];
}

void chick_melee(edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_start_slash;
}


void chick_attack(edict_t *self)
{
	self->monsterinfo.currentmove = chick_move_start_attack1;
}

void chick_sight(edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

SFPEnt(pain, chick_pain)
SFPEnt(die, chick_die)


SFPEnt(monsterinfo.walk, chick_walk)

SFPEnt(monsterinfo.dodge, chick_dodge)
SFPEnt(monsterinfo.attack, chick_attack)
SFPEnt(monsterinfo.melee, chick_melee)
SFPEnt(monsterinfo.sight, chick_sight)
SFPEnt(monsterinfo.get_currentmove, chick_get_currentmove)
/*QUAKED monster_chick (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_chick (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_missile_prelaunch	= gi.soundindex ("chick/chkatck1.wav");	
	sound_missile_launch	= gi.soundindex ("chick/chkatck2.wav");	
	sound_melee_swing		= gi.soundindex ("chick/chkatck3.wav");	
	sound_melee_hit			= gi.soundindex ("chick/chkatck4.wav");	
	sound_missile_reload	= gi.soundindex ("chick/chkatck5.wav");	
	sound_death1			= gi.soundindex ("chick/chkdeth1.wav");	
	sound_death2			= gi.soundindex ("chick/chkdeth2.wav");	
	sound_fall_down			= gi.soundindex ("chick/chkfall1.wav");	
	sound_idle1				= gi.soundindex ("chick/chkidle1.wav");	
	sound_idle2				= gi.soundindex ("chick/chkidle2.wav");	
	sound_pain1				= gi.soundindex ("chick/chkpain1.wav");	
	sound_pain2				= gi.soundindex ("chick/chkpain2.wav");	
	sound_pain3				= gi.soundindex ("chick/chkpain3.wav");	
	sound_sight				= gi.soundindex ("chick/chksght1.wav");	
	sound_search			= gi.soundindex ("chick/chksrch1.wav");	

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	VectorSet (self->s.mins, -16, -16, 0);
	VectorSet (self->s.maxs, 16, 16, 56);

	self->health = 175;
	self->gib_health = -70;
	self->mass = 200;

	self->pain = SFP::chick_pain;
	self->die = SFP::chick_die;

	self->monsterinfo.stand = SFP::chick_stand;
	self->monsterinfo.walk = SFP::chick_walk;
	self->monsterinfo.run = SFP::chick_run;
	self->monsterinfo.dodge = SFP::chick_dodge;
	self->monsterinfo.attack = SFP::chick_attack;
	self->monsterinfo.melee = SFP::chick_melee;
	self->monsterinfo.sight = SFP::chick_sight;
	self->monsterinfo.get_currentmove = SFP::chick_get_currentmove;

	gi.linkentity (self);

	self->monsterinfo.currentmove = chick_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
