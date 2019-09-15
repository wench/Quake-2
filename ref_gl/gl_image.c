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

#include "gl_local.h"

image_t		gltextures[MAX_GLTEXTURES];
int			numgltextures;
int			base_textureid;		// gltextures[i] = base_textureid+i

static byte			 intensitytable[256];
static unsigned char gammatable[256];

cvar_t		*intensity;

unsigned	d_8to24table[256];

int GL_Upload8 (byte *data, byte* palette, int width, int height,  qboolean mipmap, qboolean is_sky );
int GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap);


void	*Hunk_Begin2 (int maxsize);
void	*Hunk_Alloc2 (int size);
int		Hunk_End2 (void);
void	Hunk_Free2 (void *base);



int		gl_solid_format = 3;
int		gl_alpha_format = 4;

int		gl_tex_solid_format = GL_RGB8;
int		gl_tex_alpha_format = GL_RGBA8;

int		gl_filter_min = GL_LINEAR_MIPMAP_NEAREST;
int		gl_filter_max = GL_LINEAR;
int		gl_cinfilter_min = GL_LINEAR;
int		gl_cinfilter_max = GL_LINEAR;

void GL_SetTexturePalette( unsigned palette[256] )
{
	int i;
	unsigned char temptable[768];

	if ( qglColorTableEXT && gl_ext_palettedtexture->value )
	{
		for ( i = 0; i < 256; i++ )
		{
			temptable[i*3+0] = ( palette[i] >> 0 ) & 0xff;
			temptable[i*3+1] = ( palette[i] >> 8 ) & 0xff;
			temptable[i*3+2] = ( palette[i] >> 16 ) & 0xff;
		}

		qglColorTableEXT( GL_SHARED_TEXTURE_PALETTE_EXT,
						   GL_RGB,
						   256,
						   GL_RGB,
						   GL_UNSIGNED_BYTE,
						   temptable );
	}
}

void GL_EnableMultitexture( qboolean enable )
{
	if ( !qglSelectTextureSGIS && !qglActiveTextureARB )
		return;

	if ( enable )
	{
		GL_SelectTexture( GL_TEXTURE1 );
		qglEnable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	}
	else
	{
		GL_SelectTexture( GL_TEXTURE1 );
		qglDisable( GL_TEXTURE_2D );
		GL_TexEnv( GL_REPLACE );
	}
	GL_SelectTexture( GL_TEXTURE0 );
	GL_TexEnv( GL_REPLACE );
}

void GL_SelectTexture( GLenum texture )
{
	int tmu;

	if ( !qglSelectTextureSGIS && !qglActiveTextureARB )
		return;

	if ( texture == GL_TEXTURE0 )
	{
		tmu = 0;
	}
	else
	{
		tmu = 1;
	}

	if ( tmu == gl_state.currenttmu )
	{
		return;
	}

	gl_state.currenttmu = tmu;

	if ( qglSelectTextureSGIS )
	{
		qglSelectTextureSGIS( texture );
	}
	else if ( qglActiveTextureARB )
	{
		qglActiveTextureARB( texture );
		qglClientActiveTextureARB( texture );
	}
}

void GL_TexEnv( GLenum mode )
{
	static int lastmodes[2] = { -1, -1 };

	if ( mode != lastmodes[gl_state.currenttmu] )
	{
		qglTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode );
		lastmodes[gl_state.currenttmu] = mode;
	}
}

void GL_Bind (int texnum)
{
	extern	image_t	*draw_chars;

	if (gl_nobind->value && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
	if ( gl_state.currenttextures[gl_state.currenttmu] != texnum) 
	{
		gl_state.currenttextures[gl_state.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_2D, texnum);
	}
}

void GL_BindCube (int texnum)
{
	extern	image_t	*draw_chars;

	if (gl_nobind->value && draw_chars)		// performance evaluation option
		texnum = draw_chars->texnum;
	if ( gl_state.currenttextures[gl_state.currenttmu] != texnum) 
	{
		gl_state.currenttextures[gl_state.currenttmu] = texnum;
		qglBindTexture (GL_TEXTURE_CUBE_MAP_ARB, texnum);
	}
}

void GL_BindImage (image_t *image)
{
	if (image->type != it_cubemap_ext)
		GL_Bind(image->texnum);
	else
		GL_BindCube(image->texnum);

	if (image->atd && !gl_nobind->value ) 
		ATD_Update(image);
}

void GL_MBind( GLenum target, int texnum )
{
	GL_SelectTexture( target );
	if ( target == GL_TEXTURE0 )
	{
		if ( gl_state.currenttextures[0] == texnum )
			return;
	}
	else
	{
		if ( gl_state.currenttextures[1] == texnum )
			return;
	}
	GL_Bind( texnum );
}

void GL_MBindCube( GLenum target, int texnum )
{
	GL_SelectTexture( target );
	if ( target == GL_TEXTURE0 )
	{
		if ( gl_state.currenttextures[0] == texnum )
			return;
	}
	else
	{
		if ( gl_state.currenttextures[1] == texnum )
			return;
	}
	GL_BindCube( texnum );
}

void GL_MBindImage( GLenum target, image_t *image )
{
	if (image->type != it_cubemap_ext)
		GL_MBind( target, image->texnum );
	else
		GL_MBindCube( target, image->texnum );

	if (image->atd && !gl_nobind->value ) 
		ATD_Update(image);
}

typedef struct
{
	char *name;
	int	minimize, maximize;
} glmode_t;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

#define NUM_GL_MODES (sizeof(modes) / sizeof (glmode_t))

glmode_t cinmodes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
};

#define NUM_GL_CINMODES (sizeof(cinmodes) / sizeof (glmode_t))

typedef struct
{
	char *name;
	int mode;
} gltmode_t;

gltmode_t gl_alpha_modes[] = {
	{"default", 4},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8", GL_RGBA8},
	{"GL_RGB5_A1", GL_RGB5_A1},
	{"GL_RGBA4", GL_RGBA4},
	{"GL_RGBA2", GL_RGBA2},
};

#define NUM_GL_ALPHA_MODES (sizeof(gl_alpha_modes) / sizeof (gltmode_t))

gltmode_t gl_solid_modes[] = {
	{"default", 3},
	{"GL_RGB", GL_RGB},
	{"GL_RGB8", GL_RGB8},
	{"GL_RGB5", GL_RGB5},
	{"GL_RGB4", GL_RGB4},
	{"GL_R3_G3_B2", GL_R3_G3_B2},
#ifdef GL_RGB2_EXT
	{"GL_RGB2", GL_RGB2_EXT},
#endif
};

#define NUM_GL_SOLID_MODES (sizeof(gl_solid_modes) / sizeof (gltmode_t))
byte gPalette[768];
/*
===============
GL_TextureMode
===============
*/
void GL_TextureMode( char *string )
{
	int		i;
	image_t	*glt;

	for (i=0 ; i< NUM_GL_MODES ; i++)
	{
		if ( !Q_stricmp( modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->type != it_pic && glt->type != it_sky && glt->type != it_cubemap_ext && glt->type != it_clamped )
		{
			GL_Bind (glt->texnum);

			// Special ATD handling 
			if (glt->atd) 
			{
				if (glt->atd->bilinear) 
				{
					// If we can generate mipmap, then use the user specified mipmap min filter
					// Otherwise use the 'max' filter for both up and down
					if (gl_config.have_generate_mipmap)
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
					else
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
					qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
				}
				else 
				{
					// Nearest, but we will allow mipmapping
					if (gl_config.have_generate_mipmap)
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
					else
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
			}
			else 
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
			}
		}
	}
}

/*
===============
GL_TextureCinMode
===============
*/
void GL_TextureCinMode( char *string )
{
	int		i;

	for (i=0 ; i< NUM_GL_CINMODES ; i++)
	{
		if ( !Q_stricmp( cinmodes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_CINMODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad filter name\n");
		return;
	}

	gl_cinfilter_min = cinmodes[i].minimize;
	gl_cinfilter_max = cinmodes[i].maximize;
}

/*
===============
GL_TextureAlphaMode
===============
*/
void GL_TextureAlphaMode( char *string )
{
	/*
	int		i;

	for (i=0 ; i< NUM_GL_ALPHA_MODES ; i++)
	{
		if ( !Q_stricmp( gl_alpha_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_ALPHA_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad alpha texture mode name\n");
		return;
	}

	gl_tex_alpha_format = gl_alpha_modes[i].mode;
	*/
}

/*
===============
GL_TextureSolidMode
===============
*/
void GL_TextureSolidMode( char *string )
{
	/*
	int		i;

	for (i=0 ; i< NUM_GL_SOLID_MODES ; i++)
	{
		if ( !Q_stricmp( gl_solid_modes[i].name, string ) )
			break;
	}

	if (i == NUM_GL_SOLID_MODES)
	{
		ri.Con_Printf (PRINT_ALL, "bad solid texture mode name\n");
		return;
	}

	gl_tex_solid_format = gl_solid_modes[i].mode;
	*/
}

/*
===============
GL_ImageList_f
===============
*/
void	GL_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;
	const char *palstrings[2] =
	{
		"RGB",
		"PAL"
	};

	ri.Con_Printf (PRINT_ALL, "------------------\n");
	texels = 0;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (image->texnum <= 0)
			continue;
		texels += image->upload_width*image->upload_height;
		switch (image->type)
		{
		case it_skin:
			ri.Con_Printf (PRINT_ALL, "M");
			break;
		case it_sprite:
			ri.Con_Printf (PRINT_ALL, "S");
			break;
		case it_wall:
			ri.Con_Printf (PRINT_ALL, "W");
			break;
		case it_pic:
			ri.Con_Printf (PRINT_ALL, "P");
			break;
		default:
			ri.Con_Printf (PRINT_ALL, " ");
			break;
		}

		ri.Con_Printf (PRINT_ALL,  " %3i %3i %s: %s\n",
			image->upload_width, image->upload_height, palstrings[image->paletted], image->name);
	}
	ri.Con_Printf (PRINT_ALL, "Total texel count (not counting mipmaps): %i\n", texels);
}


/*
=============================================================================

  scrap allocation

  Allocate all the little status bar obejcts into a single texture
  to crutch up inefficient hardware / drivers

=============================================================================
*/

#define	MAX_SCRAPS		1
#define	BLOCK_WIDTH		256
#define	BLOCK_HEIGHT	256

int			scrap_allocated[MAX_SCRAPS][BLOCK_WIDTH];
byte		scrap_texels[MAX_SCRAPS][BLOCK_WIDTH*BLOCK_HEIGHT];
qboolean	scrap_dirty;

// returns a texture number and the position inside it
int Scrap_AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_SCRAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (scrap_allocated[texnum][i+j] >= best)
					break;
				if (scrap_allocated[texnum][i+j] > best2)
					best2 = scrap_allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			scrap_allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	return -1;
//	Sys_Error ("Scrap_AllocBlock: full");
}

