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

insane

==============================================================================
*/

#include "g_local.h"
#include "m_insane.h"


static int	sound_fist;
static int	sound_shake;
static int	sound_moan;
static int	sound_scream[8];

enum {
	insane_move_stand_normal = 1,
	insane_move_stand_insane,
	insane_move_uptodown,
	insane_move_downtoup,
	insane_move_jumpdown,
	insane_move_down,
	insane_move_walk_normal,
	insane_move_run_normal,
	insane_move_walk_insane,
	insane_move_run_insane,
	insane_move_stand_pain,
	insane_move_stand_death,
	insane_move_crawl,
	insane_move_runcrawl,
	insane_move_crawl_pain,
	insane_move_crawl_death,
	insane_move_cross,
	insane_move_struggle_cross,
};
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
void insane_fist (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_fist, 1, ATTN_IDLE, 0);
}

void insane_shake (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_shake, 1, ATTN_IDLE, 0);
}

void insane_moan (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_moan, 1, ATTN_IDLE, 0);
}

void insane_scream (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_scream[rand()%8], 1, ATTN_IDLE, 0);
}


void insane_stand (edict_t *self);
void insane_dead (edict_t *self);
void insane_cross (edict_t *self);
void insane_walk (edict_t *self);
void insane_run (edict_t *self);
void insane_checkdown (edict_t *self);
void insane_checkup (edict_t *self);
void insane_onground (edict_t *self);
AutoSFP(insane_stand)
AutoSFP(insane_dead)
AutoSFP(insane_cross)
AutoSFP(insane_walk)
AutoSFP(insane_run)
AutoSFP(insane_checkdown)
AutoSFP(insane_checkup)
AutoSFP(insane_onground)
mframe_t insane_frames_stand_normal [] =
{
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, nullptr,
	SFP::ai_stand, 0, SFP::insane_checkdown
};
AutoSFP(insane_shake)
AutoSFP(insane_fist)
mframe_t insane_frames_stand_insane [] =
{
	SFP::ai_stand,	0,	SFP::insane_shake,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_stand,	0,	nullptr,
	SFP::ai_move,	0,	SFP::insane_fist,
	SFP::ai_stand,	0,	SFP::insane_checkdown
};
AutoSFP(insane_moan)
mframe_t insane_frames_uptodown [] =
{
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::insane_fist,
	SFP::ai_move,	0,	SFP::insane_moan,
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

	SFP::ai_move,	2.7,	nullptr,
	SFP::ai_move,	4.1,	nullptr,
	SFP::ai_move,	6,		nullptr,
	SFP::ai_move,	7.6,	nullptr,
	SFP::ai_move,	3.6,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::insane_fist,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,

	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	SFP::insane_fist,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr,
	SFP::ai_move,	0,	nullptr
};


mframe_t insane_frames_downtoup [] =
{
	SFP::ai_move,	-0.7,	nullptr,			// 41
	SFP::ai_move,	-1.2,	nullptr,			// 42
	SFP::ai_move,	-1.5,		nullptr,		// 43
	SFP::ai_move,	-4.5,		nullptr,		// 44
	SFP::ai_move,	-3.5,	nullptr,			// 45
	SFP::ai_move,	-0.2,	nullptr,			// 46
	SFP::ai_move,	0,	nullptr,			// 47
	SFP::ai_move,	-1.3,	nullptr,			// 48
	SFP::ai_move,	-3,	nullptr,				// 49
	SFP::ai_move,	-2,	nullptr,			// 50
	SFP::ai_move,	0,	nullptr,				// 51
	SFP::ai_move,	0,	nullptr,				// 52
	SFP::ai_move,	0,	nullptr,				// 53
	SFP::ai_move,	-3.3,	nullptr,			// 54
	SFP::ai_move,	-1.6,	nullptr,			// 55
	SFP::ai_move,	-0.3,	nullptr,			// 56
	SFP::ai_move,	0,	nullptr,				// 57
	SFP::ai_move,	0,	nullptr,				// 58
	SFP::ai_move,	0,	nullptr				// 59
};

mframe_t insane_frames_jumpdown [] =
{
	SFP::ai_move,	0.2,	nullptr,
	SFP::ai_move,	11.5,	nullptr,
	SFP::ai_move,	5.1,	nullptr,
	SFP::ai_move,	7.1,	nullptr,
	SFP::ai_move,	0,	nullptr
};

