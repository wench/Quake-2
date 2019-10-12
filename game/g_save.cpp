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

#include "g_local.h"

#define Function(f) {#f, f}

mmove_t mmove_reloc;

template<typename T>
static void* constructinplace(void* where, void* what)
{
	if (where)
	{
		if (what) return new(where) T(*(T*)what);
		else return new(where) T;
	}
	if (what) return new T(*(T*)what);
	else return new T;
}
template<>
static void* constructinplace<vec3_t>(void* where, void* what)
{
	if (where)
	{
		if (what) {
			memcpy(where, what, sizeof(vec3_t));
			where;
		}
		else return where;
	}
	where = new vec3_t();
	if (what) {
		memcpy(where, what, sizeof(vec3_t));
	}
	return where;
}
#include<type_traits>
#define FCTOR(fieldname) constructinplace<std::remove_reference<decltype(((edict_s *)0)->fieldname)>::type>
#define LLCTOR(fieldname) constructinplace<std::remove_reference<decltype(((level_locals_t *)0)->fieldname)>::type>
#define CLCTOR(fieldname) constructinplace<std::remove_reference<decltype(((gclient_t *)0)->fieldname)>::type>
#define FTYPEID(fieldname) &typeid(std::remove_reference<decltype(((edict_s *)0)->fieldname)>::type)
#define LLTYPEID(fieldname) &typeid(std::remove_reference<decltype(((level_locals_t *)0)->fieldname)>::type)
#define CLTYPEID(fieldname) &typeid(std::remove_reference<decltype(((gclient_t *)0)->fieldname)>::type)

