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

GUNNER

==============================================================================
*/

#include "g_local.h"
#include "m_gunner.h"


static int	sound_pain;
static int	sound_pain2;
static int	sound_death;
static int	sound_idle;
static int	sound_open;
static int	sound_search;
static int	sound_sight;

enum {
	gunner_move_fidget = 1,
	gunner_move_stand,
	gunner_move_walk,
	gunner_move_run,
	gunner_move_runandshoot,
	gunner_move_pain3,
	gunner_move_pain2,
	gunner_move_pain1,
	gunner_move_death,
	gunner_move_duck,
	gunner_move_attack_chain,
	gunner_move_fire_chain,
	gunner_move_endfire_chain,
	gunner_move_attack_grenade
};
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)


void gunner_idlesound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void gunner_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void gunner_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


qboolean visible (edict_t *self, edict_t *other);
void GunnerGrenade (edict_t *self);
void GunnerFire (edict_t *self);
void gunner_fire_chain(edict_t *self);
void gunner_refire_chain(edict_t *self);


void gunner_stand (edict_t *self);
AutoSFP(gunner_idlesound)

mframe_t gunner_frames_fidget [] =
{
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, SFP::gunner_idlesound,
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

void gunner_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() <= 0.05)
		self->monsterinfo.currentmove = gunner_move_fidget;
}
AutoSFP(gunner_fidget)

mframe_t gunner_frames_stand [] =
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
	SFP::ai_stand, 0, SFP::gunner_fidget,

	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, SFP::gunner_fidget,

	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, SFP::gunner_fidget
};

void gunner_stand (edict_t *self)
{
		self->monsterinfo.currentmove = gunner_move_stand;
}


mframe_t gunner_frames_walk [] =
{
	SFP::ai_walk, 0, nullptr,
	SFP::ai_walk, 3, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 7, nullptr,
	SFP::ai_walk, 2, nullptr,
	SFP::ai_walk, 6, nullptr,
	SFP::ai_walk, 4, nullptr,
	SFP::ai_walk, 2, nullptr,
	SFP::ai_walk, 7, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 7, nullptr,
	SFP::ai_walk, 4, nullptr
};

void gunner_walk (edict_t *self)
{
	self->monsterinfo.currentmove = gunner_move_walk;
}

mframe_t gunner_frames_run [] =
{
	SFP::ai_run, 26, nullptr,
	SFP::ai_run, 9,  nullptr,
	SFP::ai_run, 9,  nullptr,
	SFP::ai_run, 9,  nullptr,
	SFP::ai_run, 15, nullptr,
	SFP::ai_run, 10, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 6,  nullptr
};


void gunner_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = gunner_move_stand;
	else
		self->monsterinfo.currentmove = gunner_move_run;
}

mframe_t gunner_frames_runandshoot [] =
{
	SFP::ai_run, 32, nullptr,
	SFP::ai_run, 15, nullptr,
	SFP::ai_run, 10, nullptr,
	SFP::ai_run, 18, nullptr,
	SFP::ai_run, 8,  nullptr,
	SFP::ai_run, 20, nullptr
};


void gunner_runandshoot (edict_t *self)
{
	self->monsterinfo.currentmove = gunner_move_runandshoot;
}

mframe_t gunner_frames_pain3 [] =
{
	SFP::ai_move, -3, nullptr,
	SFP::ai_move, 1,	 nullptr,
	SFP::ai_move, 1,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 1,	 nullptr
};

mframe_t gunner_frames_pain2 [] =
{
	SFP::ai_move, -2, nullptr,
	SFP::ai_move, 11, nullptr,
	SFP::ai_move, 6,	 nullptr,
	SFP::ai_move, 2,	 nullptr,
	SFP::ai_move, -1, nullptr,
	SFP::ai_move, -7, nullptr,
	SFP::ai_move, -2, nullptr,
	SFP::ai_move, -7, nullptr
};

