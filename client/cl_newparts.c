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
// cl_newpart.c -- New Particles Anachronox Style

#include "client.h"
#include "../ref_gl/qgl.h"
// 

extern void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up);

cnewparticle_t	*free_newparticles;

cnewparticle_t	newparticles[MAX_PARTICLES];
int			cl_numnewparticles = MAX_PARTICLES;

#define simple_lerp(first, second, frac)  ((first) + ((second)-(first)) * (frac))

#define APDHEADER		(('1'<<24)+('D'<<16)+('P'<<8)+'A')


void CL_FreeEnvelopeChain( np_envelope_t *env )
{
	while (env)
	{
		np_envelope_t *next = env->next;
		free(env);
		env = next;
	}
}

void CL_FreeGeneratorChain (cnp_generator_t	*gen)
{
	while (gen)
	{
		cnp_generator_t	*next = gen->next;

		CL_FreeEnvelopeChain(gen->flow);
		CL_FreeEnvelopeChain(gen->angles[0]);
		CL_FreeEnvelopeChain(gen->angles[1]);
		CL_FreeEnvelopeChain(gen->velocity[0]);
		CL_FreeEnvelopeChain(gen->velocity[1]);

		CL_FreeEnvelopeChain(gen->radius);
		CL_FreeEnvelopeChain(gen->rgba[0]);
		CL_FreeEnvelopeChain(gen->rgba[1]);
		CL_FreeEnvelopeChain(gen->rgba[2]);
		CL_FreeEnvelopeChain(gen->rgba[3]);
		CL_FreeEnvelopeChain(gen->texcoord[0]);
		CL_FreeEnvelopeChain(gen->texcoord[1]);
		CL_FreeEnvelopeChain(gen->texcoord[2]);
		CL_FreeEnvelopeChain(gen->texcoord[3]);

		free(gen);
		gen = next;
	}
}

/*
===============
CL_ClearNewParticles
===============
*/
void CL_ClearNewParticles (void)
{
	int		i;
	
	// Free all generators
	for (i = 1; i < MAX_APD; i++)
	{
		CL_FreeGeneratorChain(cl.part_generators[i]);
		cl.part_generators[i] = NULL;
	}

	free_newparticles = &newparticles[0];

	memset(newparticles, 0, sizeof(cnewparticle_t) * cl_numnewparticles); 

	for (i=0 ;i<cl_numnewparticles ; i++)
		newparticles[i].next = &newparticles[i+1];
	newparticles[cl_numnewparticles-1].next = NULL;
}


/*
===============
CL_RegisterAPD
===============
*/

void CL_GeneratorDefaults(cnp_generator_t *apd)
{
	apd->next = 0;
	apd->first = 0;

	apd->info.src_blend = GL_SRC_ALPHA;
	apd->info.dest_blend = GL_ONE;
	apd->info.depth_write = 0;
	apd->info.depth_func = GL_LESS;
	apd->info.gennorm = GENNORM_SCREEN;
	apd->info.image = NULL;	// ref_gl will automatically use the default newparticle texture if this is null

	apd->resist = 0;
	VectorSet(apd->accel,0,0,0);
	apd->vartime = 0;
	apd->flowmod = FLOWMOD_NONE;
	apd->follow = false;
	apd->randloop[0] = apd->randloop[1] = -1;
	apd->loops = 0;
	apd->rotate[0] = apd->rotate[1] = 0;
	apd->random_angle = false;
	apd->volflags = 0;

	apd->flicker = 0;
	apd->flicker_all = false;
	apd->rand_value = 0;

	apd->gen_time = 0;
	apd->flow = 0;
	apd->angles[0] = apd->angles[1] = 0;
	apd->velocity[0] = apd->velocity[1] = 0;
	apd->burst = 0;

	apd->decay_time = 0;
	apd->radius = 0;
	apd->rgba[0] = apd->rgba[1] = apd->rgba[2] = apd->rgba[3] = 0;
	apd->texcoord[0] = apd->texcoord[1] = apd->texcoord[2] = apd->texcoord[3] = 0;
}

