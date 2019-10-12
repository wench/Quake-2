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

MEDIC

==============================================================================
*/

#include "g_local.h"
#include "m_medic.h"

qboolean visible (edict_t *self, edict_t *other);

enum {
	medic_move_stand = 1,
	medic_move_walk,
	medic_move_run,
	medic_move_pain1,
	medic_move_pain2,
	medic_move_death,
	medic_move_duck,
	medic_move_attackHyperBlaster,
	medic_move_attackBlaster,
	medic_move_attackCable
};

static int	sound_idle1;
static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_sight;
static int	sound_search;
static int	sound_hook_launch;
static int	sound_hook_hit;
static int	sound_hook_heal;
static int	sound_hook_retract;

AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)

edict_t *medic_FindDeadMonster (edict_t *self)
{
	edict_t	*ent = nullptr;
	edict_t	*best = nullptr;

	while ((ent = findradius(ent, self->s.origin, 1024)) != nullptr)
	{
		if (ent == self)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (ent->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		if (ent->owner)
			continue;
		if (ent->health > 0)
			continue;
		if (ent->nextthink)
			continue;
		if (!visible(self, ent))
			continue;
		if (!best)
		{
			best = ent;
			continue;
		}
		if (ent->max_health <= best->max_health)
			continue;
		best = ent;
	}

	return best;
}
AutoSFP(medic_FindDeadMonster)

void medic_idle (edict_t *self)
{
	edict_t	*ent;

	gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);

	ent = medic_FindDeadMonster(self);
	if (ent)
	{
		self->enemy = ent;
		self->enemy->owner = self;
		self->monsterinfo.aiflags |= AI_MEDIC;
		FoundTarget (self);
	}
}
AutoSFP(medic_idle)

void medic_search (edict_t *self)
{
	edict_t	*ent;

	gi.sound (self, CHAN_VOICE, sound_search, 1, ATTN_IDLE, 0);

	if (!self->oldenemy)
	{
		ent = medic_FindDeadMonster(self);
		if (ent)
		{
			self->oldenemy = self->enemy;
			self->enemy = ent;
			self->enemy->owner = self;
			self->monsterinfo.aiflags |= AI_MEDIC;
			FoundTarget (self);
		}
	}
}
AutoSFP(medic_search)

void medic_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}


mframe_t medic_frames_stand [] =
{
	SFP::ai_stand, 0, SFP::medic_idle,
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

};

void medic_stand (edict_t *self)
{
	self->monsterinfo.currentmove = medic_move_stand;
}
AutoSFP(medic_stand)


mframe_t medic_frames_walk [] =
{
	SFP::ai_walk, 6.2,	nullptr,
	SFP::ai_walk, 18.1,  nullptr,
	SFP::ai_walk, 1,		nullptr,
	SFP::ai_walk, 9,		nullptr,
	SFP::ai_walk, 10,	nullptr,
	SFP::ai_walk, 9,		nullptr,
	SFP::ai_walk, 11,	nullptr,
	SFP::ai_walk, 11.6,  nullptr,
	SFP::ai_walk, 2,		nullptr,
	SFP::ai_walk, 9.9,	nullptr,
	SFP::ai_walk, 14,	nullptr,
	SFP::ai_walk, 9.3,	nullptr
};

void medic_walk (edict_t *self)
{
	self->monsterinfo.currentmove = medic_move_walk;
}
AutoSFP(medic_walk)


mframe_t medic_frames_run [] =
{
	SFP::ai_run, 18,		nullptr,
	SFP::ai_run, 22.5,	nullptr,
	SFP::ai_run, 25.4,	nullptr,
	SFP::ai_run, 23.4,	nullptr,
	SFP::ai_run, 24,		nullptr,
	SFP::ai_run, 35.6,	nullptr
	
};

