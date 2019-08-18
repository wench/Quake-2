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

#include "server.h"

server_static_t	svs;				// persistant server info
server_t		sv;					// local server

/*
================
SV_FindIndex

================
*/
int SV_FindIndex (char *name, int start, int max, qboolean create)
{
	int		i;
	
	if (!name || !name[0])
		return 0;

	for (i=1 ; i<max && sv.configstrings[start+i][0] ; i++)
		if (!strcmp(sv.configstrings[start+i], name))
			return i;

	if (!create)
		return 0;

	if (i == max)
		Com_Error (ERR_DROP, "*Index: overflow");

	strncpy (sv.configstrings[start+i], name, sizeof(sv.configstrings[i]));

	if (sv.state != ss_loading)
	{	// send the update to everyone
		SZ_Clear (&sv.multicast);
		MSG_WriteChar (&sv.multicast, svc_configstring);
		MSG_WriteShort (&sv.multicast, start+i);
		MSG_WriteString (&sv.multicast, name);
		SV_Multicast (vec3_origin, MULTICAST_ALL_R);
	}

	return i;
}


int SV_ModelIndex (char *name)
{
	return SV_FindIndex (name, CS_MODELS, MAX_MODELS, true);
}

int SV_SoundIndex (char *name)
{
	return SV_FindIndex (name, CS_SOUNDS, MAX_SOUNDS, true);
}

int SV_ImageIndex (char *name)
{
	return SV_FindIndex (name, CS_IMAGES, MAX_IMAGES, true);
}

int SV_APDIndex (char *name)
{
	return SV_FindIndex (name, CS_APD, MAX_APD, true);
}

