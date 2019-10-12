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

TANK

==============================================================================
*/

#include "g_local.h"
#include "m_tank.h"


void tank_refire_rocket (edict_t *self);
void tank_doattack_rocket (edict_t *self);
void tank_reattack_blaster (edict_t *self);

static int	sound_thud;
static int	sound_pain;
static int	sound_idle;
static int	sound_die;
static int	sound_step;
static int	sound_sight;
static int	sound_windup;
static int	sound_strike;

enum {
	tank_move_stand = 1,
	tank_move_start_walk,
	tank_move_walk,
	tank_move_stop_walk,
	tank_move_start_run,
	tank_move_run,
	tank_move_stop_run,
	tank_move_pain1,
	tank_move_pain2,
	tank_move_pain3,
	tank_move_attack_blast,
	tank_move_reattack_blast,
	tank_move_attack_post_blast,
	tank_move_attack_strike,
	tank_move_attack_pre_rocket,
	tank_move_attack_fire_rocket,
	tank_move_attack_post_rocket,
	tank_move_attack_chain,
	tank_move_death,
};
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
//
// misc
//

void tank_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


void tank_footstep (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step, 1, ATTN_NORM, 0);
}

void tank_thud (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_thud, 1, ATTN_NORM, 0);
}

void tank_windup (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_windup, 1, ATTN_NORM, 0);
}

void tank_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}
AutoSFP(tank_sight)
AutoSFP(tank_footstep)
AutoSFP(tank_thud)
AutoSFP(tank_windup)
AutoSFP(tank_idle)

//
// stand
//

mframe_t tank_frames_stand []=
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
	
void tank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = tank_move_stand;
}


//
// walk
//

void tank_walk (edict_t *self);
AutoSFP(tank_stand)
AutoSFP(tank_walk)
mframe_t tank_frames_start_walk [] =
{
	SFP::ai_walk,  0, nullptr,
	SFP::ai_walk,  6, nullptr,
	SFP::ai_walk,  6, nullptr,
	SFP::ai_walk, 11, SFP::tank_footstep
};

mframe_t tank_frames_walk [] =
{
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 5,	nullptr,
	SFP::ai_walk, 3,	nullptr,
	SFP::ai_walk, 2,	nullptr,
	SFP::ai_walk, 5,	nullptr,
	SFP::ai_walk, 5,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 4,	SFP::tank_footstep,
	SFP::ai_walk, 3,	nullptr,
	SFP::ai_walk, 5,	nullptr,
	SFP::ai_walk, 4,	nullptr,
	SFP::ai_walk, 5,	nullptr,
	SFP::ai_walk, 7,	nullptr,
	SFP::ai_walk, 7,	nullptr,
	SFP::ai_walk, 6,	nullptr,
	SFP::ai_walk, 6,	SFP::tank_footstep
};

mframe_t tank_frames_stop_walk [] =
{
	SFP::ai_walk,  3, nullptr,
	SFP::ai_walk,  3, nullptr,
	SFP::ai_walk,  2, nullptr,
	SFP::ai_walk,  2, nullptr,
	SFP::ai_walk,  4, SFP::tank_footstep
};

void tank_walk (edict_t *self)
{
		self->monsterinfo.currentmove = tank_move_walk;
}


//
// run
//

void tank_run (edict_t *self);

mframe_t tank_frames_start_run [] =
{
	SFP::ai_run,  0, nullptr,
	SFP::ai_run,  6, nullptr,
	SFP::ai_run,  6, nullptr,
	SFP::ai_run, 11, SFP::tank_footstep
};