int	scrap_uploads;

void Scrap_Upload (void)
{
	scrap_uploads++;
	GL_Bind(TEXNUM_SCRAPS);
	GL_Upload8 (scrap_texels[0], gPalette, BLOCK_WIDTH, BLOCK_HEIGHT, false, false );
	scrap_dirty = false;
}

/*
=================================================================

PCX LOADING

=================================================================
*/


/*
==============
LoadPCX
==============
*/
void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;
	*palette = NULL;

	//
	// load the file
	//
	len = ri.FS_LoadFile (filename, (void **)&raw);
	if (!raw)
	{
		//ri.Con_Printf (PRINT_DEVELOPER, "Bad pcx file %s\n", filename);
		return;
	}

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		ri.Con_Printf (PRINT_ALL, "Bad pcx file %s\n", filename);
		return;
	}

	out = malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = *raw++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len)
	{
		ri.Con_Printf (PRINT_DEVELOPER, "PCX file %s was malformed", filename);
		free (*pic);
		*pic = NULL;
	}

	ri.FS_FreeFile (pcx);
}

/*
=========================================================

TARGA LOADING

=========================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGA
=============
*/
void LoadTGA (char *name, byte **pic, int *width, int *height, qboolean hunk)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;
	byte tmp[2];

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Con_Printf (PRINT_DEVELOPER, "Bad tga file %s\n", name);
		return;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_index = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	tmp[0] = buf_p[0];
	tmp[1] = buf_p[1];
	targa_header.colormap_length = LittleShort ( *((short *)tmp) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) 
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only type 2 and 10 targa RGB images supported\n");

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		ri.Sys_Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	if (hunk)
		targa_rgba = Hunk_Alloc(numPixels*4);
	else
		targa_rgba = malloc (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	ri.FS_FreeFile (buffer);
}


/*
=============
LoadPNG
=============
*/
#include <png.h>

typedef struct LoadPNGReadStruct_s {
	byte	*start;
	byte	*cur;
	int		len;
} LoadPNGReadStruct_t;

void PNGAPI LoadPNGRead (png_structp png_ptr, png_bytep buf, png_size_t num)
{
	LoadPNGReadStruct_t *str;

    str = (LoadPNGReadStruct_t*) png_get_io_ptr(png_ptr);

	if (((str->cur-str->start)+num) > str->len) {
		png_error(png_ptr, "Trying to read past end of file");
	}

	memcpy (buf, str->cur, num);
	str->cur += num;
}

void LoadPNG (char *name, byte **pic, int *width, int *height, qboolean hunk)
{
	byte	*buffer;
	int		length;
	LoadPNGReadStruct_t readstruct;

	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info;
	png_bytep *row_pointers;

	png_uint_32 pwidth;
	png_uint_32 pheight;
    int pbit_depth;
	int pcolor_type;
	byte *picture;
	size_t row, rowbytes;

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
		return;
	}

    if (png_sig_cmp(buffer, 0, 8))
	{
		ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
		ri.FS_FreeFile (buffer);
		return;
	}

    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		ri.Con_Printf (PRINT_ALL, "Error creating PNG read struct\n");
		ri.FS_FreeFile (buffer);
		return;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
		ri.Con_Printf (PRINT_ALL, "Error creating PNG info struct\n");
		ri.FS_FreeFile (buffer);
		return;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
		ri.Con_Printf (PRINT_ALL, "Error creating PNG info struct\n");
		ri.FS_FreeFile (buffer);
		return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
		ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		ri.FS_FreeFile (buffer);
		return;
	}

	readstruct.cur = buffer;
	readstruct.start = buffer;
	readstruct.len = length;

	// Set read function
	png_set_read_fn(png_ptr, (void *)&readstruct, LoadPNGRead);

	/* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &pwidth, &pheight, &pbit_depth, &pcolor_type,
		NULL, NULL, NULL);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr);

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	* byte into separate bytes (useful for paletted and grayscale images).
	*/
	png_set_packing(png_ptr);

	/* Expand paletted colors into true RGB triplets */
	if (pcolor_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
	if (pcolor_type == PNG_COLOR_TYPE_GRAY && pbit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if (pcolor_type == PNG_COLOR_TYPE_GRAY)
		png_set_gray_to_rgb(png_ptr);

	/* Expand paletted or RGB images with transparency to full alpha channels
	* so the data will be available as RGBA quartets.
	*/
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	else 
		/* Add filler (or alpha) byte (before/after each RGB triplet) */
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	/* Optional call to gamma correct and add the background to the palette
	* and update info structure.  REQUIRED if you are expecting libpng to
	* update the palette for you (ie you selected such a transform above).
	*/

	png_read_update_info(png_ptr, info_ptr);

	/* Allocate the memory to hold the image using the fields of info_ptr. */
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	row_pointers = png_malloc(png_ptr, pheight*sizeof(png_bytep));

	*width = pwidth;
	*height = pheight;

	if (hunk) *pic = picture = Hunk_Alloc2(pheight * pwidth*4);
	else *pic = picture = malloc (pheight * pwidth*4);

	// Yes we don't have to bother with allocating memory for the rows
	if (rowbytes == pwidth*4)
	{
		for (row = 0; row < pheight; row++)
		{
			row_pointers[row] = picture;
			picture += rowbytes;
		}
	}
	else
	{
		for (row = 0; row < pheight; row++)
			row_pointers[row] = png_malloc(png_ptr, rowbytes);
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);

	// Hmph! We need to copy the rows
	if (rowbytes != pwidth*4)
	{
		// Now copy lines go over the file
		for (row = 0; row < pheight; row++) 
		{
			memcpy (picture, row_pointers[row], pwidth*4);
			picture += pwidth*4;
			
		}
	}

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	ri.FS_FreeFile (buffer);

	return;

}

// Load a 8-bit PNG (Greyscale or Paletted)
void LoadPNG_8Bit (char *name, byte **pic, int *width, int *height, qboolean hunk)
{
	byte	*buffer;
	int		length;
	LoadPNGReadStruct_t readstruct;

	png_structp png_ptr;
	png_infop info_ptr;
	png_infop end_info;
	png_bytep *row_pointers;

	png_uint_32 pwidth;
	png_uint_32 pheight;
    int pbit_depth;
	int pcolor_type;
	byte *picture;
	size_t row, rowbytes;

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
		return;
	}

    if (png_sig_cmp(buffer, 0, 8))
	{
		ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
		ri.FS_FreeFile (buffer);
		return;
	}

    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		ri.Con_Printf (PRINT_ALL, "Error creating PNG read struct\n");
		ri.FS_FreeFile (buffer);
		return;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr,
           (png_infopp)NULL, (png_infopp)NULL);
		ri.Con_Printf (PRINT_ALL, "Error creating PNG info struct\n");
		ri.FS_FreeFile (buffer);
		return;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr,
          (png_infopp)NULL);
		ri.Con_Printf (PRINT_ALL, "Error creating PNG info struct\n");
		ri.FS_FreeFile (buffer);
		return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
		ri.Con_Printf (PRINT_ALL, "Bad png file %s\n", name);
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		ri.FS_FreeFile (buffer);
		return;
	}

	readstruct.cur = buffer;
	readstruct.start = buffer;
	readstruct.len = length;

	// Set read function
	png_set_read_fn(png_ptr, (void *)&readstruct, LoadPNGRead);

	/* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &pwidth, &pheight, &pbit_depth, &pcolor_type,
		NULL, NULL, NULL);

	/* Only 8 bit grey or paletted supported */
	if (pbit_depth != 8 || (pcolor_type != PNG_COLOR_TYPE_PALETTE && pcolor_type != PNG_COLOR_TYPE_GRAY)) 
	{
		ri.Con_Printf (PRINT_ALL, "LoadPNG_8Bit: PNG (%s) wasn't 8-bit grey or paletted\n", name);

		/* clean up after the read, and free any memory allocated - REQUIRED */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		ri.FS_FreeFile (buffer);
		return;
	}

	/* Optional call to gamma correct and add the background to the palette
	* and update info structure.  REQUIRED if you are expecting libpng to
	* update the palette for you (ie you selected such a transform above).
	*/

	png_read_update_info(png_ptr, info_ptr);

	/* Allocate the memory to hold the image using the fields of info_ptr. */
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	row_pointers = png_malloc(png_ptr, pheight*sizeof(png_bytep));

	*width = pwidth;
	*height = pheight;

	if (hunk) *pic = picture = Hunk_Alloc2(pheight * pwidth);
	else *pic = picture = malloc (pheight * pwidth);

	// Yes we don't have to bother with allocating memory for the rows
	if (rowbytes == pwidth)
	{
		for (row = 0; row < pheight; row++)
		{
			row_pointers[row] = picture;
			picture += rowbytes;
		}
	}
	else
	{
		for (row = 0; row < pheight; row++)
			row_pointers[row] = png_malloc(png_ptr, rowbytes);
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);

	// Hmph! We need to copy the rows
	if (rowbytes != pwidth)
	{
		// Now copy lines go over the file
		for (row = 0; row < pheight; row++) 
		{
			memcpy (picture, row_pointers[row], pwidth*4);
			picture += pwidth;
			
		}
	}

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	ri.FS_FreeFile (buffer);

	return;

}