// For an MDA we just go find the MD2 file, then pass it onto SV_LoadAliasInfo
static md2_info_t *SV_LoadAliasInfo (void *buffer, int modfilelen)
{
	int					i, j;
	dmdl_anox_t			*header;
	int					version;
	md2_info_t			*info;
	md2_frameset_t		*f;
	daliasframe_t		*pinframe;
	int					num_frames;
	int					framesize;
	int					ofs_frames;
	int					len;
	char				prevname[17];
	int					prevlen;
	
	// Note anox header will work, just the extra anox fields wont be valid for Q2 MD2s
	header = (dmdl_anox_t *)buffer;

	version = LittleLong (header->version);

	// Just verify version
	if (version != ALIAS_VERSION && version != ALIAS_VERSION_ANOX_3_BYTE && 
		version != ALIAS_VERSION_ANOX_4_BYTE && version != ALIAS_VERSION_ANOX_6_BYTE &&
		version != ALIAS_VERSION_ANOX_OLD )
		return NULL;

	// Number of frames
	num_frames = LittleLong (header->num_frames);
	if (num_frames <= 0) return NULL;

	framesize = LittleLong (header->framesize);
	// If it's less than 44 bytes, it can't possibly be a valid model (even 1 point needs more bytes)
	if (framesize < 44) return NULL;

	ofs_frames = LittleLong (header->ofs_frames);

	info = Z_TagMalloc(sizeof(md2_info_t), 766);
	info->num_framesets = 0;
	info->num_frames = num_frames;
	info->framename = Z_TagMalloc(num_frames*sizeof(char*), 766);

	//
	// Pass 1.... Ickness going through each frame to count animation sets
	//

	prevname[0] = 0;
	prevlen = 0;
	for (i = 0; i < num_frames; i++)
	{
		pinframe = (daliasframe_t*) ((byte*)header + ofs_frames + i*framesize);

		pinframe->name[16] = 0;
		len = strlen(pinframe->name);
		info->framename[i] = Z_TagMalloc((len+1)*sizeof(char), 766);
		memcpy(info->framename[i], pinframe->name, len+1);

		// Get find first character of the number
		if (len > 4)
		{
			// Anox and Quake 2 use different methods for their numbers. 
			// Anox uses ANIM_t_001 while quake 2 uses ANIMt01 (where t is the type if it's a number)

			if (version != ALIAS_VERSION)
			{
				// Anox, length must be greater than 7
				if (len > 7)
				{
					char a,b,c,u1,t,u2;
					a = pinframe->name[len-1];
					b = pinframe->name[len-2];
					c = pinframe->name[len-3];
					u1 = pinframe->name[len-4];
					t = pinframe->name[len-5];
					u2 = pinframe->name[len-6];

					// Make sure all are correct
					if (a >= '0' && a <= '9' &&
						b >= '0' && b <= '9' &&
						c >= '0' && c <= '9' &&
						t >= 'a' && t <= 'z' &&
						u1 == '_' && u2 == '_')
					{
						len -= 4;
					}
				}
			}
			else
			{
				//Quake 2 must be more than 3
				char a,b;
				a = pinframe->name[len-1];
				b = pinframe->name[len-2];
				if (a >= '0' && a <= '9')
				{
					len --;
					if (b >= '0' && b <= '9') len --;
				}
			}
		}

		// New set?
		if (prevlen != len || memcmp(prevname, pinframe->name, len))
		{
			prevlen = len;
			memcpy(prevname, pinframe->name, len);
			prevname[len] = 0;
			info->num_framesets++;
		}
	}

	//
	// Pass 2.... actually copy the info
	//
	info->frameset = Z_TagMalloc(info->num_framesets * sizeof(md2_frameset_t), 766);

	prevname[0] = 0;
	prevlen = 0;
	f = 0;
	j = 0;
	for (i = 0; i < num_frames; i++)
	{
		char type = 0;
		int anim_name_len;

		pinframe = (daliasframe_t*) ((byte*)header + ofs_frames + i*framesize);

		anim_name_len = len = strlen(info->framename[i]);

		// Get find first character of the number
		if (len > 4)
		{
			// Anox and Quake 2 use different methods for their numbers. 
			// Anox uses ANIM_t_001 while quake 2 uses ANIMt01 (where t is the type if it's a number)

			if (version != ALIAS_VERSION)
			{
				// Anox, length must be greater than 7
				if (len > 7)
				{
					char a,b,c,u1,t,u2;
					a = pinframe->name[len-1];
					b = pinframe->name[len-2];
					c = pinframe->name[len-3];
					u1 = pinframe->name[len-4];
					t = pinframe->name[len-5];
					u2 = pinframe->name[len-6];

					// Make sure all are correct
					if (a >= '0' && a <= '9' &&
						b >= '0' && b <= '9' &&
						c >= '0' && c <= '9' &&
						t >= 'a' && t <= 'z' &&
						u1 == '_' && u2 == '_')
					{
						anim_name_len = len-6;
						type = t;
						len -= 4;
					}
				}
			}
			else
			{
				//Quake 2 must be more than 3
				char a,b,c;
				a = pinframe->name[len-1];
				b = pinframe->name[len-2];
				c = pinframe->name[len-3];
				if (a >= '0' && a <= '9')
				{
					len --;
					anim_name_len--;
					if (b >= '0' && b <= '9') 
					{
						len --;
						anim_name_len--;
						if (c >= '0' && c <= '9') 
						{
							type = c;
							anim_name_len--;
						}
					}
				}
			}
		}

		// New set?
		if (prevlen != len || memcmp(prevname, pinframe->name, len))
		{
			// Finish off prev frameset, now set up new
			if (f) f->num_frames = i - f->first_frame;
			
			// Start new one
			f = info->frameset + j;
			f->first_frame = i;
			f->subtype = type;
			memcpy(f->name, pinframe->name, len);
			f->name[len] = 0;
			memcpy(f->anim, pinframe->name, anim_name_len);
			f->anim[anim_name_len] = 0;

			prevlen = len;
			memcpy(prevname, pinframe->name, len);
			prevname[len] = 0;
			j++;
		}
	}

	// Finish off final frameset
	f->num_frames = i - f->first_frame;

	return info;
}

// For an MDA we just go find the MD2 file, then pass it onto SV_LoadAliasInfo
static md2_info_t *SV_LoadMDAInfo (void *buffer, int modfilelen)
{
	char		*data, *mdastring;
	char		*token;
	qboolean	loaded_md2 = false;
	qboolean	dead = true;
	qboolean	profiles_generated = false;
	md2_info_t *info = 0;

	// We need to make the MDA file into a NULL terminated string
	mdastring = malloc (modfilelen+1);
	memcpy(mdastring, buffer, modfilelen);
	mdastring[modfilelen] = 0;

	//
	// Parse file for MD2 name
	//

	data = mdastring;
	while (1) 
	{
		token = COM_Parse2 (&data);
		if (!data) break;

		// Basemodel specifies the MD2
		if (Q_stricmp(token, "basemodel") == 0)
		{
			int			size;
			unsigned	*md2buf;

			token = COM_Parse2 (&data);

			// Corrupt MDA
			if (!data) break;		// Screwed

			// Load the md2
			size = FS_LoadFile (token, &md2buf);

			// Dead
			if (md2buf)
			{
				info = SV_LoadAliasInfo (md2buf,size);
				FS_FreeFile(md2buf);
			}

			break;
		}
	}

	// Free the MDA string
	free (mdastring);

	return info;
}

