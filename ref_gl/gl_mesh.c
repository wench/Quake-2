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
// gl_mesh.c: triangle model functions

#include "gl_local.h"

/*
=============================================================

  ALIAS MODELS

=============================================================
*/

#define NUMVERTEXNORMALS	162

typedef float vec4_t[4];

static	vec4_t	s_lerped[MAX_VERTS];
static	vec4_t	s_lerped_norms[MAX_VERTS];
static	float colorArray[MAX_VERTS*4];
//static	vec3_t	lerped[MAX_VERTS];

vec3_t	shadevector;
vec3_t	shadevector2;
float	shadelight[4];
static qboolean invert_cull = false;
static float diffuse_strength[4];


// precalculated dot products for quantized angles
#define SHADEDOT_QUANT 16
float	r_avertexnormal_dots[SHADEDOT_QUANT][256] =
#include "anormtab.h"
;

mda_profile_t *mda_profile = 0;

mda_pass_t mda_pass_generic = {
	"",
	NULL,
	GL_FRONT,
	GL_ONE, GL_ZERO,
	1,
	GL_LEQUAL,
	GL_ALWAYS,
	0,
	MDA_RGBGEN_DIFFUSE,
	MDA_UVGEN_BASE,
	0, 0,
	NULL
};

mda_skin_t mda_skin_generic = {
	false,
	&mda_pass_generic,
	NULL
};

mda_profile_t mda_profile_generic =
{
	// Name (profile name)
	0,
	// opaque_gen_mask
	MDA_UVGEN_BASE|MDA_RGBGEN_DIFFUSE,
	// alpha_gen_mask
	0,
	// skins
	&mda_skin_generic,
	// next
	NULL
};

//float	*shadedots = r_avertexnormal_dots[0];

void GL_LerpVerts( int nverts, dtrivertx_t *v, dtrivertx_t *ov, dtrivertx_t *verts, float *lerp, float move[3], float frontv[3], float backv[3], float *normal, float frontlerp, float backlerp )
{
	int i;

	//PMM -- added RF_SHELL_DOUBLE, RF_SHELL_HALF_DAM
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
	{
		for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4, normal+=4 )
		{
//			float *normal = r_avertexnormals[verts[i].lightnormalindex];
			
			normal[0] = ov->normal[0]*backlerp + v->normal[0]*frontlerp;
			normal[1] = ov->normal[1]*backlerp + v->normal[1]*frontlerp;
			normal[2] = ov->normal[2]*backlerp + v->normal[2]*frontlerp; 
			// Normalize??

			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0] + normal[0] * POWERSUIT_SCALE;
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1] + normal[1] * POWERSUIT_SCALE;
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2] + normal[2] * POWERSUIT_SCALE; 
		}
	}
	else
	{
		for (i=0 ; i < nverts; i++, v++, ov++, lerp+=4, normal+=4)
		{
			normal[0] = ov->normal[0]*backlerp + v->normal[0]*frontlerp;
			normal[1] = ov->normal[1]*backlerp + v->normal[1]*frontlerp;
			normal[2] = ov->normal[2]*backlerp + v->normal[2]*frontlerp; 
			// Normalize??

			lerp[0] = move[0] + ov->v[0]*backv[0] + v->v[0]*frontv[0];
			lerp[1] = move[1] + ov->v[1]*backv[1] + v->v[1]*frontv[1];
			lerp[2] = move[2] + ov->v[2]*backv[2] + v->v[2]*frontv[2];
		}
	}

}

/*
=============
GL_DrawAliasFrameLerp

interpolates between two frames and origins
FIXME: batch lerp all vertexes
=============
*/
void GL_DrawAliasFrameLerp (dmdl_t *paliashdr, float backlerp, qboolean transpass)
{
	daliasframe_t	*frame, *oldframe;
	dtrivertx_t	*v, *ov, *verts;
	int		*order;
	int		count;
	float	frontlerp;
	vec3_t	move, delta, vectors[3];
	vec3_t	frontv, backv;
	int		i;
	int		index_xyz;
	float	*lerp;
	dmdl_anox_t *anox = (dmdl_anox_t*)paliashdr;
	int		anox_pass = 0;
	short	*anox_passes;
	float	rgb_shadelight[3];
	mda_skin_t	*mda_skin = 0;
	mda_pass_t	*mda_pass = 0;
	vec3_t	scaled_shadevector;
	int			gen_mask = 0;
	qboolean	pop_tmatrix = false;
	qboolean	kill_tex_gen = false;
	qboolean	kill_cubemap = false;
	qboolean	kill_multitex = false;
	int			tmu = GL_TEXTURE0;

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = v = frame->verts;

	oldframe = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->oldframe * paliashdr->framesize);
	ov = oldframe->verts;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

