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

SUPERTANK

==============================================================================
*/

#include "g_local.h"
#include "m_supertank.h"

qboolean visible (edict_t *self, edict_t *other);

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_death;
static int	sound_search1;
static int	sound_search2;

static	int	tread_sound;

enum {
	supertank_move_stand = 1,
	supertank_move_run,
	supertank_move_forward,
	supertank_move_turn_right,
	supertank_move_turn_left,
	supertank_move_pain3,
	supertank_move_pain2,
	supertank_move_pain1,
	supertank_move_death,
	supertank_move_backward,
	supertank_move_attack4,
	supertank_move_attack3,
	supertank_move_attack2,
	supertank_move_attack1,
	supertank_move_end_attack1
};

void BossExplode (edict_t *self);

void TreadSound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, tread_sound, 1, ATTN_NORM, 0);
}

void supertank_search (edict_t *self)
{
	if (random() < 0.5)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_search2, 1, ATTN_NORM, 0);
}


void supertank_dead (edict_t *self);
void supertankRocket (edict_t *self);
void supertankMachineGun (edict_t *self);
void supertank_reattack1(edict_t *self);

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)


AutoSFP(BossExplode)
AutoSFP(TreadSound)
AutoSFP(supertank_search)
AutoSFP(supertank_dead)
AutoSFP(supertankRocket)
AutoSFP(supertankMachineGun)
AutoSFP(supertank_reattack1)

//
// stand
//

mframe_t supertank_frames_stand []=
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
	
void supertank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = supertank_move_stand;
}


mframe_t supertank_frames_run [] =
{
	SFP::ai_run, 12,	SFP::TreadSound,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 12,	nullptr
};

//
// walk
//


mframe_t supertank_frames_forward [] =
{
	SFP::ai_walk, 4,	SFP::TreadSound,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	nullptr
};

void supertank_forward (edict_t *self)
{
		self->monsterinfo.currentmove = supertank_move_forward;
}

void supertank_walk (edict_t *self)
{
		self->monsterinfo.currentmove = supertank_move_forward;
}

void supertank_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = supertank_move_stand;
	else
		self->monsterinfo.currentmove = supertank_move_run;
}

mframe_t supertank_frames_turn_right [] =
{
	SFP::ai_move,	0,	SFP::TreadSound,
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

mframe_t supertank_frames_turn_left [] =
{
	SFP::ai_move,	0,	SFP::TreadSound,
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


mframe_t supertank_frames_pain3 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t supertank_frames_pain2 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t supertank_frames_pain1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t supertank_frames_death1 [] =
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
	SFP::ai_move,	0,	SFP::BossExplode
};

mframe_t supertank_frames_backward[] =
{
	SFP::ai_walk, 0,	SFP::TreadSound,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr,
	SFP::ai_walk, 0,	nullptr
};

mframe_t supertank_frames_attack4[]=
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t supertank_frames_attack3[]=
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
	SFP::ai_move,	0,	nullptr
};

mframe_t supertank_frames_attack2[]=
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::supertankRocket,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::supertankRocket,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::supertankRocket,
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

mframe_t supertank_frames_attack1[]=
{
	SFP::ai_charge,	0,	SFP::supertankMachineGun,
	SFP::ai_charge,	0,	SFP::supertankMachineGun,
	SFP::ai_charge,	0,	SFP::supertankMachineGun,
	SFP::ai_charge,	0,	SFP::supertankMachineGun,
	SFP::ai_charge,	0,	SFP::supertankMachineGun,
	SFP::ai_charge,	0,	SFP::supertankMachineGun,

};

mframe_t supertank_frames_end_attack1[]=
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
	SFP::ai_move,	0,	nullptr
};
AutoSFP(supertank_run)


mmove_t	supertank_moves[] = {
	{FRAME_stand_1, FRAME_stand_60, supertank_frames_stand, nullptr},
	{FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_run, nullptr},
	{FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_forward, nullptr},
	{FRAME_right_1, FRAME_right_18, supertank_frames_turn_right, SFP::supertank_run},
	{FRAME_left_1, FRAME_left_18, supertank_frames_turn_left, SFP::supertank_run},
	{FRAME_pain3_9, FRAME_pain3_12, supertank_frames_pain3, SFP::supertank_run},
	{FRAME_pain2_5, FRAME_pain2_8, supertank_frames_pain2, SFP::supertank_run},
	{FRAME_pain1_1, FRAME_pain1_4, supertank_frames_pain1, SFP::supertank_run},
	{FRAME_death_1, FRAME_death_24, supertank_frames_death1, SFP::supertank_dead},
	{FRAME_backwd_1, FRAME_backwd_18, supertank_frames_backward, nullptr},
	{FRAME_attak4_1, FRAME_attak4_6, supertank_frames_attack4, SFP::supertank_run},
	{FRAME_attak3_1, FRAME_attak3_27, supertank_frames_attack3, SFP::supertank_run},
	{FRAME_attak2_1, FRAME_attak2_27, supertank_frames_attack2, SFP::supertank_run},
	{FRAME_attak1_1, FRAME_attak1_6, supertank_frames_attack1, SFP::supertank_reattack1},
	{FRAME_attak1_7, FRAME_attak1_20, supertank_frames_end_attack1, SFP::supertank_run}
};

mmove_t * supertank_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &supertank_moves[self->monsterinfo.currentmove-1];
}

