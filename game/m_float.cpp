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

floater

==============================================================================
*/

#include "g_local.h"
#include "m_float.h"


static int	sound_attack2;
static int	sound_attack3;
static int	sound_death1;
static int	sound_idle;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_sight;

enum {
	floater_move_stand1 = 1,
	floater_move_stand2,
	floater_move_activate,
	floater_move_attack1,
	floater_move_attack2,
	floater_move_attack3,
	floater_move_death,
	floater_move_pain1,
	floater_move_pain2,
	floater_move_pain3,
	floater_move_walk,
	floater_move_run
};

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
AutoSFP(ai_turn)

void floater_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void floater_idle (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}


//void floater_stand1 (edict_t *self);
void floater_dead (edict_t *self);
void floater_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void floater_run (edict_t *self);
void floater_wham (edict_t *self);
void floater_zap (edict_t *self);


void floater_fire_blaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;

	if ((self->s.frame == FRAME_attak104) || (self->s.frame == FRAME_attak107))
		effect = EF_HYPERBLASTER;
	else
		effect = 0;
	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_FLOAT_BLASTER_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, 1, 1000, MZ2_FLOAT_BLASTER_1, effect);
}

AutoSFP(floater_sight)
AutoSFP(floater_idle)
AutoSFP(floater_dead)
AutoSFP(floater_run)
AutoSFP(floater_wham)
AutoSFP(floater_zap)
AutoSFP(floater_fire_blaster)


mframe_t floater_frames_stand1 [] =
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
	SFP::ai_stand, 0, nullptr
};

mframe_t floater_frames_stand2 [] =
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
	SFP::ai_stand, 0, nullptr
};

void floater_stand (edict_t *self)
{
	if (random() <= 0.5)		
		self->monsterinfo.currentmove = floater_move_stand1;
	else
		self->monsterinfo.currentmove = floater_move_stand2;
}

mframe_t floater_frames_activate [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t floater_frames_attack1 [] =
{
	SFP::ai_charge,	0,	nullptr,			// Blaster attack
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,			// BOOM (0, -25.8, 32.5)	-- LOOP Starts
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	SFP::floater_fire_blaster,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr			//							-- LOOP Ends
};
mframe_t floater_frames_attack2 [] =
{
	SFP::ai_charge,	0,	nullptr,			// Claws
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::floater_wham,			// WHAM (0, -45, 29.6)		-- LOOP Starts
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,			//							-- LOOP Ends
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr
};

mframe_t floater_frames_attack3 [] =
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::floater_zap,		//								-- LOOP Starts
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,		//								-- LOOP Ends
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr
};

mframe_t floater_frames_death [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t floater_frames_pain1 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t floater_frames_pain2 [] =
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

mframe_t floater_frames_pain3 [] =
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

mframe_t floater_frames_walk [] =
{
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr,
	SFP::ai_walk, 5, nullptr
};

mframe_t floater_frames_run [] =
{
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr,
	SFP::ai_run, 13, nullptr
};

mmove_t floater_moves[] = {
	{FRAME_stand101, FRAME_stand152, floater_frames_stand1, nullptr},
	{FRAME_stand201, FRAME_stand252, floater_frames_stand2, nullptr},
	{FRAME_actvat01, FRAME_actvat31, floater_frames_activate, nullptr},
	{FRAME_attak101, FRAME_attak114, floater_frames_attack1, SFP::floater_run},
	{FRAME_attak201, FRAME_attak225, floater_frames_attack2, SFP::floater_run},
	{FRAME_attak301, FRAME_attak334, floater_frames_attack3, SFP::floater_run},
	{FRAME_death01, FRAME_death13, floater_frames_death, SFP::floater_dead},
	{FRAME_pain101, FRAME_pain107, floater_frames_pain1, SFP::floater_run},
	{FRAME_pain201, FRAME_pain208, floater_frames_pain2, SFP::floater_run},
	{FRAME_pain301, FRAME_pain312, floater_frames_pain3, SFP::floater_run},
	{FRAME_stand101, FRAME_stand152, floater_frames_walk, nullptr},
	{FRAME_stand101, FRAME_stand152, floater_frames_run, nullptr}
};

mmove_t * floater_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &floater_moves[self->monsterinfo.currentmove-1];
}
AutoSFP(floater_get_currentmove)