//	glTranslatef (frame->translate[0], frame->translate[1], frame->translate[2]);
//	glScalef (frame->scale[0], frame->scale[1], frame->scale[2]);

	// PMM - added double shell
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
		qglDisable( GL_TEXTURE_2D );

	frontlerp = 1.0 - backlerp;


	// move should be the delta back to the previous frame * backlerp
	VectorSubtract (currententity->oldorigin, currententity->origin, delta);
	AngleVectors (currententity->angles, vectors[0], vectors[1], vectors[2]);

	move[0] = DotProduct (delta, vectors[0]);	// forward
	move[1] = -DotProduct (delta, vectors[1]);	// left
	move[2] = DotProduct (delta, vectors[2]);	// up

	VectorAdd (move, oldframe->translate, move);

	for (i=0 ; i<3 ; i++)
	{
		move[i] = (backlerp*move[i] + frontlerp*frame->translate[i])*currententity->scale[i];
		frontv[i] = frontlerp*frame->scale[i]*currententity->scale[i];
		backv[i] = backlerp*oldframe->scale[i]*currententity->scale[i];
	}

	lerp = s_lerped[0];

	GL_LerpVerts( paliashdr->num_xyz, v, ov, verts, lerp, move, frontv, backv, s_lerped_norms[0], frontlerp, backlerp );

	anox_passes = (short *)((byte *)anox + anox->ofs_passes);
	if (transpass) gen_mask = mda_profile->alpha_gen_mask;
	else gen_mask = mda_profile->opaque_gen_mask;

	rgb_shadelight[0] = currententity->rgb[0];
	rgb_shadelight[1] = currententity->rgb[1];
	rgb_shadelight[2] = currententity->rgb[2];
	VectorScale(shadevector, 0.9, scaled_shadevector);

	qglEnableClientState( GL_VERTEX_ARRAY );
	qglVertexPointer( 3, GL_FLOAT, 16, s_lerped );	// padded for SIMD

	qglNormalPointer( GL_FLOAT, 16, s_lerped_norms );	// padded for SIMD
	qglEnableClientState( GL_NORMAL_ARRAY );

