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
#ifndef __REF_H
#define __REF_H

#include "../qcommon/qcommon.h"

#define	MAX_DLIGHTS		32
#define	MAX_ENTITIES	512
#define	MAX_PARTICLES	4096
#define	MAX_LIGHTSTYLES	256

#define POWERSUIT_SCALE		4.0F

#define SHELL_RED_COLOR		0xF2
#define SHELL_GREEN_COLOR	0xD0
#define SHELL_BLUE_COLOR	0xF3

#define SHELL_RG_COLOR		0xDC
//#define SHELL_RB_COLOR		0x86
#define SHELL_RB_COLOR		0x68
#define SHELL_BG_COLOR		0x78

//ROGUE
#define SHELL_DOUBLE_COLOR	0xDF // 223
#define	SHELL_HALF_DAM_COLOR	0x90
#define SHELL_CYAN_COLOR	0x72
//ROGUE

#define SHELL_WHITE_COLOR	0xD7

typedef struct entity_s
{
	struct model_s		*model;			// opaque type outside refresh
	float				angles[3];

	/*
	** most recent data
	*/
	float				origin[3];		// also used as RF_BEAM's "from"
	int					frame;			// also used as RF_BEAM's diameter
	float				scale[3];
	float				rgb[3];

	/*
	** previous data for lerping
	*/
	float				oldorigin[3];	// also used as RF_BEAM's "to"
	int					oldframe;

	/*
	** misc
	*/
	float	backlerp;				// 0.0 = current, 1.0 = old
	int		skinnum;				// also used as RF_BEAM's palette index

	int		lightstyle;				// for flashing entities
	float	alpha;					// ignore if RF_TRANSLUCENT isn't set

	struct image_s	*skin;			// NULL for inline skin
	int		flags;

} entity_t;

#define ENTITY_FLAGS  68

typedef struct
{
	vec3_t	origin;
	vec3_t	color;
	float	intensity;
} dlight_t;

typedef struct
{
	vec3_t	origin;
	int		color;
	float	alpha;
} particle_t;

typedef struct
{
	float		rgb[3];			// 0.0 - 2.0
	float		white;			// highest of rgb
} lightstyle_t;

typedef enum np_gennorm_s {
	GENNORM_SCREEN		= 0,	// Face screen always
	GENNORM_UP			= 1,	// Face up in world
	GENNORM_DIR_GEN		= 2,	// Particle normal is direction of generator
	GENNORM_SPRITE		= 3,	// Face screen in yaw only. Pitch is always 0
	GENNORM_UP_GEN		= 4,	// Particles top is always in gen dir, attempt to rotate around axis and face screen
	GENNORM_UP_GEN_FLAT	= 5		// Particles top is always in gen dir, is vertically flat
} np_gennorm_t;

typedef struct
{
	int						src_blend;		// GL_ONE
	int						dest_blend;		// GL_ONE
	int						depth_write;	// 0
	int						depth_func;		// GL_LESS
	int						gennorm;		// GENNORM_SCREEN
	struct image_s			*image;			// Image to use
} np_generator_t;

typedef struct
{
	np_generator_t			*gen;			// Generator properties
	vec3_t					gen_forward;	// Normal of the generator
	vec3_t					origin;	
	float					radius;
	float					rgba[4];		
	float					texcoord[4];	//  (l=0,t=0,r=1,b=1)
	float					roll;
} newparticle_t;

typedef struct
{
	int			x, y, width, height;// in virtual screen coordinates
	float		fov_x, fov_y;
	float		vieworg[3];
	float		viewangles[3];
	float		blend[4];			// rgba 0-1 full screen blend
	float		time;				// time is uesed to auto animate
	int			rdflags;			// RDF_UNDERWATER, etc

	byte		*areabits;			// if not NULL, only areas with set bits will be drawn

	lightstyle_t	*lightstyles;	// [MAX_LIGHTSTYLES]

	int			num_entities;
	entity_t	*entities;

	int			num_dlights;
	dlight_t	*dlights;

	int			num_particles;
	particle_t	*particles;

	int			num_newparticles;
	newparticle_t	*newparticles;
} refdef_t;