void medic_run (edict_t *self)
{
	if (!(self->monsterinfo.aiflags & AI_MEDIC))
	{
		edict_t	*ent;

		ent = medic_FindDeadMonster(self);
		if (ent)
		{
			self->oldenemy = self->enemy;
			self->enemy = ent;
			self->enemy->owner = self;
			self->monsterinfo.aiflags |= AI_MEDIC;
			FoundTarget (self);
			return;
		}
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = medic_move_stand;
	else
		self->monsterinfo.currentmove = medic_move_run;
}
AutoSFP(medic_run)


mframe_t medic_frames_pain1 [] =
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

mframe_t medic_frames_pain2 [] =
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
	SFP::ai_move, 0, nullptr
};

void medic_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	if (random() < 0.5)
	{
		self->monsterinfo.currentmove = medic_move_pain1;
		gi.sound (self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	}
	else
	{
		self->monsterinfo.currentmove = medic_move_pain2;
		gi.sound (self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);
	}
}

void medic_fire_blaster (edict_t *self)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	end;
	vec3_t	dir;
	int		effect;

	if ((self->s.frame == FRAME_attack9) || (self->s.frame == FRAME_attack12))
		effect = EF_BLASTER;
	else if ((self->s.frame == FRAME_attack19) || (self->s.frame == FRAME_attack22) || (self->s.frame == FRAME_attack25) || (self->s.frame == FRAME_attack28))
		effect = EF_HYPERBLASTER;
	else
		effect = 0;

	AngleVectors (self->s.angles, forward, right, nullptr);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_MEDIC_BLASTER_1], forward, right, start);

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);

	monster_fire_blaster (self, start, dir, 2, 1000, MZ2_MEDIC_BLASTER_1, effect);
}


void medic_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}
AutoSFP(medic_fire_blaster)
AutoSFP(medic_dead)
mframe_t medic_frames_death [] =
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

void medic_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// if we had a pending patient, free him up for another medic
	if ((self->enemy) && (self->enemy->owner == self))
		self->enemy->owner = nullptr;

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

	self->monsterinfo.currentmove = medic_move_death;
}


void medic_duck_down (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_DUCKED)
		return;
	self->monsterinfo.aiflags |= AI_DUCKED;
	self->s.maxs[2] -= 32;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.pausetime = level.time + 1;
	gi.linkentity (self);
}

void medic_duck_hold (edict_t *self)
{
	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void medic_duck_up (edict_t *self)
{
	self->monsterinfo.aiflags &= ~AI_DUCKED;
	self->s.maxs[2] += 32;
	self->takedamage = DAMAGE_AIM;
	gi.linkentity (self);
}

AutoSFP(medic_duck_down)
AutoSFP(medic_duck_hold)
AutoSFP(medic_duck_up)
mframe_t medic_frames_duck [] =
{
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	SFP::medic_duck_down,
	SFP::ai_move, -1,	SFP::medic_duck_hold,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	SFP::medic_duck_up,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr,
	SFP::ai_move, -1,	nullptr
};

void medic_dodge (edict_t *self, edict_t *attacker, float eta)
{
	if (random() > 0.25)
		return;

	if (!self->enemy)
		self->enemy = attacker;

	self->monsterinfo.currentmove = medic_move_duck;
}
AutoSFP(medic_dodge)
mframe_t medic_frames_attackHyperBlaster [] =
{
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster
};


void medic_continue (edict_t *self)
{
	if (visible (self, self->enemy) )
		if (random() <= 0.95)
			self->monsterinfo.currentmove = medic_move_attackHyperBlaster;
}
AutoSFP(medic_continue)

mframe_t medic_frames_attackBlaster [] =
{
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 5,	nullptr,
	SFP::ai_charge, 5,	nullptr,
	SFP::ai_charge, 3,	nullptr,
	SFP::ai_charge, 2,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::medic_fire_blaster,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_charge, 0,	SFP::medic_continue	// Change to medic_continue... Else, go to frame 32
};


void medic_hook_launch (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_hook_launch, 1, ATTN_NORM, 0);
}

void ED_CallSpawn (edict_t *ent);

