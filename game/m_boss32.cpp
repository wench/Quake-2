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

Makron -- Final Boss

==============================================================================
*/

#include "g_local.h"
#include "m_boss32.h"

qboolean visible (edict_t *self, edict_t *other);

void MakronRailgun (edict_t *self);
void MakronSaveloc (edict_t *self);
void MakronHyperblaster (edict_t *self);
void makron_step_left (edict_t *self);
void makron_step_right (edict_t *self);
void makronBFG (edict_t *self);
void makron_dead (edict_t *self);

AutoSFP( MakronRailgun)
AutoSFP( MakronSaveloc)
AutoSFP( MakronHyperblaster)
AutoSFP( makron_step_left)
AutoSFP( makron_step_right)
AutoSFP( makronBFG)

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
enum {
	makron_move_stand = 1,
	makron_move_run,
	makron_move_walk,
	makron_move_pain6,
	makron_move_pain5,
	makron_move_pain4,
	makron_move_death2,
	makron_move_death3,
	makron_move_sight,
	makron_move_attack3,
	makron_move_attack4,
	makron_move_attack5
};

static int	sound_pain4;
static int	sound_pain5;
static int	sound_pain6;
static int	sound_death;
static int	sound_step_left;
static int	sound_step_right;
static int	sound_attack_bfg;
static int	sound_brainsplorch;
static int	sound_prerailgun;
static int	sound_popup;
static int	sound_taunt1;
static int	sound_taunt2;
static int	sound_taunt3;
static int	sound_hit;

void makron_taunt (edict_t *self)
{
	float r;

	r=random();
	if (r <= 0.3)
		gi.sound (self, CHAN_AUTO, sound_taunt1, 1, ATTN_NONE, 0);
	else if (r <= 0.6)
		gi.sound (self, CHAN_AUTO, sound_taunt2, 1, ATTN_NONE, 0);
	else
		gi.sound (self, CHAN_AUTO, sound_taunt3, 1, ATTN_NONE, 0);
}

//
// stand
//

mframe_t makron_frames_stand []=
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
	SFP::ai_stand, 0, nullptr,		// 10
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 20
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 30
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 40
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,		// 50
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr		// 60
};
	
void makron_stand (edict_t *self)
{
	self->monsterinfo.currentmove = makron_move_stand;
}

mframe_t makron_frames_run [] =
{
	SFP::ai_run, 3,	SFP::makron_step_left,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 8,	nullptr,
	SFP::ai_run, 8,	nullptr,
	SFP::ai_run, 8,	SFP::makron_step_right,
	SFP::ai_run, 6,	nullptr,
	SFP::ai_run, 12,	nullptr,
	SFP::ai_run, 9,	nullptr,
	SFP::ai_run, 6,	nullptr,
	SFP::ai_run, 12,	nullptr
};

void makron_hit (edict_t *self)
{
	gi.sound (self, CHAN_AUTO, sound_hit, 1, ATTN_NONE,0);
}

void makron_popup (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_popup, 1, ATTN_NONE,0);
}

void makron_step_left (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_left, 1, ATTN_NORM,0);
}

void makron_step_right (edict_t *self)
{
	gi.sound (self, CHAN_BODY, sound_step_right, 1, ATTN_NORM,0);
}

void makron_brainsplorch (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_brainsplorch, 1, ATTN_NORM,0);
}

void makron_prerailgun (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_prerailgun, 1, ATTN_NORM,0);
}
AutoSFP(makron_brainsplorch)
AutoSFP(makron_prerailgun)
AutoSFP(makron_popup)
AutoSFP(makron_hit)
AutoSFP(makron_taunt)

mframe_t makron_frames_walk [] =
{
	SFP::ai_walk, 3,	SFP::makron_step_left,
	SFP::ai_walk, 12,	nullptr,
	SFP::ai_walk, 8,	nullptr,
	SFP::ai_walk, 8,	nullptr,
	SFP::ai_walk, 8,	SFP::makron_step_right,
	SFP::ai_walk, 6,	nullptr,
	SFP::ai_walk, 12,	nullptr,
	SFP::ai_walk, 9,	nullptr,
	SFP::ai_walk, 6,	nullptr,
	SFP::ai_walk, 12,	nullptr
};

void makron_walk (edict_t *self)
{
		self->monsterinfo.currentmove = makron_move_walk;
}

void makron_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = makron_move_stand;
	else
		self->monsterinfo.currentmove = makron_move_run;
}

