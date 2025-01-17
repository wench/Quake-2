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

BERSERK

==============================================================================
*/

#include "g_local.h"
#include "m_berserk.h"


static int sound_pain;
static int sound_die;
static int sound_idle;
static int sound_punch;
static int sound_sight;
static int sound_search;


enum {
	berserk_move_stand=1,
	berserk_move_stand_fidget,
	berserk_move_walk,
	berserk_move_run1,
	berserk_move_attack_spike,
	berserk_move_attack_club,
	berserk_move_attack_strike,
	berserk_move_pain1,
	berserk_move_pain2,
	berserk_move_death1,
	berserk_move_death2
};

void berserk_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void berserk_search (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_NORM, 0);
}


void berserk_fidget (edict_t *self);
AutoSFP(berserk_fidget)
AutoSFP( ai_stand)
mframe_t berserk_frames_stand [] =
{
	SFP::ai_stand, 0, SFP::berserk_fidget,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr
};

void berserk_stand (edict_t *self)
{
	self->monsterinfo.currentmove = berserk_move_stand;
}

mframe_t berserk_frames_stand_fidget [] =
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
	SFP::ai_stand, 0, nullptr
};

void berserk_fidget (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		return;
	if (random() > 0.15)
		return;

	self->monsterinfo.currentmove = berserk_move_stand_fidget;
	gi.sound (self, CHAN_WEAPON, sound_idle, 1, ATTN_IDLE, 0);
}

AutoSFP(ai_walk)
mframe_t berserk_frames_walk [] =
{
	SFP::ai_walk, 9.1, nullptr,
	SFP::ai_walk, 6.3, nullptr,
	SFP::ai_walk, 4.9, nullptr,
	SFP::ai_walk, 6.7, nullptr,
	SFP::ai_walk, 6.0, nullptr,
	SFP::ai_walk, 8.2, nullptr,
	SFP::ai_walk, 7.2, nullptr,
	SFP::ai_walk, 6.1, nullptr,
	SFP::ai_walk, 4.9, nullptr,
	SFP::ai_walk, 4.7, nullptr,
	SFP::ai_walk, 4.7, nullptr,
	SFP::ai_walk, 4.8, nullptr
};

void berserk_walk (edict_t *self)
{
	self->monsterinfo.currentmove = berserk_move_walk;
}

/*

  *****************************
  SKIPPED THIS FOR NOW!
  *****************************

   Running -> Arm raised in air

void()	berserk_runb1	=[	$r_att1 ,	berserk_runb2	] {ai_run(21);};
void()	berserk_runb2	=[	$r_att2 ,	berserk_runb3	] {ai_run(11);};
void()	berserk_runb3	=[	$r_att3 ,	berserk_runb4	] {ai_run(21);};
void()	berserk_runb4	=[	$r_att4 ,	berserk_runb5	] {ai_run(25);};
void()	berserk_runb5	=[	$r_att5 ,	berserk_runb6	] {ai_run(18);};
void()	berserk_runb6	=[	$r_att6 ,	berserk_runb7	] {ai_run(19);};
// running with arm in air : start loop
void()	berserk_runb7	=[	$r_att7 ,	berserk_runb8	] {ai_run(21);};
void()	berserk_runb8	=[	$r_att8 ,	berserk_runb9	] {ai_run(11);};
void()	berserk_runb9	=[	$r_att9 ,	berserk_runb10	] {ai_run(21);};
void()	berserk_runb10	=[	$r_att10 ,	berserk_runb11	] {ai_run(25);};
void()	berserk_runb11	=[	$r_att11 ,	berserk_runb12	] {ai_run(18);};
void()	berserk_runb12	=[	$r_att12 ,	berserk_runb7	] {ai_run(19);};
// running with arm in air : end loop
*/

AutoSFP(ai_run)
mframe_t berserk_frames_run1 [] =
{
	SFP::ai_run, 21, nullptr,
	SFP::ai_run, 11, nullptr,
	SFP::ai_run, 21, nullptr,
	SFP::ai_run, 25, nullptr,
	SFP::ai_run, 18, nullptr,
	SFP::ai_run, 19, nullptr
};