void floater_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = floater_move_stand1;
	else
		self->monsterinfo.currentmove = floater_move_run;
}

void floater_walk (edict_t *self)
{
	self->monsterinfo.currentmove = floater_move_walk;
}

void floater_wham (edict_t *self)
{
	static	vec3_t	aim = {MELEE_DISTANCE, 0, 0};
	gi.sound (self, CHAN_WEAPON, sound_attack3, 1, ATTN_NORM, 0);
	fire_hit (self, aim, 5 + rand() % 6, -50);
}

void floater_zap (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	origin;
	vec3_t	dir;
	vec3_t	offset;

	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);

	AngleVectors (self->s.angles, forward, right, nullptr);
	//FIXME use a flash and replace these two lines with the commented one
	VectorSet (offset, 18.5, -0.9, 10);
	G_ProjectSource (self->s.origin, offset, forward, right, origin);
//	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, origin);

	gi.sound (self, CHAN_WEAPON, sound_attack2, 1, ATTN_NORM, 0);

	//FIXME use the flash, Luke
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (32);
	gi.WritePosition (origin);
	gi.WriteDir (dir);
	gi.WriteByte (1);	//sparks
	gi.multicast (origin, MULTICAST_PVS);

	T_Damage (self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, 5 + rand() % 6, -10, DAMAGE_ENERGY, MOD_UNKNOWN);
}

void floater_attack(edict_t *self)
{
	self->monsterinfo.currentmove = floater_move_attack1;
}


void floater_melee(edict_t *self)
{
	if (random() < 0.5)		
		self->monsterinfo.currentmove = floater_move_attack3;
	else
		self->monsterinfo.currentmove = floater_move_attack2;
}


void floater_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		n;

	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	n = (rand() + 1) % 3;
	if (n == 0)
	{
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = floater_move_pain1;
	}
	else
	{
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
		self->monsterinfo.currentmove = floater_move_pain2;
	}
}

void floater_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void floater_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound (self, CHAN_VOICE, sound_death1, 1, ATTN_NORM, 0);
	BecomeExplosion1(self);
}

SFPEnt(pain, floater_pain)
SFPEnt(die, floater_die)
AutoSFP(floater_stand)
AutoSFP(floater_walk)
AutoSFP(floater_attack)
AutoSFP(floater_melee)

/*QUAKED monster_floater (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_floater (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_attack2 = gi.soundindex ("floater/fltatck2.wav");
	sound_attack3 = gi.soundindex ("floater/fltatck3.wav");
	sound_death1 = gi.soundindex ("floater/fltdeth1.wav");
	sound_idle = gi.soundindex ("floater/fltidle1.wav");
	sound_pain1 = gi.soundindex ("floater/fltpain1.wav");
	sound_pain2 = gi.soundindex ("floater/fltpain2.wav");
	sound_sight = gi.soundindex ("floater/fltsght1.wav");

	gi.soundindex ("floater/fltatck1.wav");

	self->s.sound = gi.soundindex ("floater/fltsrch1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/float/tris.md2");
	VectorSet (self->s.mins, -24, -24, -24);
	VectorSet (self->s.maxs, 24, 24, 32);

	self->health = 200;
	self->gib_health = -80;
	self->mass = 300;

	self->pain = SFP::floater_pain;
	self->die = SFP::floater_die;

	self->monsterinfo.stand = SFP::floater_stand;
	self->monsterinfo.walk = SFP::floater_walk;
	self->monsterinfo.run = SFP::floater_run;
//	self->monsterinfo.dodge = floater_dodge;
	self->monsterinfo.attack = SFP::floater_attack;
	self->monsterinfo.melee = SFP::floater_melee;
	self->monsterinfo.sight = SFP::floater_sight;
	self->monsterinfo.idle = SFP::floater_idle;
	self->monsterinfo.get_currentmove = SFP::floater_get_currentmove;

	gi.linkentity (self);

	if (random() <= 0.5)		
		self->monsterinfo.currentmove = floater_move_stand1;	
	else
		self->monsterinfo.currentmove = floater_move_stand2;	
	
	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start (self);
}
