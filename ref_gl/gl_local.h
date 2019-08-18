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

#ifdef _WIN32
#  include <windows.h>
#endif

#include <stdio.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#ifndef __linux__
#ifndef GL_COLOR_INDEX8_EXT
#define GL_COLOR_INDEX8_EXT GL_COLOR_INDEX
#endif
#endif

#include "../client/ref.h"

#include "qgl.h"

#define	REF_VERSION	"GL 0.01 ANOX"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


#ifndef __VIDDEF_T
#define __VIDDEF_T
typedef struct
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;
#endif

extern	viddef_t	vid;

/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky,
	it_clamped,
	it_cubemap_ext	// data is in order +X, -X, +Y, -Y, +Z, -Z when uploading
} imagetype_t;

typedef struct image_s
{
	char	name[MAX_QPATH];			// game path, including extension
	imagetype_t	type;
	int		width, height;				// source image
	int		upload_width, upload_height;	// after power of two and picmip
	int		registration_sequence;		// 0 = free
	struct msurface_s	*texturechain;	// for sort-by-texture world drawing
	int		texnum;						// gl texture binding
	float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
	qboolean	scrap;
	int			has_alpha;				// 1 if blend, 2 if alpha test

	qboolean paletted;
	struct atd_s *atd;					// ATD data 
} image_t;

#define		MAX_GLTEXTURES	2048

#define	TEXNUM_LIGHTMAPS	(MAX_GLTEXTURES)
#define	TEXNUM_SCRAPS		(TEXNUM_LIGHTMAPS+128)
#define	TEXNUM_IMAGES		(TEXNUM_SCRAPS+1)

/*
===============
Anachronox Texture Definition Support
===============
*/

#define ATDHEADER		(('1'<<24)+('D'<<16)+('T'<<8)+'A')

typedef enum 
{
	atd_type_whitenoise,			// Has no data
	atd_type_interform,				// cast to atd_interform_t
	atd_type_animation,				// cast to atd_animation_t

	atd_type_unknown				// Unknown type
} atd_type_t;

// Structure to hold loaded 32 bit bitmap
typedef struct atd_bitmap_s 
{
	int					width;
	int					height;

	// Image data 32 bit (8 bit for interform mother&father)
	byte				*pixels;

	// only used by animation type
	struct atd_bitmap_s	*listnext;
} atd_bitmap_t;

// interform move types
typedef enum 
{
	atd_move_none,
	atd_move_scroll,
	atd_move_wander
} atd_move_type_t;

// animation frame structure
typedef struct atd_frame_s 
{
	// index of the bitmap for the frame
	int					bitmap;	

	// index of next animation. (default = -1)
	// -1 = stop animating
	int					next;		

	// ms to wait before doing next image. (default 0)
	// -1 = do next immediately 
	//  0 = do next on next rendering pass
	float				wait;		

	// x,y pos to do an update (default 0??)
	int					x, y;

	// House keeping
	float				last_time;		// Last time updated

	struct atd_frame_s	*listnext;
} atd_frame_t;

typedef struct atd_s
{
	atd_type_t			type;			// Type of ATD 
	float				last_time;		// Last time updated
	qboolean			bilinear;		// always false for whitenoise

	// Keep the above same between various types

} atd_t;

typedef struct atd_interform_s
{
	atd_type_t			type;			// Type of ATD 
	float				last_time;		// Last time updated
	qboolean			bilinear;		// always true for interform

	// Keep the above same between various types

	// These are used to work out if an update is actually needed
	float				mother_tc[2];
	float				father_tc[2];

	atd_bitmap_t		palette;		// 32 bit image (must be 256 wide!!!)

	atd_bitmap_t		mother;			// 8 bit image
	atd_move_type_t		mother_move;	// Default atd_move_none
	float				mother_vx;		// Default 0
	float				mother_vy;		// Default 0
	float				mother_speed;	// Default 0.3
	float				mother_rate;	// Default 1
	float				mother_strength;// Detault 1

	atd_bitmap_t		father;			// 8 bit image
	atd_move_type_t		father_move;	// Default atd_move_none
	float				father_vx;		// Default 0
	float				father_vy;		// Default 0
	float				father_speed;	// Default 0.3
	float				father_rate;	// Default 1
	float				father_strength;// Detault 1

} atd_interform_t;