static vec3_t	medic_cable_offsets[] =
{
	45.0,  -9.2, 15.5,
	48.4,  -9.7, 15.2,
	47.8,  -9.8, 15.8,
	47.3,  -9.3, 14.3,
	45.4, -10.1, 13.1,
	41.9, -12.7, 12.0,
	37.8, -15.8, 11.2,
	34.3, -18.4, 10.7,
	32.7, -19.7, 10.4,
	32.7, -19.7, 10.4
};

void medic_cable_attack (edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	trace_t	tr;
	vec3_t	dir, angles;
	float	distance;

	if (!self->enemy->inuse)
		return;

	AngleVectors (self->s.angles, f, r, nullptr);
	VectorCopy (medic_cable_offsets[self->s.frame - FRAME_attack42], offset);
	G_ProjectSource (self->s.origin, offset, f, r, start);

	// check for max distance
	VectorSubtract (start, self->enemy->s.origin, dir);
	distance = VectorLength(dir);
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	tr = gi.trace (start, nullptr, nullptr, self->enemy->s.origin, self, MASK_SHOT);
	if (tr.fraction != 1.0 && tr.ent != self->enemy)
		return;

	if (self->s.frame == FRAME_attack43)
	{
		gi.sound (self->enemy, CHAN_AUTO, sound_hook_hit, 1, ATTN_NORM, 0);
		self->enemy->monsterinfo.aiflags |= AI_RESURRECTING;
	}
	else if (self->s.frame == FRAME_attack50)
	{
		self->enemy->spawnflags = 0;
		self->enemy->monsterinfo.aiflags = 0;
		self->enemy->target = nullptr;
		self->enemy->targetname = nullptr;
		self->enemy->combattarget = nullptr;
		self->enemy->deathtarget = nullptr;
		self->enemy->owner = self;
		ED_CallSpawn (self->enemy);
		self->enemy->owner = nullptr;
		if (self->enemy->think)
		{
			self->enemy->nextthink = level.time;
			self->enemy->think (self->enemy);
		}
		self->enemy->monsterinfo.aiflags |= AI_RESURRECTING;
		if (self->oldenemy && self->oldenemy->client)
		{
			self->enemy->enemy = self->oldenemy;
			FoundTarget (self->enemy);
		}
	}
	else
	{
		if (self->s.frame == FRAME_attack44)
			gi.sound (self, CHAN_WEAPON, sound_hook_heal, 1, ATTN_NORM, 0);
	}

	// adjust start for beam origin being in middle of a segment
	VectorMA (start, 8, f, start);

	// adjust end z for end spot since the monster is currently dead
	VectorCopy (self->enemy->s.origin, end);
	end[2] = self->enemy->absmin[2] + self->enemy->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (end);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void medic_hook_retract (edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_hook_retract, 1, ATTN_NORM, 0);
	self->enemy->monsterinfo.aiflags &= ~AI_RESURRECTING;
}
AutoSFP(medic_hook_launch)
AutoSFP(medic_cable_attack)
AutoSFP(medic_hook_retract)
mframe_t medic_frames_attackCable [] =
{
	SFP::ai_move, 2,		nullptr,
	SFP::ai_move, 3,		nullptr,
	SFP::ai_move, 5,		nullptr,
	SFP::ai_move, 4.4,	nullptr,
	SFP::ai_charge, 4.7,	nullptr,
	SFP::ai_charge, 5,	nullptr,
	SFP::ai_charge, 6,	nullptr,
	SFP::ai_charge, 4,	nullptr,
	SFP::ai_charge, 0,	nullptr,
	SFP::ai_move, 0,		SFP::medic_hook_launch,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, 0,		SFP::medic_cable_attack,
	SFP::ai_move, -15,	SFP::medic_hook_retract,
	SFP::ai_move, -1.5,	nullptr,
	SFP::ai_move, -1.2,	nullptr,
	SFP::ai_move, -3,	nullptr,
	SFP::ai_move, -2,	nullptr,
	SFP::ai_move, 0.3,	nullptr,
	SFP::ai_move, 0.7,	nullptr,
	SFP::ai_move, 1.2,	nullptr,
	SFP::ai_move, 1.3,	nullptr
};