//		if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) )
	// PMM - added double damage shell
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
	{
		qglColor4f( rgb_shadelight[0], rgb_shadelight[1], rgb_shadelight[2], shadelight[3] );
	}
	else
	{
		// We can actually do this with per pixel with a cube map and DOT3
		if (gen_mask & MDA_RGBGEN_DIFFUSEZERO)
		{
			if (gl_config.have_cube_map && gl_config.have_dot3)
			{
				qglTexCoordPointer( 3, GL_FLOAT, 16, s_lerped_norms );
			}
			else
			{
				qglColorPointer( 3, GL_FLOAT, 0, colorArray );

				//
				// pre light everything
				//
				for ( i = 0; i < paliashdr->num_xyz; i++ )
				{
					float l = -DotProduct(s_lerped_norms[i],shadevector2);

					colorArray[i*3+0] = l * diffuse_strength[0];
					colorArray[i*3+1] = l * diffuse_strength[1];
					colorArray[i*3+2] = l * diffuse_strength[2];
				}
			}
		}
	}

	if ( qglLockArraysEXT != 0 )
		qglLockArraysEXT( 0, paliashdr->num_xyz );

	anox_pass = 0;

	while (1) 
	{
		int first_cmd;
		int last_cmd;
		int cmd_counter;

		// Increment pass
		if (mda_pass) mda_pass = mda_pass->next;		

		// Increment skin
		if (!mda_pass)
		{
			if (!mda_skin) 
			{
				mda_skin = mda_profile->skins;
				anox_pass = 0;
			}
			else 
			{
				mda_skin = mda_skin->next;
				anox_pass++;
			}

			// Make sure the pass type is correct
			while (mda_skin)
			{
				// If trans, just do all
				if (mda_skin->sort_blend == transpass || (currententity->flags & RF_TRANSLUCENT))
					break;

				mda_skin = mda_skin->next;
				anox_pass++;
			}

			// We are done
			if (!mda_skin) break;

			mda_pass = mda_skin->passes;
		}

		first_cmd = 0;
		last_cmd = 0;

		// Get cmds
		for (i = 0; i <= anox_pass; i++)
		{
			first_cmd = last_cmd;
			last_cmd += anox_passes[i];
		}

		// Setup our state

		// Cull
		if (mda_pass->cull_mode == GL_NEVER ) 
		{
			qglDisable(GL_CULL_FACE);
		}
		else
		{
			qglEnable(GL_CULL_FACE);

			if (!invert_cull)
				qglCullFace(mda_pass->cull_mode);
			else if (mda_pass->cull_mode == GL_BACK)
				qglCullFace(GL_FRONT);
			else if (mda_pass->cull_mode == GL_FRONT)
				qglCullFace(GL_BACK);
		}


		// Alpha Func
		if (mda_pass->alpha_func == GL_ALWAYS)
			qglDisable(GL_ALPHA_TEST);
		else
		{
			qglAlphaFunc(mda_pass->alpha_func, mda_pass->alpha_test_ref);
			qglEnable(GL_ALPHA_TEST);
		}

		// Blending
		if (mda_pass->src_blend == GL_ONE && mda_pass->dest_blend == GL_ZERO)
		{
			if (transpass)
			{
				qglBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
				qglEnable(GL_BLEND);
			}
			else
				qglDisable(GL_BLEND);
		}
		else
		{
			qglBlendFunc(mda_pass->src_blend, mda_pass->dest_blend);
			qglEnable(GL_BLEND);
		}

		// Depth write
		qglDepthMask(mda_pass->depth_write);

		// Depth func
		qglDepthFunc(mda_pass->depth_func);

		// RGB Gen
		if (mda_pass->rgbgen == MDA_RGBGEN_IDENTITY)
		{
			qglDisable(GL_LIGHTING);
			qglColor4f(1.0, 1.0, 1.0, 1.0);
		}
		else if (mda_pass->rgbgen == MDA_RGBGEN_AMBIENT)
		{
			qglDisable(GL_LIGHTING);
			qglColor4f(0.1, 0.1, 0.1, 1.0);
		}
		else if (mda_pass->rgbgen == MDA_RGBGEN_DIFFUSEZERO)
		{
			if (gl_config.have_cube_map && gl_config.have_dot3)
			{
				float vec[4];
				GL_BindImage(r_norm_cube);
				qglEnable (GL_TEXTURE_CUBE_MAP_ARB);

				GL_EnableMultitexture( true );

				qglLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, TRUE);

				qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
				qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
				qglTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_ARB);
				qglEnable( GL_TEXTURE_GEN_S );
				qglEnable( GL_TEXTURE_GEN_T );
				qglEnable( GL_TEXTURE_GEN_R );
				kill_tex_gen = true;

				VectorSet(vec,(1+shadevector2[1])/2,
							(1+shadevector2[2])/2,
							(1+shadevector2[0])/2);
				vec[3] = 1;
				qglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, vec);

				GL_TexEnv(GL_COMBINE_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB);
				
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
				
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

				qglActiveTextureARB( GL_TEXTURE1_ARB+1 );
				qglClientActiveTextureARB( GL_TEXTURE1_ARB+1 );
				qglBindTexture(GL_TEXTURE_2D, r_notexture->texnum);
				qglEnable(GL_TEXTURE_2D);

				qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
				qglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
				qglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
				
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PRIMARY_COLOR_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PRIMARY_COLOR_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
				
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
				qglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
				qglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);

				GL_SelectTexture( GL_TEXTURE1 );
				GL_TexEnv(GL_MODULATE);

				tmu = GL_TEXTURE1;
				kill_cubemap = true;
				kill_multitex = true;

				//qglColor4fv(diffuse_strength);
				qglColor4f(
					diffuse_strength[0]*1.25+0.1,
					diffuse_strength[1]*1.25+0.1,
					diffuse_strength[2]*1.25+0.1,
					diffuse_strength[3]);
			}
			else
				qglEnableClientState( GL_COLOR_ARRAY );

			qglDisable(GL_LIGHTING);
		}
		else //if (mda_pass->rgbgen == MDA_RGBGEN_DIFFUSE)
		{
			qglEnable(GL_LIGHTING);
			//qglDisable(GL_LIGHTING);
			//qglColor4f(0,0,0,1);
		}
		
		// UV Get
		if (mda_pass->uvgen == MDA_UVGEN_SPHERE)
		{
			qglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			qglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			qglEnable(GL_TEXTURE_GEN_S);
			qglEnable(GL_TEXTURE_GEN_T);
			kill_tex_gen = true;
		}
		else // if (mda_pass->uvgen == MDA_UVGEN_BASE)
		{
			// We don't actually need to do anything
		}

		// UV Scroll
		if (mda_pass->uvscroll[0] || mda_pass->uvscroll[1])
		{
			float u = (r_newrefdef.time*mda_pass->uvscroll[0]) - (int)(r_newrefdef.time*mda_pass->uvscroll[0]);
			float v = (r_newrefdef.time*mda_pass->uvscroll[1]) - (int)(r_newrefdef.time*mda_pass->uvscroll[1]);

			qglMatrixMode(GL_TEXTURE);
			if (!pop_tmatrix) qglPushMatrix();
			qglTranslatef(u, v, 0);
			qglMatrixMode( GL_MODELVIEW );
			pop_tmatrix = true;
		}

		// Bind the correct texture
		GL_MBindImage(tmu, mda_pass->image);

		// Increment the pass counter
		cmd_counter = 0;

		order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

		while (1)
		{
			// get the vertex count and primitive type
			count = *order++;
			if (!count)
				break;		// done

			if (last_cmd && cmd_counter >= last_cmd)
				break;		// done

			// Skip crap if required
			if (first_cmd && cmd_counter < first_cmd) 
			{
				if (count < 0) order -= count * 3;
				else order += count * 3;
				cmd_counter++;	// Increment the counter
				continue;
			}
			cmd_counter++;		// Increment the counter

			if (count < 0)
			{
				count = -count;
				qglBegin (GL_TRIANGLE_FAN);
			}
			else
			{
				qglBegin (GL_TRIANGLE_STRIP);
			}

			// PMM - added double damage shell
			if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
			{
				do
				{
					index_xyz = order[2];
					order += 3;

					//qglNormal3fv( s_lerped_norms[index_xyz] );
					qglVertex3fv( s_lerped[index_xyz] );

				} while (--count);
			}
			else
			{
				if (tmu == GL_TEXTURE0) 
				{
					do
					{
						// texture coordinates come from the draw list
						qglTexCoord2f (((float *)order)[0], ((float *)order)[1]);
						index_xyz = order[2];

						order += 3;

						qglArrayElement( index_xyz );

					} while (--count);
				}
				else
				{
					do
					{
						// texture coordinates come from the draw list
						qglMTexCoord2fSGIS( tmu, ((float *)order)[0], ((float *)order)[1]);
						index_xyz = order[2];

						order += 3;

						qglArrayElement( index_xyz );

					} while (--count);
				}
			}
			qglEnd ();
		}

		if ( kill_multitex )
		{
			qglActiveTextureARB( GL_TEXTURE1_ARB+1 );
			qglClientActiveTextureARB( GL_TEXTURE1_ARB+1 );
			qglDisable(GL_TEXTURE_2D);
			qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			qglActiveTextureARB( GL_TEXTURE1_ARB );
			qglClientActiveTextureARB( GL_TEXTURE1_ARB );

			tmu = GL_TEXTURE0;
			GL_EnableMultitexture( false );
			GL_TexEnv( GL_MODULATE );
			kill_multitex = false;
		}

		if ( pop_tmatrix )
		{
			qglMatrixMode( GL_TEXTURE );
			qglPopMatrix();
			qglMatrixMode( GL_MODELVIEW );
			pop_tmatrix = false;
		}

		if ( kill_tex_gen )
		{
			qglDisable( GL_TEXTURE_GEN_S );
			qglDisable( GL_TEXTURE_GEN_T );
			qglDisable( GL_TEXTURE_GEN_R );
			kill_tex_gen = false;
		}

		if ( kill_cubemap )
		{
			qglDisable ( GL_TEXTURE_CUBE_MAP_ARB );
			if (gl_config.have_cube_map && gl_config.have_dot3)
				qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
			else
				qglDisableClientState( GL_COLOR_ARRAY );
			GL_TexEnv( GL_MODULATE );
			kill_cubemap = false;
		}

		qglColor4f(1,1,1,diffuse_strength[3]);
		qglEnable (GL_LIGHTING);
	}

	if ( qglUnlockArraysEXT != 0 )
		qglUnlockArraysEXT();