/*
=============
LoadBMP
=============
*/

void LoadBMP (char *name, byte **pic, int *width, int *height)
{
	byte	*buffer;
	int		length;

	*pic = NULL;

	//
	// load the file
	//
	length = ri.FS_LoadFile (name, (void **)&buffer);
	if (!buffer)
	{
		//ri.Con_Printf (PRINT_ALL, "Bad bmp file %s\n", name);
		return;
	}

	ri.FS_FreeFile (buffer);

	return;
}


/*
====================================================================

IMAGE FLOOD FILLING

====================================================================
*/


/*
=================
Mod_FloodFillSkin

Fill background pixels so mipmapping doesn't have haloes
=================
*/

typedef struct
{
	short		x, y;
} floodfill_t;

// must be a power of 2
#define FLOODFILL_FIFO_SIZE 0x8000
#define FLOODFILL_FIFO_MASK (FLOODFILL_FIFO_SIZE - 1)

#define FLOODFILL_STEP( off, dx, dy ) \
{ \
	if (pos[off] == fillcolor) \
	{ \
		pos[off] = 255; \
		fifo[inpt].x = x + (dx), fifo[inpt].y = y + (dy); \
		inpt = (inpt + 1) & FLOODFILL_FIFO_MASK; \
	} \
	else if (pos[off] != 255) fdc = pos[off]; \
}

void R_FloodFillSkin( byte *skin, int skinwidth, int skinheight )
{
	byte				fillcolor = *skin; // assume this is the pixel to fill
	floodfill_t			fifo[FLOODFILL_FIFO_SIZE];
	int					inpt = 0, outpt = 0;
	int					filledcolor = -1;
	int					i;

	if (filledcolor == -1)
	{
		filledcolor = 0;
		// attempt to find opaque black
		for (i = 0; i < 256; ++i)
			if (d_8to24table[i] == (255 << 0)) // alpha 1.0
			{
				filledcolor = i;
				break;
			}
	}

	// can't fill to filled color or to transparent color (used as visited marker)
	if ((fillcolor == filledcolor) || (fillcolor == 255))
	{
		//printf( "not filling skin from %d to %d\n", fillcolor, filledcolor );
		return;
	}

	fifo[inpt].x = 0, fifo[inpt].y = 0;
	inpt = (inpt + 1) & FLOODFILL_FIFO_MASK;

	while (outpt != inpt)
	{
		int			x = fifo[outpt].x, y = fifo[outpt].y;
		int			fdc = filledcolor;
		byte		*pos = &skin[x + skinwidth * y];

		outpt = (outpt + 1) & FLOODFILL_FIFO_MASK;

		if (x > 0)				FLOODFILL_STEP( -1, -1, 0 );
		if (x < skinwidth - 1)	FLOODFILL_STEP( 1, 1, 0 );
		if (y > 0)				FLOODFILL_STEP( -skinwidth, 0, -1 );
		if (y < skinheight - 1)	FLOODFILL_STEP( skinwidth, 0, 1 );
		skin[x + skinwidth * y] = fdc;
	}
}

//=======================================================


