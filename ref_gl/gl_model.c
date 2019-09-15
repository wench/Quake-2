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
// models.c -- model loading and caching

#include "gl_local.h"

float	r_avertexnormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

float	r_avertexnormals_anox[2048][3] = {
#include "anoxnorms.h"
};

model_t	*loadmodel;
int		modfilelen;

void Mod_LoadSpriteModel (model_t *mod, void *buffer);
void Mod_LoadBrushModel (model_t *mod, void *buffer);
void Mod_LoadAliasModel (model_t *mod, void *buffer, qboolean gen_profiles);
void Mod_LoadMDAModel (model_t *mod, void *buffer);
model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];

#define	MAX_MOD_KNOWN	2048
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

// the inline * models from the current map are kept seperate
model_t	mod_inline[MAX_MOD_KNOWN];

int		registration_sequence;

/*
===============
Mod_PointInLeaf
===============
*/
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	cplane_t	*plane;
	
	if (!model || !model->nodes)
		ri.Sys_Error (ERR_DROP, "Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents != -1)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}
	
	return NULL;	// never reached
}


/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->vis->numclusters+7)>>3;	
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
	
	return decompressed;
}

/*
==============
Mod_ClusterPVS
==============
*/
byte *Mod_ClusterPVS (int cluster, model_t *model)
{
	if (cluster == -1 || !model->vis)
		return mod_novis;
	return Mod_DecompressVis ( (byte *)model->vis + model->vis->bitofs[cluster][DVIS_PVS],
		model);
}


//===============================================================================

/*
================
Mod_Modellist_f
================
*/
void Mod_Modellist_f (void)
{
	int		i;
	model_t	*mod;
	int		total;

	total = 0;
	ri.Con_Printf (PRINT_ALL,"Loaded models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		ri.Con_Printf (PRINT_ALL, "%8i : %s\n",mod->extradatasize, mod->name);
		total += mod->extradatasize;
	}
	ri.Con_Printf (PRINT_ALL, "Total resident: %i\n", total);
}

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	memset (mod_novis, 0xff, sizeof(mod_novis));
}



/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (char *name, qboolean crash)
{
	model_t	*mod;
	unsigned *buf;
	int		i;
	int		profile = 0;
	
	if (!name[0])
		ri.Sys_Error (ERR_DROP, "Mod_ForName: NULL name");
		
	//
	// inline models are grabbed only from worldmodel
	//
	if (name[0] == '*')
	{
		i = atoi(name+1);
		if (i < 1 || !r_worldmodel || i >= r_worldmodel->numsubmodels)
			ri.Sys_Error (ERR_DROP, "bad inline model number");
		return &mod_inline[i];
	}

	//
	// search the currently loaded models
	//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (!strcmp (mod->name, name) )
			return mod;
	}
	
	//
	// find a free model slot spot
	//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			break;	// free spot
	}
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			ri.Sys_Error (ERR_DROP, "mod_numknown == MAX_MOD_KNOWN");
		mod_numknown++;
	}
	

	//
	// load the file
	//
	strcpy(mod->name, name);
	modfilelen = ri.FS_LoadFile (mod->name, &buf);
	if (!buf)
	{
		if (crash)
			ri.Sys_Error (ERR_DROP, "Mod_NumForName: %s not found", mod->name);
		memset (mod->name, 0, sizeof(mod->name));
		return NULL;
	}
	
	loadmodel = mod;

	//
	// fill it in
	//


	// call the apropriate loader
	
	switch (LittleLong(*(unsigned *)buf))
	{
	case IDALIASHEADER:
		loadmodel->extradata = Hunk_Begin (0xA00000);
		Mod_LoadAliasModel (mod, buf, true);
		break;
		
	case IDMDAHEADER:
		loadmodel->extradata = Hunk_Begin (0xA00000);
		Mod_LoadMDAModel (mod, buf);
		break;
		
	case IDSPRITEHEADER:
		loadmodel->extradata = Hunk_Begin (0x20000);
		Mod_LoadSpriteModel (mod, buf);
		mod->mda_opaque = true;
		break;
	
	case IDBSPHEADER:
		loadmodel->extradata = Hunk_Begin (0x2000000);
		Mod_LoadBrushModel (mod, buf);
		mod->mda_opaque = true;
		break;

	default:
		ri.Sys_Error (ERR_DROP,"Mod_NumForName: unknown fileid for %s", mod->name);
		break;
	}

	loadmodel->extradatasize = Hunk_End ();

	ri.FS_FreeFile (buf);

	return mod;
}

/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/

byte	*mod_base;


/*
=================
Mod_LoadLighting
=================
*/
void Mod_LoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}
	loadmodel->lightdata = Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}


/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	int		i;

	if (!l->filelen)
	{
		loadmodel->vis = NULL;
		return;
	}
	loadmodel->vis = Hunk_Alloc ( l->filelen);	
	memcpy (loadmodel->vis, mod_base + l->fileofs, l->filelen);

	loadmodel->vis->numclusters = LittleLong (loadmodel->vis->numclusters);
	for (i=0 ; i<loadmodel->vis->numclusters ; i++)
	{
		loadmodel->vis->bitofs[i][0] = LittleLong (loadmodel->vis->bitofs[i][0]);
		loadmodel->vis->bitofs[i][1] = LittleLong (loadmodel->vis->bitofs[i][1]);
	}
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
	}
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);
	}

	return VectorLength (corner);
}


/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	mmodel_t	*out;
	int			i, j, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		out->radius = RadiusFromBounds (out->mins, out->maxs);
		out->headnode = LittleLong (in->headnode);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( (count + 1) * sizeof(*out));	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out, *step;
	int 	i, j, count;
	char	name[MAX_QPATH];
	int		next;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);

		out->flags = LittleLong (in->flags);
		next = LittleLong (in->nexttexinfo);
		if (next > 0)
			out->next = loadmodel->texinfo + next;
		else
		    out->next = NULL;

		Com_sprintf (name, sizeof(name), "textures/%s", in->texture);
		out->image = GL_FindImage (name,0, it_wall);
		if (!out->image)
		{
			ri.Con_Printf (PRINT_ALL, "Couldn't load %s\n", name);
			out->image = r_notexture;
		}
	}

	// count animation frames
	for (i=0 ; i<count ; i++)
	{
		out = &loadmodel->texinfo[i];
		out->numframes = 1;
		for (step = out->next ; step && step != out ; step=step->next)
			out->numframes++;
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2], val;
	int		i,j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;
	
	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		
		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floor(mins[i]/16);
		bmaxs[i] = ceil(maxs[i]/16);

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;