mframe_t tank_frames_run [] =
{
	SFP::ai_run, 4,	nullptr,
	SFP::ai_run, 5,	nullptr,
	SFP::ai_run, 3,	nullptr,
	SFP::ai_run, 2,	nullptr,
	SFP::ai_run, 5,	nullptr,
	SFP::ai_run, 5,	nullptr,
	SFP::ai_run, 4,	nullptr,
	SFP::ai_run, 4,	SFP::tank_footstep,
	SFP::ai_run, 3,	nullptr,
	SFP::ai_run, 5,	nullptr,
	SFP::ai_run, 4,	nullptr,
	SFP::ai_run, 5,	nullptr,
	SFP::ai_run, 7,	nullptr,
	SFP::ai_run, 7,	nullptr,
	SFP::ai_run, 6,	nullptr,
	SFP::ai_run, 6,	SFP::tank_footstep
};

mframe_t tank_frames_stop_run [] =
{
	SFP::ai_run,  3, nullptr,
	SFP::ai_run,  3, nullptr,
	SFP::ai_run,  2, nullptr,
	SFP::ai_run,  2, nullptr,
	SFP::ai_run,  4, SFP::tank_footstep
};

void tank_run (edict_t *self)
{
	if (self->enemy && self->enemy->client)
		self->monsterinfo.aiflags |= AI_BRUTAL;
	else
		self->monsterinfo.aiflags &= ~AI_BRUTAL;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.currentmove = tank_move_stand;
		return;
	}

	if (self->monsterinfo.currentmove == tank_move_walk ||
		self->monsterinfo.currentmove == tank_move_start_run)
	{
		self->monsterinfo.currentmove = tank_move_run;
	}
	else
	{
		self->monsterinfo.currentmove = tank_move_start_run;
	}
}

//
// pain
//

mframe_t tank_frames_pain1 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t tank_frames_pain2 [] =
{
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr,
	SFP::ai_move, 0, nullptr
};

mframe_t tank_frames_pain3 [] =
{
	SFP::ai_move, -7, nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 2,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 3,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 2,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  nullptr,
	SFP::ai_move, 0,  SFP::tank_footstep
};


void tank_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
			self->s.skinnum |= 1;

	if (damage <= 10)
		return;

	if (level.time < self->pain_debounce_time)
			return;

	if (damage <= 30)
		if (random() > 0.2)
			return;
	
	// If hard or nightmare, don't go into pain while attacking
	if ( skill->value >= 2)
	{
		if ( (self->s.frame >= FRAME_attak301) && (self->s.frame <= FRAME_attak330) )
			return;
		if ( (self->s.frame >= FRAME_attak101) && (self->s.frame <= FRAME_attak116) )
			return;
	}

	self->pain_debounce_time = level.time + 3;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (damage <= 30)
		self->monsterinfo.currentmove = tank_move_pain1;
	else if (damage <= 60)
		self->monsterinfo.currentmove = tank_move_pain2;
	else
		self->monsterinfo.currentmove = tank_move_pain3;
};


//
// attacks
//

void TankBlaster (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	end;
	vec3_t	dir;
	int		flash_number;

	if (self->s.frame == FRAME_attak110)
		flash_number = MZ2_TANK_BLASTER_1;
	else if (self->s.frame == FRAME_attak113)
		flash_number = MZ2_TANK_BLASTER_2;
	else // (self->s.frame == FRAME_attak116)
		flash_number = MZ2_TANK_BLASTER_3;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, 30, 800, flash_number, EF_BLASTER);
}	

void TankStrike (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_strike, 1, ATTN_NORM, 0);
}	

void TankRocket (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;
	int		flash_number;

	if (self->s.frame == FRAME_attak324)
		flash_number = MZ2_TANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak327)
		flash_number = MZ2_TANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak330)
		flash_number = MZ2_TANK_ROCKET_3;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);

	monster_fire_rocket (self, start, dir, 50, 550, flash_number);
}	