/*
================
GL_ResampleTexture
================
*/
void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight)
{
	int		i, j;
	unsigned	*inrow, *inrow2;
	unsigned	frac, fracstep;
	unsigned	p1[8192], p2[8192];
	byte		*pix1, *pix2, *pix3, *pix4;

	fracstep = inwidth*0x10000/outwidth;

	frac = fracstep>>2;
	for (i=0 ; i<outwidth ; i++)
	{
		p1[i] = 4*(frac>>16);
		frac += fracstep;
	}
	frac = 3*(fracstep>>2);
	for (i=0 ; i<outwidth ; i++)
	{
		p2[i] = 4*(frac>>16);
		frac += fracstep;
	}

	for (i=0 ; i<outheight ; i++, out += outwidth)
	{
		inrow = in + inwidth*(int)((i+0.25)*inheight/outheight);
		inrow2 = in + inwidth*(int)((i+0.75)*inheight/outheight);
		frac = fracstep >> 1;
		for (j=0 ; j<outwidth ; j++)
		{
			pix1 = (byte *)inrow + p1[j];
			pix2 = (byte *)inrow + p2[j];
			pix3 = (byte *)inrow2 + p1[j];
			pix4 = (byte *)inrow2 + p2[j];
			((byte *)(out+j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			((byte *)(out+j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			((byte *)(out+j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;
			((byte *)(out+j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3])>>2;
		}
	}
}

/*
================
GL_LightScaleTexture

Scale up the pixel values in a texture to increase the
lighting range
================
*/
void GL_LightScaleTexture (unsigned *in, int inwidth, int inheight, qboolean only_gamma )
{
	if ( only_gamma )
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[p[0]];
			p[1] = gammatable[p[1]];
			p[2] = gammatable[p[2]];
		}
	}
	else
	{
		int		i, c;
		byte	*p;

		p = (byte *)in;

		c = inwidth*inheight;
		for (i=0 ; i<c ; i++, p+=4)
		{
			p[0] = gammatable[intensitytable[p[0]]];
			p[1] = gammatable[intensitytable[p[1]]];
			p[2] = gammatable[intensitytable[p[2]]];
		}
	}
}

/*
================
GL_MipMap

Operates in place, quartering the size of the texture
================
*/
void GL_MipMap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

/*
===============
GL_Upload32

Returns has_alpha
===============
*/
void GL_BuildPalettedTexture( unsigned char *paletted_texture, unsigned char *scaled, int scaled_width, int scaled_height )
{
	int i;

	for ( i = 0; i < scaled_width * scaled_height; i++ )
	{
		unsigned int r, g, b, c;

		r = ( scaled[0] >> 3 ) & 31;
		g = ( scaled[1] >> 2 ) & 63;
		b = ( scaled[2] >> 3 ) & 31;

		c = r | ( g << 5 ) | ( b << 11 );

		paletted_texture[i] = gl_state.d_16to8table[c];

		scaled += 4;
	}
}

int		upload_width, upload_height;
qboolean uploaded_paletted;
static qboolean no_pic_mip = false;

int GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap)
{
	int			samples;
	static unsigned	scaled[2048*2048];
	static unsigned char paletted_texture[2048*2048];
	int			scaled_width, scaled_height;
	int			i, c;
	byte		*scan;
	int comp;
	qboolean	want_blend = false;

	uploaded_paletted = false;

	for (scaled_width = 1 ; scaled_width < width ; scaled_width<<=1)
		;
	if (gl_round_down->value && scaled_width > width && mipmap)
		scaled_width >>= 1;
	for (scaled_height = 1 ; scaled_height < height ; scaled_height<<=1)
		;
	if (gl_round_down->value && scaled_height > height && mipmap)
		scaled_height >>= 1;

	// let people sample down the world textures for speed
	if (mipmap && !no_pic_mip)
	{
		scaled_width >>= (int)gl_picmip->value;
		scaled_height >>= (int)gl_picmip->value;
	}

	// don't ever bother with >256 textures
	if (scaled_width > gl_config.max_texture_size)
		scaled_width = gl_config.max_texture_size;
	if (scaled_height > gl_config.max_texture_size)
		scaled_height = gl_config.max_texture_size;

	if (scaled_width < 1)
		scaled_width = 1;
	if (scaled_height < 1)
		scaled_height = 1;

	upload_width = scaled_width;
	upload_height = scaled_height;

	if (scaled_width * scaled_height > sizeof(scaled)/4)
		ri.Sys_Error (ERR_DROP, "GL_Upload32: too big");

	// scan the texture for any non-255 alpha
	c = width*height;
	scan = ((byte *)data) + 3;
	samples = gl_solid_format;
	if (data) {
		for (i = 0; i < c; i++, scan += 4)
	{
		if (*scan != 255)
		{
			samples = gl_alpha_format;
			if (*scan != 0) {
				want_blend = true;
				break;
			}
		}
	}}

	if (samples == gl_solid_format)
	    comp = gl_tex_solid_format;
	else if (samples == gl_alpha_format)
	    comp = gl_tex_alpha_format;
	else {
	    ri.Con_Printf (PRINT_ALL,
			   "Unknown number of texture components %i\n",
			   samples);
	    comp = samples;
	}

#if 0
	if (mipmap)
		gluBuild2DMipmaps (GL_TEXTURE_2D, samples, width, height, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else if (scaled_width == width && scaled_height == height)
		qglTexImage2D (GL_TEXTURE_2D, 0, comp, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	else
	{
		gluScaleImage (GL_RGBA, width, height, GL_UNSIGNED_BYTE, trans,
			scaled_width, scaled_height, GL_UNSIGNED_BYTE, scaled);
		qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
#else

	if (!data ||(scaled_width == width && scaled_height == height))
	{
		if (!mipmap || gl_config.have_generate_mipmap)
		{
			if (mipmap) qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, TRUE);

			if ( data&&qglColorTableEXT && gl_ext_palettedtexture->value && samples == gl_solid_format )
			{
				uploaded_paletted = true;
				GL_BuildPalettedTexture( paletted_texture, ( unsigned char * ) data, scaled_width, scaled_height );
				qglTexImage2D( GL_TEXTURE_2D,
							  0,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  paletted_texture );
			}
			else
			{
				qglTexImage2D (GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			if (mipmap) qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, FALSE);

			goto done;
		}
		if (data) memcpy (scaled, data, width*height*4);
	}
	else
		GL_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

	GL_LightScaleTexture (scaled, scaled_width, scaled_height, !mipmap );

	if ( qglColorTableEXT && gl_ext_palettedtexture->value && ( samples == gl_solid_format ) )
	{
		uploaded_paletted = true;
		GL_BuildPalettedTexture( paletted_texture, ( unsigned char * ) scaled, scaled_width, scaled_height );
		qglTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_COLOR_INDEX8_EXT,
					  scaled_width,
					  scaled_height,
					  0,
					  GL_COLOR_INDEX,
					  GL_UNSIGNED_BYTE,
					  paletted_texture );
	}
	else
	{
		if (mipmap && gl_config.have_generate_mipmap) qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, TRUE);

		qglTexImage2D( GL_TEXTURE_2D, 0, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );

		if (mipmap && gl_config.have_generate_mipmap) qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, FALSE);
	}

	if (mipmap && !gl_config.have_generate_mipmap)
	{
		int		miplevel;

		miplevel = 0;
		while (scaled_width > 1 || scaled_height > 1)
		{
			GL_MipMap ((byte *)scaled, scaled_width, scaled_height);
			scaled_width >>= 1;
			scaled_height >>= 1;
			if (scaled_width < 1)
				scaled_width = 1;
			if (scaled_height < 1)
				scaled_height = 1;
			miplevel++;
			if ( qglColorTableEXT && gl_ext_palettedtexture->value && samples == gl_solid_format )
			{
				uploaded_paletted = true;
				GL_BuildPalettedTexture( paletted_texture, ( unsigned char * ) scaled, scaled_width, scaled_height );
				qglTexImage2D( GL_TEXTURE_2D,
							  miplevel,
							  GL_COLOR_INDEX8_EXT,
							  scaled_width,
							  scaled_height,
							  0,
							  GL_COLOR_INDEX,
							  GL_UNSIGNED_BYTE,
							  paletted_texture );
			}
			else
			{
				qglTexImage2D (GL_TEXTURE_2D, miplevel, comp, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
			}
		}
	}
done: ;
#endif


	if (mipmap)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}

	if (want_blend) return 2;
	else if (samples == gl_alpha_format) return 1;

	return 0;
}

/*
===============
GL_Upload8

Returns has_alpha
===============
*/
static qboolean IsPowerOf2( int value )
{
	int i = 1;


	while ( 1 )
	{
		if ( value == i )
			return true;
		if ( i > value )
			return false;
		i <<= 1;
	}
}

int GL_Upload8 (byte *data, byte* palette, int width, int height,  qboolean mipmap, qboolean is_sky )
{
	unsigned	trans[640*256];
	int			i, s;
	int			p;

	s = width*height;

	if (s > sizeof(trans)/4)
		ri.Sys_Error (ERR_DROP, "GL_Upload8: too large");

	if ( qglColorTableEXT && 
		 gl_ext_palettedtexture->value && 
		 is_sky )
	{
		qglTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_COLOR_INDEX8_EXT,
					  width,
					  height,
					  0,
					  GL_COLOR_INDEX,
					  GL_UNSIGNED_BYTE,
					  data );

		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	}
	else
	{
		for (i=0 ; i<s ; i++)
		{
			p = data[i];

			((byte *)&trans[i])[0] = (&palette[p * 3])[0];
			((byte *)&trans[i])[1] = ((byte *)&palette[p * 3])[1];
			((byte *)&trans[i])[2] = ((byte *)&palette[p * 3])[2];
			((byte *)&trans[i])[3] = 0xff;

			if (p == 255)
			{	// transparent, so scan around for another color
				// to avoid alpha fringes
				// FIXME: do a full flood fill so mips work...
				if (i > width && data[i-width] != 255)
					p = data[i-width];
				else if (i < s-width && data[i+width] != 255)
					p = data[i+width];
				else if (i > 0 && data[i-1] != 255)
					p = data[i-1];
				else if (i < s-1 && data[i+1] != 255)
					p = data[i+1];
				else
					p = 0;
				// copy rgb components
				((byte *)&trans[i])[0] = (&palette[p*3])[0];
				((byte *)&trans[i])[1] = ((byte *)&palette[p*3])[1];
				((byte *)&trans[i])[2] = ((byte *)&palette[p*3])[2];
				((byte *)&trans[i])[3] = 0;
			}
		}

		return GL_Upload32 (trans, width, height, mipmap);
	}
	return false;
}


/*
================
GL_LoadPic

This is also used as an entry point for the generated r_notexture
================
*/
image_t *GL_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits, byte* palette)
{
	image_t		*image;
	int			i;

	// find a free image_t
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!image->texnum)
			break;
	}
	if (i == numgltextures)
	{
		if (numgltextures == MAX_GLTEXTURES)
			ri.Sys_Error (ERR_DROP, "MAX_GLTEXTURES");
		numgltextures++;
	}
	image = &gltextures[i];

	if (strlen(name) >= sizeof(image->name))
		ri.Sys_Error (ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
	strcpy (image->name, name);
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

	if (type == it_skin && bits == 8)
		R_FloodFillSkin(pic, width, height);

	// load little pics into the scrap
	if (image->type == it_pic && bits == 8
		&& image->width < 64 && image->height < 64)
	{
		int		x, y;
		int		i, j, k;
		int		texnum;

		texnum = Scrap_AllocBlock (image->width, image->height, &x, &y);
		if (texnum == -1)
			goto nonscrap;
		scrap_dirty = true;

		// copy the texels into the scrap block
		k = 0;
		for (i=0 ; i<image->height ; i++)
			for (j=0 ; j<image->width ; j++, k++)
				scrap_texels[texnum][(y+i)*BLOCK_WIDTH + x + j] = pic[k];
		image->texnum = TEXNUM_SCRAPS + texnum;
		image->scrap = true;
		image->has_alpha = true;
		image->sl = (x+0.01)/(float)BLOCK_WIDTH;
		image->sh = (x+image->width-0.01)/(float)BLOCK_WIDTH;
		image->tl = (y+0.01)/(float)BLOCK_WIDTH;
		image->th = (y+image->height-0.01)/(float)BLOCK_WIDTH;
	}
	else if (image->type == it_cubemap_ext)
	{
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_BindCube (image->texnum);

		qglTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		pic += width*height*4;
		qglTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		pic += width*height*4;
		qglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		pic += width*height*4;
		qglTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		pic += width*height*4;
		qglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		pic += width*height*4;
		qglTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic );

		qglTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		qglTexParameterf(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		image->upload_width = width;
		image->upload_height = height;
		image->paletted = false;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}
	else
	{
nonscrap:
		image->scrap = false;
		image->texnum = TEXNUM_IMAGES + (image - gltextures);
		GL_Bind(image->texnum);
		if (bits == 8)
			image->has_alpha = GL_Upload8 (pic, palette, width, height, (image->type != it_pic && image->type != it_sky && image->type != it_clamped), image->type == it_sky );
		else
			image->has_alpha = GL_Upload32 ((unsigned *)pic, width, height, (image->type != it_pic && image->type != it_sky && image->type != it_clamped) );
		image->upload_width = upload_width;		// after power of 2 and scales
		image->upload_height = upload_height;
		image->paletted = uploaded_paletted;
		image->sl = 0;
		image->sh = 1;
		image->tl = 0;
		image->th = 1;
	}

	if (image->type == it_clamped)
	{
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}


	return image;
}


/*
================
GL_LoadWal
================
*/
image_t *GL_LoadWal (char *name, char *filename)
{
	miptex_t	*mt;
	int			width, height, ofs;
	image_t		*image;

	ri.FS_LoadFile (filename, (void **)&mt);
	if (!mt)
	{
		return NULL;
	}

	width = LittleLong (mt->width);
	height = LittleLong (mt->height);
	ofs = LittleLong (mt->offsets[0]);

	image = GL_LoadPic (name, (byte *)mt + ofs, width, height, it_wall, 8, gPalette);

	ri.FS_FreeFile ((void *)mt);

	return image;
}