void SV_WipeModelInfo()
{
	memset(&sv.md2_info[0], 0, MAX_MODELS * sizeof(md2_info_t*));
}

md2_info_t *SV_GetModelInfo (int index)
{
	int size;
	char *name;
	byte *buf;
	md2_info_t *info = NULL;

	// Out of range, return NULL
	if (index < 0 || index >= MAX_MODELS) return NULL;

	// Already loaded so return it
	if (sv.md2_info[index]) return sv.md2_info[index];

	// Get filename
	name = sv.configstrings[CS_MODELS+index];

	// Doesn't have a filename, return NULL
	if (!name[0]) return NULL;

	// Is a cmodel
	if (name[0] == '*')
	{
		int i;
		extern int numcmodels;
		static char *cmodel_frame_name = "cmodel";
		i = atoi(name+1);

		// we really shouldn't really fail here
		if (i < 1 || i >= numcmodels) return NULL;

		sv.md2_info[i] = info = Z_TagMalloc(sizeof(md2_info_t), 766);
		VectorCopy(sv.models[i+1]->mins, info->mins);
		VectorCopy(sv.models[i+1]->maxs, info->maxs);
		info->num_frames = 1;
		info->framename = &cmodel_frame_name;
		info->num_framesets = 1;
		info->frameset = Z_TagMalloc(sizeof(md2_frameset_t), 766);
		info->frameset->anim[0] = 0;
		info->frameset->first_frame = 0;
		info->frameset->subtype = 0;
		info->frameset->num_frames = 1;
		strcpy(info->frameset->name,cmodel_frame_name);
		return info;
	}

	size = FS_LoadFile(name, &buf);

	// Doesn't exist?!?!
	if (size == -1) return NULL;

	// If header is not MD2, oh well we can't do anything 
	switch (LittleLong(*(unsigned *)buf))
	{
	case IDALIASHEADER:
		info = SV_LoadAliasInfo (buf, size);
		break;
		
	case IDMDAHEADER:
		info = SV_LoadMDAInfo (buf, size);
		break;
		
	case IDSPRITEHEADER:
		// Can't be bothered at the moment
		break;
	
	default:
		break;
	}

	FS_FreeFile(buf);
	sv.md2_info[index] = info;
	return info;
}


/*
================
SV_CreateBaseline

Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline (void)
{
	edict_t			*svent;
	int				entnum;	

	for (entnum = 1; entnum < ge->num_edicts ; entnum++)
	{
		svent = EDICT_NUM(entnum);
		if (!svent->inuse)
			continue;
		if (!svent->s.modelindex && !svent->s.sound && !svent->s.effects)
			continue;
		svent->s.number = entnum;

		//
		// take current state as baseline
		//
		VectorCopy (svent->s.origin, svent->s.old_origin);
		sv.baselines[entnum] = svent->s;
	}
}


/*
=================
SV_CheckForSavegame
=================
*/
void SV_CheckForSavegame (void)
{
	char		name[MAX_OSPATH];
	FILE		*f;
	int			i;

	if (sv_noreload->value)
		return;

	if (Cvar_VariableValue ("deathmatch"))
		return;

	Com_sprintf (name, sizeof(name), "%s/save/current/%s.sav", FS_Gamedir(), sv.name);
	f = fopen (name, "rb");
	if (!f)
		return;		// no savegame

	fclose (f);

	SV_ClearWorld ();

	// get configstrings and areaportals
	SV_ReadLevelFile ();

	if (!sv.loadgame)
	{	// coming back to a level after being in a different
		// level, so run it for ten seconds

		// rlava2 was sending too many lightstyles, and overflowing the
		// reliable data. temporarily changing the server state to loading
		// prevents these from being passed down.
		server_state_t		previousState;		// PGM

		previousState = sv.state;				// PGM
		sv.state = ss_loading;					// PGM
		for (i=0 ; i<100 ; i++)
			ge->RunFrame ();

		sv.state = previousState;				// PGM
	}
}