field_t fields[] = {
	{"classname", FOFS(classname), FCTOR(classname), FTYPEID(classname),F_LSTRING},
	{"model", FOFS(model), FCTOR(model), FTYPEID(model),F_LSTRING},
	{"mapedict", FOFS(mapedict), FCTOR(mapedict), FTYPEID(mapedict),F_LSTRING,FFL_NOSPAWN},
	{"spawnflags", FOFS(spawnflags), FCTOR(spawnflags), FTYPEID(spawnflags),F_INT},
	{"speed", FOFS(speed), FCTOR(speed), FTYPEID(speed),F_FLOAT},
	{"accel", FOFS(accel), FCTOR(accel), FTYPEID(accel),F_FLOAT},
	{"decel", FOFS(decel), FCTOR(decel), FTYPEID(decel),F_FLOAT},
	{"target", FOFS(target), FCTOR(target), FTYPEID(target),F_LSTRING},
	{"targetname", FOFS(targetname), FCTOR(targetname), FTYPEID(targetname),F_LSTRING},
	{"pathtarget", FOFS(pathtarget), FCTOR(pathtarget), FTYPEID(pathtarget),F_LSTRING},
	{"deathtarget", FOFS(deathtarget), FCTOR(deathtarget), FTYPEID(deathtarget),F_LSTRING},
	{"killtarget", FOFS(killtarget), FCTOR(killtarget), FTYPEID(killtarget),F_LSTRING},
	{"combattarget", FOFS(combattarget), FCTOR(combattarget), FTYPEID(combattarget),F_LSTRING},
	{"message", FOFS(message), FCTOR(message), FTYPEID(message),F_LSTRING},
	{"team", FOFS(team), FCTOR(team), FTYPEID(team),F_LSTRING},
	{"wait", FOFS(wait), FCTOR(wait), FTYPEID(wait),F_FLOAT},
	{"delay", FOFS(delay), FCTOR(delay), FTYPEID(delay),F_FLOAT},
	{"random", FOFS(random), FCTOR(random), FTYPEID(random),F_FLOAT},
	{"move_origin", FOFS(move_origin), FCTOR(move_origin), FTYPEID(move_origin),F_VECTOR},
	{"move_angles", FOFS(move_angles), FCTOR(move_angles), FTYPEID(move_angles),F_VECTOR},
	{"style", FOFS(style), FCTOR(style), FTYPEID(style),F_INT},
	{"count", FOFS(count), FCTOR(count), FTYPEID(count),F_INT},
	{"health", FOFS(health), FCTOR(health), FTYPEID(health),F_INT},
	{"sounds", FOFS(sounds), FCTOR(sounds), FTYPEID(sounds),F_INT},
	{"light", 0, nullptr, &typeid(nullptr_t), F_IGNORE},
	{"dmg", FOFS(dmg), FCTOR(dmg), FTYPEID(dmg),F_INT},
	{"mass", FOFS(mass), FCTOR(mass), FTYPEID(mass),F_INT},
	{"volume", FOFS(volume), FCTOR(volume), FTYPEID(volume),F_FLOAT},
	{"attenuation", FOFS(attenuation), FCTOR(attenuation), FTYPEID(attenuation),F_FLOAT},
	{"map", FOFS(map), FCTOR(map), FTYPEID(map),F_LSTRING},
	{"origin", FOFS(s.origin), FCTOR(s.origin), FTYPEID(s.origin),F_VECTOR},
	{"angles", FOFS(s.angles), FCTOR(s.angles), FTYPEID(s.angles),F_VECTOR},
	{"angle", FOFS(s.angles), FCTOR(s.angles), FTYPEID(s.angles),F_ANGLEHACK},
	{"scale", FOFS(s.scale), FCTOR(s.scale), FTYPEID(s.scale),F_VECTOR},
	{"r", FOFS(s.rgb[0]), FCTOR(s.rgb[0]), FTYPEID(s.rgb[0]),F_FLOAT},
	{"g", FOFS(s.rgb[1]), FCTOR(s.rgb[1]), FTYPEID(s.rgb[1]),F_FLOAT},
	{"b", FOFS(s.rgb[2]), FCTOR(s.rgb[2]), FTYPEID(s.rgb[2]),F_FLOAT},
	{"rgb", FOFS(s.rgb), FCTOR(s.rgb), FTYPEID(s.rgb),F_VECTOR},
	{"offset", FOFS(s.offset), FCTOR(s.offset), FTYPEID(s.offset),F_VECTOR},

	{"goalentity", FOFS(goalentity), FCTOR(goalentity), FTYPEID(goalentity),F_EDICT, FFL_NOSPAWN},
	{"movetarget", FOFS(movetarget), FCTOR(movetarget), FTYPEID(movetarget),F_EDICT, FFL_NOSPAWN},
	{"enemy", FOFS(enemy), FCTOR(enemy), FTYPEID(enemy),F_EDICT, FFL_NOSPAWN},
	{"oldenemy", FOFS(oldenemy), FCTOR(oldenemy), FTYPEID(oldenemy),F_EDICT, FFL_NOSPAWN},
	{"activator", FOFS(activator), FCTOR(activator), FTYPEID(activator),F_EDICT, FFL_NOSPAWN},
	{"groundentity", FOFS(groundentity), FCTOR(groundentity), FTYPEID(groundentity),F_EDICT, FFL_NOSPAWN},
	{"teamchain", FOFS(teamchain), FCTOR(teamchain), FTYPEID(teamchain),F_EDICT, FFL_NOSPAWN},
	{"teammaster", FOFS(teammaster), FCTOR(teammaster), FTYPEID(teammaster),F_EDICT, FFL_NOSPAWN},
	{"owner", FOFS(owner), FCTOR(owner), FTYPEID(owner),F_EDICT, FFL_NOSPAWN},
	{"mynoise", FOFS(mynoise), FCTOR(mynoise), FTYPEID(mynoise),F_EDICT, FFL_NOSPAWN},
	{"mynoise2", FOFS(mynoise2), FCTOR(mynoise2), FTYPEID(mynoise2),F_EDICT, FFL_NOSPAWN},
	{"target_ent", FOFS(target_ent), FCTOR(target_ent), FTYPEID(target_ent),F_EDICT, FFL_NOSPAWN},
	{"chain", FOFS(chain), FCTOR(chain), FTYPEID(chain),F_EDICT, FFL_NOSPAWN},

	{"prethink", FOFS(prethink), FCTOR(prethink), FTYPEID(prethink),F_FUNCTION, FFL_NOSPAWN},
	{"think", FOFS(think), FCTOR(think), FTYPEID(think),F_FUNCTION, FFL_NOSPAWN},
	{"blocked", FOFS(blocked), FCTOR(blocked), FTYPEID(blocked),F_FUNCTION, FFL_NOSPAWN},
	{"touch", FOFS(touch), FCTOR(touch), FTYPEID(touch),F_FUNCTION, FFL_NOSPAWN},
	{"use", FOFS(use), FCTOR(use), FTYPEID(use),F_FUNCTION, FFL_NOSPAWN},
	{"pain", FOFS(pain), FCTOR(pain), FTYPEID(pain),F_FUNCTION, FFL_NOSPAWN},
	{"die", FOFS(die), FCTOR(die), FTYPEID(die),F_FUNCTION, FFL_NOSPAWN},

	{"stand", FOFS(monsterinfo.stand), FCTOR(monsterinfo.stand), FTYPEID(monsterinfo.stand),F_FUNCTION, FFL_NOSPAWN},
	{"idle", FOFS(monsterinfo.idle), FCTOR(monsterinfo.idle), FTYPEID(monsterinfo.idle),F_FUNCTION, FFL_NOSPAWN},
	{"search", FOFS(monsterinfo.search), FCTOR(monsterinfo.search), FTYPEID(monsterinfo.search),F_FUNCTION, FFL_NOSPAWN},
	{"walk", FOFS(monsterinfo.walk), FCTOR(monsterinfo.walk), FTYPEID(monsterinfo.walk),F_FUNCTION, FFL_NOSPAWN},
	{"run", FOFS(monsterinfo.run), FCTOR(monsterinfo.run), FTYPEID(monsterinfo.run),F_FUNCTION, FFL_NOSPAWN},
	{"dodge", FOFS(monsterinfo.dodge), FCTOR(monsterinfo.dodge), FTYPEID(monsterinfo.dodge),F_FUNCTION, FFL_NOSPAWN},
	{"attack", FOFS(monsterinfo.attack), FCTOR(monsterinfo.attack), FTYPEID(monsterinfo.attack),F_FUNCTION, FFL_NOSPAWN},
	{"melee", FOFS(monsterinfo.melee), FCTOR(monsterinfo.melee), FTYPEID(monsterinfo.melee),F_FUNCTION, FFL_NOSPAWN},
	{"sight", FOFS(monsterinfo.sight), FCTOR(monsterinfo.sight), FTYPEID(monsterinfo.sight),F_FUNCTION, FFL_NOSPAWN},
	{"checkattack", FOFS(monsterinfo.checkattack), FCTOR(monsterinfo.checkattack), FTYPEID(monsterinfo.checkattack),F_FUNCTION, FFL_NOSPAWN},
	{"get_currentmove", FOFS(monsterinfo.get_currentmove), FCTOR(monsterinfo.get_currentmove), FTYPEID(monsterinfo.get_currentmove),F_FUNCTION, FFL_NOSPAWN},
	{"custom_anim", FOFS(monsterinfo.custom_anim), FCTOR(monsterinfo.custom_anim), FTYPEID(monsterinfo.custom_anim),F_FUNCTION, FFL_NOSPAWN},

	{"endfunc", FOFS(moveinfo.endfunc), FCTOR(moveinfo.endfunc), FTYPEID(moveinfo.endfunc),F_FUNCTION, FFL_NOSPAWN},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), 0,&typeid(int),F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), 0,&typeid(int),F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), 0,&typeid(int),F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), 0,&typeid(const char *),F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime),0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item),0,&typeid(const char *),F_LSTRING, FFL_SPAWNTEMP},