//		if ( !(tex->flags & TEX_SPECIAL) && s->extents[i] > 512 /* 256 */ )
//			ri.Sys_Error (ERR_DROP, "Bad surface extents");
	}
}


void GL_BuildPolygonFromSurface(msurface_t *fa);
void GL_CreateSurfaceLightmap (msurface_t *surf);
void GL_EndBuildingLightmaps (void);
void GL_BeginBuildingLightmaps (model_t *m);

/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;
	int			ti;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	currentmodel = loadmodel;

	GL_BeginBuildingLightmaps (loadmodel);

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);		
		out->flags = 0;
		out->polys = NULL;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;			

		out->plane = loadmodel->planes + planenum;

		ti = LittleShort (in->texinfo);
		if (ti < 0 || ti >= loadmodel->numtexinfo)
			ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad texinfo number");
		out->texinfo = loadmodel->texinfo + ti;

		CalcSurfaceExtents (out);
				
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else
			out->samples = loadmodel->lightdata + i;
		
	// set the drawing flags
		
		if (out->texinfo->flags & SURF_WARP)
		{
			out->flags |= SURF_DRAWTURB;
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			GL_SubdivideSurface (out);	// cut up polygon for warps
		}

		// create lightmaps and polygons
//		if ( !(out->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP) ) )
		if ( !(out->texinfo->flags & (SURF_SKY|SURF_WARP) ) )
			GL_CreateSurfaceLightmap (out);

		if (! (out->texinfo->flags & SURF_WARP) ) 
			GL_BuildPolygonFromSurface(out);

	}

	GL_EndBuildingLightmaps ();
}


/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents != -1)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		out->contents = -1;	// differentiate from leafs

		for (j=0 ; j<2 ; j++)
		{
			p = LittleLong (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;
//	glpoly_t	*poly;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->minmaxs[j] = LittleShort (in->mins[j]);
			out->minmaxs[3+j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->contents);
		out->contents = p;

		out->cluster = LittleShort(in->cluster);
		out->area = LittleShort(in->area);

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstleafface);
		out->nummarksurfaces = LittleShort(in->numleaffaces);
		
		// gl underwater warp
#if 0
		if (out->contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_THINWATER) )
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
			{
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
				for (poly = out->firstmarksurface[j]->polys ; poly ; poly=poly->next)
					poly->flags |= SURF_UNDERWATER;
			}
		}
#endif
	}	
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j < 0 ||  j >= loadmodel->numsurfaces)
			ri.Sys_Error (ERR_DROP, "Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{	
	int		i, count;
	int		*in, *out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	if (count < 1 || count >= MAX_MAP_SURFEDGES)
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: bad surfedges count in %s: %i",
		loadmodel->name, count);

	out = Hunk_Alloc ( count*sizeof(*out));	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}


/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Sys_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_Alloc ( count*2*sizeof(*out));	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

/*
=================
Mod_LoadBrushModel
=================
*/
void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i;
	dheader_t	*header;
	mmodel_t 	*bm;
	
	loadmodel->type = mod_brush;
	if (loadmodel != mod_known)
		ri.Sys_Error (ERR_DROP, "Loaded a brush model after the world");

	header = (dheader_t *)buffer;

	i = LittleLong (header->version);
	//if (i != BSPVERSION)
		//ri.Sys_Error (ERR_DROP, "Mod_LoadBrushModel: %s has wrong version number (%i should be %i)", mod->name, i, BSPVERSION);

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