AutoSFP(insane_scream)
mframe_t insane_frames_down [] =
{
	SFP::ai_move,	0,		nullptr,		// 100
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,		// 110
	SFP::ai_move,	-1.7,		nullptr,
	SFP::ai_move,	-1.6,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		SFP::insane_fist,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,		// 120
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,		// 130
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		SFP::insane_moan,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,		// 140
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,		// 150
	SFP::ai_move,	0.5,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	-0.2,		SFP::insane_scream,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0.2,		nullptr,
	SFP::ai_move,	0.4,		nullptr,
	SFP::ai_move,	0.6,		nullptr,
	SFP::ai_move,	0.8,		nullptr,
	SFP::ai_move,	0.7,		nullptr,
	SFP::ai_move,	0,		SFP::insane_checkup		// 160
};

mframe_t insane_frames_walk_normal [] =
{
	SFP::ai_walk,	0,		SFP::insane_scream,
	SFP::ai_walk,	2.5,	nullptr,
	SFP::ai_walk,	3.5,	nullptr,
	SFP::ai_walk,	1.7,	nullptr,
	SFP::ai_walk,	2.3,	nullptr,
	SFP::ai_walk,	2.4,	nullptr,
	SFP::ai_walk,	2.2,	nullptr,
	SFP::ai_walk,	4.2,	nullptr,
	SFP::ai_walk,	5.6,	nullptr,
	SFP::ai_walk,	3.3,	nullptr,
	SFP::ai_walk,	2.4,	nullptr,
	SFP::ai_walk,	0.9,	nullptr,
	SFP::ai_walk,	0,		nullptr
};

mframe_t insane_frames_walk_insane [] =
{
	SFP::ai_walk,	0,		SFP::insane_scream,		// walk 1
	SFP::ai_walk,	3.4,	nullptr,		// walk 2
	SFP::ai_walk,	3.6,	nullptr,		// 3
	SFP::ai_walk,	2.9,	nullptr,		// 4
	SFP::ai_walk,	2.2,	nullptr,		// 5
	SFP::ai_walk,	2.6,	nullptr,		// 6
	SFP::ai_walk,	0,		nullptr,		// 7
	SFP::ai_walk,	0.7,	nullptr,		// 8
	SFP::ai_walk,	4.8,	nullptr,		// 9
	SFP::ai_walk,	5.3,	nullptr,		// 10
	SFP::ai_walk,	1.1,	nullptr,		// 11
	SFP::ai_walk,	2,		nullptr,		// 12
	SFP::ai_walk,	0.5,	nullptr,		// 13
	SFP::ai_walk,	0,		nullptr,		// 14
	SFP::ai_walk,	0,		nullptr,		// 15
	SFP::ai_walk,	4.9,	nullptr,		// 16
	SFP::ai_walk,	6.7,	nullptr,		// 17
	SFP::ai_walk,	3.8,	nullptr,		// 18
	SFP::ai_walk,	2,		nullptr,		// 19
	SFP::ai_walk,	0.2,	nullptr,		// 20
	SFP::ai_walk,	0,		nullptr,		// 21
	SFP::ai_walk,	3.4,	nullptr,		// 22
	SFP::ai_walk,	6.4,	nullptr,		// 23
	SFP::ai_walk,	5,		nullptr,		// 24
	SFP::ai_walk,	1.8,	nullptr,		// 25
	SFP::ai_walk,	0,		nullptr		// 26
};

mframe_t insane_frames_stand_pain [] =
{
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};

mframe_t insane_frames_stand_death [] =
{
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};

mframe_t insane_frames_crawl [] =
{
	SFP::ai_walk,	0,		SFP::insane_scream,
	SFP::ai_walk,	1.5,	nullptr,
	SFP::ai_walk,	2.1,	nullptr,
	SFP::ai_walk,	3.6,	nullptr,
	SFP::ai_walk,	2,		nullptr,
	SFP::ai_walk,	0.9,	nullptr,
	SFP::ai_walk,	3,		nullptr,
	SFP::ai_walk,	3.4,	nullptr,
	SFP::ai_walk,	2.4,	nullptr
};

mframe_t insane_frames_crawl_pain [] =
{
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};

mframe_t insane_frames_crawl_death [] =
{
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};


mframe_t insane_frames_cross [] =
{
	SFP::ai_move,	0,		SFP::insane_moan,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};

mframe_t insane_frames_struggle_cross [] =
{
	SFP::ai_move,	0,		SFP::insane_scream,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr,
	SFP::ai_move,	0,		nullptr
};

void insane_cross (edict_t *self)
{
	if (random() < 0.8)		
		self->monsterinfo.currentmove = insane_move_cross;
	else
		self->monsterinfo.currentmove = insane_move_struggle_cross;
}