//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	{"item", FOFS(item), FCTOR(item), FTYPEID(item),F_ITEM},

	{"gravity", STOFS(gravity), 0,&typeid(const char *),F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), 0,&typeid(const char *),F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), 0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), 0, &typeid(float[3]),F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), 0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), 0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch),0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch),0, &typeid(float), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap),0,&typeid(const char *),F_LSTRING, FFL_SPAWNTEMP},

	// Aquakronox fields

	// New Particles stuff

	// Simple: filename
	{"npsimple", STOFS(npsimple),0,&typeid(const char *),F_LSTRING2, FFL_SPAWNTEMP  },	
	// Complex: id,submodel,surf,flags,filename
	{"np", STOFS(np_0), 0,&typeid(const char *),F_LSTRING2, FFL_SPAWNTEMP },		
	{"np_1", STOFS(np_1), 0,&typeid(const char *),F_LSTRING2, FFL_SPAWNTEMP },
	{"np_2", STOFS(np_2),0,&typeid(const char *),F_LSTRING2, FFL_SPAWNTEMP },
    
	{"newscaling", FOFS(newscaling), FCTOR(newscaling), FTYPEID(newscaling),F_INT },
	{"pathanim", FOFS(pathanim), FCTOR(pathanim), FTYPEID(pathanim),F_LSTRING2 },		// Anim to play when reached pathh
	{"sequence", FOFS(sequence), FCTOR(sequence), FTYPEID(sequence),F_LSTRING2 },		// APE Sequence 
	{"falloff", FOFS(falloff), FCTOR(falloff), FTYPEID(falloff),F_FLOAT },			// Sound Falloff
	{"spawncondition", STOFS(spawncondition),0,&typeid(const char *),F_LSTRING2, FFL_SPAWNTEMP} ,
	{"default_anim", FOFS(default_anim), FCTOR(default_anim), FTYPEID(default_anim),F_LSTRING2 },
	{"defualt_anim", FOFS(default_anim), 0,&typeid(const char *),F_LSTRING2, FFL_NOSAVE },
	
	{0, 0, 0,&typeid(int), F_IGNORE, 0}

};