typedef struct atd_animation_s
{
	atd_type_t			type;			// Type of ATD 
	float				last_time;		// Last time updated
	qboolean			bilinear;		// default true for animation
	int					*pixels;		// Texture pixels

	// Keep above same between various types

	float				est_next;		// estimated time for next frame (0 if no est)
	atd_frame_t			*nextframe;		// Next frame	

	atd_bitmap_t		*bitmaps;		// Linked list
	atd_frame_t			*frames;		// Linked list

} atd_animation_t;

atd_t *ATD_Load (char *name, byte **pic, int *width, int *height, int *clamp);
extern void ATD_Free (atd_t *atd);
extern void ATD_Update (image_t *image);

//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "gl_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_SetDefaultState( void );
void GL_UpdateSwapInterval( void );
void GL_UpdateGamma(void);

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;


extern	image_t		*r_notexture;
extern	image_t		*r_particletexture;
extern	image_t		*r_newparticletexture;
extern	image_t		*r_norm_cube;	// Normalizing Cubemap... SPECIAL SUPPORT!
extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


extern	int			gl_filter_min, gl_filter_max;
extern	int			gl_cinfilter_min, gl_cinfilter_max;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern cvar_t	*gl_vertex_arrays;

extern cvar_t	*gl_ext_swapinterval;
extern cvar_t	*gl_ext_palettedtexture;
extern cvar_t	*gl_ext_multitexture;
extern cvar_t	*gl_ext_pointparameters;
extern cvar_t	*gl_ext_compiled_vertex_array;

extern cvar_t	*gl_particle_min_size;
extern cvar_t	*gl_particle_max_size;
extern cvar_t	*gl_particle_size;
extern cvar_t	*gl_particle_att_a;
extern cvar_t	*gl_particle_att_b;
extern cvar_t	*gl_particle_att_c;

extern	cvar_t	*gl_nosubimage;
extern	cvar_t	*gl_bitdepth;
extern	cvar_t	*gl_mode;
extern	cvar_t	*gl_log;
extern	cvar_t	*gl_lightmap;
extern	cvar_t	*gl_shadows;
extern	cvar_t	*gl_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_nobind;
extern	cvar_t	*gl_round_down;
extern	cvar_t	*gl_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl_ztrick;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_poly;
extern	cvar_t	*gl_texsort;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_lightmaptype;
extern	cvar_t	*gl_modulate;
extern	cvar_t	*gl_playermip;
extern	cvar_t	*gl_drawbuffer;
extern	cvar_t	*gl_3dlabs_broken;
extern  cvar_t  *gl_driver;
extern	cvar_t	*gl_swapinterval;
extern	cvar_t	*gl_texturemode;
extern	cvar_t	*gl_texturealphamode;
extern	cvar_t	*gl_texturesolidmode;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *gl_lockpvs;
extern  cvar_t	*gl_hardware_gamma;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;

extern	cvar_t		*intensity;

extern	cvar_t	*gl_large_cins;

extern	cvar_t	*gl_ext_3dfx_multisample;
extern  cvar_t	*gl_atd_subimage_update;

extern	int		gl_lightmap_format;
extern	int		gl_solid_format;
extern	int		gl_alpha_format;
extern	int		gl_tex_solid_format;
extern	int		gl_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

void R_TranslatePlayerSkin (int playernum);
void GL_Bind (int texnum);
void GL_MBind( GLenum target, int texnum );
void GL_BindImage (image_t *image );
void GL_MBindImage( GLenum target, image_t *image );
void GL_TexEnv( GLenum value );
void GL_EnableMultitexture( qboolean enable );
void GL_SelectTexture( GLenum );

void R_LightPoint (vec3_t p, vec3_t color);
void R_RealLights (float ambient[4], float rgba[4], vec3_t diffuse_strength);
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


void V_AddBlend (float r, float g, float b, float a, float *v_blend);

int 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t *e, qboolean transpass);
void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_RenderBrushPoly (msurface_t *fa);
void R_InitParticleTexture (void);
void Draw_InitLocal (void);
void GL_SubdivideSurface (msurface_t *fa);
qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
void EmitWaterPolys (msurface_t *fa);
void R_AddSkySurface (msurface_t *fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

#if 0
short LittleShort (short l);
short BigShort (short l);
int	LittleLong (int l);
float LittleFloat (float f);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer
#endif

void COM_StripExtension (char *in, char *out);

void	Draw_GetPicSize (int *w, int *h, char *name);
void	Draw_Pic (int x, int y, char *name);
void	Draw_StretchPic (int x, int y, int w, int h, char *name);
void	Draw_Char (int x, int y, int c);
void	Draw_TileClear (int x, int y, int w, int h, char *name);
void	Draw_Fill (int x, int y, int w, int h, int c);
void	Draw_FadeScreen (void);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame( float camera_separation );
void	R_SwapBuffers( int );
void	R_SetPalette ( const unsigned char *palette);

int		Draw_GetPalette (void);

void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

struct image_s *R_RegisterSkin (char *name);
struct image_s *R_RegisterClamped (char *name);

void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height);
image_t *GL_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*GL_FindImage (char *name, imagetype_t type);
void	GL_TextureMode( char *string );
void	GL_TextureCinMode( char *string );
void	GL_ImageList_f (void);