// load into heap
	
	Mod_LoadVertexes (&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges (&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges (&header->lumps[LUMP_SURFEDGES]);
	Mod_LoadLighting (&header->lumps[LUMP_LIGHTING]);
	Mod_LoadPlanes (&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo (&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces (&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces (&header->lumps[LUMP_LEAFFACES]);
	Mod_LoadVisibility (&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs (&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes (&header->lumps[LUMP_NODES]);
	Mod_LoadSubmodels (&header->lumps[LUMP_MODELS]);
	mod->numframes = 2;		// regular and alternate animation
	
//
// set up the submodels
//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		model_t	*starmod;

		bm = &mod->submodels[i];
		starmod = &mod_inline[i];

		*starmod = *loadmodel;
		starmod->mda_opaque = true;
		
		starmod->firstmodelsurface = bm->firstface;
		starmod->nummodelsurfaces = bm->numfaces;
		starmod->firstnode = bm->headnode;
		if (starmod->firstnode >= loadmodel->numnodes)
			ri.Sys_Error (ERR_DROP, "Inline model %i has bad firstnode", i);

		VectorCopy (bm->maxs, starmod->maxs);
		VectorCopy (bm->mins, starmod->mins);
		starmod->radius = bm->radius;
	
		if (i == 0)
			*loadmodel = *starmod;

		starmod->numleafs = bm->visleafs;
	}
}

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

/*
=================
Mod_AutoGenAnoxProfile
=================
*/

// This func will auto generate a profile for normal Anox models
void Mod_AutoGenAnoxProfile(model_t *mod, dmdl_anox_t *anox)
{
	typedef char skinname_t[64];
	int i;
	mda_profile_t *last_prof;

	// A bit of a hack for q2 md2's
	int skin_passes = anox->num_skins/anox->num_passes;
	int	skin_counter;
	skinname_t	*skinnames;

	skinnames = (skinname_t	*) ((char *)anox + anox->ofs_skins);

	for (skin_counter = 0; skin_counter < skin_passes; skin_counter++)
	{
		mda_profile_t *prof = (mda_profile_t *) Hunk_Alloc(sizeof(mda_profile_t));
		memset(prof,0, sizeof(mda_profile_t));

		prof->name = *(int*)"DFLT";

		prof->skins = (mda_skin_t *) Hunk_Alloc(sizeof(mda_skin_t)*anox->num_passes);
		memset(prof->skins,0, sizeof(mda_skin_t)*anox->num_passes);

		for (i = 0; i < anox->num_passes; i++)
		{
			mda_pass_t *pass;

			// Note the array access only works here
			prof->skins[i].next = &prof->skins[i+1];
			prof->skins[i].passes = pass = Hunk_Alloc(sizeof(mda_pass_t));

			pass->imagename[0] = 0;
			// Anox skins need model path appended to start of skinname
			if (anox->version != ALIAS_VERSION) 
			{
				COM_FilePath(mod->name, pass->imagename);
				strcat(pass->imagename, "/");
			}

			strcat(pass->imagename, skinnames[i+skin_counter*anox->num_passes]);

			// Get the image
			mod->skins[i] = pass->image = GL_FindImage (pass->imagename,0, it_skin);

			// Cull
			pass->cull_mode = GL_FRONT;

			// Depth func
			pass->depth_func = GL_LEQUAL;

			// Alpha Test
			pass->alpha_func = GL_ALWAYS;
			pass->alpha_test_ref = 0;

			// RGB gen
			pass->rgbgen = MDA_RGBGEN_DIFFUSE;

			// UV gen
			pass->uvgen = MDA_UVGEN_BASE;

			// UV Scroll
			pass->uvscroll[0] = 0;
			pass->uvscroll[1] = 0;

			// Alpha Blend (and depth write)
			if (pass->image->has_alpha && anox->version != ALIAS_VERSION)
			{
				pass->src_blend = GL_SRC_ALPHA;
				pass->dest_blend = GL_ONE_MINUS_SRC_ALPHA;			
				pass->depth_write = false;
				prof->skins[i].sort_blend = true;
				mod->mda_blend = true;
				prof->alpha_gen_mask = pass->uvgen|pass->rgbgen;
			}
			else
			{
				pass->src_blend = GL_ONE;
				pass->dest_blend = GL_ZERO;			
				pass->depth_write = true;
				prof->skins[i].sort_blend = false;
				mod->mda_opaque = true;
				prof->opaque_gen_mask = pass->uvgen|pass->rgbgen;
			}

			// Next
			pass->next = 0;
		}
		prof->skins[anox->num_passes-1].next = 0;
	
		if (!mod->profiles) mod->profiles = prof;
		else last_prof->next = prof;
		last_prof = prof;
	}

	if (skin_passes == 0)
	{
		mod->mda_blend = false;
		mod->mda_opaque = true;
	}
}

/*
=================
Mod_LoadAliasModel
=================
*/
void Mod_LoadAliasModel (model_t *mod, void *buffer, qboolean gen_profiles)
{
	int					i, j;
	dmdl_t				*pinmodel, *pheader;
	dmdl_anox_t			*anox;
	dstvert_t			*pinst, *poutst;
	dtriangle_t			*pintri, *pouttri;
	daliasframe_t		*pinframe, *poutframe;
	int					*pincmd, *poutcmd;
	short				*pinpasses, *poutpasses;
	int					version;
	int					frame_extra;
	int					total_extra;
	int					total_frame_extra;
	int					header_extra = 0;
	int					passes_extra = 0;
	int					new_framesize;
	int					estfs;
	unsigned short		normal;
	byte				*a_verts;

	pinmodel = (dmdl_t *)buffer;

	version = LittleLong (pinmodel->version);

	if (version != ALIAS_VERSION && version != ALIAS_VERSION_ANOX_3_BYTE && 
		version != ALIAS_VERSION_ANOX_4_BYTE && version != ALIAS_VERSION_ANOX_6_BYTE &&
		version != ALIAS_VERSION_ANOX_OLD )
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
				 mod->name, version, ALIAS_VERSION);

	/*
	if (version != ALIAS_VERSION && !floats_loaded) {
		int len;
		void *buf;
		FILE *f;

		//len = ri.FS_LoadFile ("normals.dat", &buf);
		// Dead
		//if (!buf) ri.Sys_Error (ERR_DROP, "Unable to load normals.dat\n");
		floats_loaded = true;
        
		qsort (r_avertexnormals_anox, 2048, 12, FloatCompare);

		f = fopen ("floats", "wb");
		fwrite(r_avertexnormals_anox, 2048, 12, f);
		fclose(f);
		//memcpy(r_avertexnormals_anox,buf,len);
		//ri.FS_FreeFile(buf);

	}
	*/

	// Calculate the extra memory we will require for the new frames
	if (version == ALIAS_VERSION_ANOX_3_BYTE || version == ALIAS_VERSION_ANOX_OLD) 
		frame_extra = 5;
	else if (version == ALIAS_VERSION_ANOX_4_BYTE)
		frame_extra = 6;
	else if (version == ALIAS_VERSION_ANOX_6_BYTE) 
		frame_extra = 8;
	else 
	{
		frame_extra = 4;
		header_extra = sizeof(dmdl_anox_t) - sizeof(dmdl_t);
		passes_extra = 2;
	}

	estfs = frame_extra * LittleLong(pinmodel->num_xyz);

	frame_extra = sizeof(dtrivertx_t) - frame_extra;
	frame_extra *= LittleLong(pinmodel->num_xyz);

	estfs += sizeof(daliasframe_t) - sizeof(dtrivertx_t);

	// Don't bother changing size if new is smaller
	//if (frame_extra < 0) frame_extra = 0;

	total_frame_extra = frame_extra * LittleLong(pinmodel->num_frames);
	// Actual total size difference
	total_extra = total_frame_extra + header_extra + passes_extra;

	pheader = Hunk_Alloc (LittleLong(pinmodel->ofs_end) + total_extra);
	anox = (dmdl_anox_t*)pheader;
	
	// byte swap the header fields and sanity check
	for (i=0 ; i<sizeof(dmdl_t)/4 ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	// Continue reading the rest of the header for anox files
	if (version != ALIAS_VERSION) for ( ; i<sizeof(dmdl_anox_t)/4 ; i++)
		((int *)pheader)[i] = LittleLong (((int *)buffer)[i]);

	pinmodel->ofs_end += total_extra;
	new_framesize = pheader->framesize + frame_extra;

	if (pheader->skinheight > MAX_LBM_HEIGHT)
		ri.Sys_Error (ERR_DROP, "model %s has a skin taller than %d", mod->name,
				   MAX_LBM_HEIGHT);

	if (pheader->num_xyz <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no vertices", mod->name);

	if (pheader->num_xyz > MAX_VERTS)
		ri.Sys_Error (ERR_DROP, "model %s has too many vertices", mod->name);

	if (pheader->num_st <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no st vertices", mod->name);

	if (pheader->num_tris <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no triangles", mod->name);

	if (pheader->num_frames <= 0)
		ri.Sys_Error (ERR_DROP, "model %s has no frames", mod->name);

//
// load base s and t vertices (not used in gl version)
//
	pinst = (dstvert_t *) ((byte *)pinmodel + pheader->ofs_st);
	if (pheader->ofs_st > pheader->ofs_frames) pheader->ofs_st += total_frame_extra + header_extra;
	else pheader->ofs_st += header_extra;
	poutst = (dstvert_t *) ((byte *)pheader + pheader->ofs_st);

	for (i=0 ; i<pheader->num_st ; i++)
	{
		poutst[i].s = LittleShort (pinst[i].s);
		poutst[i].t = LittleShort (pinst[i].t);
	}

//
// load triangle lists
//
	pintri = (dtriangle_t *) ((byte *)pinmodel + pheader->ofs_tris);
	if (pheader->ofs_tris > pheader->ofs_frames) pheader->ofs_tris += total_frame_extra + header_extra;
	else pheader->ofs_tris += header_extra;
	pouttri = (dtriangle_t *) ((byte *)pheader + pheader->ofs_tris);

	for (i=0 ; i<pheader->num_tris ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			pouttri[i].index_xyz[j] = LittleShort (pintri[i].index_xyz[j]);
			pouttri[i].index_st[j] = LittleShort (pintri[i].index_st[j]);
		}
	}

//
// load the frames
//
	for (i=0 ; i<pheader->num_frames ; i++)
	{
		pinframe = (daliasframe_t *) ((byte *)pinmodel 
			+ pheader->ofs_frames + i * pheader->framesize);
		poutframe = (daliasframe_t *) ((byte *)pheader 
			+ pheader->ofs_frames + header_extra + i * new_framesize);

		memcpy (poutframe->name, pinframe->name, sizeof(poutframe->name));
		for (j=0 ; j<3 ; j++)
		{
			poutframe->scale[j] = LittleFloat (pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat (pinframe->translate[j]);
		}

		a_verts = (byte*)pinframe->verts;

		if (version == ALIAS_VERSION_ANOX_3_BYTE || version == ALIAS_VERSION_ANOX_OLD) {
			for (j = 0; j < pheader->num_xyz; j++) {
				poutframe->verts[j].v[0] = *a_verts++;
				poutframe->verts[j].v[1] = *a_verts++;
				poutframe->verts[j].v[2] = *a_verts++;

				normal = LittleShort(*(short*)a_verts) & 0x7FF;
				VectorCopy(r_avertexnormals_anox[normal], poutframe->verts[j].normal);
				a_verts += 2;
			}
		}
		else if (version == ALIAS_VERSION_ANOX_4_BYTE) {
			for (j = 0; j < pheader->num_xyz; j++) {
				unsigned int v = LittleLong(*(long*)a_verts);
				poutframe->verts[j].v[0] =  v      & 0x7FF;
				poutframe->verts[j].v[1] = (v>>11) & 0x3FF;
				poutframe->verts[j].v[2] = (v>>21) & 0x7FF;
				a_verts += 4;

				normal = LittleShort(*(short*)a_verts) & 0x7FF;
				VectorCopy(r_avertexnormals_anox[normal], poutframe->verts[j].normal);
				a_verts += 2;
			}
		}
		else if (version == ALIAS_VERSION_ANOX_6_BYTE) {
			for (j = 0; j < pheader->num_xyz; j++) {
				poutframe->verts[j].v[0] = LittleShort(*(short*)a_verts);
				a_verts+=2;
				poutframe->verts[j].v[1] = LittleShort(*(short*)a_verts);
				a_verts+=2;
				poutframe->verts[j].v[2] = LittleShort(*(short*)a_verts);
				a_verts+=2;

				normal = LittleShort(*(short*)a_verts) & 0x7FF;
				VectorCopy(r_avertexnormals_anox[normal], poutframe->verts[j].normal);
				a_verts += 2;
			}
		}
		else {
			for (j = 0; j < pheader->num_xyz; j++) {
				poutframe->verts[j].v[0] = *a_verts++;
				poutframe->verts[j].v[1] = *a_verts++;
				poutframe->verts[j].v[2] = *a_verts++;

				normal = *a_verts++;
				VectorCopy(r_avertexnormals[normal], poutframe->verts[j].normal);
			}
		}

	}
	pheader->ofs_frames += header_extra;
	pheader->framesize = new_framesize;

	mod->type = mod_alias;

	//
	// load the glcmds
	//
	pincmd = (int *) ((byte *)pinmodel + pheader->ofs_glcmds);
	if (pheader->ofs_glcmds > pheader->ofs_frames) pheader->ofs_glcmds += total_frame_extra + header_extra;
	else pheader->ofs_glcmds += header_extra;
	poutcmd = (int *) ((byte *)pheader + pheader->ofs_glcmds);

	for (i=0 ; i<pheader->num_glcmds ; i++)
		poutcmd[i] = LittleLong (pincmd[i]);

	// register all skins
	if (pheader->ofs_skins > pheader->ofs_frames) 
	{
		memcpy ((char *)pheader + pheader->ofs_skins + total_frame_extra + header_extra, 
				(char *)pinmodel + pheader->ofs_skins,
				pheader->num_skins*MAX_SKINNAME);
		pheader->ofs_skins += total_frame_extra + header_extra;
	}
	else 
	{
		memcpy ((char *)pheader + pheader->ofs_skins + header_extra, 
				(char *)pinmodel + pheader->ofs_skins,
				pheader->num_skins*MAX_SKINNAME);
		pheader->ofs_skins += header_extra;
	}

	if (version == ALIAS_VERSION) 
	{
		short	*passes;
		int		*order;
		anox->ofs_passes = pinmodel->ofs_end-2;
		anox->num_passes = 1;
		passes = (short*)((char *)anox + anox->ofs_passes);
		passes[0] = 0;

		// Quickly count num gl_cmds for passes
		order = (int *)((byte *)anox + anox->ofs_glcmds);
		while (1)
		{
			i = *order++;
			if (i == 0) break;
			if (i < 0) order -= i * 3;
			else order += i * 3;
			passes[0]++;
		}

		anox->scale[0] = 1;
		anox->scale[1] = 1;
		anox->scale[2] = 1;

		for (i=0 ; i<pheader->num_skins ; i++)
		{
			mod->skins[i] = GL_FindImage ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME,0
				, it_skin);
		}

	}
	// Anox stuff
	else
	{
		// Load emits
		pinpasses = (short *) ((byte *)pinmodel + anox->ofs_passes);
		if (anox->ofs_passes > anox->ofs_frames) anox->ofs_passes += total_frame_extra + header_extra;
		else anox->ofs_passes += header_extra;
		poutpasses = (short *) ((byte *)pheader + anox->ofs_passes);

		for (i=0 ; i<anox->num_passes; i++) poutpasses[i] = LittleShort (pinpasses[i]);

		//if (anox->num_skins != anox->num_passes)
		//	ri.Sys_Error (ERR_DROP, "%s number of skins (%i) doesn't match emit passes (%i)",
		//			 mod->name, anox->num_skins, anox->num_passes);

		// Ignore tags for now
	}

	// Do shaders 
	if (gen_profiles) Mod_AutoGenAnoxProfile(mod, anox);

	mod->mins[0] = -32;
	mod->mins[1] = -32;
	mod->mins[2] = -32;
	mod->maxs[0] = 32;
	mod->maxs[1] = 32;
	mod->maxs[2] = 32;
}

/*
==============================================================================

MDA MODELS (Alias + Extras)

==============================================================================
*/

// Parse a profile
mda_pass_t* Mod_ParsePass (model_t *mod, char **tokens, float minmax[2])
{
	int			i;
	char		*token;
	mda_pass_t	*pass = Hunk_Alloc(sizeof(mda_pass_t));
	memset(pass, 0, sizeof(mda_pass_t));

	minmax[0] = 0;
	minmax[1] = 1;

	// Defaults

	pass->cull_mode = GL_FRONT;
	pass->depth_func = GL_LEQUAL;
	pass->alpha_func = GL_ALWAYS;
	pass->alpha_test_ref = 0;
	pass->rgbgen = MDA_RGBGEN_DIFFUSE;
	pass->uvgen = MDA_UVGEN_BASE;
	pass->uvscroll[0] = 0;
	pass->uvscroll[1] = 0;
	pass->src_blend = GL_ONE;
	pass->dest_blend = GL_ZERO;			
	pass->depth_write = -1;
	
	token = COM_Parse2 (tokens);
	// Corrupt MDA
	if (!*tokens) return NULL;
	// Corrupt MDA
	if (token[0] != '{') return NULL;

	while (1) 
	{
		token = COM_Parse2 (tokens);
		// Corrupt MDA
		if (!*tokens) return NULL;

		// end of profile
		if (Q_stricmp(token, "}") == 0)
		{
			break;
		}
		// Texture map
		else if (Q_stricmp(token, "map") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			strcpy(pass->imagename, token);
			pass->image = GL_FindImage(pass->imagename,0, it_skin );
		}
		else if (Q_stricmp(token, "blendmode") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (!Q_strcasecmp(token, "add"))
			{
				pass->src_blend = GL_ONE;
				pass->dest_blend = GL_ONE;			
			}
			else if (!Q_strcasecmp(token, "multiply"))
			{
				pass->src_blend = GL_DST_COLOR;
				pass->dest_blend = GL_ZERO;			
			}
			else if (!Q_strcasecmp(token, "none"))
			{
				pass->src_blend = GL_ONE;
				pass->dest_blend = GL_ZERO;			
			}
			else if (!Q_strcasecmp(token, "normal"))
			{
				pass->src_blend = GL_SRC_ALPHA;
				pass->dest_blend = GL_ONE_MINUS_SRC_ALPHA;			
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown blendmode type (%s) in Pass", mod->name, token);
			}
		}
		else if (Q_stricmp(token, "cull") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			else if (!Q_strcasecmp(token, "disable"))
			{
				pass->cull_mode = GL_NEVER;
			}
			else if (!Q_strcasecmp(token, "back"))
			{
				pass->cull_mode = GL_BACK;
			}
			else if (!Q_strcasecmp(token, "front"))
			{
				pass->cull_mode = GL_FRONT;
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown cull type (%s) in Pass", mod->name, token);
			}
		}

		else if (Q_stricmp(token, "uvgen") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (!Q_strcasecmp(token, "base"))
			{
				pass->uvgen = MDA_UVGEN_BASE;
			}
			else if (!Q_strcasecmp(token, "sphere"))
			{
				pass->uvgen = MDA_UVGEN_SPHERE;
			}
			else if (!Q_strcasecmp(token, "parabxneg"))
			{
				pass->uvgen = MDA_UVGEN_PARABXNEG;
			}
			else if (!Q_strcasecmp(token, "parabxpos"))
			{
				pass->uvgen = MDA_UVGEN_PARABXPOS;
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown uvgen type (%s) in Pass", mod->name, token);
			}
		}

		else if (Q_stricmp(token, "uvmod") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			else if (!Q_strcasecmp(token, "scroll"))
			{
				// X
				token = COM_Parse2 (tokens);
				if (!*tokens) return NULL;	// Corrupt MDA
				pass->uvscroll[0] = atof (token);

				// Y
				token = COM_Parse2 (tokens);
				if (!*tokens) return NULL;	// Corrupt MDA
				pass->uvscroll[1] = atof (token);
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown uvmod type (%s) in Pass", mod->name, token);
			}
		}

		else if (Q_stricmp(token, "alphafunc") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (!Q_strcasecmp(token, "ge128"))
			{
				pass->alpha_func = GL_GEQUAL;
				pass->alpha_test_ref = 128/255.0;

				minmax[0] = 0.5;
				minmax[1] = 1;
			}
			else if (!Q_strcasecmp(token, "lt128"))
			{
				pass->alpha_func = GL_LESS;
				pass->alpha_test_ref = 128/255.0;

				minmax[0] = 0;
				minmax[1] = 0.5;
			}
			else if (!Q_strcasecmp(token, "ge64"))
			{
				pass->alpha_func = GL_GEQUAL;
				pass->alpha_test_ref = 64/255.0;

				minmax[0] = 0.125;
				minmax[1] = 1;
			}
			else if (!Q_strcasecmp(token, "gt0"))
			{
				pass->alpha_func = GL_GREATER;
				pass->alpha_test_ref = 0;

				minmax[0] = 1/510.0;
				minmax[1] = 1;
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown alphafunc type (%s) in Pass", mod->name, token);
			}
		}

		else if (Q_stricmp(token, "depthfunc") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (!Q_strcasecmp(token, "notequal"))
			{
				pass->depth_func = GL_NOTEQUAL;
			}
			else if (!Q_strcasecmp(token, "greater"))
			{
				pass->depth_func = GL_GREATER;
			}
			else if (!Q_strcasecmp(token, "gequal"))
			{
				pass->depth_func = GL_GEQUAL;
			}
			else if (!Q_strcasecmp(token, "equal"))
			{
				pass->depth_func = GL_EQUAL;
			}
			else if (!Q_strcasecmp(token, "lequal"))
			{
				pass->depth_func = GL_LEQUAL;
			}
			else if (!Q_strcasecmp(token, "less"))
			{
				pass->depth_func = GL_LESS;
			}
			else if (!Q_strcasecmp(token, "always"))
			{
				pass->depth_func = GL_ALWAYS;
			}
			else if (!Q_strcasecmp(token, "never"))
			{
				pass->depth_func = GL_NEVER;
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown depthfunc type (%s) in Pass", mod->name, token);
			}
		}

		else if (Q_stricmp(token, "depthwrite") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (pass->depth_write != -1)
				ri.Sys_Error (ERR_DROP, "%s only one depthwrite allowed per Pass", mod->name);

			i = atoi(token);
			if (i == 1)
				pass->depth_write = true;
			else if (i == 0)
				pass->depth_write = false;
			else
				ri.Sys_Error (ERR_DROP, "%s unknown depthwrite value (%s) in Pass", mod->name, token);
		}

		else if (Q_stricmp(token, "rgbgen") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			if (!Q_strcasecmp(token, "identity"))
			{
				pass->rgbgen = MDA_RGBGEN_IDENTITY;
			}
			else if (!Q_strcasecmp(token, "ambient"))
			{
				pass->rgbgen = MDA_RGBGEN_AMBIENT;
			}
			else if (!Q_strcasecmp(token, "diffusezero"))
			{
				pass->rgbgen = MDA_RGBGEN_DIFFUSEZERO;
			}
			else if (!Q_strcasecmp(token, "diffuse"))
			{
				pass->rgbgen = MDA_RGBGEN_DIFFUSE;
			}
			else
			{
				ri.Sys_Error (ERR_DROP, "%s unknown rgbgen type (%s) in Pass", mod->name, token);
			}
		}

	}

	return pass;

}

// Parse a profile
mda_skin_t* Mod_ParseSkin (model_t *mod, char **tokens, int *mask)
{
	float		solid[2], trans[2], *type, minmax[2];
	qboolean	sort_blend_explicit = false;
	char		*token;
	mda_pass_t	*last_pass = 0;
	mda_pass_t	*pass = 0;
	mda_skin_t	*skin = Hunk_Alloc(sizeof(mda_skin_t));
	memset(skin, 0, sizeof(mda_skin_t));

	solid[0] = trans[0] = 1;
	solid[1] = trans[1] = 0;

	token = COM_Parse2 (tokens);
	// Corrupt MDA
	if (!*tokens) return NULL;
	// Corrupt MDA
	if (token[0] != '{') return NULL;

	while (1) 
	{
		token = COM_Parse2 (tokens);
		// Corrupt MDA
		if (!*tokens) return NULL;

		// end of profile
		if (Q_stricmp(token, "}") == 0)
		{
			break;
		}
		// Sort order
		else if (Q_stricmp(token, "sort") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;

			sort_blend_explicit = true;

			if (!Q_strcasecmp(token, "opaque"))
				skin->sort_blend = false;
			else if (!Q_strcasecmp(token, "blend"))
				skin->sort_blend = true;
			else
				ri.Sys_Error (ERR_DROP, "%s unknown sort order type (%s) in Skin", mod->name, token);
		}
		// Pass parsing
		else if (Q_stricmp(token, "pass") == 0)
		{
			pass = Mod_ParsePass(mod, tokens, minmax);
			if (!pass) return NULL;

			if (!last_pass) skin->passes = pass;
			else last_pass->next = pass;
			last_pass = pass;

				// Solid pass (depth write default enabled if not equal depth func)
			if (pass->src_blend == GL_ONE && pass->dest_blend == GL_ZERO)
			{
				if (pass->depth_write == -1)
				{
					if (pass->depth_func == GL_EQUAL) pass->depth_write = 0;
					else pass->depth_write = 1;
				}
				type = solid;
			}
			else // Trans pass (depth write default disabled)
			{
				if (pass->depth_write == -1) pass->depth_write = 0;
				type = trans;
			}

			if (minmax[0] < type[0]) type[0] = minmax[0];
			else if (minmax[1] > type[1]) type[1] = minmax[1];

			*mask = pass->rgbgen | pass->uvgen;
		}
	}

	// In here we are supposed to work out sort order and stuff
	if (skin->passes) 
	{
		// Sort type calculating (from range of coverage of each type)
		if (!sort_blend_explicit && (trans[0] < solid[0] || trans[1] > solid[1]))
			skin->sort_blend = true;

		return skin;
	}

	ri.Sys_Error (ERR_DROP, "%s Skin didn't have any passes", mod->name);

	return NULL;
}

// Parse a profile
mda_profile_t* Mod_ParseProfile (model_t *mod, dmdl_anox_t *anox, CHAR **tokens)
{
	int			num_skins = 0;
	int			mask = 0;
	char		*token;
	mda_skin_t	*last_skin = 0;
	mda_skin_t	*skin = 0;
	mda_profile_t	*profile = Hunk_Alloc(sizeof(mda_profile_t));
	memset(profile, 0, sizeof(mda_profile_t));

	token = COM_Parse2 (tokens);
	// Corrupt MDA
	if (!*tokens) return NULL;

	// Profile name
	if (token[0] != '{')
	{
		profile->name = *(int*)token;

		token = COM_Parse2 (tokens);
		// Corrupt MDA
		if (!*tokens) return NULL;
	}	

	// Corrupt MDA
	if (token[0] != '{') return NULL;

	while (1) 
	{
		token = COM_Parse2 (tokens);
		// Corrupt MDA
		if (!*tokens) return NULL;

		// end of profile
		if (Q_stricmp(token, "}") == 0)
		{
			break;
		}
		// Script evaluation
		else if (Q_stricmp(token, "evaluate") == 0)
		{
			token = COM_Parse2 (tokens);
			// Corrupt MDA
			if (!*tokens) return NULL;
		}
		// Skin parsing
		else if (Q_stricmp(token, "skin") == 0)
		{
			skin = Mod_ParseSkin(mod, tokens, &mask);
			if (!skin) return NULL;

			if (!last_skin) profile->skins = skin;
			else last_skin->next = skin;
			last_skin = skin;

			if (skin->sort_blend) profile->alpha_gen_mask |= mask;
			else profile->opaque_gen_mask |= mask;

			num_skins++;
		}
	}

	//if (num_skins != anox->num_passes)
	//	ri.Sys_Error (ERR_DROP, "%s number of skins (%i) in profile didn't didn't match number of emit passes in MD2", mod->name, num_skins, anox->num_passes);

	if (!profile->skins) 
		ri.Sys_Error (ERR_DROP, "%s Profile didn't have any skins", mod->name);

	return profile;
}

void Mod_LoadMDAModel (model_t *mod, void *buffer)
{
	dmdl_anox_t *anox = (dmdl_anox_t *)mod->extradata;
	int			i, j;
	char		*data, *mdastring;
	char		*token;
	qboolean	loaded_md2 = false;
	qboolean	dead = true;
	qboolean	profiles_generated = false;
	mda_profile_t	*profile = 0;
	mda_profile_t	*last_profile = 0;

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
			int			our_len = modfilelen;
			unsigned	*md2buf;

			token = COM_Parse2 (&data);

			// Corrupt MDA
			if (!data) break;		// Screwed

			// Load the md2
			modfilelen = ri.FS_LoadFile (token, &md2buf);

			// Dead
			if (!md2buf) ri.Sys_Error (ERR_DROP, "Mod_NumForName: %s not found", mod->name);
         
			Mod_LoadAliasModel(mod,md2buf,false);
			ri.FS_FreeFile(md2buf);

			modfilelen = our_len; 

			loaded_md2 = true;
			break;
		}
	}

	// Uh oh, we're in trouble now
	if (!loaded_md2) 
	{
		// Free the MDA string
		free (mdastring);

		ri.Sys_Error (ERR_DROP, "%s didn't have a basemodel specified or the MD2 couldn't be loaded", mod->name);
	}

	//
	// Now read profiles
	//

	data = mdastring;
	while (1) 
	{
		// Ok, we got this far, so thing are looking ok.... for now
		token = COM_Parse2 (&data);

		// This is the only spot where we are allowed to actually leave without causing an error
		if (!data) 
		{
			dead = false;
			break;
		}

		// Basemodel specifies the MD2
		if (Q_stricmp(token, "basemodel") == 0)
		{
			token = COM_Parse2 (&data);

			// Corrupt MDA
			if (!data) break;		// Screwed
		}
		// Profiles are shader definitions. Maybe Handle this later
		else if (Q_stricmp(token, "profile") == 0) 
		{
			profile = Mod_ParseProfile (mod, anox, &data);

			if (!profile) break;

			if (!last_profile) mod->profiles = profile;
			else last_profile->next = profile;
			last_profile = profile;

			if (profile->alpha_gen_mask) mod->mda_blend = true;
			if (profile->opaque_gen_mask) mod->mda_opaque = true;

		}
		// headtri is used to orientate morphs
		else if (Q_stricmp(token, "headtri") == 0) 
		{
			int tris[3];

			token = COM_Parse2 (&data);
			if (!data) break;		// Corrupt MDA
			tris[0] = atoi(token);

			token = COM_Parse2 (&data);
			if (!data) break;		// Corrupt MDA
			tris[1] = atoi(token);

			token = COM_Parse2 (&data);
			if (!data) break;		// Corrupt MDA
			tris[2] = atoi(token);

			// Now do what????
		}
		// We can't deal with this stuff (Morphs)
		else if (token[0] == '$' || token[0] == '&') 
		{
		}
	}

	// Free the MDA string
	free (mdastring);

	// Failed somewhere
	if (dead)
	{
		ri.Sys_Error (ERR_DROP, "%s Corrupt MDA file", mod->name);
	}
	else if (!mod->profiles)
	{
		Mod_AutoGenAnoxProfile(mod, anox);
	}
}
/*

MDA profile keywords

profile DFLT					- default profile
profile XXXX					- profile XXXX
profile							- Unnamed profile (DFLT if no others i guess)
{
    evaluate "Party_Wears_Labcoats"	- May be used to specify a different default skin when specific flags are set

    skin
    {
        sort blend				- do in 'blend' phase
        sort opaque				- do in 'opaque' phase

        pass
        {
            map "texturename"	- normal wrap texture mode
            clampmap "texturename"	- clamped texture mode
            blendmode add	
            blendmode multiply
			blendmode none		- (default)
			blendmode normal	- GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
            cull disable		- 
			cull back			- 
			cull front			- (default)
            uvgen base			- (default)
            uvgen sphere		- Generate sphere map coords 
            uvgen parabxpos		- Generate positive parabolic env map coords
			uvgen parabxneg		- Generate negetive parabolic env map coords
			uvmod scroll x y    - uv scroll (per second?)
			alphafunc ge128
			alphafunc lt127
			alphafunc ge64
			alphafunc gt0
			depthfunc notequal	
			depthfunc greater
			depthfunc gequal	
			depthfunc equal	
			depthfunc lequal	- (default)	
			depthfunc less
			depthfunc always
			depthfunc never
			depthwrite 1		- force depth writing this frame (disabled for blended and equal depth func)
			rgbgen identity		- 1
			rgbgen ambient		- Ambient light level
            rgbgen diffusezero	- infinite diffuse lighting from 0,0,1 with no ambient
            rgbgen diffuse      - (default)
        }
    }

// Defaults
depth writing is enabled for opaque, disabled for blended
blendmode NONE
cull front


*/
/*
==============================================================================

SPRITE MODELS

==============================================================================
*/

/*
=================
Mod_LoadSpriteModel
=================
*/
void Mod_LoadSpriteModel (model_t *mod, void *buffer)
{
	dsprite_t	*sprin, *sprout;
	int			i;

	sprin = (dsprite_t *)buffer;
	sprout = Hunk_Alloc (modfilelen);

	sprout->ident = LittleLong (sprin->ident);
	sprout->version = LittleLong (sprin->version);
	sprout->numframes = LittleLong (sprin->numframes);

	if (sprout->version != SPRITE_VERSION)
		ri.Sys_Error (ERR_DROP, "%s has wrong version number (%i should be %i)",
				 mod->name, sprout->version, SPRITE_VERSION);

	if (sprout->numframes > MAX_MD2SKINS)
		ri.Sys_Error (ERR_DROP, "%s has too many frames (%i > %i)",
				 mod->name, sprout->numframes, MAX_MD2SKINS);

	// byte swap everything
	for (i=0 ; i<sprout->numframes ; i++)
	{
		sprout->frames[i].width = LittleLong (sprin->frames[i].width);
		sprout->frames[i].height = LittleLong (sprin->frames[i].height);
		sprout->frames[i].origin_x = LittleLong (sprin->frames[i].origin_x);
		sprout->frames[i].origin_y = LittleLong (sprin->frames[i].origin_y);
		memcpy (sprout->frames[i].name, sprin->frames[i].name, MAX_SKINNAME);
		mod->skins[i] = GL_FindImage (sprout->frames[i].name,0,
			it_sprite);
	}

	mod->type = mod_sprite;
}

//=============================================================================

/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginRegistration

Specifies the model that will be used as the world
@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndRegistration (void);
void R_BeginRegistration (char *model)
{
	char	fullname[MAX_QPATH];
	cvar_t	*flushmap;

	registration_sequence++;

	/* Ok, we will free everything first before entering a new level */
	R_EndRegistration();

	r_oldviewcluster = -1;		// force markleafs

	Com_sprintf (fullname, sizeof(fullname), "maps/%s.bsp", model);

	// explicitly free the old map if different
	// this guarantees that mod_known[0] is the world map
	//flushmap = ri.Cvar_Get ("flushmap", "0", 0);
	//if ( strcmp(mod_known[0].name, fullname) || flushmap->value)
		Mod_Free (&mod_known[0]);
	r_worldmodel = Mod_ForName(fullname, true);

	r_viewcluster = -1;
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_RegisterModel

@@@@@@@@@@@@@@@@@@@@@
*/
struct model_s *R_RegisterModel (char *name)
{
	model_t	*mod;
	int		i;
	dsprite_t	*sprout;
	dmdl_t		*pheader;

	mod = Mod_ForName (name, false);
	if (mod)
	{
		mod->registration_sequence = registration_sequence;

		// register any images used by the models
		if (mod->type == mod_sprite)
		{
			sprout = (dsprite_t *)mod->extradata;
			for (i=0 ; i<sprout->numframes ; i++)
				mod->skins[i] = GL_FindImage (sprout->frames[i].name, 0,it_sprite);
		}
		else if (mod->type == mod_alias)
		{
			pheader = (dmdl_t *)mod->extradata;

			if (mod->profiles)
			{
				mda_profile_t *profile;
				mda_skin_t *skin;
				mda_pass_t *pass;

				for (profile = mod->profiles; profile != NULL; profile = profile->next)
				{
					for (skin = profile->skins; skin != NULL; skin = skin->next)
					{
						for (pass = skin->passes; pass != NULL; pass = pass->next)
						{
							pass->image = GL_FindImage (pass->imagename,0, it_skin);
						}
					}
				}
			}
			else
			{
				for (i=0 ; i<pheader->num_skins ; i++)
				{
					if (pheader->version != ALIAS_VERSION) {
						// Anox skins need model path appended to start of skinname
						char skinname[MAX_QPATH];
						COM_FilePath(mod->name, skinname);
						strcat(skinname, "/");
						strcat(skinname, (char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME);

						mod->skins[i] = GL_FindImage (skinname, 0,it_skin);
					}
					else
						mod->skins[i] = GL_FindImage ((char *)pheader + pheader->ofs_skins + i*MAX_SKINNAME, 0,it_skin);
				}
			}
//PGM
			mod->numframes = pheader->num_frames;
//PGM
		}
		else if (mod->type == mod_brush)
		{
			for (i=0 ; i<mod->numtexinfo ; i++)
				mod->texinfo[i].image->registration_sequence = registration_sequence;
		}
	}
	return mod;
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_EndRegistration

@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndRegistration (void)
{
	int		i;
	model_t	*mod;

	for (i=0, mod=mod_known ; i<mod_numknown ; i++, mod++)
	{
		if (!mod->name[0])
			continue;
		if (mod->registration_sequence != registration_sequence)
		{	// don't need this model
			Mod_Free (mod);
		}
	}

	GL_FreeUnusedImages ();
}


//=============================================================================


/*
================
Mod_Free
================
*/
void Mod_Free (model_t *mod)
{
	Hunk_Free (mod->extradata);
	memset (mod, 0, sizeof(*mod));
}

/*
================
Mod_FreeAll
================
*/
void Mod_FreeAll (void)
{
	int		i;

	for (i=0 ; i<mod_numknown ; i++)
	{
		if (mod_known[i].extradatasize)
			Mod_Free (&mod_known[i]);
	}
}