field_t		levelfields[] =
{
	{"changemap", LLOFS(changemap), LLCTOR(changemap), LLTYPEID(changemap),F_LSTRING},
                   
	{"sight_client", LLOFS(sight_client), LLCTOR(sight_client), LLTYPEID(sight_client),F_EDICT},
	{"sight_entity", LLOFS(sight_entity), LLCTOR(sight_entity), LLTYPEID(sight_entity),F_EDICT},
	{"sound_entity", LLOFS(sound_entity), LLCTOR(sound_entity), LLTYPEID(sound_entity),F_EDICT},
	{"sound2_entity", LLOFS(sound2_entity), LLCTOR(sound2_entity), LLTYPEID(sound2_entity),F_EDICT},

	{"changemap_target", LLOFS(changemap_target), LLCTOR(changemap_target), LLTYPEID(changemap_target),F_LSTRING},

	{nullptr, 0,0, &typeid(int), F_INT}
};

field_t		clientfields[] =
{
	{"pers.weapon", CLOFS(pers.weapon), CLCTOR(pers.weapon), CLTYPEID(pers.weapon), F_ITEM},
	{"pers.lastweapon", CLOFS(pers.lastweapon), CLCTOR(pers.lastweapon), CLTYPEID(pers.lastweapon), F_ITEM},
	{"newweapon", CLOFS(newweapon), CLCTOR(newweapon), CLTYPEID(newweapon), F_ITEM},

	{nullptr, 0,0,&typeid(int), F_INT}
};

/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is started or a save game
is loaded.
============
*/
void InitGame (void)
{
	gi.dprintf ("==== InitGame ====\n");

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "0", CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_SERVERINFO|CVAR_LATCH);
	gi.cvar ("gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__ , CVAR_SERVERINFO | CVAR_LATCH);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	maxspectators = gi.cvar ("maxspectators", "4", CVAR_SERVERINFO);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	password = gi.cvar ("password", "", CVAR_USERINFO);
	spectator_password = gi.cvar ("spectator_password", "", CVAR_USERINFO);
	needpass = gi.cvar ("needpass", "0", CVAR_SERVERINFO);
	filterban = gi.cvar ("filterban", "1", 0);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);

	// dm map list
	sv_maplist = gi.cvar ("sv_maplist", "", 0);

	// items
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");

	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  new(TAG_GAME)edict_t[game.maxentities ];
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = new(TAG_GAME) gclient_t[game.maxclients];
	globals.num_edicts = game.maxclients+1;

	// Make sure anox entities are loaded
	ANOX_load_entity_descs();
}

//=========================================================

void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p=0;
	size_t			len=0;
	int			index=0;

	if (field->flags & (FFL_NOSAVE|FFL_SPAWNTEMP))
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_LSTRING2:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = (int)len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == nullptr)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == nullptr)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == nullptr)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = index;
		break;

	//relative to code segment
	case F_FUNCTION:
	{
		assert(strstr(field->tinfo->name(), "SerializableFunctionPointer"));
		ISerializableFunctionPointer* sfp = (ISerializableFunctionPointer*)p;
		const char* name = sfp->get_name();
		if(name) len = strlen(name)+1;
		*(int*)p = len;
	}
	break;

	//relative to data segment
	case F_MMOVE:
		if (*(byte **)p == nullptr)
			index = 0;
		else
			index = *(byte **)p - (byte *)&mmove_reloc;
		*(int *)p = index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}