void insane_walk (edict_t *self)
{
	if ( self->spawnflags & 16 )			// Hold Ground?
		if (self->s.frame == FRAME_cr_pain10)
		{
			self->monsterinfo.currentmove = insane_move_down;
			return;
		}
	if (self->spawnflags & 4)
		self->monsterinfo.currentmove = insane_move_crawl;
	else
		if (random() <= 0.5)
			self->monsterinfo.currentmove = insane_move_walk_normal;
		else
			self->monsterinfo.currentmove = insane_move_walk_insane;
}

void insane_run (edict_t *self)
{
	if ( self->spawnflags & 16 )			// Hold Ground?
		if (self->s.frame == FRAME_cr_pain10)
		{
			self->monsterinfo.currentmove = insane_move_down;
			return;
		}
	if (self->spawnflags & 4)				// Crawling?
		self->monsterinfo.currentmove = insane_move_runcrawl;
	else
		if (random() <= 0.5)				// Else, mix it up
			self->monsterinfo.currentmove = insane_move_run_normal;
		else
			self->monsterinfo.currentmove = insane_move_run_insane;
}


void insane_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int	l,r;

//	if (self->health < (self->max_health / 2))
//		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	r = 1 + (rand()&1);
	if (self->health < 25)
		l = 25;
	else if (self->health < 50)
		l = 50;
	else if (self->health < 75)
		l = 75;
	else
		l = 100;
	gi.sound (self, CHAN_VOICE, gi.soundindex (va("player/male/pain%i_%i.wav", l, r)), 1, ATTN_IDLE, 0);

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	// Don't go into pain frames if crucified.
	if (self->spawnflags & 8)
	{
		self->monsterinfo.currentmove = insane_move_struggle_cross;			
		return;
	}
	
	if  ( ((self->s.frame >= FRAME_crawl1) && (self->s.frame <= FRAME_crawl9)) || ((self->s.frame >= FRAME_stand99) && (self->s.frame <= FRAME_stand160)) )
	{
		self->monsterinfo.currentmove = insane_move_crawl_pain;
	}
	else
		self->monsterinfo.currentmove = insane_move_stand_pain;

}

void insane_onground (edict_t *self)
{
	self->monsterinfo.currentmove = insane_move_down;
}

void insane_checkdown (edict_t *self)
{
//	if ( (self->s.frame == FRAME_stand94) || (self->s.frame == FRAME_stand65) )
	if (self->spawnflags & 32)				// Always stand
		return;
	if (random() < 0.3)
		if (random() < 0.5)
			self->monsterinfo.currentmove = insane_move_uptodown;
		else
			self->monsterinfo.currentmove = insane_move_jumpdown; 
}

void insane_checkup (edict_t *self)
{
	// If Hold_Ground and Crawl are set
	if ( (self->spawnflags & 4) && (self->spawnflags & 16) )
		return;
	if (random() < 0.5)
		self->monsterinfo.currentmove = insane_move_downtoup;				

}

void insane_stand (edict_t *self)
{
	if (self->spawnflags & 8)			// If crucified
	{
		self->monsterinfo.currentmove = insane_move_cross;
		self->monsterinfo.aiflags |= AI_STAND_GROUND;
	}
	// If Hold_Ground and Crawl are set
	else if ( (self->spawnflags & 4) && (self->spawnflags & 16) )
		self->monsterinfo.currentmove = insane_move_down;
	else
		if (random() < 0.5)
			self->monsterinfo.currentmove = insane_move_stand_normal;
		else
			self->monsterinfo.currentmove = insane_move_stand_insane;
}

void insane_dead (edict_t *self)
{
	if (self->spawnflags & 8)
	{
		self->flags |= FL_FLY;
	}
	else
	{
		VectorSet (self->s.mins, -16, -16, -24);
		VectorSet (self->s.maxs, 16, 16, -8);
		self->movetype = MOVETYPE_TOSS;
	}
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}


void insane_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health <= self->gib_health)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_IDLE, 0);
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

	gi.sound (self, CHAN_VOICE, gi.soundindex(va("player/male/death%i.wav", (rand()%4)+1)), 1, ATTN_IDLE, 0);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if (self->spawnflags & 8)
	{
		insane_dead (self);
	}
	else
	{
		if ( ((self->s.frame >= FRAME_crawl1) && (self->s.frame <= FRAME_crawl9)) || ((self->s.frame >= FRAME_stand99) && (self->s.frame <= FRAME_stand160)) )		
			self->monsterinfo.currentmove = insane_move_crawl_death;
		else
			self->monsterinfo.currentmove = insane_move_stand_death;
	}
}