/*
===============
GL_FindImage

Finds or loads the given image
===============
*/
image_t	*GL_FindImage (char *name, char *fallback, imagetype_t type)
{
	image_t	*image = NULL;
	size_t		i, len;
	int		width, height;
	char	newname[MAX_QPATH];

	if (!name)
		return NULL;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: NULL name");

	// Strip extension
	COM_StripExtension(name,newname);
	name = newname;

	len = strlen(name);

	// look for it
	for (i=0, image=gltextures ; i<numgltextures ; i++,image++)
	{
		if (!Q_strcasecmp(name, image->name) || fallback && !Q_strcasecmp(fallback, image->name))
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	//
	// load the pic from disk
	//

	image = NULL;

	// Try a TGA first
	if (!image)
	{
		byte	*pic = NULL;

		strcat(name+len, ".tga");
		LoadTGA (name, &pic, &width, &height, false);
		name[len] = 0; 

		if (pic) 
		{
			image = GL_LoadPic (name, pic, width, height, type, 32,0);
			free(pic);
			pic = NULL;
		}
	}

	// Try a PNG first
	if (!image)
	{
		byte	*pic = NULL;

		strcat(name+len, ".png");
		LoadPNG (name, &pic, &width, &height, false);
		name[len] = 0; 

		if (pic) 
		{
			image = GL_LoadPic (name, pic, width, height, type, 32,0);
			free(pic);
			pic = NULL;
		}
	}

	// Try a PCX
	if (!image)
	{
		byte	*pic = NULL, *palette;

		strcat(name+len, ".pcx");
		LoadPCX (name, &pic, &palette, &width, &height);
		name[len] = 0; 

		if (pic) 
		{
			image = GL_LoadPic (name, pic, width, height, type, 8,palette);
			free(pic);
			free(palette);
		}
	}

	// Try an ATD
	if (!image) 
	{
		byte	*pic = NULL;
		int		clamp = 0;
		atd_t	*atd = NULL;

		strcat(name+len, ".atd");
		atd = ATD_Load (name, &pic, &width, &height, &clamp);
		name[len] = 0; 

		if (atd) 
		{
			image = GL_LoadPic (name, pic, width, height, type, 32,0);
			image->atd = atd;

			// Ok just make sure the filtering modes are correct
			if (type != it_pic && type != it_sky)
			{
				if (atd->bilinear) 
				{
					// If we can generate mipmaps, then use the user specified mipmap min filter
					// Otherwise use the 'max' filter for both up and down
					if (gl_config.have_generate_mipmap)
					{
						qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, TRUE);
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
					}
					else
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);

					qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
				}
				else 
				{
					// Nearest, but we will allow mipmapping
					if (gl_config.have_generate_mipmap)
					{
						qglTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, TRUE);
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
					}
					else
						qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

					qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
			} 
			else if (atd->bilinear) 
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			// Clamping
			if (clamp) 
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
			else
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			// Now update it
			ATD_Update(image);
		}

	}

	// Try a wal
	if (!image) 
	{
		char	filename[MAX_QPATH];

		memcpy(filename, name, len);
		filename[len] = 0;
		strcat(filename+len, ".wal");
		image = GL_LoadWal (name, filename);
	}

	// Now just put the 'bad' texture
	if (!image)
	{
		if (fallback) return GL_FindImage(fallback, 0, type);
		ri.Con_Printf(PRINT_ALL, "GL_FindImage: can't load %s\n", name);
		// find a free image_t
		for (i = 0, image = gltextures; i < numgltextures; i++, image++)
		{
			if (!image->texnum)
				break;
		}
		if (i == numgltextures)
		{
			if (numgltextures == MAX_GLTEXTURES)
				ri.Sys_Error(ERR_DROP, "MAX_GLTEXTURES");
			numgltextures++;
		}
		image = &gltextures[i];
		*image = *r_notexture;

		if (strlen(name) >= sizeof(image->name))
			ri.Sys_Error(ERR_DROP, "Draw_LoadPic: \"%s\" is too long", name);
		strcpy(image->name, name);
		image->registration_sequence = registration_sequence;
	}

	return image;
}



void GL_FindImage2 (char *name, byte **pic, int *width, int *height, qboolean hunk)
{
	size_t		i, len;
	char	newname[MAX_QPATH];

	*pic = NULL;
	if (!name) return;	//	ri.Sys_Error (ERR_DROP, "GL_FindImage: NULL name");

	// Strip extension
	COM_StripExtension(name,newname);
	name = newname;

	len = strlen(name);

	//
	// load the pic from disk
	//

	// Try a TGA first
	*pic = NULL;
	strcat(name+len, ".tga");
	LoadTGA (name, pic, width, height, hunk);
	name[len] = 0; 
	if (*pic) return;

	// Try a PNG first
	*pic = NULL;
	strcat(name+len, ".png");
	LoadPNG (name, pic, width, height, hunk);
	name[len] = 0; 

	if (*pic) return;
}



/*
===============
R_RegisterSkin
===============
*/
struct image_s *R_RegisterSkin (char *name)
{
	return GL_FindImage (name, 0,it_skin);
}


/*
===============
R_RegisterClamped
===============
*/
struct image_s *R_RegisterClamped (char *name)
{
	return GL_FindImage (name,0, it_clamped);
}


/*
================
GL_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void GL_FreeUnusedImages (void)
{
	int		i;
	image_t	*image;

	// never free r_notexture or particle texture
	r_notexture->registration_sequence = registration_sequence;
	r_particletexture->registration_sequence = registration_sequence;
	r_newparticletexture->registration_sequence = registration_sequence;
	if (r_norm_cube) r_norm_cube->registration_sequence = registration_sequence;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
			continue;		// used this sequence
		if (!image->registration_sequence)
			continue;		// free image_t slot
		if (image->type == it_pic)
			continue;		// don't free pics
		// free it
		qglDeleteTextures (1, &image->texnum);
		if (image->atd) ATD_Free(image->atd);
		memset (image, 0, sizeof(*image));
	}
}

/*
===============
Draw_GetPalette
===============
*/

int Draw_GetPalette (void)
{
	int		i;
	int		r, g, b;
	unsigned	v;
	byte	*pic, *pal;
	int		width, height;

	// get the palette
	// TODO make palette stuff optional
	LoadPCX("graphics/colormap.pcx", &pic, &pal, &width, &height);
	if (!pal)
	{
		LoadPCX("pics/colormap.pcx", &pic, &pal, &width, &height);
		if (!pal)
			ri.Sys_Error(ERR_FATAL, "Couldn't load pics/colormap.pcx");
	}

	for (i=0 ; i<256 ; i++)
	{
		r = pal[i*3+0];
		g = pal[i*3+1];
		b = pal[i*3+2];
		gPalette[i * 3 + 0] = r;
		gPalette[i * 3 + 1] = g;
		gPalette[i * 3 + 2] = b;

		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		d_8to24table[i] = LittleLong(v);
	}

	d_8to24table[255] &= LittleLong(0xffffff);	// 255 is transparent

	free (pic);
	free (pal);

	return 0;
}


/*
===============
GL_InitImages
===============
*/
void	GL_InitImages (void)
{
	int		i, j;
	float	g = vid_gamma->value, intens = 1;

	registration_sequence = 1;

	// init intensity conversions
	intensity = ri.Cvar_Get ("intensity", "1", CVAR_ARCHIVE);

	if ( intensity->value <= 1 ) ri.Cvar_Set( "intensity", "1" );
	else intens = intensity->value;

	gl_state.inverse_intensity = 1 / intens;

	if (qwglSetDeviceGammaRamp) intens = 1;

	Draw_GetPalette ();

	if ( qglColorTableEXT )
	{
		ri.FS_LoadFile( "pics/16to8.dat", &gl_state.d_16to8table );
		if ( !gl_state.d_16to8table )
			ri.Sys_Error( ERR_FATAL, "Couldn't load pics/16to8.pcx");
	}

	if ( qwglSetDeviceGammaRamp || gl_config.renderer & ( GL_RENDERER_VOODOO ) )
	{
		g = 1.0F;
	}

	for ( i = 0; i < 256; i++ )
	{
		if ( g == 1 )
		{
			gammatable[i] = i;
		}
		else
		{
			float inf;

			inf = 255 * pow ( (i+0.5)/255.5 , g ) + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			gammatable[i] = inf;
		}
	}

	for (i=0 ; i<256 ; i++)
	{
		j = i*intens;
		if (j > 255)
			j = 255;
		intensitytable[i] = j;
	}
}

/*
===============
GL_ShutdownImages
===============
*/
void	GL_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	for (i=0, image=gltextures ; i<numgltextures ; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free image_t slot
		// free it
		qglDeleteTextures (1, &image->texnum);
		if (image->atd) ATD_Free(image->atd);
		memset (image, 0, sizeof(*image));
	}
}


static int atd_update_buffer[256*256];

/*
===============
ATD Interform
===============
*/
void GenerateScrollCoords(float vx, float vy, float new_tc[2])
{
	// vx and vy are units per second
	new_tc[0] = (r_newrefdef.time*vx) - (int)(r_newrefdef.time*vx);
	new_tc[1] = (r_newrefdef.time*vy) - (int)(r_newrefdef.time*vy);
}