#define	API_VERSION		3
typedef void(* function_t)();
//
// these are the functions exported by the refresh module
//
typedef struct
{
	union {
		function_t funcs[0x42];
		struct {
			// if api_version is different, the dll cannot be used
			int		api_version;
			// called when the library is loaded
			qboolean(*Init) (void* hinstance, void* wndproc);

			// called before the library is unloaded
			void	(*Shutdown) (void);

			// All data that will be used in a level should be
			// registered before rendering any frames to prevent disk hits,
			// but they can still be registered at a later time
			// if necessary.
			//
			// EndRegistration will free any remaining data that wasn't registered.
			// Any model_s or skin_s pointers from before the BeginRegistration
			// are no longer valid after EndRegistration.
			//
			// Skins and images need to be differentiated, because skins
			// are flood filled to eliminate mip map edge errors, and pics have
			// an implicit "pics/" prepended to the name. (a pic name that starts with a
			// slash will not use the "pics/" prefix or the ".pcx" postfix)
			void	(*BeginRegistration) (char* map);
			struct model_s* (*RegisterModel) (char* name);
			struct image_s* (*RegisterSkin) (char* name);
			struct image_s* (*RegisterPic) (char* name);
			struct image_s* (*RegisterClamped) (char* name);
			void	(*SetSky) (char* name, float rotate, vec3_t axis);
			void	(*EndRegistration) (void);
			function_t unk[27];
			void	(*BeginFrame)(float camera_separation);
			void	(*EndFrame) (void);

			/*
					mov	dword ptr [ebp-00000104h],R_Init
					mov	dword ptr [ebp-00000100h],R_Shutdown
					mov	dword ptr [ebp-000000FCh],R_BeginRegistration
					mov	dword ptr [ebp-000000F8h],SUB_L1001119B
					mov	dword ptr [ebp-000000F4h],L100112DC
					mov	dword ptr [ebp-000000F0h],L10007277
					mov	dword ptr [ebp-000000ECh],L100072AE
					mov	dword ptr [ebp-000000E8h],SUB_L1002EC82
					mov	dword ptr [ebp-000000E4h],SUB_L1002ED4F
					mov	dword ptr [ebp-000000E0h],L1002ED62
					mov	dword ptr [ebp-000000D8h],R_SetSky
					mov	dword ptr [ebp-000000D4h],L1001147F
					mov	dword ptr [ebp-000000DCh],L1002F0BC
					mov	dword ptr [ebp-000000D0h],L1001B24F
					mov	dword ptr [ebp-000000CCh],L10013C0F
					mov	dword ptr [ebp-000000C8h],SUB_L1001B4E3
					mov	dword ptr [ebp-000000C4h],L1001B512
					mov	dword ptr [ebp-000000C0h],L10007C7E
					mov	dword ptr [ebp-000000BCh],SUB_L100071EC
					mov	dword ptr [ebp-000000B8h],L1001814E
					mov	dword ptr [ebp-000000B4h],L10003AD4
					mov	dword ptr [ebp-000000B0h],L100038DE
					mov	dword ptr [ebp-000000ACh],L100039DB
					mov	dword ptr [ebp-000000A8h],L10003784
					mov	dword ptr [ebp-000000A4h],L100042A6
					mov	dword ptr [ebp-000000A0h],SUB_L10003498
					mov	dword ptr [ebp-0000009Ch],L1000360C
					mov	dword ptr [ebp-00000098h],L10003E10
					mov	dword ptr [ebp-00000094h],L10003EA8
					mov	dword ptr [ebp-00000090h],L10003F74
					mov	dword ptr [ebp-0000008Ch],L10004038
					mov	dword ptr [ebp-00000088h],L10004110
					mov	dword ptr [ebp-00000084h],L1000429C
					mov	dword ptr [ebp-80h],L100042A1
					mov	dword ptr [ebp-7Ch],L1001C12E
					mov	dword ptr [ebp-78h],L1001C31A
					mov	dword ptr [ebp-74h],L10012FEC
					mov	dword ptr [ebp-70h],L100132E0
					mov	dword ptr [ebp-6Ch],R_BeginFrame
					mov	dword ptr [ebp-68h],GLimp_EndFrame
					mov	dword ptr [ebp-64h],L1002E78E
					mov	dword ptr [ebp-5Ch],L1000C19D
					mov	dword ptr [ebp-60h],L1000C065
					mov	dword ptr [ebp-58h],SUB_L1000C742
					mov	dword ptr [ebp-54h],L1000CCA9
					mov	dword ptr [ebp-50h],L1000C4F8
					mov	dword ptr [ebp-4Ch],L1000C611
					mov	dword ptr [ebp-48h],L1000C2D4
					mov	dword ptr [ebp-44h],L1000C41D
					mov	dword ptr [ebp-3Ch],L1001AEFD
					mov	dword ptr [ebp-38h],L1001B15E
					mov	dword ptr [ebp-34h],L1001B163
					mov	dword ptr [ebp-30h],SUB_L1003EB10
					mov	dword ptr [ebp-2Ch],L1003EC77
					mov	dword ptr [ebp-28h],SUB_L1003ECA5
					mov	dword ptr [ebp-24h],L1003EC7C
					mov	dword ptr [ebp-20h],L1003ED55
					mov	dword ptr [ebp-1Ch],L1001B168
					mov	dword ptr [ebp-18h],L1001B177
					mov	dword ptr [ebp-14h],L1003F79A
					mov	dword ptr [ebp-10h],L1003F837
					mov	dword ptr [ebp-0Ch],L1001B186
					mov	dword ptr [ebp-08h],L1000D9AA
					mov	dword ptr [ebp-04h],SUB_L1000D83F

					call	SUB_L10032E11

			*/
			


			

			void	(*RenderFrame) (refdef_t* fd);

			void	(*DrawGetPicSize) (int* w, int* h, char* name);	// will return 0 0 if not found
			void	(*DrawPic) (int x, int y, char* name);
			void	(*DrawStretchPic) (int x, int y, int w, int h, char* name, char* alt);
			void	(*DrawChar) (int x, int y, int c);
			void	(*DrawTileClear) (int x, int y, int w, int h, char* name);
			void	(*DrawFill) (int x, int y, int w, int h, int c);
			void	(*DrawFadeScreen) (void);

			// Draw images for cinematic rendering (which can have a different palette). Note that calls
			void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, byte* data);

			
			/** video mode and refresh state management entry points
			*/
			void	(*CinematicSetPalette)(const unsigned char* palette);	// NULL = game palette

			void	(*AppActivate)(qboolean activate); 
		};
	};
	
} refexport_t;