/*
================
SV_SpawnServer

Change the server to a new map, taking all connected
clients along with it.

================
*/
void SV_SpawnServer (char *server, char *spawnpoint, server_state_t serverstate, qboolean attractloop, qboolean loadgame)
{
	int			i;
	unsigned	checksum;

	if (attractloop)
		Cvar_Set ("paused", "0");

	Com_Printf ("------- Server Initialization -------\n");

	Com_DPrintf ("SpawnServer: %s\n",server);
	if (sv.demofile)
		fclose (sv.demofile);

	svs.spawncount++;		// any partially connected client will be
							// restarted
	sv.state = ss_dead;
	Com_SetServerState (sv.state);

	// wipe the entire per-level structure
	memset (&sv, 0, sizeof(sv));
	svs.realtime = 0;
	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	// save name for levels that don't set message
	strcpy (sv.configstrings[CS_NAME], server);
	if (Cvar_VariableValue ("deathmatch"))
	{
		sprintf(sv.configstrings[CS_AIRACCEL], "%g", sv_airaccelerate->value);
		pm_airaccelerate = sv_airaccelerate->value;
	}
	else
	{
		strcpy(sv.configstrings[CS_AIRACCEL], "0");
		pm_airaccelerate = 0;
	}

	SZ_Init (&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));

	strcpy (sv.name, server);

	// leave slots at start for clients only
	for (i=0 ; i<maxclients->value ; i++)
	{
		// needs to reconnect
		if (svs.clients[i].state > cs_connected)
			svs.clients[i].state = cs_connected;
		svs.clients[i].lastframe = -1;
	}

	sv.time = 1000;
	
	strcpy (sv.name, server);
	strcpy (sv.configstrings[CS_NAME], server);

	if (serverstate != ss_game)
	{
		sv.models[1] = CM_LoadMap ("", false, &checksum);	// no real map
	}
	else
	{
		Com_sprintf (sv.configstrings[CS_MODELS+1],sizeof(sv.configstrings[CS_MODELS+1]),
			"maps/%s.bsp", server);
		sv.models[1] = CM_LoadMap (sv.configstrings[CS_MODELS+1], false, &checksum);
	}
	Com_sprintf (sv.configstrings[CS_MAPCHECKSUM],sizeof(sv.configstrings[CS_MAPCHECKSUM]),
		"%i", checksum);

	//
	// clear physics interaction links
	//
	SV_ClearWorld ();
	
	for (i=1 ; i< CM_NumInlineModels() ; i++)
	{
		Com_sprintf (sv.configstrings[CS_MODELS+1+i], sizeof(sv.configstrings[CS_MODELS+1+i]),
			"*%i", i);
		sv.models[i+1] = CM_InlineModel (sv.configstrings[CS_MODELS+1+i]);
	}

	//
	// spawn the rest of the entities on the map
	//	

	// precache and static commands can be issued during
	// map initialization
	sv.state = ss_loading;
	Com_SetServerState (sv.state);

	// load and spawn all other entities
	ge->SpawnEntities ( sv.name, CM_EntityString(), spawnpoint );

	// run two frames to allow everything to settle
	ge->RunFrame ();
	ge->RunFrame ();

	// all precaches are complete
	sv.state = serverstate;
	Com_SetServerState (sv.state);
	
	// create a baseline for more efficient communications
	SV_CreateBaseline ();

	// check for a savegame
	SV_CheckForSavegame ();

	// set serverinfo variable
	Cvar_FullSet ("mapname", sv.name, CVAR_SERVERINFO | CVAR_NOSET);

	Com_Printf ("-------------------------------------\n");
}