// Ok, just crapping on here
void GenerateWanderCoords(float last_time, float rate, float strength, float speed, float tc_delta[2])
{
	float diff = r_newrefdef.time - last_time;
	float speed_frac;
	float rate_frac;
	float rand_dir;

	if (diff < 0) diff = 0;

	speed_frac = (diff*speed) - (int)(diff*speed);
	rate_frac = (diff*rate) - (int)(diff*rate);

//	rand_dir = (rand() / (float)(RAND_MAX)) * M_PI;
	rand_dir = rate_frac * M_PI * 2;
	tc_delta[0] = speed_frac/20 + (cos(rand_dir)+0.5) * strength /150;//4 *  * (1-strength) + ;
	tc_delta[1] = speed_frac/30 + (sin(rand_dir)-0.5) * strength /100;

	tc_delta[0] = tc_delta[0] - (int)tc_delta[0];
	tc_delta[1] = tc_delta[1] - (int)tc_delta[1];
}

static void ATD_Update_Interform (image_t *image, qboolean force)
{
	atd_interform_t *atd = (atd_interform_t *) image->atd;
	int	mother_tc[2];
	int	father_tc[2];
	float tc_temp[2];
	int	*out, *line_end, *end, *palette;
	byte *mother, *mother_line_end, *mother_end, *mother_next;
	byte *father, *father_line_end, *father_end, *father_next;

	// Mother Texture Coord Generation
	tc_temp[0] = tc_temp[1] = 0;
	if (atd->mother_move == atd_move_scroll) 
		GenerateScrollCoords(atd->mother_vx, atd->mother_vy, tc_temp);
	else if (atd->mother_move == atd_move_wander)
	{
		float tc_delta[2];
		GenerateWanderCoords(atd->last_time, atd->mother_rate, atd->mother_strength, atd->mother_speed, tc_delta);

		if (atd->mother.width) tc_temp[0] = atd->mother_tc[0]/(float)(atd->mother.width) + tc_delta[0];
		if (atd->mother.height) tc_temp[1] = atd->mother_tc[1]/(float)(atd->mother.height) + tc_delta[1];

		tc_temp[0] = tc_temp[0] - (int) tc_temp[0];
		tc_temp[1] = tc_temp[1] - (int) tc_temp[1];
	}

	while (tc_temp[0] < 0) tc_temp[0] += 1;
	while (tc_temp[1] < 0) tc_temp[1] += 1;
	mother_tc[0] = (int) (atd->mother_tc[0] = tc_temp[0] * atd->mother.width);
	mother_tc[1] = (int) (atd->mother_tc[1] = tc_temp[1] * atd->mother.height);

	// Father Texture Coord Generation
	tc_temp[0] = tc_temp[1] = 0;
	if (atd->father_move == atd_move_scroll) 
	{
		GenerateScrollCoords(atd->father_vx, atd->father_vy, tc_temp);
	}
	else if (atd->father_move == atd_move_wander)
	{
		float tc_delta[2];
		GenerateWanderCoords(atd->last_time, atd->father_rate, atd->father_strength, atd->father_speed, tc_delta);

		if (atd->father.width) tc_temp[0] = atd->father_tc[0]/(float)(atd->father.width) + tc_delta[0];
		if (atd->father.height) tc_temp[1] = atd->father_tc[1]/(float)(atd->father.height) + tc_delta[1];

		tc_temp[0] = tc_temp[0] - (int) tc_temp[0];
		tc_temp[1] = tc_temp[1] - (int) tc_temp[1];
	}

	while (tc_temp[0] < 0) tc_temp[0] += 1;
	while (tc_temp[1] < 0) tc_temp[1] += 1;
	father_tc[0] = (int) (atd->father_tc[0] = tc_temp[0] * atd->father.width);
	father_tc[1] = (int) (atd->father_tc[1] = tc_temp[1] * atd->father.height);


	// Ok we need to regenerate it

	palette = (int*) atd->palette.pixels;

	out      = atd_update_buffer;
	line_end = atd_update_buffer + image->width;
	end      = atd_update_buffer + image->width*image->height;

	mother          = atd->mother.pixels + atd->mother.width*mother_tc[1] + mother_tc[0];
	mother_line_end = atd->mother.pixels + atd->mother.width*mother_tc[1] + atd->mother.width;
	mother_end      = atd->mother.pixels + atd->mother.width*atd->mother.height + mother_tc[0];


	// Only have mother so it's easy. Just the palette lookup
	if (!atd->father.pixels)
	{
		while (out != end) 
		{
			mother_next = mother + atd->mother.width;

			while (out != line_end) 
			{
				*out++ = palette[*mother++];
				//*out++ = *mother++;
				if (mother == mother_line_end) mother -= atd->mother.width;
			}

			mother = mother_next;

			line_end += image->width;
			mother_line_end += atd->mother.width;

			if (mother == mother_end) 
			{
				mother -= atd->mother.width * atd->mother.height;
				mother_line_end -= atd->mother.width * atd->mother.height;
			}
		}
	}
	else
	{
		father          = atd->father.pixels + atd->father.width*father_tc[1] + father_tc[0];
		father_line_end = atd->father.pixels + atd->father.width*father_tc[1] + atd->father.width;
		father_end      = atd->father.pixels + atd->father.width*atd->father.height + father_tc[0];

		while (out != end) 
		{
			mother_next = mother + atd->mother.width;
			father_next = father + atd->father.width;
			while (out != line_end) 
			{
				*out++ = palette[((*mother++) + (*father++))>>1];
				//*out++ = *mother++ + (*father++ << 8);
				if (mother == mother_line_end) mother -= atd->mother.width;
				if (father == father_line_end) father -= atd->father.width;
			}

			mother = mother_next;
			father = father_next;
			line_end += image->width;
			mother_line_end += atd->mother.width;
			father_line_end += atd->father.width;

			if (mother == mother_end) 
			{
				mother -= atd->mother.width * atd->mother.height;
				mother_line_end -= atd->mother.width * atd->mother.height;
			}

			if (father == father_end) 
			{
				father -= atd->father.width * atd->father.height;
				father_line_end -= atd->father.width * atd->father.height;
			}
		}
	}

	qglTexImage2D(GL_TEXTURE_2D, 
					0,
					GL_RGBA8,
					image->width,
					image->height,
					0,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					atd_update_buffer);
}

#define TRY_ATD_FLOAT_TOKEN(name)		\
	if (strcmp(token, #name) == 0) do	\
	{									\
		token = COM_Parse4(&tokens);	\
		if (!tokens) return NULL;		\
			atd->name = atof(token);	\
	} while(0)

atd_t *ATD_LoadInterform (char *tokens, byte **pic, int *width, int *height, int *clamp)
{
	atd_interform_t	*atd;
	char *token;

	atd = (atd_interform_t*)Hunk_Alloc2(sizeof(atd_interform_t));
	memset(atd,0,sizeof(atd_interform_t));
	atd->type = atd_type_interform;
	atd->bilinear = true;

	// Defaults
	atd->mother_move = atd_move_none;
	atd->mother_speed = 0.3;
	atd->mother_rate = 1;
	atd->mother_strength = 1;

	atd->father_move = atd_move_none;
	atd->father_speed = 0.3;
	atd->father_rate = 1;
	atd->father_strength = 1;

	*clamp = false;

	// Find our type
	while (1) {
		token = COM_Parse4(&tokens);
		if (!tokens) break;

		// Look for 'width' key
		if (strcmp(token, "width") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*width = atoi(token);
		}
		// Look for 'height' key
		else if (strcmp(token, "height") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*height = atoi(token);
		}
		// Look for 'palette' key
		else if (strcmp(token, "palette") == 0)
		{
			// Only 1 file per bitmap please
			if (atd->palette.pixels) return NULL;

			// read data
			token = COM_Parse4(&tokens);
			if (!*tokens) break;

			// Attempt to load the PGN
			GL_FindImage2 (token, &atd->palette.pixels, &atd->palette.width, &atd->palette.height, true);
		}
		// Look for 'mother' key
		else if (strcmp(token, "mother") == 0)
		{
			// Only 1 file per bitmap please
			if (atd->mother.pixels) return NULL;

			// read data
			token = COM_Parse4(&tokens);
			if (!*tokens) break;

			// Attempt to load the PGN
			LoadPNG_8Bit (token, &atd->mother.pixels, &atd->mother.width, &atd->mother.height, true);
		}
		// Look for 'father' key
		else if (strcmp(token, "father") == 0)
		{
			// Only 1 file per bitmap please
			if (atd->father.pixels) return NULL;

			// read data
			token = COM_Parse4(&tokens);
			if (!*tokens) break;

			// Attempt to load the PGN
			LoadPNG_8Bit (token, &atd->father.pixels, &atd->father.width, &atd->father.height, true);
		}
		// Look for 'mother_move' key
		else if (strcmp(token, "mother_move") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			if (strcmp(token, "none") == 0)
				atd->mother_move = atd_move_none;
			else if (strcmp(token, "scroll") == 0)
				atd->mother_move = atd_move_scroll;
			else if (strcmp(token, "wander") == 0)
				atd->mother_move = atd_move_wander;
			else 
			{
				ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. Unknown mother_move type (%s)\n", token);
				return NULL;
			}
		}
		// Look for 'father_move' key
		else if (strcmp(token, "father_move") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			if (strcmp(token, "none") == 0)
				atd->father_move = atd_move_none;
			else if (strcmp(token, "scroll") == 0)
				atd->father_move = atd_move_scroll;
			else if (strcmp(token, "wander") == 0)
				atd->father_move = atd_move_wander;
			else 
			{
				ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. Unknown father_move type (%s)\n", token);
				return NULL;
			}
		}
		else TRY_ATD_FLOAT_TOKEN(mother_vx);
		else TRY_ATD_FLOAT_TOKEN(mother_vy);
		else TRY_ATD_FLOAT_TOKEN(mother_speed);
		else TRY_ATD_FLOAT_TOKEN(mother_rate);
		else TRY_ATD_FLOAT_TOKEN(mother_strength);
		else TRY_ATD_FLOAT_TOKEN(father_vx);
		else TRY_ATD_FLOAT_TOKEN(father_vy);
		else TRY_ATD_FLOAT_TOKEN(father_speed);
		else TRY_ATD_FLOAT_TOKEN(father_rate);
		else TRY_ATD_FLOAT_TOKEN(father_strength);
		// Eat unknown crap
		else
		{
			// now the data
			token = COM_Parse4(&tokens);
			if (!tokens) break;
		}
	}


	if (!IsPowerOf2(*width) || !IsPowerOf2(*height))
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. Width and Height must be POW2\n");
		return NULL;
	}

	if (*width < 1 || *width > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. width out of range (0-256) - %i\n", *width);
		return NULL;
	}

	if (*height < 1 || *height > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. height out of range (0-256) - %i\n", *height);
		return NULL;
	}

	if (!atd->palette.pixels) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. No palette\n");
		return NULL;
	}

	if (atd->palette.width != 256 )
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. Palette PNG must be exactly 256 pixel wide\n");
		return NULL;
	}

	if (!atd->mother.pixels && !atd->father.pixels) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. No mother and father. At least 1 must exist.\n");
		return NULL;
	}

	// A little trick. If no mother, copy father to mother and clear father
	if (!atd->mother.pixels)
	{
		atd->mother = atd->father;
		atd->mother_move = atd->father_move;
		atd->mother_rate = atd->father_rate;
		atd->mother_speed = atd->father_speed;
		atd->mother_strength = atd->father_strength;
		atd->mother_vx = atd->father_vx;
		atd->mother_vy = atd->father_vy;
		atd->father.pixels = 0;
		atd->father.height = 0;
		atd->father.width = 0;
	}

	memset(atd_update_buffer, 255, (*width)*(*height)*4);
	memcpy (atd_update_buffer, atd->palette.pixels, 768);

	*pic = (byte*) atd_update_buffer;

	return (atd_t*) atd;
}