np_envelope_t *CL_GeneratorDefaultEnvelope(np_envelope_t *env, float def)
{
	if (!env)
	{
		env = malloc(sizeof(np_envelope_t));
		env->time = 0;
		env->value = def;
		env->next = NULL;
	}

	return env;
}

np_envelope_t *CL_AddEnvelopeToChain( np_envelope_t *chain, np_envelope_t *env )
{
	np_envelope_t *first = chain;
	if (!chain || chain->time >= env->time)
	{
		//if (!chain)
		//	Com_Printf ("Creating new chain: %i %f\n", env->time, env->value);
		//else
		//	Com_Printf ("Adding env to start of chain: %i %f\n", env->time, env->value);
		env->next = chain;
		return env;
	}

	while (chain->next)
	{
		if (env->time >= chain->time && env->time <= chain->next->time)
			break;

		//Com_Printf ("Skipping: %i %f\n", chain->time, chain->value);

		chain = chain->next;
	}

	//Com_Printf ("Inserting after: %i-%i %f-%f\n", chain->time, env->time, chain->value, env->value );
	env->next = chain->next;
	chain->next = env;

	return first;
}

cnp_generator_t *CL_RegisterAPD (char *name)
{
	int				i, num_gens= 0;
	char			*data = NULL, *tokens = NULL, *token = NULL;
	int				time, flags;
	float			value[4];
	np_envelope_t	*env; 
	cnp_generator_t *gen = NULL;

	i = FS_LoadFile (va("particles/%s.apd",name), (void **)&data);
	if (!data)
	{
		Com_Printf ("Unable to open APD '%s'.\n", name);
		return NULL;
	}

	// Need to make it a null terminated string
	tokens = malloc(i+1);
	memcpy(tokens, data, i);
	tokens[i] = 0;

	FS_FreeFile (data);
	data = tokens;

	// Read header
	if (LittleLong(*(long*)data) != APDHEADER) 
	{
		Com_Printf ("Error: Missing APD header.\n");
		goto error;
	}

	tokens += 4;

	// Loop over till the end of the file
	while (1)
	{
		token = COM_Parse5(&tokens);

			// First token MUST be generator
		if (!gen && !tokens) 
		{ 
			Com_Printf ("Error: No tokens in APD\n");
			goto error;
		}
		else if (!gen && Q_strcasecmp(token,"generator")) 
		{ 
			Com_Printf ("Error: generator wasn't first token, %s was\n", token);
			goto error;
		}
		else if (gen && !tokens) 
		{
			goto end;
		}

		//  generator  : Generator #1
		if (!Q_strcasecmp(token,"generator"))		
		{
			cnp_generator_t *newgen = NULL;

			// Skip over spaces and colon ':' 
			while (*tokens)
			{
				if (*tokens == '\n') break;

				if (*tokens > ' ' && *tokens != ':') break;
				tokens++;
			}

			if (!*tokens)
			{ 
				Com_Printf ("Error: Unexpected end of file reading generator name\n");
				goto error;
			}
			else if (*tokens == '\n')
			{ 
				Com_Printf ("Error: Unexpected end of line reading generator name\n");
				goto error;
			}

			// Read to end of line
			while (*tokens && *tokens != '\n') tokens++;

			if (!*tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file reading generator name\n");
				goto error;
			}
			 
			tokens++;
		    
			// Setting base defaults
			newgen = malloc(sizeof(cnp_generator_t));
			CL_GeneratorDefaults(newgen);
			newgen->next = gen;
			gen = newgen;
			gen->num_generators = ++num_gens;
		}
		// genresist : 1.6
		else if (!Q_strcasecmp(token,"genresist"))	
		{
			token = COM_Parse5(&tokens);

			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

			gen->resist = atof(token);
		}
		// genaccel : 10 : 0 : 0
		else if (!Q_strcasecmp(token,"genaccel"))	
		{
			for (i = 0; i < 3; i++)
			{
				token = COM_Parse5(&tokens);

				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}

				gen->accel[i] = atof(token);
			}
		}
		// genblend : 0x0302 : 0x0303
		else if (!Q_strcasecmp(token,"genblend"))	
		{
			// Source Blend
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.src_blend = strtol(token, NULL, 0);

			// Dest Blend
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.dest_blend = strtol(token, NULL, 0);
		}
		// gendepth : 0 : 0x0203
		else if (!Q_strcasecmp(token,"gendepth"))	
		{
			// Depth Write
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.depth_write = strtol(token, NULL, 0);

			// Depth Func
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.depth_func = strtol(token, NULL, 0);
		}
		// gentexture : PARTICLES\magical_04.png
		else if (!Q_strcasecmp(token,"gentexture"))	
		{
			// Texture filename (spaces???)
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.image = re.RegisterClamped(token);

		}
		// genloop : 3 : 0
		else if (!Q_strcasecmp(token,"genloop"))	
		{
			// Loop count
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->loops = strtol(token, NULL, 0);

			// Skip 2nd number, meaning unknown
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

		}
		// genrandloop : 1000 : 2000
		else if (!Q_strcasecmp(token,"genrandloop"))
		{
			for ( i = 0; i < 2; i++)
			{
				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				gen->randloop[i] = strtol(token, NULL, 0);
			}
		}
		// genrotate : 0.1 : 2 : 1
		else if (!Q_strcasecmp(token,"genrotate"))	
		{
			for ( i = 0; i < 2; i++)
			{
				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				gen->rotate[i] = atof(token);
			}

			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->random_angle = strtol(token, NULL, 0);
		}
		// gennorm : 4
		else if (!Q_strcasecmp(token,"gennorm"))	
		{
			// Normal type
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->info.gennorm = strtol(token, NULL, 0);
		}
		// genflicker : 10 : 1
		else if (!Q_strcasecmp(token,"genflicker"))
		{
			// Flicker Percentage
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->flicker = (strtol(token, NULL, 0)*RAND_MAX)/99;

			// Flicker All
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->flicker_all = strtol(token, NULL, 0);
		}
		// genfollow : 1
		else if (!Q_strcasecmp(token,"genfollow"))
		{
			// Follow
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->follow = strtol(token, NULL, 0);
		}
		// genvol : 1 : 1 : 0 : 1
		else if (!Q_strcasecmp(token,"genvol"))
		{
			gen->volflags = 0;

			for ( i = 0; i < 4; i++)
			{
				while (*tokens)
				{
					if (*tokens == '\n') break;

					if (*tokens > ' ' && *tokens != ':') break;
					tokens++;
				}

				if (!*tokens)
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				else if (*tokens == '\n')
				{ 
					break;
				}

				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				gen->volflags |= strtol(token, NULL, 0)?1<<i:0;
			}
		}
		// genvartime : 1 
		else if (!Q_strcasecmp(token,"genvartime"))
		{
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->vartime = atof(token);
		}
		// genflowmod : 1 
		else if (!Q_strcasecmp(token,"genflowmod"))
		{
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			gen->flowmod = atoi(token);
		}


		//
		// Particle Data
		//

		// alpha    : 304      : 0x00000000 : 124
		else if (!Q_strcasecmp(token,"alpha"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

			env = malloc(sizeof(np_envelope_t));
			env->time = time;
			env->value = strtol(token, NULL, 0) / 255.0;
			env->next = NULL;
			gen->rgba[3] = CL_AddEnvelopeToChain(gen->rgba[3], env);
			if (gen->decay_time < time) gen->decay_time = time;
		}
		// color    : <time>   : <flags>    : <0xRRGGBB>
		else if (!Q_strcasecmp(token,"color"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			i = strtol(token, NULL, 0);
			value[2] = ((i>>0)  & 0xFF)/255.0;	// Blue
			value[1] = ((i>>8)  & 0xFF)/255.0;	// Green
			value[0] = ((i>>16) & 0xFF)/255.0;	// Red

			for (i = 0; i < 3; i++)
			{
				env = malloc(sizeof(np_envelope_t));
				env->time = time;
				env->value = value[i];
				env->next = NULL;
				gen->rgba[i] = CL_AddEnvelopeToChain(gen->rgba[i], env);
			}
			if (gen->decay_time < time) gen->decay_time = time;
		}
		// radius   : 256      : 0x00000000 : 0.400000
		else if (!Q_strcasecmp(token,"radius"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

			env = malloc(sizeof(np_envelope_t));
			env->time = time;
			env->value = atof(token);
			env->next = NULL;
			gen->radius = CL_AddEnvelopeToChain(gen->radius, env);
			if (gen->decay_time < time) gen->decay_time = time;
		}
		// texture  : <time>   : <flags>    : <s1> : <t1> : <s2> : <t2>
		else if (!Q_strcasecmp(token,"texture"))	
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			for (i = 0; i < 4; i++)
			{
				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				value[i] = atof(token);
			}

			for (i = 0; i < 4; i++)
			{
				env = malloc(sizeof(np_envelope_t));
				env->time = time;
				env->value = value[i];
				env->next = NULL;
				gen->texcoord[i] = CL_AddEnvelopeToChain(gen->texcoord[i], env);
			}
			if (gen->decay_time < time) gen->decay_time = time;
		}

		//
		// Spew Data
		//

		// flow     : <time>   : <flags>    : <value> 
		else if (!Q_strcasecmp(token,"flow"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

			env = malloc(sizeof(np_envelope_t));
			env->time = time;
			env->value = atof(token);
			env->next = NULL;
			gen->flow = CL_AddEnvelopeToChain(gen->flow, env);
			if (gen->gen_time < time) gen->gen_time = time;
		}
		// burst    : <time>   : <flags>    : <value>  
		else if (!Q_strcasecmp(token,"burst"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}

			env = malloc(sizeof(np_envelope_t));
			env->time = time;
			env->value = atof(token);
			env->next = NULL;
			gen->burst = CL_AddEnvelopeToChain(gen->burst, env);
			if (gen->gen_time < time) gen->gen_time = time;
		}
		// angle    : <time>   : <flags>    : <mindeg> : <maxdeg>
		else if (!Q_strcasecmp(token,"angle"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			for (i = 0; i < 2; i++)
			{
				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				value[i] = atof(token);
			}

			for (i = 0; i < 2; i++)
			{
				env = malloc(sizeof(np_envelope_t));
				env->time = time;
				env->value = value[i];
				env->next = NULL;
				gen->angles[i] = CL_AddEnvelopeToChain(gen->angles[i], env);
			}
			if (gen->gen_time < time) gen->gen_time = time;
		}
		// initvel  : <time>   : <flags>    : <minvel> : <maxvel>
		else if (!Q_strcasecmp(token,"initvel"))		
		{
			// Time
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			time = strtol(token, NULL, 0);

			// Flags
			token = COM_Parse5(&tokens);
			if (!tokens) 
			{ 
				Com_Printf ("Error: Unexpected end of file\n");
				goto error;
			}
			flags = strtol(token, NULL, 0);

			// Value
			for (i = 0; i < 2; i++)
			{
				token = COM_Parse5(&tokens);
				if (!tokens) 
				{ 
					Com_Printf ("Error: Unexpected end of file\n");
					goto error;
				}
				value[i] = atof(token);
			}

			for (i = 0; i < 2; i++)
			{
				env = malloc(sizeof(np_envelope_t));
				env->time = time;
				env->value = value[i];
				env->next = NULL;
				gen->velocity[i] = CL_AddEnvelopeToChain(gen->velocity[i], env);
			}
			if (gen->gen_time < time) gen->gen_time = time;
		}
		//
		// Unknown
		//
		else
		{
			Com_Printf ("Error: unknown generator token: %s\n", token);
			goto error;
		}
	}

error:
	Com_Printf ("Unable to load APD file '%s'\n", name);
	CL_FreeGeneratorChain(gen);
	gen = NULL;

end:
	if (data) free(data);

	if (gen)
	{
		cnp_generator_t *first = gen;

		while (gen)
		{
			gen->flow = CL_GeneratorDefaultEnvelope(gen->flow, 0);
			gen->angles[0] = CL_GeneratorDefaultEnvelope(gen->angles[0], 0);
			gen->angles[1] = CL_GeneratorDefaultEnvelope(gen->angles[1], 180);
			gen->velocity[0] = CL_GeneratorDefaultEnvelope(gen->velocity[0], 0);
			gen->velocity[1] = CL_GeneratorDefaultEnvelope(gen->velocity[1], 0);

			gen->radius = CL_GeneratorDefaultEnvelope(gen->radius, 1);
			gen->rgba[0] = CL_GeneratorDefaultEnvelope(gen->rgba[0], 1);
			gen->rgba[1] = CL_GeneratorDefaultEnvelope(gen->rgba[1], 1);
			gen->rgba[2] = CL_GeneratorDefaultEnvelope(gen->rgba[2], 1);
			gen->rgba[3] = CL_GeneratorDefaultEnvelope(gen->rgba[3], 1);
			gen->texcoord[0] = CL_GeneratorDefaultEnvelope(gen->texcoord[0], 0);
			gen->texcoord[1] = CL_GeneratorDefaultEnvelope(gen->texcoord[1], 0);
			gen->texcoord[2] = CL_GeneratorDefaultEnvelope(gen->texcoord[2], 1);
			gen->texcoord[3] = CL_GeneratorDefaultEnvelope(gen->texcoord[3], 1);
			gen = gen->next;
		}

		gen = first;
	}


	return gen;
}

/*
===============
CL_SampleEnvelopeValue
===============
*/
//qboolean test_sample = false;
float CL_SampleEnvelopeValue(np_envelope_t *first, int time)
{
	np_envelope_t *env;

	// If we haven't reached the first one, just return our value
	if (first->time >= time) 
	{
		//if (test_sample) Com_Printf ("first->time >= time = %i >= %i\n", first->time, time);
		return first->value;
	}

	// Loop over until we hit an envelope value that's time is before 'time'
	for (env = first; env->next; env = env->next)
	{
		//if (test_sample) Com_Printf ("env->time : time = %i : %i\n", env->time, time);

		// No lerping required here
		if (env->time == time)
		{
			//if (test_sample) Com_Printf ("No lerp\n");
			break;
		}
		// Lerp it
		else if (env->time < time && env->next->time > time )
		{
			float factor = (time - env->time)/(float) (env->next->time - env->time);
			float v = simple_lerp(env->value, env->next->value, factor);

			//if (test_sample) Com_Printf ("lerping %i/%f -> %f (%f-%f) -> %f= \n", 
			//	(time - env->time),
			//	(float) (env->next->time - env->time),
			//	factor,
			//	env->value,
			//	env->next->value,
			//	v);

			return v;
		}
	}

	//if (test_sample) Com_Printf ("env->time : env->value = %i : %f\n", env->time, env->value);
	// No next obviously so just return our value
	return env->value;
}

/*
===============
CL_SpawnNewParticle
===============
*/

void CL_SpawnNewParticle (centity_t *owner, 
						  int part_time,
						  cnp_generator_t *gen, 
						  int gen_time,
						  float *gen_forward, 
						  vec3_t gen_origin)
{
	int i;
	float	angles[2];
	float	velocity[2];
	float	pitch, roll, vel;
	vec3_t	direction;
	vec3_t	forward, right, up, temp;
	cnewparticle_t *p;

	// No free particles
	if (!free_newparticles) return;

	p = free_newparticles;
	free_newparticles = p->next;

	// Add it sorted into the generators list
	p->next = gen->first;
	gen->first = p;

	//
	// Now set values
	//
	p->gen_forward = gen_forward;
	p->owner = owner;
	p->time = part_time;
	p->time_rate = 1 + (frand() * gen->vartime);


	//
	// Rotation
	//
	if (gen->random_angle) p->angle = crand() * 180;
	else p->angle = 0;
	p->rotation = simple_lerp(gen->rotate[0], gen->rotate[1], frand())*360;

	//
	// Origin
	//

	// Only bother with volume IF we have any vol flags
	if (gen->volflags & 7)
	{
		for (i = 0 ; i < 3; i++)
		{
			if (gen->volflags & (1<<i))
				temp[i] = simple_lerp(owner->current.mins[i], owner->current.maxs[i], frand());
			else 
				temp[i] = 0;
		}


		AngleVectors(owner->current.angles, forward, right, up);

		p->origin[0] = temp[0]*forward[0] + temp[1]*right[0] + temp[2]*up[0];
		p->origin[1] = temp[0]*forward[1] + temp[1]*right[1] + temp[2]*up[1];
		p->origin[2] = temp[0]*forward[2] + temp[1]*right[2] + temp[2]*up[2];
		
		VectorAdd(p->origin, gen_origin, p->origin);

		if (!gen->follow) VectorAdd(p->origin, owner->lerp_origin, p->origin);
	}
	else if (gen->follow) VectorCopy(gen_origin, p->origin);
	else VectorAdd(gen_origin, owner->lerp_origin, p->origin);

	//
	// Velocity 
	//
	velocity[0] = CL_SampleEnvelopeValue(gen->velocity[0], gen_time);
	velocity[1] = CL_SampleEnvelopeValue(gen->velocity[1], gen_time);
	vel = simple_lerp(velocity[0], velocity[1], frand());

	// Movement Direction (relative to generator normal)
	angles[0] = CL_SampleEnvelopeValue(gen->angles[0], gen_time);
	angles[1] = CL_SampleEnvelopeValue(gen->angles[1], gen_time);
	if (angles[0]!=0 || angles[1]!=0)
	{
		pitch = simple_lerp(angles[0], angles[1], frand());
		roll = M_PI*crand();

		direction[0] = cos(pitch*M_PI/180) * vel;
		direction[1] = sin(pitch*M_PI/180) * sin(roll) * vel;
		direction[2] = sin(pitch*M_PI/180) * cos(roll) * vel;
		
		// Now make it relative to the world by transforming us by the normal vector
		MakeNormalVectors(gen_forward, right, up);

		p->velocity[0] = direction[0]*gen_forward[0] + direction[1]*right[0] + direction[2]*up[0];
		p->velocity[1] = direction[0]*gen_forward[1] + direction[1]*right[1] + direction[2]*up[1];
		p->velocity[2] = direction[0]*gen_forward[2] + direction[1]*right[2] + direction[2]*up[2];
	}
	// No need to do the complex stuff
	else
		VectorScale(gen_forward, vel, p->velocity);

}

/*
===============
CL_SpawnParticlesForEnt
===============
*/
void CL_SpawnParticlesForEnt (centity_t *ent, 
							  int num_parts, 
							  int part_time,
							  cnp_generator_t *gen,
							  int gen_start,			
							  int gen_end, 
							  float *gen_forward,
							  vec3_t gen_origin)
{
	float inc, time;
	inc = (float) (gen_end - gen_start) / (float) num_parts;
	time = 0;
	while (num_parts--)
	{
		CL_SpawnNewParticle(ent,
							part_time + (int) time,
							gen, 
							gen_start  + (int) time,
							gen_forward, 
							gen_origin);

		time += inc;
	}
}

/*
===============
CL_RunGenerators
===============
*/
void CL_RunGenerators (centity_t *ent)
{
	np_envelope_t *env;
	float start_val, end_val;
	int i, g, end, start;
	int last_time;
	vec3_t origin;
	cnp_generator_t *gen;
	float *gen_rem;

	// First thing, do random value		
	ent->rand_value = rand();

	for (g = 0; g < 4; g++)
	{
		// It's been changed, reset it
		if (ent->np[g].index != ent->current.np[g])
		{
			ent->np[g].index = ent->current.np[g];
			memcpy(ent->np[g].triangle, &(ent->current.np_tri[g][0]), 8);
			ent->np[g].first_time = cls.realtime;
			last_time = cls.realtime;
			if (ent->np[g].gen_rem) free(ent->np[g].gen_rem);

			gen = cl.part_generators[ent->np[g].index];
			if (!gen)
			{
				ent->np[g].gen_rem = 0;
			}
			else
			{
				ent->np[g].gen_rem = malloc(gen->num_generators * sizeof(float));
				memset(ent->np[g].gen_rem, 0, gen->num_generators * sizeof(float));
			}
			continue;
		}

		//if (ent->np[g].index != 4) continue;

		// If generator is gone, don't change anything
		if (!cl.part_generators[ent->np[g].index]) continue;

		// Set our normal
		if ((*(int*)ent->np[g].triangle) == -1)
		{
			// At origin of ent, pointing up
			AngleVectors(ent->current.angles, NULL, NULL, ent->np[g].normal);
			VectorClear(origin);
		}
		else
		{
			// TODO Triangle lookup for origin an normal
			VectorClear(origin);
		}

		gen_rem = ent->np[g].gen_rem;
		for (gen = cl.part_generators[ent->np[g].index]; gen; gen = gen->next, gen_rem ++) 
		{
			last_time = ent->np[g].last_time;

			//Com_Printf ("Spawning for time: %i\n", cls.realtime);

			// TODO Random Looping

			// Looping
			while (last_time < cls.realtime)
			{
				int time_total = last_time - ent->np[g].first_time;
				int time_start = time_total%gen->gen_time;
				int time_left = gen->gen_time-time_start;
				int time_delta = cls.realtime-last_time;
				int time_end;

				if (time_left < time_delta) time_delta = time_left;
				time_end = time_start+time_delta;

				// Break looping if we reached the 'end'
				if (gen->loops && (time_total/gen->gen_time) >= gen->loops) break;

				// Do flow
				env = gen->flow;

				i  = 0;
				while (i < time_delta)
				{
					//
					// Get the starting time, pretty simple just sample at the point
					// but also make sure that env remains the one before (if possible)
					//
					start = time_start + i;
					end = time_end;

					// Note end can be clamped here to make more accurate generation
					// Essentially the envelope graph is quantized by this code.
					// The way it is the slower the frame rate, the more quantized 
					// it gets. However, by limiting how far from start that end 
					// can be, the amount of quantization can be reduced

					// This is the equiv of generating particles at a min of 50 FPS
					if (end > (start + 20)) end = start + 20;

					// We haven't reached the first one yet so just use it's value
					if (env->time >= start) 
					{
						start_val = env->value;
					}
					else
					{
						// Loop over until we hit an envelope value that's time is 
						// before the start time
						while ( env )
						{
							// No lerping required here since we reached the end
							// or the env's time was exact
							if (env->time == start || !env->next)
							{
								start_val = env->value;
								break;
							}
							// Lerp it
							else if (env->time < start)
							{
								float factor = (start - env->time)/(float) (env->next->time - env->time);

								start_val = simple_lerp(env->value, env->next->value, factor);
								break;
							}

							env = env->next;
						}
					}

					// Now we need to get the finishing time

					// Time of first env is at or after the end, so just use it's value
					// There is no next one, so we will need to use it's value
					if (env->time >= end || !env->next)
					{
						end_val = env->value;
					}
					// The next event is before our limit so use it
					else if (env->next->time <= end)
					{
						// Increment the event to 'speed' things up next time round
						env = env->next;
						end = env->time;
						end_val = env->value;
					}
					// Next time is after our limit so we will need to lerp
					else 
					{
						float factor = (end - env->time)/(float) (env->next->time - env->time);

						end_val = simple_lerp(env->value, env->next->value, factor);
						break;
					}


					//Com_Printf ("Spawn?   %f\n", *gen_rem);

					// Lets work out number of particles 
					// Avarage of start and end flow rates multiplied by time
					*gen_rem = *gen_rem + (end-start)*(start_val+end_val)/2000.0F;

					//Com_Printf ("start    %i\n", start);
					//Com_Printf ("end      %i\n", end);
					//Com_Printf ("startval %f\n", start_val);
					//Com_Printf ("endval   %f\n", end_val);
					//Com_Printf ("diff     %f\n", (end-start)*(start_val+end_val)/2000.0F);
					//Com_Printf ("Spawn?   %f\n", *gen_rem);

					if (*gen_rem >= 1)
					{
						// We will evenly generate '*gen_rem' particles
						// over the time period from start to end
						//
						// Not the most accurate way of doing things if the time
						// between end and start becomes too large

						//Com_Printf ("Spawning %i\n", (int) *gen_rem);

						CL_SpawnParticlesForEnt(ent, 
												(int) *gen_rem, 
												last_time+i,
												gen,
												start,
												end,
												ent->np[g].normal,
												origin);

						*gen_rem = *gen_rem -(int) *gen_rem;
					}

					// Increment 'i'
					i += end - start;
				}

				// Do bursts. For every burst between time_start and time_end
				// generate particles
				for ( env = gen->burst; env; env = env->next)
				{
					// No more bursts to check
					if (env->time >= time_end)
					{
						break;
					}
					// Found a valid burst
					else if (env->time >= start && env->value >= 1)
					{
						CL_SpawnParticlesForEnt(ent, 
												(int) env->value, 
												last_time+env->time-start,
												gen,
												env->time,
												env->time,
												ent->np[g].normal,
												origin);
					}
				}
				last_time += time_delta;
			}
		}

		// Update time
		ent->np[g].last_time = cls.realtime;
	}
}

/*
===============
CL_AddNewParticles
===============
*/
void CL_AddNewParticles (void)
{
	int					i, g;
	newparticle_t		vp;
	cnewparticle_t		*p, *next, *prev;
	float				time, t;

	// Update generator random values, if requred
	for (g = 1; g < MAX_APD; g++)
	{
		cnp_generator_t *gen = cl.part_generators[g];

		while (gen)
		{
			if (gen->flicker && gen->flicker_all) gen->rand_value = rand();

			// Now generate particles
			prev = NULL;
			for (p=gen->first ; p ; p=next)
			{
				next = p->next;

				time = (cls.realtime - p->time) * p->time_rate;
				t = time / 1000.0;

				// If after decay time, we're finished so remove us
				if (time > gen->decay_time)
				{
					if (gen->first == p) gen->first = p->next;
					else prev->next = p->next;
					p->next = free_newparticles;
					free_newparticles = p;
					continue;
				}
				else 
					prev = p;

				// Flicker
				if (gen->flicker)
				{
					int rand_value;

					// Will never be seen
					if (gen->flicker > RAND_MAX) continue;

					if (!gen->flicker_all) rand_value = rand();
					else rand_value = (p->owner->rand_value + gen->rand_value)/2;

					// Don't show
					if (rand_value < gen->flicker) continue;
				}

				// Generator
				vp.gen = &(gen->info);

				// Forward Vector
				VectorCopy(p->gen_forward, vp.gen_forward);

				// Origin
				for (i = 0; i < 3; i++)
				{
					const float * const vel = p->velocity;
					const float * const acc = gen->accel;
					const float resist = gen->resist;

					// New particles position with Air Resistance
					if (resist && (acc[i] || vel[i]))
					{
						if (acc[i])
							vp.origin[i] = p->origin[i] + acc[i]*t/resist + 1/resist * (vel[i]-acc[i]/resist) * (1-exp(-resist*t));
						else
							vp.origin[i] = p->origin[i] + 1/resist * vel[i] * (1-exp(-resist*t));
					}
					else if (acc[i] || vel[i])
					{
						vp.origin[i] = p->origin[i] + (vel[i]+0.5*acc[i]*t)*t;
					}
					else
					{
						vp.origin[i] = p->origin[i];
					}
				}

				// Handle follow
				if (gen->follow)
				{
					// Yeah this isn't too 'correct' as particle could 'jump' around as owner enters and leaves pvs
					if (p->owner->serverframe != cl.frame.serverframe)
						VectorAdd(vp.origin, p->owner->current.origin, vp.origin);
					else
						VectorAdd(vp.origin, p->owner->lerp_origin, vp.origin);
				}

				// Rotation
				if (p->rotation)
					vp.roll = anglemod(p->angle + p->rotation*t);
				else
					vp.roll = p->angle;

				// Now onto the envelopes
				//test_sample = true;
				vp.radius = CL_SampleEnvelopeValue(gen->radius, (int) time);
				//test_sample = false;

				for (i = 0; i < 4; i++)
				{
					vp.rgba[i] = CL_SampleEnvelopeValue(gen->rgba[i], (int) time);
					vp.texcoord[i] = CL_SampleEnvelopeValue(gen->texcoord[i], (int) time);
				}

				V_AddNewParticle(&vp);
			}

			gen = gen->next;
		}
	}
}