void berserk_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = berserk_move_stand;
	else
		self->monsterinfo.currentmove = berserk_move_run1;
}


void berserk_attack_spike (edict_t *self)
{
	static	vec3_t	aim = {MELEE_DISTANCE, 0, -24};
	fire_hit (self, aim, (15 + (rand() % 6)), 400);		//	Faster attack -- upwards and backwards
}


void berserk_swing (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_punch, 1, ATTN_NORM, 0);
}
AutoSFP(ai_charge)
AutoSFP(berserk_swing)
AutoSFP(berserk_attack_spike)
mframe_t berserk_frames_attack_spike [] =
{
		SFP::ai_charge, 0, nullptr,
		SFP::ai_charge, 0, nullptr,
		SFP::ai_charge, 0, SFP::berserk_swing,
		SFP::ai_charge, 0, SFP::berserk_attack_spike,
		SFP::ai_charge, 0, nullptr,
		SFP::ai_charge, 0, nullptr,
		SFP::ai_charge, 0, nullptr,
		SFP::ai_charge, 0, nullptr
};


void berserk_attack_club (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->s.mins[0], -4);
	fire_hit (self, aim, (5 + (rand() % 6)), 400);		// Slower attack
}
AutoSFP(berserk_attack_club)
mframe_t berserk_frames_attack_club [] =
{	
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::berserk_swing,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, SFP::berserk_attack_club,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};


void berserk_strike (edict_t *self)
{
	//FIXME play impact sound
}

AutoSFP(berserk_strike);
AutoSFP(ai_move)
mframe_t berserk_frames_attack_strike [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, SFP::berserk_swing,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, SFP::berserk_strike,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 9.7, nullptr,
	SFP::ai_move, 13.6, nullptr
};
	


void berserk_melee (edict_t *self)
{
	if ((rand() % 2) == 0)
		self->monsterinfo.currentmove = berserk_move_attack_spike;
	else
		self->monsterinfo.currentmove = berserk_move_attack_club;
}


/*
void() 	berserk_atke1	=[	$r_attb1,	berserk_atke2	] {ai_run(9);};
void() 	berserk_atke2	=[	$r_attb2,	berserk_atke3	] {ai_run(6);};
void() 	berserk_atke3	=[	$r_attb3,	berserk_atke4	] {ai_run(18.4);};
void() 	berserk_atke4	=[	$r_attb4,	berserk_atke5	] {ai_run(25);};
void() 	berserk_atke5	=[	$r_attb5,	berserk_atke6	] {ai_run(14);};
void() 	berserk_atke6	=[	$r_attb6,	berserk_atke7	] {ai_run(20);};
void() 	berserk_atke7	=[	$r_attb7,	berserk_atke8	] {ai_run(8.5);};
void() 	berserk_atke8	=[	$r_attb8,	berserk_atke9	] {ai_run(3);};
void() 	berserk_atke9	=[	$r_attb9,	berserk_atke10	] {ai_run(17.5);};
void() 	berserk_atke10	=[	$r_attb10,	berserk_atke11	] {ai_run(17);};
void() 	berserk_atke11	=[	$r_attb11,	berserk_atke12	] {ai_run(9);};
void() 	berserk_atke12	=[	$r_attb12,	berserk_atke13	] {ai_run(25);};
void() 	berserk_atke13	=[	$r_attb13,	berserk_atke14	] {ai_run(3.7);};
void() 	berserk_atke14	=[	$r_attb14,	berserk_atke15	] {ai_run(2.6);};
void() 	berserk_atke15	=[	$r_attb15,	berserk_atke16	] {ai_run(19);};
void() 	berserk_atke16	=[	$r_attb16,	berserk_atke17	] {ai_run(25);};
void() 	berserk_atke17	=[	$r_attb17,	berserk_atke18	] {ai_run(19.6);};
void() 	berserk_atke18	=[	$r_attb18,	berserk_run1	] {ai_run(7.8);};
*/