//	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE ) )
	// PMM - added double damage shell
	if ( currententity->flags & ( RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_DOUBLE | RF_SHELL_HALF_DAM) )
		qglEnable( GL_TEXTURE_2D );
}


#if 1
/*
=============
GL_DrawAliasShadow
=============
*/
extern	vec3_t			lightspot;

void GL_DrawAliasShadow (dmdl_t *paliashdr, int posenum)
{
	dtrivertx_t	*verts;
	int		*order;
	vec3_t	point;
	float	height, lheight;
	int		count;
	daliasframe_t	*frame;

	lheight = currententity->origin[2] - lightspot[2];

	frame = (daliasframe_t *)((byte *)paliashdr + paliashdr->ofs_frames 
		+ currententity->frame * paliashdr->framesize);
	verts = frame->verts;

	height = 0;

	order = (int *)((byte *)paliashdr + paliashdr->ofs_glcmds);

	height = -lheight + 1.0;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			qglBegin (GL_TRIANGLE_FAN);
		}
		else
			qglBegin (GL_TRIANGLE_STRIP);

		do
		{
			// normals and vertexes come from the frame list
/*
			point[0] = verts[order[2]].v[0] * frame->scale[0] + frame->translate[0];
			point[1] = verts[order[2]].v[1] * frame->scale[1] + frame->translate[1];
			point[2] = verts[order[2]].v[2] * frame->scale[2] + frame->translate[2];
*/

			memcpy( point, s_lerped[order[2]], sizeof( point )  );

			point[0] -= shadevector[0]*(point[2]+lheight);
			point[1] -= shadevector[1]*(point[2]+lheight);
			point[2] = height;
//			height -= 0.001;
			qglVertex3fv (point);

			order += 3;

//			verts++;

		} while (--count);

		qglEnd ();
	}	
}

