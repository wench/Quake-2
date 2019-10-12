// 
// This function is intended to search through all the list of 'anox' entities and spawn them.
//

#include "g_local.h"

anox_entity_desc_t *anox_entities = 0;

#define ANOX_entity_string(field)	\
do {								\
	token = COM_Parse3(&entities, false);	\
	if (!entities) goto error;		\
	gi.TTagMalloc(desc->field,strlen(token)+1, TAG_GAME);	\
	strcpy(desc->field, token);		\
} while(0)

#define ANOX_entity_float(field)	\
do {								\
	token = COM_Parse3(&entities, false);	\
	if (!entities) goto error;		\
	desc->field = atof(token);		\
} while(0)

#define ANOX_entity_bool(field,trueval)		\
do {										\
	token = COM_Parse3(&entities, false);			\
	if (!entities) goto error;				\
	desc->field = Q_strcasecmp(token,trueval)==0;	\
} while(0)


// Parse a entity
char *ANOX_parse_entity(char *classname, char *entities) 
{
	int i;
	char *token;
	anox_entity_desc_t	*desc;

	// Allocate our entity (only load ents once per 'game')
	gi.TTagMalloc(desc,sizeof(anox_entity_desc_t), TAG_GAME);
	memset(desc, 0, sizeof(anox_entity_desc_t));

	// Copy classname
	gi.TTagMalloc(desc->classname,strlen(classname)+1, TAG_GAME);
	strcpy(desc->classname, classname);

	//
	// Model path and profile
	//
	token = COM_Parse3(&entities, false);
	if (!entities) goto error;
	i = (int)strlen(token);
	if (i > 5 && token[i-5] == '!') 
	{
		desc->profile = *(int*)(token+i-4);
		i -= 5;
	}

	 gi.TTagMalloc(desc->model_path,i+1, TAG_GAME);
	memcpy(desc->model_path, token, i);
	desc->model_path[i] = 0;

	// All other fields

	for (i = 0; i < 3; i++) ANOX_entity_float(scale[i]);
	ANOX_entity_string(entity_type);
	for (i = 0; i < 3; i++) ANOX_entity_float(mins[i]);
	for (i = 0; i < 3; i++) ANOX_entity_float(maxs[i]);
	ANOX_entity_bool(noshadow,"noshadow");
	ANOX_entity_bool(solidflag,"1");
	ANOX_entity_float(walk_speed);
	ANOX_entity_float(run_speed);
	ANOX_entity_float(speed);
	ANOX_entity_bool(lighting,"1");
	ANOX_entity_bool(blending,"1");
	ANOX_entity_string(target_sequence);
	ANOX_entity_string(misc_value);
	ANOX_entity_bool(no_mip,"1");
	ANOX_entity_string(target_sequence);
	ANOX_entity_string(description);

	// Only created when required
	desc->moves = nullptr;	

	// Finally set us as the first in the list
	desc->next = anox_entities;
	anox_entities = desc;

	return entities;

error:
	// This will clean up a messed up entity
	if (desc->classname)  
		gi.TagFree(desc->classname);

	if (desc->description)  
		gi.TagFree(desc->description);

	if (desc->entity_type)  
		gi.TagFree(desc->entity_type);

	if (desc->target_sequence)  
		gi.TagFree(desc->target_sequence);

	if (desc->misc_value)  
		gi.TagFree(desc->misc_value);

	if (desc->spawn_sequence)  
		gi.TagFree(desc->spawn_sequence);

	if (desc->description)  
		gi.TagFree(desc->description);

	gi.TagFree(desc);

	// nullptr will indicate some sort of error
	return nullptr;
}

// Warning, assumption is models/entity.dat is formatted correctly!!
void ANOX_load_entity_descs() 
{
	char *data, *entities, *classname;
	int length; 
	length = gi.FS_LoadFile("models/entity.dat", (void**)&data);

	if (!data) return;

	// Yeah, shouldn't 'really' do this but it will allow 
	// COM_Parse3() to know the end of the file
	data[length-1] = 0;

	entities = data;

	// Now we parse all the entities
	while (1) {
		classname = COM_Parse3(&entities, true);

		// End of file
		if (!entities) break;

		// Read the entity
		entities = ANOX_parse_entity(classname, entities);

		if (!entities) 
		{
			Com_Printf ("Warning: problems parseing ANOX entity.dat\n");
			break;
		}
	}

	gi.FS_FreeFile(data);
}