void	GL_SetTexturePalette( unsigned palette[256] );

void	GL_InitImages (void);
void	GL_ShutdownImages (void);

void	GL_FreeUnusedImages (void);

void GL_TextureAlphaMode( char *string );
void GL_TextureSolidMode( char *string );

void GL_MultisampleAABitMask(GLuint mask);
void GL_DisableAA();
void GL_EnableAA();

/*
** GL extension emulation functions
*/
void GL_DrawParticles( int n, const particle_t particles[], const unsigned colortable[768] );

/*
** GL config stuff
*/
#define GL_RENDERER_VOODOO		0x00000001
#define GL_RENDERER_VOODOO_4_5	0x00000002
#define GL_RENDERER_VOODOO_RUSH	0x00000004
#define GL_RENDERER_VOODOO_BANSHEE	0x00000008
#define		GL_RENDERER_3DFX		0x0000000F

#define GL_RENDERER_PCX1		0x00000010
#define GL_RENDERER_PCX2		0x00000020
#define GL_RENDERER_PMX			0x00000040
#define		GL_RENDERER_POWERVR		0x00000070

#define GL_RENDERER_PERMEDIA2	0x00000100
#define GL_RENDERER_GLINT_MX	0x00000200
#define GL_RENDERER_GLINT_TX	0x00000400
#define GL_RENDERER_3DLABS_MISC	0x00000800
#define		GL_RENDERER_3DLABS	0x00000F00

#define GL_RENDERER_REALIZM		0x00001000
#define GL_RENDERER_REALIZM2	0x00002000
#define		GL_RENDERER_INTERGRAPH	0x00003000

#define GL_RENDERER_3DPRO		0x00004000
#define GL_RENDERER_REAL3D		0x00008000
#define GL_RENDERER_RIVA128		0x00010000
#define GL_RENDERER_DYPIC		0x00020000

#define GL_RENDERER_V1000		0x00040000
#define GL_RENDERER_V2100		0x00080000
#define GL_RENDERER_V2200		0x00100000
#define		GL_RENDERER_RENDITION	0x001C0000

#define GL_RENDERER_O2          0x00100000
#define GL_RENDERER_IMPACT      0x00200000
#define GL_RENDERER_RE			0x00400000
#define GL_RENDERER_IR			0x00800000
#define		GL_RENDERER_SGI			0x00F00000

#define GL_RENDERER_MCD			0x01000000
#define GL_RENDERER_OTHER		0x80000000

#define MULTISAMPLE_TYPE_NONE	0
#define MULTISAMPLE_TYPE_3DFX	1
#define MULTISAMPLE_TYPE_ARB	2
#define MULTISAMPLE_TYPE_SGI	3

typedef struct
{
	int         renderer;
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	const char *extensions_string;
	const char *wglextensions_string;

	qboolean	allow_cds;

	int			max_texture_size;
	int			multisample_samples;
	int			multisample_type;		// MULTISAMPLE_TYPE_3DFX, MULTISAMPLE_TYPE_ARB or MULTISAMPLE_TYPE_SGI

	qboolean	have_generate_mipmap;
	qboolean	have_cube_map;
	qboolean	have_dot3;				// Implies have ARB_texture_env_combine 


} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int     prev_mode;

	unsigned char *d_16to8table;

	int lightmap_textures;

	int	currenttextures[3];
	int currenttmu;

	float camera_separation;
	qboolean stereo_enabled;

	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];
} glstate_t;

extern glconfig_t  gl_config;
extern glstate_t   gl_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_BeginFrame( float camera_separation );
void		GLimp_EndFrame( void );
int 		GLimp_Init( void *hinstance, void *hWnd );
void		GLimp_Shutdown( void );
int     	GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen );
void		GLimp_AppActivate( qboolean active );
void		GLimp_EnableLogging( qboolean enable );
void		GLimp_LogNewFrame( void );