void medic_attack(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_MEDIC)
		self->monsterinfo.currentmove = medic_move_attackCable;
	else
		self->monsterinfo.currentmove = medic_move_attackBlaster;
}

qboolean medic_checkattack (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_MEDIC)
	{
		medic_attack(self);
		return true;
	}

	return M_CheckAttack (self);
}
//AutoSFP*()
mmove_t medic_moves[] = {
	{FRAME_wait1, FRAME_wait90, medic_frames_stand, nullptr},
	{FRAME_walk1, FRAME_walk12, medic_frames_walk, nullptr},
	{FRAME_run1, FRAME_run6, medic_frames_run, nullptr},
	{FRAME_paina1, FRAME_paina8, medic_frames_pain1, SFP::medic_run},
	{FRAME_painb1, FRAME_painb15, medic_frames_pain2, SFP::medic_run},
	{FRAME_death1, FRAME_death30, medic_frames_death,SFP::medic_dead},
	{FRAME_duck1, FRAME_duck16, medic_frames_duck, SFP::medic_run},
	{FRAME_attack15, FRAME_attack30, medic_frames_attackHyperBlaster, SFP::medic_run},
	{FRAME_attack1, FRAME_attack14, medic_frames_attackBlaster, SFP::medic_run},
	{FRAME_attack33, FRAME_attack60, medic_frames_attackCable, SFP::medic_run}
};

mmove_t * medic_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &medic_moves[self->monsterinfo.currentmove-1];
}

SFPEnt(pain, medic_pain)
SFPEnt(die, medic_die)

SFPEnt(monsterinfo.attack, medic_attack)
SFPEnt(monsterinfo.sight, medic_sight)
SFPEnt(monsterinfo.checkattack, medic_checkattack)
SFPEnt(monsterinfo.get_currentmove, medic_get_currentmove)
/*QUAKED monster_medic (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_medic (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_idle1 = gi.soundindex ("medic/idle.wav");
	sound_pain1 = gi.soundindex ("medic/medpain1.wav");
	sound_pain2 = gi.soundindex ("medic/medpain2.wav");
	sound_die = gi.soundindex ("medic/meddeth1.wav");
	sound_sight = gi.soundindex ("medic/medsght1.wav");
	sound_search = gi.soundindex ("medic/medsrch1.wav");
	sound_hook_launch = gi.soundindex ("medic/medatck2.wav");
	sound_hook_hit = gi.soundindex ("medic/medatck3.wav");
	sound_hook_heal = gi.soundindex ("medic/medatck4.wav");
	sound_hook_retract = gi.soundindex ("medic/medatck5.wav");

	gi.soundindex ("medic/medatck1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/medic/tris.md2");
	VectorSet (self->s.mins, -24, -24, -24);
	VectorSet (self->s.maxs, 24, 24, 32);

	self->health = 300;
	self->gib_health = -130;
	self->mass = 400;

	self->pain = SFP::medic_pain;
	self->die = SFP::medic_die;

	self->monsterinfo.stand = SFP::medic_stand;
	self->monsterinfo.walk = SFP::medic_walk;
	self->monsterinfo.run = SFP::medic_run;
	self->monsterinfo.dodge = SFP::medic_dodge;
	self->monsterinfo.attack = SFP::medic_attack;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = SFP::medic_sight;
	self->monsterinfo.idle = SFP::medic_idle;
	self->monsterinfo.search = SFP::medic_search;
	self->monsterinfo.checkattack = SFP::medic_checkattack;
	self->monsterinfo.get_currentmove = SFP::medic_get_currentmove;

	gi.linkentity (self);

	self->monsterinfo.currentmove = medic_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