void TankMachineGun (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;

	flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak406);

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorCopy (self->enemy->s.origin, vec);
		vec[2] += self->enemy->viewheight;
		VectorSubtract (vec, start, vec);
		vectoangles (vec, vec);
		dir[0] = vec[0];
	}
	else
	{
		dir[0] = 0;
	}
	if (self->s.frame <= FRAME_attak415)
		dir[1] = self->s.angles[1] - 8 * (self->s.frame - FRAME_attak411);
	else
		dir[1] = self->s.angles[1] + 8 * (self->s.frame - FRAME_attak419);
	dir[2] = 0;

	AngleVectors (dir, forward, nullptr, nullptr);

	monster_fire_bullet (self, start, forward, 20, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	

AutoSFP(TankBlaster)
mframe_t tank_frames_attack_blast [] =
{
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, -1,	nullptr,
	SFP::ai_charge, -2,	nullptr,
	SFP::ai_charge, -1,	nullptr,
	SFP::ai_charge, -1,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::TankBlaster,		// 10
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::TankBlaster,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::TankBlaster			// 16
};

mframe_t tank_frames_reattack_blast [] =
{
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::TankBlaster,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::TankBlaster			// 16
};

mframe_t tank_frames_attack_post_blast [] =	
{
	SFP::ai_move, 0,		nullptr,				// 17
	SFP::ai_move, 0,		nullptr,
	SFP::ai_move, 2,		nullptr,
	SFP::ai_move, 3,		nullptr,
	SFP::ai_move, 2,		nullptr,
	SFP::ai_move, -2,	SFP::tank_footstep		// 22
};

void tank_reattack_blaster (edict_t *self)
{
	if (skill->value >= 2)
		if (visible (self, self->enemy))
			if (self->enemy->health > 0)
				if (random() <= 0.6)
				{
					self->monsterinfo.currentmove = tank_move_reattack_blast;
					return;
				}
	self->monsterinfo.currentmove = tank_move_attack_post_blast;
}


void tank_poststrike (edict_t *self)
{
	self->enemy = nullptr;
	tank_run (self);
}
AutoSFP(TankStrike)
mframe_t tank_frames_attack_strike [] =
{
	SFP::ai_move, 3,   nullptr,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 1,   nullptr,
	SFP::ai_move, 6,   nullptr,
	SFP::ai_move, 7,   nullptr,
	SFP::ai_move, 9,   SFP::tank_footstep,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 1,   nullptr,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 2,   SFP::tank_footstep,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, 0,   SFP::tank_windup,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   SFP::TankStrike,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -1,  nullptr,
	SFP::ai_move, -3,  nullptr,
	SFP::ai_move, -10, nullptr,
	SFP::ai_move, -10, nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, -3,  nullptr,
	SFP::ai_move, -2,  SFP::tank_footstep
};

mframe_t tank_frames_attack_pre_rocket [] =
{
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,			// 10

	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 1,  nullptr,
	SFP::ai_charge, 2,  nullptr,
	SFP::ai_charge, 7,  nullptr,
	SFP::ai_charge, 7,  nullptr,
	SFP::ai_charge, 7,  SFP::tank_footstep,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,			// 20

	SFP::ai_charge, -3, nullptr
};
AutoSFP(TankRocket)
mframe_t tank_frames_attack_fire_rocket [] =
{
	SFP::ai_charge, -3, nullptr,			// Loop Start	22 
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  SFP::TankRocket,		// 24
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  SFP::TankRocket,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, -1, SFP::TankRocket		// 30	Loop End
};

mframe_t tank_frames_attack_post_rocket [] =
{	
	SFP::ai_charge, 0,  nullptr,			// 31
	SFP::ai_charge, -1, nullptr,
	SFP::ai_charge, -1, nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 2,  nullptr,
	SFP::ai_charge, 3,  nullptr,
	SFP::ai_charge, 4,  nullptr,
	SFP::ai_charge, 2,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,			// 40

	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, -9, nullptr,
	SFP::ai_charge, -8, nullptr,
	SFP::ai_charge, -7, nullptr,
	SFP::ai_charge, -1, nullptr,
	SFP::ai_charge, -1, SFP::tank_footstep,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,			// 50

	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr,
	SFP::ai_charge, 0,  nullptr
};
AutoSFP(TankMachineGun)
mframe_t tank_frames_attack_chain [] =
{
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	nullptr,      0, SFP::TankMachineGun,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr,
	SFP::ai_charge, 0, nullptr
};

void tank_refire_rocket (edict_t *self)
{
	// Only on hard or nightmare
	if ( skill->value >= 2 )
		if (self->enemy->health > 0)
			if (visible(self, self->enemy) )
				if (random() <= 0.4)
				{
					self->monsterinfo.currentmove = tank_move_attack_fire_rocket;
					return;
				}
	self->monsterinfo.currentmove = tank_move_attack_post_rocket;
}

void tank_doattack_rocket (edict_t *self)
{
	self->monsterinfo.currentmove = tank_move_attack_fire_rocket;
}

void tank_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	float	r;

	if (self->enemy->health < 0)
	{
		self->monsterinfo.currentmove = tank_move_attack_strike;
		self->monsterinfo.aiflags &= ~AI_BRUTAL;
		return;
	}

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);

	r = random();

	if (range <= 125)
	{
		if (r < 0.4)
			self->monsterinfo.currentmove = tank_move_attack_chain;
		else 
			self->monsterinfo.currentmove = tank_move_attack_blast;
	}
	else if (range <= 250)
	{
		if (r < 0.5)
			self->monsterinfo.currentmove = tank_move_attack_chain;
		else
			self->monsterinfo.currentmove = tank_move_attack_blast;
	}
	else
	{
		if (r < 0.33)
			self->monsterinfo.currentmove = tank_move_attack_chain;
		else if (r < 0.66)
		{
			self->monsterinfo.currentmove = tank_move_attack_pre_rocket;
			self->pain_debounce_time = level.time + 5.0;	// no pain for a while
		}
		else
			self->monsterinfo.currentmove = tank_move_attack_blast;
	}
}