void WriteField2 (FILE *f, field_t *field, byte *base)
{
	size_t			len;
	void		*p;

	if (field->flags & (FFL_NOSAVE|FFL_SPAWNTEMP) )
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_LSTRING:
	case F_LSTRING2:
		if (*(char**)p)
		{
			len = strlen(*(char**)p) + 1;
			fwrite(*(char**)p, len, 1, f);
		}
		break;

		case F_FUNCTION:
		if (p)
		{
			ISerializableFunctionPointer* sfp = (ISerializableFunctionPointer*)p;
			const char* name = sfp->get_name();
			if (name) {
				len = strlen(name) + 1;
				fwrite(name, len, 1, f);
			}
		}
		break;
}
}

void ReadField (FILE *f, field_t *field, byte *base)
{
	void		*p;
	int			len;
	int			index;

	if (field->flags & (FFL_NOSAVE|FFL_SPAWNTEMP) )
		return;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_LSTRING2:
		len = *(int *)p;
		if (!len)
			*(char **)p = nullptr;
		else
		{
			*(char **)p = new(TAG_LEVEL) char[len];
			assert(p);
			fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = nullptr;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = nullptr;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = nullptr;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	//relative to code segment
	case F_FUNCTION:/*
		index = *(int *)p;
		if ( index == 0 )
			*(byte **)p = nullptr;
		else
			*(byte **)p = ((byte *)InitGame) + index;*/
	{
		len = *(int*)p;
		if (!len)
			* (char**)p = nullptr;
		else
		{
			char*name = new(TAG_LEVEL) char[len];
			fread(name, len, 1, f);
			auto sfp = ISerializableFunctionPointer::GetInterface(name);
			delete[]name;
			field->construct(p, sfp);
		}
	}
		break;

	//relative to data segment
	case F_MMOVE:
		index = *(int *)p;
		if (index == 0)
			*(byte **)p = nullptr;
		else
			*(byte **)p = (byte *)&mmove_reloc + index;
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;

	fread (client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i=0 ; i<game.maxclients ; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];

	gi.FreeTags (TAG_GAME);

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts = new(TAG_GAME)edict_t[game.maxentities];
	globals.edicts = g_edicts;

	fread (&game, sizeof(game), 1, f);
	game.clients = new(TAG_GAME) gclient_t[game.maxclients];
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=fields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=fields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;

	fread (ent, sizeof(*ent), 1, f);

	for (field=fields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
void ReadLevelLocals (FILE *f)
{
	field_t		*field;

	fread (&level, sizeof(level), 1, f);

	for (field=levelfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	*base;

	f = fopen (filename, "wb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = (void *)InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	*base;
	edict_t	*ent;
	anox_entity_desc_t *anox;

	f = fopen (filename, "rb");
	if (!f)
		gi.error ("Couldn't open %s", filename);

	// free any dynamic memory allocated by loading the level
	// base state
	gi.WipeModelInfo();
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value+1;

	// check edict size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	fread (&base, sizeof(base), 1, f);
#ifdef _WIN32
	if (base != (void *)InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}
#else
	gi.dprintf("Function offsets %d\n", ((byte *)base) - ((byte *)InitGame));
#endif

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		if (fread (&entnum, sizeof(entnum), 1, f) != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		ent->anox = 0;
		if (ent->classname)
		{
			// fire any cross-level triggers
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;

			for (anox = anox_entities; anox != nullptr; anox = anox->next) 
			{
				if (!anox->classname) continue;
				if (!Q_strcasecmp(anox->classname, ent->classname)) 
				{
					ent->anox = anox;
					// Resetup monsters
					if (Q_strcasecmp(anox->entity_type, "char") == 0 || 
						Q_strcasecmp(anox->entity_type, "charhover") == 0 ||
						Q_strcasecmp(anox->entity_type, "charroll") == 0 ||
						Q_strcasecmp(anox->entity_type, "charfly") == 0 ||
						Q_strcasecmp(anox->entity_type, "scavenger") == 0 ||
						Q_strcasecmp(anox->entity_type, "bipidri") == 0 ||
						Q_strcasecmp(anox->entity_type, "floater") == 0 ||
						Q_strcasecmp(anox->entity_type, "playerchar") == 0) 
					{
						anox_setup_moves(anox);
					}

					break;
				}
			}
		}
	}
}