/*
===============
ATD Animation
===============
*/

// Note, is recursive, though we will attempt to catch stupidity 
static void ATD_Update_Animation (image_t *image)
{
	int i;
	atd_animation_t *atd = (atd_animation_t *) image->atd;
	atd_frame_t		*frame = atd->nextframe;
	atd_frame_t		*nextframe;
	atd_bitmap_t	*bitmap;

	// Only update IF est_next is less than current time
	if (atd->est_next > r_newrefdef.time) return;

	// Make sure we weren't already run this frame
	if (frame->last_time == r_newrefdef.time) return;
	frame->last_time = r_newrefdef.time;

	// Do our animation

	// Get the Bitmap
	bitmap = atd->bitmaps;
	for (i = 0; i < frame->bitmap; i++) {
		bitmap = bitmap->listnext;
	}

	// Now upload it


	// needs clipping ? (i think these are unsupported, so we wont do anything)
	if ((frame->x + bitmap->width) > image->width || 
		(frame->y + bitmap->height) > image->height ||
		frame->x < 0 || frame->y < 0)
	{
	}
	// Just upload it using glTexSubImage2D
	else if (gl_atd_subimage_update->value || !atd->pixels)
	{
		qglTexSubImage2D(GL_TEXTURE_2D, 
							0,
							frame->x,
							frame->y,
							bitmap->width,
							bitmap->height,
							GL_RGBA,
							GL_UNSIGNED_BYTE,
							bitmap->pixels);
	}
	// Do full uploads
	else
	{
		int internalformat = GL_RGBA8;
		if (!image->has_alpha) internalformat = GL_RGB8;

		// Full width updates can be done in a single memcpy
		if (frame->x == 0 && bitmap->width == image->width)
		{
			memcpy(atd->pixels+(frame->y*bitmap->width),
					bitmap->pixels, 
					bitmap->width*bitmap->height*4);
		}
		else
		{
			int i;
			int *dest, *src;
			
			dest = atd->pixels + frame->x + frame->y*image->width;
			src =(int*) bitmap->pixels;

			for (i = 0; i < bitmap->height; i++)
			{
				memcpy(dest, src, bitmap->width*4);
				src += bitmap->width;
				dest += image->width;
			}
		}

		qglTexImage2D(GL_TEXTURE_2D, 
						0,
						internalformat,
						image->width,
						image->height,
						0,
						GL_RGBA,
						GL_UNSIGNED_BYTE,
						atd->pixels);

	}


	// Get next frame
	
	// None? then nothing else for us to do
	if (frame->next == -1) 
	{
		atd->nextframe = NULL;
		return;
	}

	// Get our next
	nextframe = atd->frames;
	for (i = 0; i < frame->next; i++) 
	{
		nextframe = nextframe->listnext;
	}

	atd->nextframe = nextframe;
	
	// Run it now if our wait time was -1, but only if it wasn't already run this time
	if (frame->wait == -1) ATD_Update_Animation (image);
	else atd->est_next = r_newrefdef.time+frame->wait;	// Time in seconds to milliseconds
}

atd_bitmap_t * ATD_LoadAnimation_ParseBitmap (char **tokens)
{
	char *token;
	atd_bitmap_t *ret = NULL;

	ret = (atd_bitmap_t*) Hunk_Alloc2 (sizeof(atd_bitmap_t));
	ret->pixels = 0;	// If loaded and ok, this is set to something

	while (1)
	{
		char *prev = *tokens;
		token = COM_Parse4(tokens);
		if (!*tokens) break;

		// Look for 'colortype' key
		if (strcmp(token, "file") == 0)
		{
			// Only 1 file per bitmap please
			if (ret->pixels) return NULL;

			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			// Attempt to load the PGN
			GL_FindImage2 (token, &ret->pixels, &ret->width, &ret->height, true);

			// Was not ok!
			if (!ret->pixels) return NULL;
		}
		// Whoa.... take a step back, we've stepped out of our bounds
		else if (token[0] == '!')
		{
			*tokens = prev;
			break;
		}
	}

	if (ret->pixels) return ret;

	return NULL;
}

atd_frame_t * ATD_LoadAnimation_ParseFrame (char **tokens)
{
	char *token;
	atd_frame_t	*f;

	f = Hunk_Alloc2(sizeof(atd_frame_t));
	
	// Set our defaults
	f->bitmap = -1;		// This is set if it's ok
	f->next = -1;
	f->wait = 0;
	f->x = 0;
	f->y = 0;
	f->listnext = NULL;
	f->last_time = -1;

	while (1)
	{
		char *prev = *tokens;
		token = COM_Parse4(tokens);
		if (!*tokens) break;

		if (strcmp(token, "bitmap") == 0)
		{
			// Only 1 bitmap per animation please
			if (f->bitmap != -1) return NULL;

			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			f->bitmap = atoi(token);

			// Invalid bitmap number
			if (f->bitmap < 0) return NULL;	
		}
		else if (strcmp(token, "next") == 0)
		{
			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			f->next = atoi(token);
		}
		else if (strcmp(token, "x") == 0)
		{
			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			f->x = atoi(token);
		}
		else if (strcmp(token, "y") == 0)
		{
			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			f->y = atoi(token);
		}
		else if (strcmp(token, "wait") == 0)
		{
			// read data
			token = COM_Parse4(tokens);
			if (!*tokens) break;

			f->wait = atof(token);
		}
		// Whoa.... take a step back, we've stepped out of our bounds
		else if (token[0] == '!')
		{
			*tokens = prev;
			break;
		}
	}

	// Return the frame if ok
	if (f->bitmap != -1) return f;

	return NULL;
}