#endif

/*
** R_CullAliasModel
*/
static qboolean R_CullAliasModel( vec3_t bbox[8], entity_t *e )
{
	int i;
	vec3_t		mins, maxs;
	dmdl_t		*paliashdr;
	vec3_t		vectors[3];
	vec3_t		thismins, oldmins, thismaxs, oldmaxs;
	daliasframe_t *pframe, *poldframe;
	vec3_t angles;
	vec3_t max_vals = { 255, 255, 255 };

	paliashdr = (dmdl_t *)currentmodel->extradata;

	if ( ( e->frame >= paliashdr->num_frames ) || ( e->frame < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel %s: no such frame %d\n", 
			currentmodel->name, e->frame);
		e->frame = 0;
	}
	if ( ( e->oldframe >= paliashdr->num_frames ) || ( e->oldframe < 0 ) )
	{
		ri.Con_Printf (PRINT_ALL, "R_CullAliasModel %s: no such oldframe %d\n", 
			currentmodel->name, e->oldframe);
		e->oldframe = 0;
	}

	pframe = ( daliasframe_t * ) ( ( byte * ) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->frame * paliashdr->framesize);

	poldframe = ( daliasframe_t * ) ( ( byte * ) paliashdr + 
		                              paliashdr->ofs_frames +
									  e->oldframe * paliashdr->framesize);

	/*
	** compute axially aligned mins and maxs
	*/
	if (paliashdr->version == ALIAS_VERSION_ANOX_4_BYTE) {
		VectorSet(max_vals, 2047, 1023, 2047);
	}
	else if (paliashdr->version == ALIAS_VERSION_ANOX_6_BYTE) {
		VectorSet(max_vals, 65535, 65535, 65535);
	}

	if ( pframe == poldframe )
	{
		for ( i = 0; i < 3; i++ )
		{
			mins[i] = pframe->translate[i];
			maxs[i] = mins[i] + pframe->scale[i]*max_vals[i];

			mins[i] *= e->scale[i];
			maxs[i] *= e->scale[i];
		}
	}
	else
	{
		for ( i = 0; i < 3; i++ )
		{
			thismins[i] = pframe->translate[i];
			thismaxs[i] = thismins[i] + pframe->scale[i]*max_vals[i];

			oldmins[i]  = poldframe->translate[i];
			oldmaxs[i]  = oldmins[i] + poldframe->scale[i]*max_vals[i];

			if ( thismins[i] < oldmins[i] )
				mins[i] = thismins[i];
			else
				mins[i] = oldmins[i];

			if ( thismaxs[i] > oldmaxs[i] )
				maxs[i] = thismaxs[i];
			else
				maxs[i] = oldmaxs[i];

			mins[i] *= e->scale[i];
			maxs[i] *= e->scale[i];
		}
	}

	/*
	** compute a full bounding box
	*/
	for ( i = 0; i < 8; i++ )
	{
		vec3_t   tmp;

		if ( i & 1 )
			tmp[0] = mins[0];
		else
			tmp[0] = maxs[0];

		if ( i & 2 )
			tmp[1] = mins[1];
		else
			tmp[1] = maxs[1];

		if ( i & 4 )
			tmp[2] = mins[2];
		else
			tmp[2] = maxs[2];

		VectorCopy( tmp, bbox[i] );
	}

	/*
	** rotate the bounding box
	*/
	VectorCopy( e->angles, angles );
	angles[YAW] = -angles[YAW];
	AngleVectors( angles, vectors[0], vectors[1], vectors[2] );

	for ( i = 0; i < 8; i++ )
	{
		vec3_t tmp;

		VectorCopy( bbox[i], tmp );

		bbox[i][0] = DotProduct( vectors[0], tmp );
		bbox[i][1] = -DotProduct( vectors[1], tmp );
		bbox[i][2] = DotProduct( vectors[2], tmp );

		VectorAdd( e->origin, bbox[i], bbox[i] );
	}

	{
		int p, f, aggregatemask = ~0;

		for ( p = 0; p < 8; p++ )
		{
			int mask = 0;

			for ( f = 0; f < 4; f++ )
			{
				float dp = DotProduct( frustum[f].normal, bbox[p] );

				if ( ( dp - frustum[f].dist ) < 0 )
				{
					mask |= ( 1 << f );
				}
			}

			aggregatemask &= mask;
		}

		if ( aggregatemask )
		{
			return true;
		}

		return false;
	}
}