//
// death
//

void tank_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -16);
	VectorSet (self->s.maxs, 16, 16, -0);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t tank_frames_death1 [] =
{
	SFP::ai_move, -7,  nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, 1,   nullptr,
	SFP::ai_move, 3,   nullptr,
	SFP::ai_move, 6,   nullptr,
	SFP::ai_move, 1,   nullptr,
	SFP::ai_move, 1,   nullptr,
	SFP::ai_move, 2,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, -2,  nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, -3,  nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, -4,  nullptr,
	SFP::ai_move, -6,  nullptr,
	SFP::ai_move, -4,  nullptr,
	SFP::ai_move, -5,  nullptr,
	SFP::ai_move, -7,  nullptr,
	SFP::ai_move, -15, SFP::tank_thud,
	SFP::ai_move, -5,  nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr,
	SFP::ai_move, 0,   nullptr
};
AutoSFP(tank_reattack_blaster)
AutoSFP(tank_poststrike)
AutoSFP(tank_doattack_rocket)
AutoSFP(tank_refire_rocket)
AutoSFP(tank_dead)
AutoSFP(tank_run)
mmove_t	tank_moves[] = {
	{FRAME_stand01, FRAME_stand30, tank_frames_stand, nullptr},
	{FRAME_walk01, FRAME_walk04, tank_frames_start_walk, SFP::tank_walk},
	{FRAME_walk05, FRAME_walk20, tank_frames_walk, nullptr},
	{FRAME_walk21, FRAME_walk25, tank_frames_stop_walk, SFP::tank_stand},
	{FRAME_walk01, FRAME_walk04, tank_frames_start_run, SFP::tank_run},
	{FRAME_walk05, FRAME_walk20, tank_frames_run, nullptr},
	{FRAME_walk21, FRAME_walk25, tank_frames_stop_run, SFP::tank_walk},
	{FRAME_pain101, FRAME_pain104, tank_frames_pain1, SFP::tank_run},
	{FRAME_pain201, FRAME_pain205, tank_frames_pain2, SFP::tank_run},
	{FRAME_pain301, FRAME_pain316, tank_frames_pain3, SFP::tank_run},
	{FRAME_attak101, FRAME_attak116, tank_frames_attack_blast, SFP::tank_reattack_blaster},
	{FRAME_attak111, FRAME_attak116, tank_frames_reattack_blast, SFP::tank_reattack_blaster},
	{FRAME_attak117, FRAME_attak122, tank_frames_attack_post_blast, SFP::tank_run},
	{FRAME_attak201, FRAME_attak238, tank_frames_attack_strike, SFP::tank_poststrike},
	{FRAME_attak301, FRAME_attak321, tank_frames_attack_pre_rocket, SFP::tank_doattack_rocket},
	{FRAME_attak322, FRAME_attak330, tank_frames_attack_fire_rocket, SFP::tank_refire_rocket},
	{FRAME_attak331, FRAME_attak353, tank_frames_attack_post_rocket, SFP::tank_run},
	{FRAME_attak401, FRAME_attak429, tank_frames_attack_chain, SFP::tank_run},
	{FRAME_death101, FRAME_death132, tank_frames_death1, SFP::tank_dead}
};