atd_t *ATD_LoadAnimation (char *tokens, byte **pic, int *width, int *height, int *clamp)
{
	atd_animation_t	*atd;
	char *token;
	int	colortype = 0;
	int num_bitmaps = 0;
	int num_frames = 0;
	int i;
	atd_bitmap_t	*lastbitmap = NULL, *bitmap;
	atd_frame_t		*lastframe = NULL, *frame;

	atd = (atd_animation_t*)Hunk_Alloc2(sizeof(atd_animation_t));
	memset(atd,0,sizeof(atd_animation_t));
	atd->type = atd_type_animation;
	atd->bilinear = true;

	*clamp = false;

	// Find our type
	while (1) {
		token = COM_Parse4(&tokens);
		if (!tokens) break;

		// Look for 'colortype' key
		if (strcmp(token, "colortype") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			colortype = atoi(token);
		}
		// Look for 'width' key
		else if (strcmp(token, "width") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*width = atoi(token);
		}
		// Look for 'height' key
		else if (strcmp(token, "height") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*height = atoi(token);
		}
		// Look for 'bilinear' key
		else if (strcmp(token, "bilinear") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			atd->bilinear = atoi(token)!=0;
		}
		// Look for 'clamp' key
		else if (strcmp(token, "clamp") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*clamp = atoi(token)!=0;
		}
		// Parse bitmaps
		else if (strcmp(token, "!bitmap") == 0)
		{
			bitmap = ATD_LoadAnimation_ParseBitmap(&tokens);

			if (!bitmap) 
			{
				ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation bitmap %i\n", num_bitmaps);
				return NULL;
			}

			if (!lastbitmap) atd->bitmaps = bitmap;
			else lastbitmap->listnext = bitmap;

			lastbitmap = bitmap;
			num_bitmaps++;
		}
		// Parse frames
		else if (strcmp(token, "!frame") == 0)
		{
			frame = ATD_LoadAnimation_ParseFrame(&tokens);

			if (!frame) {
				ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation frame %i\n", num_frames);
				return NULL;
			}

			if (!lastframe) atd->frames = frame;
			else lastframe->listnext = frame;

			lastframe = frame;
			num_frames++;
		}
		// Eat unknown crap
		else
		{
			// now the data
			token = COM_Parse4(&tokens);
			if (!tokens) break;
		}
	}


	if (!num_frames) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. No frames\n");
		return NULL;
	}

	if (!num_bitmaps) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. No bitmaps\n");
		return NULL;
	}

	if (colortype < 1 || colortype > 4) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. colortype out of range (0-4) - %i\n", colortype);
		return NULL;
	}

	if (!IsPowerOf2(*width) || !IsPowerOf2(*height))
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. Width and Height must be POW2\n");
		return NULL;
	}

	if (*width < 1 || *width > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. width out of range (0-256) - %i\n", *width);
		return NULL;
	}

	if (*height < 1 || *height > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. height out of range (0-256) - %i\n", *height);
		return NULL;
	}

	// Verify frames
	i = 0;
	for (frame = atd->frames; frame != NULL; frame=frame->listnext)
	{
		if (frame->bitmap >= num_bitmaps)
		{
			ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. Bitmap index in frame (%i) out of range (0-%i) - %i\n", i, num_bitmaps-1, frame->bitmap);
			return NULL;
		}

		if (frame->next >= num_frames && frame->next != -1)
		{
			ri.Con_Printf (PRINT_ALL, "Error parsing ATD Animation. Next frame index in frame (%i) out of range (0-%i) - %i\n", i, num_frames-1, frame->next);
			return NULL;
		}

		i++;
	}

	if (gl_atd_subimage_update->value) *pic =(byte**)&atd_update_buffer[0];
	else *pic = (byte**)(atd->pixels = Hunk_Alloc2((*width)*(*height)*4));

	if (colortype & 1) memset(*pic, 255, (*width)*(*height)*4);
	else memset(*pic, 127, (*width)*(*height)*4);

	// Set next frame
	atd->nextframe = atd->frames;

	return (atd_t*) atd;
}

/*
===============
ATD Whitenoise
===============
*/

int	rand1k[] = {
#include "rand1k.h"
};

#define MASK_1K	0x3FF

int		rand1k_index = 0;

static void ATD_Update_WhiteNoise (image_t *image)
{
	static int last = 0;
	int i;
	byte v;
	byte *buf = (byte *) atd_update_buffer;
	int total = image->width * image->height;

	for (i = 0; i < total; i++)
	{
		buf[i] = rand()&0xFF;
	}

	qglTexImage2D(GL_TEXTURE_2D, 
					0,
					GL_LUMINANCE,
					image->width,
					image->height,
					0,
					GL_LUMINANCE,
					GL_UNSIGNED_BYTE,
					atd_update_buffer);

}

atd_t *ATD_LoadWhiteNoise (char *tokens, byte **pic, int *width, int *height, int *clamp)
{
	atd_t	*atd;
	char *token;

	atd = (atd_t*)Hunk_Alloc2(sizeof(atd_t));
	memset(atd,0,sizeof(atd_t));
	atd->type = atd_type_whitenoise;
	atd->bilinear = false;

	*clamp = false;

	// Find our type
	while (1) {
		token = COM_Parse4(&tokens);
		if (!tokens) break;

		// Look for 'width' key
		if (strcmp(token, "width") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*width = atoi(token);
		}
		// Look for 'height' key
		else if (strcmp(token, "height") == 0)
		{
			// read data
			token = COM_Parse4(&tokens);
			if (!tokens) return NULL;

			*height = atoi(token);
		}
		// Eat unknown crap
		else
		{
			// now the data
			token = COM_Parse4(&tokens);
			if (!tokens) break;
		}
	}


	if (!IsPowerOf2(*width) || !IsPowerOf2(*height))
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. Width and Height must be POW2\n");
		return NULL;
	}

	if (*width < 1 || *width > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. width out of range (0-256) - %i\n", *width);
		return NULL;
	}

	if (*height < 1 || *height > 256) 
	{
		ri.Con_Printf (PRINT_ALL, "Error parsing ATD Interform. height out of range (0-256) - %i\n", *height);
		return NULL;
	}

	memset(atd_update_buffer, 255, (*width)*(*height)*4);
	*pic = (byte*) atd_update_buffer;

	return (atd_t*) atd;
}

/*
===============
ATD_Load
===============
*/
atd_t *ATD_Load (char *name, byte **pic, int *width, int *height, int *clamp)
{
	int			i;
	char		*data, *tokens, *token;
	atd_t		*atd = NULL;
	atd_type_t	type = atd_type_unknown;
	void		*data_base;

	//
	// load the file
	//
	i = ri.FS_LoadFile (name, (void **)&data);
	if (!data)
	{
		return NULL;
	}

	// Allocate some storage
	data_base = Hunk_Begin2(0x300000);

	// Need to make it a null terminated string
	tokens = malloc(i+1);
	memcpy(tokens, data, i);
	tokens[i] = 0;

	ri.FS_FreeFile (data);
	data = tokens;

	// Read header
	if (LittleLong(*(long*)data) != ATDHEADER) 
	{
		ri.Con_Printf (PRINT_ALL, "Missing ATD header in file %s\n", name);
		goto end;
	}

	tokens += 4;

	// Find our type
	while (1) {
		token = COM_Parse4(&tokens);
		if (!tokens) break;

		// Look for 'type' key
		if (strcmp(token, "type") == 0)
		{
			// now the type
			token = COM_Parse4(&tokens);
			if (!tokens) break;

			if (strcmp(token, "animation") == 0)
				type = atd_type_animation;
			else if (strcmp(token, "interform") == 0)
				type = atd_type_interform;
			else if (strcmp(token, "whitenoise") == 0)
				type = atd_type_whitenoise;

			break;

		}
		// Eat remaining crap
		else
		{
			// now the data
			token = COM_Parse4(&tokens);
			if (!tokens) break;
		}

	}

	// Turn off pic mip
	no_pic_mip = true;

	switch (type) 
	{
	case atd_type_animation:
		atd = ATD_LoadAnimation (data+4, pic, width, height, clamp);
		break;

	case atd_type_interform:
		atd = ATD_LoadInterform (data+4, pic, width, height, clamp);
		break;

	case atd_type_whitenoise:
//		ri.Con_Printf (PRINT_ALL, "Unsupported ATD type (%i) in file %s\n", type, name);
		//image = ATD_LoadWhitenoise (data+4);
		atd = ATD_LoadWhiteNoise (data+4, pic, width, height, clamp);
		break;

	default:
		ri.Con_Printf (PRINT_ALL, "Unknown ATD type in file %s\n", name);
	}

end:
	free(data);

	Hunk_End2();

	// Um, not us
	if (!atd) 
	{
		// Kill the hunk
		Hunk_Free2(data_base);

		ri.Con_Printf (PRINT_ALL, "Failed to load ATD %s\n", name);
		return NULL;
	}

	return atd;
}

/*
===============
ATD_Free
===============
*/
void ATD_Free (atd_t *atd)
{
	/*
	if (atd->type == atd_type_interform) 
	{
		atd_interform_t *iform = (atd_interform_t*) atd;

		if (iform->mother.pixels) free(iform->mother.pixels);
		if (iform->father.pixels) free(iform->father.pixels);
		if (iform->palette.pixels) free(iform->palette.pixels);
	}
	else if (atd->type == atd_type_animation) 
	{
		atd_animation_t *anim = (atd_animation_t*) atd;
		atd_bitmap_t	*bitmap, *nextbitmap;
		atd_frame_t		*frame, *nextframe;

		nextbitmap = anim->bitmaps;
		while (bitmap = nextbitmap)
		{
			nextbitmap = bitmap->listnext;
			if (bitmap->pixels) free(bitmap->pixels);
			free (bitmap);
		}
	}
	*/

	Hunk_Free2(atd);
}


/*
===============
ATD_Update
===============
*/
void ATD_Update (image_t *image)
{
	// You've got to be kidding
	if (!image->atd) 
		return;

	// Don't need to update it again this frame (note THIS FRAME)
	if (image->atd->last_time && image->atd->last_time == r_newrefdef.time)
		return;

	if (image->atd->type == atd_type_animation) 
	{
		atd_animation_t *atd = (atd_animation_t *) image->atd;

		// Requires a reset
		if (r_newrefdef.time < image->atd->last_time) 
		{
			atd_frame_t		*frame;

			for (frame = atd->frames; frame != NULL; frame = frame->listnext)
				frame->last_time = 0;

			atd->nextframe = atd->frames;
			atd->est_next = 0;
		}

		if (atd->nextframe) ATD_Update_Animation(image);
	}
	else if (image->atd->type == atd_type_interform) 
	{
		ATD_Update_Interform(image, (r_newrefdef.time<image->atd->last_time || image->atd->last_time == 0));
	}
	else if (image->atd->type == atd_type_whitenoise) 
	{
		ATD_Update_WhiteNoise(image);
	}

	image->atd->last_time = r_newrefdef.time;

}