mframe_t makron_frames_pain6 [] =
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
	SFP::ai_move,	0,	nullptr,		// 10
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::makron_popup,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,		// 20
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::makron_taunt,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_pain5 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_pain4 [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_death2 [] =
{
	SFP::ai_move,	-15,	nullptr,
	SFP::ai_move,	3,	nullptr,
	SFP::ai_move,	-12,	nullptr,
	SFP::ai_move,	0,	SFP::makron_step_left,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 10
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	11,	nullptr,
	SFP::ai_move,	12,	nullptr,
	SFP::ai_move,	11,	SFP::makron_step_right,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 20
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 30
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	5,	nullptr,
	SFP::ai_move,	7,	nullptr,
	SFP::ai_move,	6,	SFP::makron_step_left,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-1,	nullptr,
	SFP::ai_move,	2,	nullptr,			// 40
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,			// 50
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-6,	nullptr,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	-6,	SFP::makron_step_right,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	-4,	SFP::makron_step_left,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 60
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	-5,	nullptr,
	SFP::ai_move,	-3,	SFP::makron_step_right,
	SFP::ai_move,	-8,	nullptr,
	SFP::ai_move,	-3,	SFP::makron_step_left,
	SFP::ai_move,	-7,	nullptr,
	SFP::ai_move,	-4,	nullptr,
	SFP::ai_move,	-4,	SFP::makron_step_right,			// 70
	SFP::ai_move,	-6,	nullptr,			
	SFP::ai_move,	-7,	nullptr,
	SFP::ai_move,	0,	SFP::makron_step_left,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 80
	SFP::ai_move,	0,	nullptr,			
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	-2,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	2,	nullptr,
	SFP::ai_move,	0,	nullptr,			// 90
	SFP::ai_move,	27,	SFP::makron_hit,
	SFP::ai_move,	26,	nullptr,
	SFP::ai_move,	0,	SFP::makron_brainsplorch,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr			// 95
};

mframe_t makron_frames_death3 [] =
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
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_sight [] =
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

void makronBFG (edict_t *self)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;
	vec3_t	vec;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_BFG], forward, right, start);

	VectorCopy (self->enemy->s.origin, vec);
	vec[2] += self->enemy->viewheight;
	VectorSubtract (vec, start, dir);
	VectorNormalize (dir);
	gi.sound (self, CHAN_VOICE, sound_attack_bfg, 1, ATTN_NORM, 0);
	monster_fire_bfg (self, start, dir, 50, 300, 100, 300, MZ2_MAKRON_BFG);
}	


mframe_t makron_frames_attack3 []=
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::makronBFG,		// FIXME: BFG Attack here
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_attack4[]=
{
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	SFP::MakronHyperblaster,		// fire
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};

mframe_t makron_frames_attack5[]=
{
	SFP::ai_charge,	0,	SFP::makron_prerailgun,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	nullptr,
	SFP::ai_charge,	0,	SFP::MakronSaveloc,
	SFP::ai_move,	0,	SFP::MakronRailgun,		// Fire railgun
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};
AutoSFP(makron_run)
AutoSFP(makron_dead)

mmove_t makron_moves[] = {
	{FRAME_stand201, FRAME_stand260, makron_frames_stand, nullptr},
	{FRAME_walk204, FRAME_walk213, makron_frames_run, nullptr},
	{FRAME_walk204, FRAME_walk213, makron_frames_run, nullptr},
	{FRAME_pain601, FRAME_pain627, makron_frames_pain6, SFP::makron_run},
	{FRAME_pain501, FRAME_pain504, makron_frames_pain5, SFP::makron_run},
	{FRAME_pain401, FRAME_pain404, makron_frames_pain4, SFP::makron_run},
	{FRAME_death201, FRAME_death295, makron_frames_death2, SFP::makron_dead},
	{FRAME_death301, FRAME_death320, makron_frames_death3, nullptr},
	{FRAME_active01, FRAME_active13, makron_frames_sight, SFP::makron_run},
	{FRAME_attak301, FRAME_attak308, makron_frames_attack3, SFP::makron_run},
	{FRAME_attak401, FRAME_attak426, makron_frames_attack4, SFP::makron_run},
	{FRAME_attak501, FRAME_attak516, makron_frames_attack5, SFP::makron_run},
};

mmove_t *makron_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return 0;
	return &makron_moves[self->monsterinfo.currentmove-1];
}

void MakronSaveloc (edict_t *self)
{
	VectorCopy (self->enemy->s.origin, self->pos1);	//save for aiming the shot
	self->pos1[2] += self->enemy->viewheight;
};

// FIXME: He's not firing from the proper Z
void MakronRailgun (edict_t *self)
{
	vec3_t	start;
	vec3_t	dir;
	vec3_t	forward, right;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MAKRON_RAILGUN_1], forward, right, start);
	
	// calc direction to where we targted
	VectorSubtract (self->pos1, start, dir);
	VectorNormalize (dir);

	monster_fire_railgun (self, start, dir, 50, 100, MZ2_MAKRON_RAILGUN_1);
}