mmove_t * tank_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &tank_moves[self->monsterinfo.currentmove-1];
}

void tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1 /*4*/; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = tank_move_death;
	
}

SFPEnt(pain, tank_pain);
SFPEnt(die, tank_die);
SFPEnt(monsterinfo.attack, tank_attack);
SFPEnt(monsterinfo.get_currentmove, tank_get_currentmove);
//
// monster_tank
//

/*QUAKED monster_tank (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
/*QUAKED monster_tank_commander (1 .5 0) (-32 -32 -16) (32 32 72) Ambush Trigger_Spawn Sight
*/
void SP_monster_tank (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	VectorSet (self->s.mins, -32, -32, -16);
	VectorSet (self->s.maxs, 32, 32, 72);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	sound_pain = gi.soundindex ("tank/tnkpain2.wav");
	sound_thud = gi.soundindex ("tank/tnkdeth2.wav");
	sound_idle = gi.soundindex ("tank/tnkidle1.wav");
	sound_die = gi.soundindex ("tank/death.wav");
	sound_step = gi.soundindex ("tank/step.wav");
	sound_windup = gi.soundindex ("tank/tnkatck4.wav");
	sound_strike = gi.soundindex ("tank/tnkatck5.wav");
	sound_sight = gi.soundindex ("tank/sight1.wav");

	gi.soundindex ("tank/tnkatck1.wav");
	gi.soundindex ("tank/tnkatk2a.wav");
	gi.soundindex ("tank/tnkatk2b.wav");
	gi.soundindex ("tank/tnkatk2c.wav");
	gi.soundindex ("tank/tnkatk2d.wav");
	gi.soundindex ("tank/tnkatk2e.wav");
	gi.soundindex ("tank/tnkatck3.wav");

	if (strcmp(self->classname, "monster_tank_commander") == 0)
	{
		self->health = 1000;
		self->gib_health = -225;
	}
	else
	{
		self->health = 750;
		self->gib_health = -200;
	}

	self->mass = 500;

	self->pain = SFP::tank_pain;
	self->die = SFP::tank_die;
	self->monsterinfo.stand = SFP::tank_stand;
	self->monsterinfo.walk = SFP::tank_walk;
	self->monsterinfo.run = SFP::tank_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = SFP::tank_attack;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = SFP::tank_sight;
	self->monsterinfo.idle = SFP::tank_idle;
	self->monsterinfo.get_currentmove = SFP::tank_get_currentmove;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = tank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);

	if (strcmp(self->classname, "monster_tank_commander") == 0)
		self->s.skinnum = 2;
}