// Think func for non monsters
void anox_think (edict_t *ent)
{
	ent->s.frame++;

	if (ent->s.frame >= (ent->anim_start+ent->anim_count))
		ent->s.frame = ent->anim_start;

	ent->nextthink = level.time + FRAMETIME;
}


// Anox 'monsters'

mmove_t * anox_get_currentmove(edict_t *self)
{
	if (self->monsterinfo.currentmove == 0) return nullptr;
	
	return &self->anox->moves[self->monsterinfo.currentmove-1];
}

int anox_get_move_by_name (edict_t *self, char *type)
{
	int i;
	md2_info_t *info;

	// Get the info
	info = gi.GetModelInfo(self->s.modelindex);

	// We're screwed
	if (!info) Sys_Error("Couldn't get model info for anox monster\n");

	for (i = 0; i < info->num_framesets; i++)
	{
		if (Q_strcasecmp(info->frameset[i].name, type) == 0)
			break;
	}

	if (i == info->num_framesets)
	{
		// If these 2 are not found, we've specially made our own versions
		if (Q_strcasecmp("walk_a", type) == 0)
			i = info->num_framesets;
		else if (Q_strcasecmp("run_a", type) == 0)
			i = info->num_framesets+1;
		else
			i = 0;
	}
	return i+1;
}

int anox_get_move_by_type (edict_t *self, char *type, char var)
{
	int i;
	md2_info_t *info;

	// Get the info
	info = gi.GetModelInfo(self->s.modelindex);

	// We're screwed
	if (!info) Sys_Error("Couldn't get model info for anox monster\n");

	for (i = 0; i < info->num_framesets; i++)
	{
		if (Q_strcasecmp(info->frameset[i].anim, type) == 0  && (var == -1 || var == info->frameset[i].subtype))
			break;
	}

	if (i == info->num_framesets)
	{
		// If these 2 are not found, we've specially made our own versions
		if (Q_strcasecmp("walk", type) == 0 && (var == -1 || var == 'a'))
			i = info->num_framesets;
		else if (Q_strcasecmp("run", type) == 0 && (var == -1 || var == 'a'))
			i = info->num_framesets+1;
		else
			i = 0;
	}
	return i+1;
}

// Set currentmove to the 'default' animation
void anox_stand(edict_t *self)
{
	self->monsterinfo.currentmove = anox_get_move_by_name(self, self->default_anim);
}

// Set currentmove to the walk animation
void anox_walk(edict_t *self)
{
	self->monsterinfo.currentmove = anox_get_move_by_name(self, "walk_a");
}

// Set currentmove to the run animation
void anox_run(edict_t *self)
{
	self->monsterinfo.currentmove = anox_get_move_by_name(self, "run_a");
}

void anox_dead (edict_t *self)
{
	VectorSet (self->s.mins, -16, -16, 0);
	VectorSet (self->s.maxs, 16, 16, 16);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

void anox_custom_anim(edict_t *self, char* animname)
{
	self->monsterinfo.currentmove = anox_get_move_by_name(self, animname);
}

void anox_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/udeath.wav"), 1, ATTN_IDLE, 0);
	for (n= 0; n < 2; n++)
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
	for (n= 0; n < 4; n++)
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
}

void anox_die_with_anim (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
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

	self->monsterinfo.currentmove = anox_get_move_by_name(self, "die_a");
}
SFPEnt(die, anox_die_with_anim)
SFPEnt(die, anox_die)

void anox_die_with_combat_anim (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	char	new_class[MAX_QPATH];
	int		n;
	anox_entity_desc_t	*new_desc = 0;

	strcpy(new_class, self->classname);
	strcat(new_class, "_com");

	for (new_desc = anox_entities; new_desc != nullptr; new_desc = new_desc->next) 
	{
		if (!new_desc->classname) continue;
		if (!Q_strcasecmp(new_desc->classname, new_class)) 
		{
			break;
		}
	}

	// check for gib
	if (self->health <= self->gib_health || !new_desc)
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

	anox_setup_moves(new_desc);
	self->anox = new_desc;
	gi.TTagMalloc(self->classname,strlen(new_class)+1, TAG_LEVEL);
	strcpy(self->classname, new_class);
	self->die = SFP::anox_die_with_anim;
	self->s.modelindex = gi.modelindex(new_desc->model_path);
	gi.linkentity(self);

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = anox_get_move_by_name(self, "die_a");
}