void supertank_reattack1(edict_t *self)
{
	if (visible(self, self->enemy))
		if (random() < 0.9)
			self->monsterinfo.currentmove = supertank_move_attack1;
		else
			self->monsterinfo.currentmove = supertank_move_end_attack1;	
	else
		self->monsterinfo.currentmove = supertank_move_end_attack1;
}

void supertank_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames
	if (damage <=25)
		if (random()<0.2)
			return;

	// Don't go into pain if he's firing his rockets
	if (skill->value >= 2)
		if ( (self->s.frame >= FRAME_attak2_1) && (self->s.frame <= FRAME_attak2_14) )
			return;

	self->pain_debounce_time = level.time + 3;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 10)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = supertank_move_pain1;
	}
	else if (damage <= 25)
	{
		gi.sound (self, CHAN_VOICE, sound_pain3, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = supertank_move_pain2;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM,0);
		self->monsterinfo.currentmove = supertank_move_pain3;
	}
};


void supertankRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;

	if (self->s.frame == FRAME_attak2_8)
		flash_number = MZ2_SUPERTANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak2_11)
		flash_number = MZ2_SUPERTANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak2_14)
		flash_number = MZ2_SUPERTANK_ROCKET_3;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, 50, 500, flash_number);
}	

void supertankMachineGun (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;

	flash_number = MZ2_SUPERTANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak1_1);

	//FIXME!!!
	dir[0] = 0;
	dir[1] = self->s.angles[1];
	dir[2] = 0;

	AngleVectors (dir, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		VectorMA (vec, 0, self->enemy->velocity, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, forward);
		VectorNormalize (forward);
  }

	monster_fire_bullet (self, start, forward, 6, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	


void supertank_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	//float	r;

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	//r = random();

	// Attack 1 == Chaingun
	// Attack 2 == Rocket Launcher

	if (range <= 160)
	{
		self->monsterinfo.currentmove = supertank_move_attack1;
	}
	else
	{	// fire rockets more often at distance
		if (random() < 0.3)
			self->monsterinfo.currentmove = supertank_move_attack1;
		else
			self->monsterinfo.currentmove = supertank_move_attack2;
	}
}


//
// death
//

void supertank_dead (edict_t *self)
{
	VectorSet (self->s.mins, -60, -60, 0);
	VectorSet (self->s.maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}
void BossExplode(edict_t* self);

void BossExplode (edict_t *self)
{
	vec3_t	org;
	int		n;

	self->think = SFP::BossExplode;
	VectorCopy (self->s.origin, org);
	org[2] += 24 + (rand()&15);
	switch (self->count++)
	{
	case 0:
		org[0] -= 24;
		org[1] -= 24;
		break;
	case 1:
		org[0] += 24;
		org[1] += 24;
		break;
	case 2:
		org[0] += 24;
		org[1] -= 24;
		break;
	case 3:
		org[0] -= 24;
		org[1] += 24;
		break;
	case 4:
		org[0] -= 48;
		org[1] -= 48;
		break;
	case 5:
		org[0] += 48;
		org[1] += 48;
		break;
	case 6:
		org[0] -= 48;
		org[1] += 48;
		break;
	case 7:
		org[0] += 48;
		org[1] -= 48;
		break;
	case 8:
		self->s.sound = 0;
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 500, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 500, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", 500, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (org);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + 0.1;
}


void supertank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->count = 0;
	self->monsterinfo.currentmove = supertank_move_death;
}

SFPEnt(pain, supertank_pain)
SFPEnt(die, supertank_die)
SFPEnt(monsterinfo.stand, supertank_stand)
SFPEnt(monsterinfo.walk, supertank_walk)
SFPEnt(monsterinfo.attack, supertank_attack)
SFPEnt(monsterinfo.get_currentmove, supertank_get_currentmove)
//
// monster_supertank
//

/*QUAKED monster_supertank (1 .5 0) (-64 -64 0) (64 64 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_supertank (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain1 = gi.soundindex ("bosstank/btkpain1.wav");
	sound_pain2 = gi.soundindex ("bosstank/btkpain2.wav");
	sound_pain3 = gi.soundindex ("bosstank/btkpain3.wav");
	sound_death = gi.soundindex ("bosstank/btkdeth1.wav");
	sound_search1 = gi.soundindex ("bosstank/btkunqv1.wav");
	sound_search2 = gi.soundindex ("bosstank/btkunqv2.wav");

//	self->s.sound = gi.soundindex ("bosstank/btkengn1.wav");
	tread_sound = gi.soundindex ("bosstank/btkengn1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss1/tris.md2");
	VectorSet (self->s.mins, -64, -64, 0);
	VectorSet (self->s.maxs, 64, 64, 112);

	self->health = 1500;
	self->gib_health = -500;
	self->mass = 800;

	self->pain = SFP::supertank_pain;
	self->die = SFP::supertank_die;
	self->monsterinfo.stand = SFP::supertank_stand;
	self->monsterinfo.walk = SFP::supertank_walk;
	self->monsterinfo.run = SFP::supertank_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = SFP::supertank_attack;
	self->monsterinfo.search = SFP::supertank_search;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = nullptr;
	self->monsterinfo.get_currentmove = SFP::supertank_get_currentmove;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = supertank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