/*
=================
R_DrawAliasModel

=================
*/
void R_DrawAliasModel (entity_t *e, qboolean transpass)
{
	int			i;
	dmdl_t		*paliashdr;
	float		an;
	vec3_t		bbox[8];
	float		rgba[4];

	if ( !( e->flags & RF_WEAPONMODEL ) )
	{
		if ( R_CullAliasModel( bbox, e ) )
			return;
	}

	if ( e->flags & RF_WEAPONMODEL )
	{
		if ( r_lefthand->value == 2 )
			return;
	}

	paliashdr = (dmdl_t *)currentmodel->extradata;

	VectorCopy(e->rgb,rgba);
	// select profile/skin

	if (currententity->skin && paliashdr->version == ALIAS_VERSION)
	{
		mda_pass_generic.image = currententity->skin;	// custom player skin
		mda_profile = &mda_profile_generic;
	}
	else
	{
		mda_profile = currentmodel->profiles;
		i = 0;

		// Find profile
		while (mda_profile != NULL)
		{
			if (i == currententity->skinnum)
				break;

			if (mda_profile->name == currententity->skinnum)
				break;

			i++;
			mda_profile = mda_profile->next;
		}
	}
	
	if (!mda_profile && currentmodel->profiles)
		mda_profile = currentmodel->profiles;
	
	if (!mda_profile)
	{
		if (currententity->skin)
			mda_pass_generic.image = currententity->skin;	// custom player skin
		else
		{
			if (currententity->skinnum >= MAX_MD2SKINS)
				mda_pass_generic.image = currentmodel->skins[0];
			else
			{
				mda_pass_generic.image = currentmodel->skins[currententity->skinnum];
				if (!mda_pass_generic.image)
					mda_pass_generic.image = currentmodel->skins[0];
			}
		}
		if (!mda_pass_generic.image)
			mda_pass_generic.image = r_notexture;	// fallback...

		mda_profile = &mda_profile_generic;

		if ( currententity->flags & RF_TRANSLUCENT )
		{
			qglEnable (GL_BLEND);
		}
	}
	else if (transpass && !mda_profile->alpha_gen_mask && !(currententity->flags & RF_TRANSLUCENT))
	{
		return;
	}
	else if (!transpass && !mda_profile->opaque_gen_mask)
	{
		return;
	}
	GL_TexEnv( GL_MODULATE );

	if (currententity->flags & RF_TRANSLUCENT)
		rgba[3] = currententity->alpha;
	else
		rgba[3] = 1.0;

	VectorCopy(currententity->rgb,rgba);

	//
	// get lighting information
	//
	// PMM - rewrote, reordered to handle new shells & mixing
	// PMM - 3.20 code .. replaced with original way of doing it to keep mod authors happy
	//
	if ( currententity->flags & ( RF_SHELL_HALF_DAM | RF_SHELL_GREEN | RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE ) )
	{
		VectorClear (shadelight);
		if (currententity->flags & RF_SHELL_HALF_DAM)
		{
				shadelight[0] = 0.56;
				shadelight[1] = 0.59;
				shadelight[2] = 0.45;
		}
		if ( currententity->flags & RF_SHELL_DOUBLE )
		{
			shadelight[0] = 0.9;
			shadelight[1] = 0.7;
		}
		if ( currententity->flags & RF_SHELL_RED )
			shadelight[0] = 1.0;
		if ( currententity->flags & RF_SHELL_GREEN )
			shadelight[1] = 1.0;
		if ( currententity->flags & RF_SHELL_BLUE )
			shadelight[2] = 1.0;
	}
	else if ( currententity->flags & RF_FULLBRIGHT )
	{
		for (i=0 ; i<3 ; i++)
			shadelight[i] = 1.0;
	}
	else
	{
		R_LightPoint (currententity->origin, shadelight);

		// player lighting hack for communication back to server
		// big hack!
		if ( currententity->flags & RF_WEAPONMODEL )
		{
			// pick the greatest component, which should be the same
			// as the mono value returned by software
			if (shadelight[0] > shadelight[1])
			{
				if (shadelight[0] > shadelight[2])
					r_lightlevel->value = 150*shadelight[0];
				else
					r_lightlevel->value = 150*shadelight[2];
			}
			else
			{
				if (shadelight[1] > shadelight[2])
					r_lightlevel->value = 150*shadelight[1];
				else
					r_lightlevel->value = 150*shadelight[2];
			}

		}

		if ( currententity->flags & RF_MINLIGHT )
		{
			shadelight[0] = 0.1;
			shadelight[1] = 0.1;
			shadelight[2] = 0.1;
		}
		else
		{
			shadelight[0] = 0;
			shadelight[1] = 0;
			shadelight[2] = 0;
		}

		if ( currententity->flags & RF_GLOW )
		{	// bonus items will pulse with time
			float	scale;
			float	min;

			scale = 0.1 * sin(r_newrefdef.time*7);
			for (i=0 ; i<3 ; i++)
			{
				min = shadelight[i] * 0.8;
				shadelight[i] += scale;
				if (shadelight[i] < min)
					shadelight[i] = min;
			}
		}
	}

	R_RealLights(shadelight, rgba, diffuse_strength);
	diffuse_strength[3] = rgba[3];
	qglEnable(GL_LIGHTING);
	qglEnable(GL_NORMALIZE);

	diffuse_strength[0] *= rgba[0];
	diffuse_strength[1] *= rgba[1];
	diffuse_strength[2] *= rgba[2];

	qglColor4f(1,1,1,rgba[3]);
//	qglDisable(GL_LIGHTING);

// =================
// PGM	ir goggles color override
	if ( r_newrefdef.rdflags & RDF_IRGOGGLES && currententity->flags & RF_IR_VISIBLE)
	{
		shadelight[0] = 1.0;
		shadelight[1] = 0.0;
		shadelight[2] = 0.0;
	}
// PGM	
// =================

	//shadedots = r_avertexnormal_dots[((int)(currententity->angles[1] * (SHADEDOT_QUANT / 360.0))) & (SHADEDOT_QUANT - 1)];
	
	an = currententity->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);

	an=(r_newrefdef.viewangles[YAW])/180*M_PI;
	shadevector2[0] = 1+cos(-an);//;
	shadevector2[1] = sin(-an);//;
	shadevector2[2] = 0;
	VectorNormalize (shadevector2);

	//
	// locate the proper data
	//

	c_alias_polys += paliashdr->num_tris;

	//
	// draw all the triangles
	//
	if (currententity->flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		qglDepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));

	if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
	{
		extern void MYgluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar );

		qglMatrixMode( GL_PROJECTION );
		qglPushMatrix();
		qglLoadIdentity();
		qglScalef( -1, 1, 1 );
	    MYgluPerspective( r_newrefdef.fov_y, ( float ) r_newrefdef.width / r_newrefdef.height,  4,  16384);
		qglMatrixMode( GL_MODELVIEW );

		invert_cull = true;
	}
	else
	{
		invert_cull = false;
	}

    qglPushMatrix ();
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.
#if 1
	R_RotateForEntity (e);