/*
==============
SV_InitGame

A brand new game has been started
==============
*/
void SV_InitGame (void)
{
	int		i;
	edict_t	*ent;
	char	idmaster[32];

	if (svs.initialized)
	{
		// cause any connected clients to reconnect
		SV_Shutdown ("Server restarted\n", true);
	}
	else
	{
		// make sure the client is down
		CL_Drop ();
		SCR_BeginLoadingPlaque ();
	}

	// get any latched variable changes (maxclients, etc)
	Cvar_GetLatchedVars ();

	svs.initialized = true;

	if (Cvar_VariableValue ("coop") && Cvar_VariableValue ("deathmatch"))
	{
		Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
		Cvar_FullSet ("coop", "0",  CVAR_SERVERINFO | CVAR_LATCH);
	}

	// dedicated servers are can't be single player and are usually DM
	// so unless they explicity set coop, force it to deathmatch
	if (dedicated->value)
	{
		if (!Cvar_VariableValue ("coop"))
			Cvar_FullSet ("deathmatch", "1",  CVAR_SERVERINFO | CVAR_LATCH);
	}

	// init clients
	if (Cvar_VariableValue ("deathmatch"))
	{
		if (maxclients->value <= 1)
			Cvar_FullSet ("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
		else if (maxclients->value > MAX_CLIENTS)
			Cvar_FullSet ("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
	}
	else if (Cvar_VariableValue ("coop"))
	{
		if (maxclients->value <= 1 || maxclients->value > 4)
			Cvar_FullSet ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
#ifdef COPYPROTECT
		if (!sv.attractloop && !dedicated->value)
			Sys_CopyProtect ();
#endif
	}
	else	// non-deathmatch, non-coop is one player
	{
		Cvar_FullSet ("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
#ifdef COPYPROTECT
		if (!sv.attractloop)
			Sys_CopyProtect ();
#endif
	}

	svs.spawncount = rand();
	svs.clients = Z_Malloc (sizeof(client_t)*maxclients->value);
	svs.num_client_entities = maxclients->value*UPDATE_BACKUP*128;
	svs.client_entities = Z_Malloc (sizeof(entity_state_t)*svs.num_client_entities);

	// init network stuff
	NET_Config ( (maxclients->value > 1) );

	// heartbeats will always be sent to the id master
	svs.last_heartbeat = -99999;		// send immediately
	Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
	NET_StringToAdr (idmaster, &master_adr[0]);

	// init game
	SV_InitGameProgs ();
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = EDICT_NUM(i+1);
		ent->s.number = i+1;
		svs.clients[i].edict = ent;
		memset (&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
	}
}


/*
======================
SV_Map

  the full syntax is:

  map [*]<map>$<startspot>+<nextserver>

command from the console or progs.
Map can also be a.cin, .pcx, or .dm2 file
Nextserver is used to allow a cinematic to play, then proceed to
another level:

	map tram.cin+jail_e3
======================
*/
void SV_Map (qboolean attractloop, char *levelstring, qboolean loadgame)
{
	char	level[MAX_QPATH];
	char	*ch;
	int		l;
	char	spawnpoint[MAX_QPATH];

	sv.loadgame = loadgame;
	sv.attractloop = attractloop;

	if (sv.state == ss_dead && !sv.loadgame)
		SV_InitGame ();	// the game is just starting

	strcpy (level, levelstring);

	// if there is a + in the map, set nextserver to the remainder
	ch = strstr(level, "+");
	if (ch)
	{
		*ch = 0;
			Cvar_Set ("nextserver", va("gamemap \"%s\"", ch+1));
	}
	else
		Cvar_Set ("nextserver", "");

	//ZOID special hack for end game screen in coop mode
	if (Cvar_VariableValue ("coop") && !Q_stricmp(level, "victory.pcx"))
		Cvar_Set ("nextserver", "gamemap \"*base1\"");

	// if there is a $, use the remainder as a spawnpoint
	ch = strstr(level, "$");
	if (ch)
	{
		*ch = 0;
		strcpy (spawnpoint, ch+1);
	}
	else
		spawnpoint[0] = 0;

	// skip the end-of-unit flag if necessary
	if (level[0] == '*')
		strcpy (level, level+1);

	l = strlen(level);
	if (l > 4 && !strcmp (level+l-4, ".cin") )
	{
		SCR_BeginLoadingPlaque ();			// for local system
		SV_BroadcastCommand ("changing\n");
		SV_SpawnServer (level, spawnpoint, ss_cinematic, attractloop, loadgame);
	}
	else if (l > 4 && !strcmp (level+l-4, ".dm2") )
	{
		SCR_BeginLoadingPlaque ();			// for local system
		SV_BroadcastCommand ("changing\n");
		SV_SpawnServer (level, spawnpoint, ss_demo, attractloop, loadgame);
	}
	else if (l > 4 && !strcmp (level+l-4, ".pcx") )
	{
		SCR_BeginLoadingPlaque ();			// for local system
		SV_BroadcastCommand ("changing\n");
		SV_SpawnServer (level, spawnpoint, ss_pic, attractloop, loadgame);
	}
	else
	{
		SCR_BeginLoadingPlaque ();			// for local system
		SV_BroadcastCommand ("changing\n");
		SV_SendClientMessages ();
		SV_SpawnServer (level, spawnpoint, ss_game, attractloop, loadgame);
		Cbuf_CopyToDefer ();
	}

	SV_BroadcastCommand ("reconnect\n");
}