mframe_t berserk_frames_pain1 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};


mframe_t berserk_frames_pain2 [] =
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
	SFP::ai_move, 0, nullptr
};

void berserk_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if ((damage < 20) || (random() < 0.5))
		self->monsterinfo.currentmove = berserk_move_pain1;
	else
		self->monsterinfo.currentmove = berserk_move_pain2;
}


void berserk_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


mframe_t berserk_frames_death1 [] =
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
	SFP::ai_move, 0, nullptr
	
};

mframe_t berserk_frames_death2 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};
AutoSFP(berserk_stand)
AutoSFP(berserk_run)
AutoSFP(berserk_dead)

mmove_t berserk_moves[] = {
	{FRAME_stand1, FRAME_stand5, berserk_frames_stand, nullptr},
	{FRAME_standb1, FRAME_standb20, berserk_frames_stand_fidget, SFP::berserk_stand},
	{FRAME_walkc1, FRAME_walkc11, berserk_frames_walk, nullptr},
	{FRAME_run1, FRAME_run6, berserk_frames_run1, nullptr},
	{FRAME_att_c1, FRAME_att_c8, berserk_frames_attack_spike, SFP::berserk_run},
	{FRAME_att_c9, FRAME_att_c20, berserk_frames_attack_club, SFP::berserk_run},
	{FRAME_att_c21, FRAME_att_c34, berserk_frames_attack_strike, SFP::berserk_run},
	{FRAME_painc1, FRAME_painc4, berserk_frames_pain1, SFP::berserk_run},
	{FRAME_painb1, FRAME_painb20, berserk_frames_pain2, SFP::berserk_run},
	{FRAME_death1, FRAME_death13, berserk_frames_death1, SFP::berserk_dead},
	{FRAME_deathc1, FRAME_deathc8, berserk_frames_death2, SFP::berserk_dead}
};


mmove_t *berserk_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &berserk_moves[self->monsterinfo.currentmove-1];
}

void berserk_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

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

	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (damage >= 50)
		self->monsterinfo.currentmove = berserk_move_death1;
	else
		self->monsterinfo.currentmove = berserk_move_death2;
}


SFPEnt(pain, berserk_pain);
SFPEnt(die, berserk_die);

SFPEnt(monsterinfo.get_currentmove, berserk_get_currentmove);
SFPEnt(monsterinfo.walk, berserk_walk);

SFPEnt(monsterinfo.melee, berserk_melee);
SFPEnt(monsterinfo.sight, berserk_sight);
SFPEnt(monsterinfo.search, berserk_search);

/*QUAKED monster_berserk (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_berserk (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	// pre-caches
	sound_pain  = gi.soundindex ("berserk/berpain2.wav");
	sound_die   = gi.soundindex ("berserk/berdeth2.wav");
	sound_idle  = gi.soundindex ("berserk/beridle1.wav");
	sound_punch = gi.soundindex ("berserk/attack.wav");
	sound_search = gi.soundindex ("berserk/bersrch1.wav");
	sound_sight = gi.soundindex ("berserk/sight.wav");

	self->s.modelindex = gi.modelindex("models/monsters/berserk/tris.md2");
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, 32);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 240;
	self->gib_health = -60;
	self->mass = 250;

	self->pain = SFP::berserk_pain;
	self->die = SFP::berserk_die;

	self->monsterinfo.get_currentmove = SFP::berserk_get_currentmove;
	self->monsterinfo.stand = SFP::berserk_stand;
	self->monsterinfo.walk = SFP::berserk_walk;
	self->monsterinfo.run = SFP::berserk_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = nullptr;
	self->monsterinfo.melee = SFP::berserk_melee;
	self->monsterinfo.sight = SFP::berserk_sight;
	self->monsterinfo.search = SFP::berserk_search;

	self->monsterinfo.currentmove = berserk_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	gi.linkentity (self);

	walkmonster_start (self);
}