#else
    qglTranslatef (e->origin[0],  e->origin[1],  e->origin[2]);
/*
	qglRotatef (r_newrefdef.viewangles[YAW]-180,  0, 0, 1);
    qglRotatef (-r_newrefdef.viewangles[PITCH],  0, 1, 0);
    //qglRotatef (-90,  0, 1, 0);
*/
	switch(GENNORM_SCREEN)
	{
	default:
	case GENNORM_SCREEN:
		{
			qglRotatef (r_newrefdef.viewangles[YAW]-180,  0, 0, 1);
			qglRotatef (-r_newrefdef.viewangles[PITCH],  0, 1, 0);
		}
		break;

	case GENNORM_UP:
		{
			qglRotatef (-90,  0, 1, 0);
		}
		break;

	case GENNORM_SPRITE:
		{
			qglRotatef (r_newrefdef.viewangles[YAW]-180,  0, 0, 1);
		}
		break;

	case GENNORM_DIR_GEN:
		{
			float	matrix[16];
			vec3_t	dir = {-1, 0, 0};				// Direction of Generator
			vec3_t	up = {0,0,1};

			// Forward (direction of generator)
			VectorNormalize2(dir,matrix);

			// Right. (Up is up if possible)
			CrossProduct(up, matrix, matrix+4);
			if(!VectorNormalize(matrix+4)) PerpendicularVector(matrix+4, matrix);

			// Up
			CrossProduct(matrix, matrix+4, matrix+8);

			matrix[3] = 0;
			matrix[7] = 0;
			matrix[11] = 0;
			matrix[12] = matrix[13] = matrix[14] = 0;
			matrix[15] = 1;

			qglMultMatrixf(matrix);
		}
		break;

	case GENNORM_UP_GEN:
		{
			float	matrix[16];
			vec3_t	dir = {1,0,0};				// Direction of Generator

			// Up (direction of generator)
			VectorNormalize2(dir,matrix+8);

			// Right (parallel to screen)
			CrossProduct(vpn, matrix+8, matrix+4);
			if(!VectorNormalize(matrix+4)) PerpendicularVector(matrix+4, matrix+8);

			// Forward
			CrossProduct(matrix+4, matrix+8, matrix);

			matrix[3] = 0;
			matrix[7] = 0;
			matrix[11] = 0;
			matrix[12] = matrix[13] = matrix[14] = 0;
			matrix[15] = 1;

			qglMultMatrixf(matrix);
		}
		break;

	case GENNORM_UP_GEN_FLAT:
		{
			float	matrix[16];
			vec3_t	dir = {1,1,100};				// Direction of Generator

			// Up (direction of generator)
			VectorNormalize2(dir,matrix+8);

			// Forward (direction particle faces) -> face up
			VectorSet(matrix,0,0,1);

			// Right
			CrossProduct(matrix+8, matrix, matrix+4);
			VectorNormalize(matrix+4);

			// Forward again
			CrossProduct(matrix+4, matrix+8, matrix);

			matrix[3] = 0;
			matrix[7] = 0;
			matrix[11] = 0;
			matrix[12] = matrix[13] = matrix[14] = 0;
			matrix[15] = 1;

			qglMultMatrixf(matrix);
		}
		break;
	};

	qglScalef(0.01, 1, 1);
	//glRotatef (e->angles[YAW],  0, 0, 1);
	//qglRotatef (-e->angles[PITCH],  0, 1, 0);
	//qglRotatef (-e->angles[ROLL],  1, 0, 0);