//
// these are the functions imported by the refresh module
//
typedef struct
{
	union {
		int dwords[0x17];
		struct {
			void	(*Sys_Error) (int err_level, char* str, ...);

			void	(*Cmd_AddCommand) (char* name, void(*cmd)(void));
			void	(*Cmd_RemoveCommand) (char* name);
			int		(*Cmd_Argc) (void);
			char* (*Cmd_Argv) (int i);
			void	(*Cmd_ExecuteText) (int exec_when, char* text);

			void	(*Con_Printf) (int print_level, char* str, ...);

			// files will be memory mapped read only
			// the returned buffer may be part of a larger pak file,
			// or a discrete file from anywhere in the quake search path
			// a -1 return means the file does not exist
			// NULL can be passed for buf to just determine existance
			int		(*FS_LoadFile) (char* name, void** buf);
			void	(*FS_FreeFile) (void* buf);

			// gamedir will be the current directory that generated
			// files should be stored to, ie: "f:\quake\id1"
			char* (*FS_Gamedir) (void);

			cvar_t* (*Cvar_Get) (char* name, char* value, int flags);
			cvar_t* (*Cvar_Set)(char* name, char* value);
			void	 (*Cvar_SetValue)(char* name, float value);
			cvar_t* (*Cvar_ForceSet)(char* name, char* value);

			qboolean(*Vid_GetModeInfo)(int* width, int* height, int mode);
			void		(*Vid_MenuInit)(void);
			void		(*Vid_NewWindow)(int width, int height);
		};
	};
} refimport_t;


// this is the only function actually exported at the linker level
typedef	refexport_t	(*GetRefAPI_t) (refimport_t);

#endif // __REF_H