mframe_t gunner_frames_pain1 [] =
{
	SFP::ai_move, 2,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, -5, nullptr,
	SFP::ai_move, 3,	 nullptr,
	SFP::ai_move, -1, nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 1,	 nullptr,
	SFP::ai_move, 1,	 nullptr,
	SFP::ai_move, 2,	 nullptr,
	SFP::ai_move, 1,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, -2, nullptr,
	SFP::ai_move, -2, nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr
};

void gunner_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (rand()&1)
		gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
		self->monsterinfo.currentmove = gunner_move_pain3;
	else if (damage <= 25)
		self->monsterinfo.currentmove = gunner_move_pain2;
	else
		self->monsterinfo.currentmove = gunner_move_pain1;
}

void gunner_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t gunner_frames_death [] =
{
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, -7, nullptr,
	SFP::ai_move, -3, nullptr,
	SFP::ai_move, -5, nullptr,
	SFP::ai_move, 8,	 nullptr,
	SFP::ai_move, 6,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr,
	SFP::ai_move, 0,	 nullptr
};

void gunner_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = gunner_move_death;
}


void gunner_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	if (skill->value >= 2)
	{
		if (random() > 0.5)
			GunnerGrenade (self);
	}

	self->s.maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void gunner_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void gunner_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->s.maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}
AutoSFP(gunner_duck_down)
AutoSFP(gunner_duck_hold)
AutoSFP(gunner_duck_up)
mframe_t gunner_frames_duck [] =
{
	SFP::ai_move, 1,  SFP::gunner_duck_down,
	SFP::ai_move, 1,  nullptr,
	SFP::ai_move, 1,  SFP::gunner_duck_hold,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, -1, nullptr,
	SFP::ai_move, -1, nullptr,
	SFP::ai_move, 0,  SFP::gunner_duck_up,
	SFP::ai_move, -1, nullptr
};

void gunner_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = gunner_move_duck;
}


void gunner_opengun (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_open, 1, ATTN_IDLE, 0);
}