void anox_pain_with_anim (edict_t *self, edict_t *other, float kick, int damage)
{
	float	r;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if (skill->value == 3)
		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = anox_get_move_by_name(self, "hit_a");
}

void anox_pain (edict_t *self, edict_t *other, float kick, int damage)
{
}
SFPEnt(pain, anox_pain)
SFPEnt(pain, anox_pain_with_anim)
SFPEnt(die,anox_die_with_combat_anim)
SFPEnt(monsterinfo.stand, anox_stand)
SFPEnt(monsterinfo.get_currentmove,anox_get_currentmove)
SFPEnt(monsterinfo.walk, anox_walk)
SFPEnt(monsterinfo.run,anox_run)
SFPEnt(monsterinfo.custom_anim,anox_custom_anim)
void anox_setup_monster(edict_t *self)
{
	monsterinfo_t	*minfo;
	int i, j;
	md2_info_t *info;
	anox_entity_desc_t	*desc = self->anox;

	anox_setup_moves(self->anox);

	// Now we need to setup the monsterinfo structure
	self->monsterinfo.aiflags |= AI_GOOD_GUY;
	self->monsterinfo.scale = 1.0;
	self->die = SFP::anox_die;
	self->pain = SFP::anox_pain;

	if (anox_get_move_by_name(self,"die_a") != 1) 
		self->die = SFP::anox_die_with_anim;
	else
	{
		char	new_class[MAX_QPATH];
		anox_entity_desc_t	*new_desc = 0;

		strcpy(new_class, self->classname);
		strcat(new_class, "_com");

		for (new_desc = anox_entities; new_desc != nullptr; new_desc = new_desc->next) 
		{
			if (!new_desc->classname) continue;
			if (!Q_strcasecmp(new_desc->classname, new_class)) 
			{
				break;
			}
		}

		if (new_desc)
		{
			anox_setup_moves(new_desc);
			self->die = SFP::anox_die_with_combat_anim;
		}
	}

	if (anox_get_move_by_name(self,"hit_a") != 1) 
		self->pain = SFP::anox_pain_with_anim;

	self->monsterinfo.get_currentmove = SFP::anox_get_currentmove;
	self->monsterinfo.stand = SFP::anox_stand;
	self->monsterinfo.walk = SFP::anox_walk;
	self->monsterinfo.run = SFP::anox_run;
	self->monsterinfo.attack = nullptr;
	self->monsterinfo.melee = nullptr;
	self->monsterinfo.sight = nullptr;

	self->monsterinfo.custom_anim = SFP::anox_custom_anim;

	self->health = 100;
	self->gib_health = -50;
	self->mass = 300;

	anox_stand(self);
}
AutoSFP(ai_charge)
AutoSFP(ai_stand)
AutoSFP(ai_walk)
AutoSFP(ai_move)
AutoSFP(ai_run)
AutoSFP(ai_turn)