mmove_t insane_moves[] = {
	{FRAME_stand60, FRAME_stand65, insane_frames_stand_normal, SFP::insane_stand},
	{FRAME_stand65, FRAME_stand94, insane_frames_stand_insane, SFP::insane_stand},
	{FRAME_stand1, FRAME_stand40, insane_frames_uptodown, SFP::insane_onground},
	{FRAME_stand41, FRAME_stand59, insane_frames_downtoup, SFP::insane_stand},
	{FRAME_stand96, FRAME_stand100, insane_frames_jumpdown, SFP::insane_onground},
	{FRAME_stand100, FRAME_stand160, insane_frames_down, SFP::insane_onground},
	{FRAME_walk27, FRAME_walk39, insane_frames_walk_normal, SFP::insane_walk},
	{FRAME_walk27, FRAME_walk39, insane_frames_walk_normal, SFP::insane_run},
	{FRAME_walk1, FRAME_walk26, insane_frames_walk_insane, SFP::insane_walk},
	{FRAME_walk1, FRAME_walk26, insane_frames_walk_insane, SFP::insane_run},
	{FRAME_st_pain2, FRAME_st_pain12, insane_frames_stand_pain, SFP::insane_run},
	{FRAME_st_death2, FRAME_st_death18, insane_frames_stand_death, SFP::insane_dead},
	{FRAME_crawl1, FRAME_crawl9, insane_frames_crawl, nullptr},
	{FRAME_crawl1, FRAME_crawl9, insane_frames_crawl, nullptr},
	{FRAME_cr_pain2, FRAME_cr_pain10, insane_frames_crawl_pain, SFP::insane_run},
	{FRAME_cr_death10, FRAME_cr_death16, insane_frames_crawl_death, SFP::insane_dead},
	{FRAME_cross1, FRAME_cross15, insane_frames_cross, SFP::insane_cross},
	{FRAME_cross16, FRAME_cross30, insane_frames_struggle_cross, SFP::insane_cross}
};

mmove_t * insane_get_currentmove(edict_t *self)
{
	if (!self->monsterinfo.currentmove) return nullptr;
	return &insane_moves[self->monsterinfo.currentmove-1];
}

SFPEnt(pain,insane_pain)
SFPEnt(die,insane_die)


SFPEnt(monsterinfo.get_currentmove,insane_get_currentmove)
/*QUAKED misc_insane (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn CRAWL CRUCIFIED STAND_GROUND ALWAYS_STAND
*/
void SP_misc_insane (edict_t *self)
{
//	static int skin = 0;	//@@

	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_fist = gi.soundindex ("insane/insane11.wav");
	sound_shake = gi.soundindex ("insane/insane5.wav");
	sound_moan = gi.soundindex ("insane/insane7.wav");
	sound_scream[0] = gi.soundindex ("insane/insane1.wav");
	sound_scream[1] = gi.soundindex ("insane/insane2.wav");
	sound_scream[2] = gi.soundindex ("insane/insane3.wav");
	sound_scream[3] = gi.soundindex ("insane/insane4.wav");
	sound_scream[4] = gi.soundindex ("insane/insane6.wav");
	sound_scream[5] = gi.soundindex ("insane/insane8.wav");
	sound_scream[6] = gi.soundindex ("insane/insane9.wav");
	sound_scream[7] = gi.soundindex ("insane/insane10.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/insane/tris.md2");

	VectorSet (self->s.mins, -16, -16, -24);
	VectorSet (self->s.maxs, 16, 16, 32);

	self->health = 100;
	self->gib_health = -50;
	self->mass = 300;

	self->pain = SFP::insane_pain;
	self->die = SFP::insane_die;

	self->monsterinfo.stand = SFP::insane_stand;
	self->monsterinfo.walk = SFP::insane_walk;
	self->monsterinfo.run = SFP::insane_run;
	self->monsterinfo.dodge = nullptr;
	self->monsterinfo.attack = nullptr;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = nullptr;
	self->monsterinfo.get_currentmove = SFP::insane_get_currentmove;
	self->monsterinfo.aiflags |= AI_GOOD_GUY;

//@@
//	self->s.skinnum = skin;
//	skin++;
//	if (skin > 12)
//		skin = 0;

	gi.linkentity (self);

	if (self->spawnflags & 16)				// Stand Ground
		self->monsterinfo.aiflags |= AI_STAND_GROUND;

	self->monsterinfo.currentmove = insane_move_stand_normal;
	
	self->monsterinfo.scale = MODEL_SCALE;

	if (self->spawnflags & 8)					// Crucified ?
	{
		VectorSet (self->s.mins, -16, 0, 0);
		VectorSet (self->s.maxs, 16, 8, 32);
		self->flags |= FL_NO_KNOCKBACK;
		flymonster_start (self);
	}
	else
	{
		walkmonster_start (self);
		self->s.skinnum = rand()%3;
	}
}