// FIXME: This is all wrong. He's not firing at the proper angles.
void MakronHyperblaster (edict_t *self)
{
	vec3_t	dir;
	vec3_t	vec;
	vec3_t	start;
	vec3_t	forward, right;
	int		flash_number;

	flash_number = MZ2_MAKRON_BLASTER_1 + (self->s.frame - FRAME_attak405);

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
	if (self->s.frame <= FRAME_attak413)
		dir[1] = self->s.angles[1] - 10 * (self->s.frame - FRAME_attak413);
	else
		dir[1] = self->s.angles[1] + 10 * (self->s.frame - FRAME_attak421);
	dir[2] = 0;

	AngleVectors (dir, forward, nullptr, nullptr);

	monster_fire_blaster (self, start, forward, 15, 1000, MZ2_MAKRON_BLASTER_1, EF_BLASTER);
}	


void makron_pain (edict_t *self, edict_t *other, float kick, int damage)
{

	if (self->health < (self->max_health / 2))
			self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
			return;

	// Lessen the chance of him going into his pain frames
	if (damage <=25)
		if (random()<0.2)
			return;

	self->pain_debounce_time = level.time + 3;
	if (skill->value == 3)
		return;		// no pain anims in nightmare


	if (damage <= 40)
	{
		gi.sound (self, CHAN_VOICE, sound_pain4, 1, ATTN_NONE,0);
		self->monsterinfo.currentmove = makron_move_pain4;
	}
	else if (damage <= 110)
	{
		gi.sound (self, CHAN_VOICE, sound_pain5, 1, ATTN_NONE,0);
		self->monsterinfo.currentmove = makron_move_pain5;
	}
	else
	{
		if (damage <= 150)
			if (random() <= 0.45)
			{
				gi.sound (self, CHAN_VOICE, sound_pain6, 1, ATTN_NONE,0);
				self->monsterinfo.currentmove = makron_move_pain6;
			}
		else
			if (random() <= 0.35)
			{
				gi.sound (self, CHAN_VOICE, sound_pain6, 1, ATTN_NONE,0);
				self->monsterinfo.currentmove = makron_move_pain6;
			}
	}
};

void makron_sight(edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = makron_move_sight;
};

void makron_attack(edict_t *self)
{
	vec3_t	vec;
	float	range;
	float	r;

	r = random();

	VectorSubtract (self->enemy->s.origin, self->s.origin, vec);
	range = VectorLength (vec);


	if (r <= 0.3)
		self->monsterinfo.currentmove = makron_move_attack3;
	else if (r <= 0.6)
		self->monsterinfo.currentmove = makron_move_attack4;
	else
		self->monsterinfo.currentmove = makron_move_attack5;
}

/*
---
Makron Torso. This needs to be spawned in
---
*/

void makron_torso_think (edict_t *self)
{
	if (++self->s.frame < 365)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 346;
		self->nextthink = level.time + FRAMETIME;
	}
}
AutoSFP(makron_torso_think);
void makron_torso (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->s.mins, -8, -8, 0);
	VectorSet (ent->s.maxs, 8, 8, 8);
	ent->s.frame = 346;
	ent->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	ent->think = SFP::makron_torso_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	ent->s.sound = gi.soundindex ("makron/spine.wav");
	gi.linkentity (ent);
}


//
// death
//

void makron_dead (edict_t *self)
{
	VectorSet (self->s.mins, -60, -60, 0);
	VectorSet (self->s.maxs, 60, 60, 72);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void makron_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	edict_t *tempent;

	int		n;

	self->s.sound = 0;
	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 1 /*4*/; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", damage, GIB_METALLIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", damage, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NONE, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	tempent = G_Spawn();
	VectorCopy (self->s.origin, tempent->s.origin);
	VectorCopy (self->s.angles, tempent->s.angles);
	tempent->s.origin[1] -= 84;
	makron_torso (tempent);

	self->monsterinfo.currentmove = makron_move_death2;
	
}