AutoSFP(anox_dead)
void anox_setup_moves(anox_entity_desc_t *desc)
{
	qboolean found_walk_a = false;
	qboolean found_run_a = false;
	mframe_t *all_frames;
	monsterinfo_t	*minfo;
	int i, j;
	md2_info_t *info;

	// Need to create the mmove_t structures?
	if (desc->moves) return;

	// Get the info
	info = gi.GetModelInfo(gi.modelindex(desc->model_path));

	// We're screwed
	if (!info) Sys_Error("Couldn't get model info for anox monster\n");


	gi.TTagMalloc(desc->moves,(info->num_framesets+2)*sizeof(mmove_t), TAG_GAME);
	gi.TTagMalloc(all_frames,info->num_frames*sizeof(mframe_t), TAG_GAME);

	for (i = 0; i < info->num_framesets; i++)
	{
		decltype(desc->moves[i].frame[j].aifunc)func;
		float	dist;

		desc->moves[i].endfunc = 0;
		desc->moves[i].firstframe = info->frameset[i].first_frame;
		desc->moves[i].lastframe = info->frameset[i].first_frame + info->frameset[i].num_frames-1;
		desc->moves[i].frame = &all_frames[info->frameset[i].first_frame];

		if (Q_strcasecmp(info->frameset[i].anim, "walk") == 0)
		{
			func = SFP::ai_walk;
			dist = desc->walk_speed * FRAMETIME;
			found_walk_a = true;
		}
		else if (Q_strcasecmp(info->frameset[i].anim, "run") == 0)
		{
			func = SFP::ai_run;
			dist = desc->run_speed * FRAMETIME;
			found_run_a = true;
		}
		else if (Q_strcasecmp(info->frameset[i].anim, "die") == 0)
		{
			desc->moves[i].endfunc = SFP::anox_dead;
			func = SFP::ai_move;
			dist = 0;
		}
		else if (Q_strcasecmp(info->frameset[i].anim, "hit") == 0)
		{
			desc->moves[i].endfunc = SFP::anox_stand;
			func = SFP::ai_move;
			dist = 0;
		}
		else
		{
			func = SFP::ai_stand;
			dist = 0;
		}

		for (j = 0; j < info->frameset[i].num_frames; j++)
		{
			desc->moves[i].frame[j].aifunc = func;
			desc->moves[i].frame[j].dist = dist;
			desc->moves[i].frame[j].thinkfunc = nullptr;
		}
	}

	if (!found_walk_a)
	{
		decltype(desc->moves[i].frame[j].aifunc) func;
		float	dist;

		i = info->num_framesets;
		desc->moves[i].endfunc = 0;
		desc->moves[i].firstframe = info->frameset[0].first_frame;
		desc->moves[i].lastframe = info->frameset[0].first_frame + info->frameset[0].num_frames-1;
		gi.TTagMalloc(desc->moves[i].frame,info->frameset[0].num_frames*sizeof(mframe_t), TAG_GAME);

		func = SFP::ai_walk;
		dist = desc->walk_speed * FRAMETIME;

		for (j = 0; j < info->frameset[0].num_frames; j++)
		{
			desc->moves[i].frame[j].aifunc = func;
			desc->moves[i].frame[j].dist = dist;
			desc->moves[i].frame[j].thinkfunc = nullptr;
		}
	}

	if (!found_run_a)
	{
		decltype(desc->moves[i].frame[j].aifunc) func;
		float	dist;

		i = info->num_framesets+1;
		desc->moves[i].endfunc = 0;
		desc->moves[i].firstframe = info->frameset[0].first_frame;
		desc->moves[i].lastframe = info->frameset[0].first_frame + info->frameset[0].num_frames-1;
		 gi.TTagMalloc(desc->moves[i].frame,info->frameset[0].num_frames*sizeof(mframe_t), TAG_GAME);

		func = SFP::ai_run;
		dist = desc->run_speed * FRAMETIME;

		for (j = 0; j < info->frameset[0].num_frames; j++)
		{
			desc->moves[i].frame[j].aifunc = func;
			desc->moves[i].frame[j].dist = dist;
			desc->moves[i].frame[j].thinkfunc = nullptr;
		}
	}
}