#endif
	e->angles[PITCH] = -e->angles[PITCH];	// sigh.

	// draw it

	qglShadeModel (GL_SMOOTH);

	if ( (currententity->frame >= paliashdr->num_frames) 
		|| (currententity->frame < 0) )
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such frame %d\n",
			currentmodel->name, currententity->frame);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ( (currententity->oldframe >= paliashdr->num_frames)
		|| (currententity->oldframe < 0))
	{
		ri.Con_Printf (PRINT_ALL, "R_DrawAliasModel %s: no such oldframe %d\n",
			currentmodel->name, currententity->oldframe);
		currententity->frame = 0;
		currententity->oldframe = 0;
	}

	if ( !r_lerpmodels->value )
		currententity->backlerp = 0;
	GL_DrawAliasFrameLerp (paliashdr, currententity->backlerp, transpass);

	GL_TexEnv( GL_REPLACE );
	qglShadeModel (GL_FLAT);

	qglPopMatrix ();

#if 0
	qglDisable( GL_CULL_FACE );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	qglDisable( GL_TEXTURE_2D );
	qglBegin( GL_TRIANGLE_STRIP );
	for ( i = 0; i < 8; i++ )
	{
		qglVertex3fv( bbox[i] );
	}
	qglEnd();
	qglEnable( GL_TEXTURE_2D );
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglEnable( GL_CULL_FACE );
#endif

	if ( ( currententity->flags & RF_WEAPONMODEL ) && ( r_lefthand->value == 1.0F ) )
	{
		qglMatrixMode( GL_PROJECTION );
		qglPopMatrix();
		qglMatrixMode( GL_MODELVIEW );
	}

	// Undo damage by profiles
	qglCullFace(GL_FRONT);
	qglEnable(GL_CULL_FACE);
	qglBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	qglDisable (GL_BLEND);
	qglDisable (GL_ALPHA_TEST);
	qglDisable(GL_LIGHTING);
	qglDisable(GL_NORMALIZE);

	if ( currententity->flags & RF_TRANSLUCENT )
		qglDepthMask (0);
	else
		qglDepthMask (1);

	if (currententity->flags & RF_DEPTHHACK)
		qglDepthRange (gldepthmin, gldepthmax);

#if 1
	if (gl_shadows->value && !(currententity->flags & (RF_TRANSLUCENT | RF_WEAPONMODEL)))
	{
		qglPushMatrix ();
		R_RotateForEntity (e);
		qglDisable (GL_TEXTURE_2D);
		qglEnable (GL_BLEND);
		qglColor4f (0,0,0,0.5);
		GL_DrawAliasShadow (paliashdr, currententity->frame );
		qglEnable (GL_TEXTURE_2D);
		qglDisable (GL_BLEND);
		qglPopMatrix ();
	}
#endif
	qglColor4f (1,1,1,1);

}