void GunnerFire (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	int		flash_number;

	flash_number = MZ2_GUNNER_MACHINEGUN_1 + (self->s.frame - FRAME_attak216);

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	VectorMA (target, -0.2, self->enemy->velocity, target);
	target[2] += self->enemy->viewheight;

	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	monster_fire_bullet (self, start, aim, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}

void GunnerGrenade (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	aim;
	int		flash_number;

	if (self->s.frame == FRAME_attak105)
		flash_number = MZ2_GUNNER_GRENADE_1;
	else if (self->s.frame == FRAME_attak108)
		flash_number = MZ2_GUNNER_GRENADE_2;
	else if (self->s.frame == FRAME_attak111)
		flash_number = MZ2_GUNNER_GRENADE_3;
	else // (self->s.frame == FRAME_attak114)
		flash_number = MZ2_GUNNER_GRENADE_4;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	//FIXME : do a spread -225 -75 75 225 degrees around forward
	VectorCopy (forward, aim);

	monster_fire_grenade (self, start, aim, 50, 600, flash_number);
}
AutoSFP(gunner_opengun)
AutoSFP(GunnerFire)
AutoSFP(GunnerGrenade)
mframe_t gunner_frames_attack_chain [] =
{
	/*
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	*/
	SFP::ai_charge, 0, SFP::gunner_opengun,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

mframe_t gunner_frames_fire_chain [] =
{
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire,
	SFP::ai_charge,   0, SFP::GunnerFire
};

mframe_t gunner_frames_endfire_chain [] =
{
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

mframe_t gunner_frames_attack_grenade [] =
{
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GunnerGrenade,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GunnerGrenade,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GunnerGrenade,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::GunnerGrenade,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

void gunner_attack(edict_t *self)
{
	if (range (self, self->enemy) == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = gunner_move_attack_chain;
	}
	else
	{
		if (random() <= 0.5)
			self->monsterinfo.currentmove = gunner_move_attack_grenade;
		else
			self->monsterinfo.currentmove = gunner_move_attack_chain;
	}
}

void gunner_fire_chain(edict_t *self)
{
	self->monsterinfo.currentmove = gunner_move_fire_chain;
}

void gunner_refire_chain(edict_t *self)
{
	if (self->enemy->health > 0)
		if ( visible (self, self->enemy) )
			if (random() <= 0.5)
			{
				self->monsterinfo.currentmove = gunner_move_fire_chain;
				return;
			}
	self->monsterinfo.currentmove = gunner_move_endfire_chain;
}
AutoSFP(gunner_stand)
AutoSFP(gunner_run)
AutoSFP(gunner_dead)
AutoSFP(gunner_fire_chain)
AutoSFP(gunner_refire_chain)
mmove_t	gunner_moves[] = {
	{FRAME_stand31, FRAME_stand70, gunner_frames_fidget, SFP::gunner_stand},
	{FRAME_stand01, FRAME_stand30, gunner_frames_stand, nullptr},
	{FRAME_walk07, FRAME_walk19, gunner_frames_walk, nullptr},
	{FRAME_run01, FRAME_run08, gunner_frames_run, nullptr},
	{FRAME_runs01, FRAME_runs06, gunner_frames_runandshoot, nullptr},
	{FRAME_pain301, FRAME_pain305, gunner_frames_pain3, SFP::gunner_run},
	{FRAME_pain201, FRAME_pain208, gunner_frames_pain2, SFP::gunner_run},
	{FRAME_pain101, FRAME_pain118, gunner_frames_pain1, SFP::gunner_run},
	{FRAME_death01, FRAME_death11, gunner_frames_death, SFP::gunner_dead},
	{FRAME_duck01, FRAME_duck08, gunner_frames_duck, SFP::gunner_run},
	{FRAME_attak209, FRAME_attak215, gunner_frames_attack_chain, SFP::gunner_fire_chain},
	{FRAME_attak216, FRAME_attak223, gunner_frames_fire_chain, SFP::gunner_refire_chain},
	{FRAME_attak224, FRAME_attak230, gunner_frames_endfire_chain, SFP::gunner_run},
	{FRAME_attak101, FRAME_attak121, gunner_frames_attack_grenade, SFP::gunner_run},
};

mmove_t * gunner_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &gunner_moves[self->monsterinfo.currentmove-1];
}

SFPEnt(pain, gunner_pain)
SFPEnt(die, gunner_die)

SFPEnt(monsterinfo.walk, gunner_walk)
SFPEnt(monsterinfo.dodge, gunner_dodge)
SFPEnt(monsterinfo.attack, gunner_attack)

SFPEnt(monsterinfo.sight, gunner_sight)
SFPEnt(monsterinfo.search, gunner_search)
SFPEnt(monsterinfo.get_currentmove, gunner_get_currentmove)
/*QUAKED monster_gunner (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_gunner (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_death = gi.soundindex ("gunner/death1.wav");	
	sound_pain = gi.soundindex ("gunner/gunpain2.wav");	
	sound_pain2 = gi.soundindex ("gunner/gunpain1.wav");	
	sound_idle = gi.soundindex ("gunner/gunidle1.wav");	
	sound_open = gi.soundindex ("gunner/gunatck1.wav");	
	sound_search = gi.soundindex ("gunner/gunsrch1.wav");	
	sound_sight = gi.soundindex ("gunner/sight1.wav");	

	gi.soundindex ("gunner/gunatck2.wav");
	gi.soundindex ("gunner/gunatck3.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/gunner/tris.md2");
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, 32);

	self->health = 175;
	self->gib_health = -70;
	self->mass = 200;

	self->pain = SFP::gunner_pain;
	self->die = SFP::gunner_die;

	self->monsterinfo.stand = SFP::gunner_stand;
	self->monsterinfo.walk = SFP::gunner_walk;
	self->monsterinfo.run = SFP::gunner_run;
	self->monsterinfo.dodge = SFP::gunner_dodge;
	self->monsterinfo.attack = SFP::gunner_attack;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = SFP::gunner_sight;
	self->monsterinfo.search = SFP::gunner_search;
	self->monsterinfo.get_currentmove = SFP::gunner_get_currentmove;

	gi.linkentity (self);

	self->monsterinfo.currentmove = gunner_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