// "1,0,effect_1,0,joey/raindrops"
void anox_parse_np_string(edict_t *ent, char *string, int index)
{
	int i;
	char *id, *submodel,*surface,*flags,*file,*s;

	id = string;
	string = strchr(string,',');
	if (!string) return;
	string[0] = 0;
	string++;

	submodel = string;
	string = strchr(string,',');
	if (!string) return;
	string[0] = 0;
	string++;

	surface = string;
	string = strchr(string,',');
	if (!string) return;
	string[0] = 0;
	string++;

	flags = string;
	string = strchr(string,',');
	if (!string) return;
	string[0] = 0;
	string++;

	file = string;
	if (!file[0]) return;

	for (i = 0; i < strlen(file); i++)
		if (file[i] == '\\') file[i] = '/';

	ent->s.np[index] = gi.apdindex(file);
	ent->s.np_tri[index][0] = -1;
	ent->s.np_tri[index][1] = -1;
}
AutoSFP(anox_think)
qboolean SP_misc_anox_spawn (edict_t *ent)
{
	int i ;
	qboolean	is_anim = false;
	anox_entity_desc_t	*desc = ent->anox;
	md2_info_t *info;
	// Need to handle the 'annoying' case of where a profile is specified
	// What we'll do is use the Profile as a FOURCC code and set the skin
	// value to it. 

	if (!desc) 
	{
		G_FreeEdict (ent);	
		return false;
	}

	ent->s.modelindex = gi.modelindex (desc->model_path);

	if (st.npsimple)
	{
		// Switch \ into /
		for (i = 0; i < strlen(st.npsimple); i++)
			if (st.npsimple[i] == '\\') st.npsimple[i] = '/';

		ent->s.np[3] = gi.apdindex(st.npsimple);
	}
	if (st.np_0) anox_parse_np_string(ent, st.np_0, 0);
	if (st.np_1) anox_parse_np_string(ent, st.np_1, 1);
	if (st.np_2) anox_parse_np_string(ent, st.np_2, 2);

	// Hack for mystech to make it use active skins
	if (!Q_strncasecmp(desc->model_path, "models/objects/mystech", strlen("models/objects/mystech")))
		ent->s.skinnum = *(int*)"ACTV";
	else
		ent->s.skinnum = desc->profile;
	//vec3_t		scale;

	info = gi.GetModelInfo(ent->s.modelindex);
	if (!ent->default_anim) ent->default_anim = "amb_a";

	if (info)
	{
		int		i;

		for (i = 0; i < info->num_framesets; i++)
		{
			if (!Q_strcasecmp(info->frameset[i].name, ent->default_anim))
			{
				is_anim = true;
				ent->anim_start = info->frameset[i].first_frame;
				ent->anim_count = info->frameset[i].num_frames;
				break;
			}
		}

	}

	//VectorCopy(desc->mins, ent->mins);
	//VectorCopy(desc->maxs, ent->maxs);

	for (i = 0; i < 3; i++)
	{
		ent->s.mins[i] = desc->mins[i] * ent->s.scale[i];
		ent->s.maxs[i] = desc->maxs[i] * ent->s.scale[i];
	}

	//qboolean	noshadow;			// shadow or noshadow

	//if (desc->solidflag)
		ent->solid = SOLID_BBOX;
	//else 
	//	ent->solid = SOLID_NOT;

	if (desc->blending)
		ent->s.renderfx |= RF_TRANSLUCENT;

	//float		walk_speed;
	//float		run_speed;
	//float		speed;				// ???
	//qboolean	lighting;			// 1 or 0
	//qboolean	blending;			// 1 or 0
	//char		*target_sequence;	// APE sequence to run when used?
	//char		*misc_value;
	//qboolean	no_mip;
	//c/har		*spawn_sequence;	// APE sequence to run when spawned or "none"
	//char		*description;

	/* All the entity types
	charfly
	charhover
	charroll
	effect
	pickup
	container
	keyitem
	scavenger
	trashspawn
	bugspawn
	general
	bipidri
	sprite
	playerchar
	noclip
	floater
	*/

	if ( strcmp(desc->entity_type, "char") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_STEP;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		walkmonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "charroll") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_STEP;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		walkmonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "scavenger") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_STEP;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		walkmonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "bipidri") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_STEP;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		walkmonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "charfly") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_FLY;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		flymonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "floater") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_FLY;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		flymonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if ( strcmp(desc->entity_type, "charhover") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_FLY;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		flymonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if (strcmp(desc->entity_type, "playerchar") == 0 ) 
	{
		is_anim = false;
		ent->movetype = MOVETYPE_STEP;
		ent->clipmask = MASK_PLAYERSOLID;
		anox_setup_monster(ent);
		walkmonster_start(ent);
		ent->s.skinnum = desc->profile;
	}
	else if (strcmp(desc->entity_type, "pickup") == 0 ) 
	{
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_SOLID;
		gi.linkentity (ent);
	}
	else if (strcmp(desc->entity_type, "noclip") == 0 ) 
	{
		ent->solid = SOLID_NOT;
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_SOLID;
		gi.linkentity (ent);
	}
	else if (strcmp(desc->entity_type, "general") == 0 ) 
	{
		ent->solid = SOLID_NOT;
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_SOLID;
		gi.linkentity (ent);
	}
	else 
	{
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_SOLID;
		gi.linkentity (ent);

	}

	if (is_anim)
	{
		ent->think = SFP::anox_think;
		ent->nextthink = level.time + FRAMETIME;
	}

	return true;
}


anox_entity_desc_t *anox_find_desc(char *classname)
{
	anox_entity_desc_t *desc;
	for (desc = anox_entities; desc != nullptr; desc = desc->next) 
	{
		if (!desc->classname) continue;
		if (!Q_strcasecmp(desc->classname, classname)) 
		{
			break;
		}
	}

	return desc;
}