qboolean Makron_CheckAttack (edict_t *self)
{
	vec3_t	spot1, spot2;
	vec3_t	temp;
	float	chance;
	trace_t	tr;
	qboolean	enemy_infront;
	int			enemy_range;
	float		enemy_yaw;

	if (self->enemy->health > 0)
	{
	// see if any entities are in the way of the shot
		VectorCopy (self->s.origin, spot1);
		spot1[2] += self->viewheight;
		VectorCopy (self->enemy->s.origin, spot2);
		spot2[2] += self->enemy->viewheight;

		tr = gi.trace (spot1, nullptr, nullptr, spot2, self, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_SLIME|CONTENTS_LAVA);

		// do we have a clear shot?
		if (tr.ent != self->enemy)
			return false;
	}
	
	enemy_infront = infront(self, self->enemy);
	enemy_range = range(self, self->enemy);
	VectorSubtract (self->enemy->s.origin, self->s.origin, temp);
	enemy_yaw = vectoyaw(temp);

	self->ideal_yaw = enemy_yaw;


	// melee attack
	if (enemy_range == RANGE_MELEE)
	{
		if (self->monsterinfo.melee)
			self->monsterinfo.attack_state = AS_MELEE;
		else
			self->monsterinfo.attack_state = AS_MISSILE;
		return true;
	}
	
// missile attack
	if (!self->monsterinfo.attack)
		return false;
		
	if (level.time < self->monsterinfo.attack_finished)
		return false;
		
	if (enemy_range == RANGE_FAR)
		return false;

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MELEE)
	{
		chance = 0.8;
	}
	else if (enemy_range == RANGE_NEAR)
	{
		chance = 0.4;
	}
	else if (enemy_range == RANGE_MID)
	{
		chance = 0.2;
	}
	else
	{
		return false;
	}

	if (random () < chance)
	{
		self->monsterinfo.attack_state = AS_MISSILE;
		self->monsterinfo.attack_finished = level.time + 2*random();
		return true;
	}

	if (self->flags & FL_FLY)
	{
		if (random() < 0.3)
			self->monsterinfo.attack_state = AS_SLIDING;
		else
			self->monsterinfo.attack_state = AS_STRAIGHT;
	}

	return false;
}


//
// monster_makron
//

void MakronPrecache (void)
{
	sound_pain4 = gi.soundindex ("makron/pain3.wav");
	sound_pain5 = gi.soundindex ("makron/pain2.wav");
	sound_pain6 = gi.soundindex ("makron/pain1.wav");
	sound_death = gi.soundindex ("makron/death.wav");
	sound_step_left = gi.soundindex ("makron/step1.wav");
	sound_step_right = gi.soundindex ("makron/step2.wav");
	sound_attack_bfg = gi.soundindex ("makron/bfg_fire.wav");
	sound_brainsplorch = gi.soundindex ("makron/brain1.wav");
	sound_prerailgun = gi.soundindex ("makron/rail_up.wav");
	sound_popup = gi.soundindex ("makron/popup.wav");
	sound_taunt1 = gi.soundindex ("makron/voice4.wav");
	sound_taunt2 = gi.soundindex ("makron/voice3.wav");
	sound_taunt3 = gi.soundindex ("makron/voice.wav");
	sound_hit = gi.soundindex ("makron/bhit.wav");

	gi.modelindex ("models/monsters/boss3/rider/tris.md2");
}
AutoSFP(makron_pain)
SFPEnt(die, makron_die)
AutoSFP(makron_stand)
AutoSFP(makron_walk)

AutoSFP(makron_attack)

AutoSFP(makron_sight)
AutoSFP(Makron_CheckAttack)
AutoSFP(makron_get_currentmove)
/*QUAKED monster_makron (1 .5 0) (-30 -30 0) (30 30 90) Ambush Trigger_Spawn Sight
*/
void SP_monster_makron (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	MakronPrecache ();

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/boss3/rider/tris.md2");
	VectorSet (self->s.mins, -30, -30, 0);
	VectorSet (self->s.maxs, 30, 30, 90);

	self->health = 3000;
	self->gib_health = -2000;
	self->mass = 500;

	self->pain = SFP::makron_pain;
	self->die = SFP::makron_die;
	self->monsterinfo.stand = SFP::makron_stand;
	self->monsterinfo.walk = SFP::makron_walk;
	self->monsterinfo.run = SFP::makron_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = SFP::makron_attack;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = SFP::makron_sight;
	self->monsterinfo.checkattack = SFP::Makron_CheckAttack;
	self->monsterinfo.get_currentmove = SFP::makron_get_currentmove;

	gi.linkentity (self);
	
//	self->monsterinfo.currentmove = makron_move_stand;
	self->monsterinfo.currentmove = makron_move_sight;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}


/*
=================
MakronSpawn

=================
*/
void MakronSpawn (edict_t *self)
{
	vec3_t		vec;
	edict_t		*player;

	SP_monster_makron (self);

	// jump at player
	player = level.sight_client;
	if (!player)
		return;

	VectorSubtract (player->s.origin, self->s.origin, vec);
	self->s.angles[YAW] = vectoyaw(vec);
	VectorNormalize (vec);
	VectorMA (vec3_origin, 400, vec, self->velocity);
	self->velocity[2] = 200;
	self->groundentity = nullptr;
}
AutoSFP(MakronSpawn)
/*
=================
MakronToss

Jorg is just about dead, so set up to launch Makron out
=================
*/
void MakronToss (edict_t *self)
{
	edict_t	*ent;

	ent = G_Spawn ();
	ent->nextthink = level.time + 0.8;
	ent->think = SFP::MakronSpawn;
	ent->target = self->target;
	VectorCopy (self->s.origin, ent->s.origin);
}
