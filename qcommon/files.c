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

#include "qcommon.h"

// define this to dissalow any data but the demo pak file
//#define	NO_ADDONS

// if a packfile directory differs from this, it is assumed to be hacked
// Full version
#define	PAK0_CHECKSUM	0x40e614e0
// Demo
//#define	PAK0_CHECKSUM	0xb2c6d7ea
// OEM
//#define	PAK0_CHECKSUM	0x78e135c

/*
=============================================================================

QUAKE FILESYSTEM

=============================================================================
*/
#include "zip/unzip.h"

//
// in memory
//

typedef struct
{
	char	name[MAX_QPATH];
	int		filepos, filelen, compressed;
} packfile_t;

typedef struct pack_s
{
	char	filename[MAX_OSPATH];	
	char	prefix[MAX_OSPATH];
	FILE	*handle;
	int		numfiles;
	packfile_t	*files;
	qboolean is_zip;
} pack_t;

char	fs_gamedir[MAX_OSPATH];
cvar_t	*fs_basedir;
cvar_t	*fs_cddir;
cvar_t	*fs_gamedirvar;

typedef struct filelink_s
{
	struct filelink_s	*next;
	char	*from;
	int		fromlength;
	char	*to;
} filelink_t;

filelink_t	*fs_links;

typedef struct searchpath_s
{
	char	filename[MAX_OSPATH];
	pack_t	*pack;		// only one of filename / pack will be used
	struct searchpath_s *next;
} searchpath_t;

searchpath_t	*fs_searchpaths;
searchpath_t	*fs_base_searchpaths;	// without gamedirs


/*

All of Quake's data access is through a hierchal file system, but the contents of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all game directories.  The sys_* files pass this to host_init in quakeparms_t->basedir.  This can be overridden with the "-basedir" command line parm to allow code debugging in a different directory.  The base directory is
only used during filesystem initialization.

The "game directory" is the first tree on the search path and directory that all generated files (savegames, screenshots, demos, config files) will be saved to.  This can be overridden with the "-game" command line parameter.  The game directory can never be changed while quake is executing.  This is a precacution against having a malicious server instruct clients to write files over areas they shouldn't.

*/


/*
================
FS_filelength
================
*/
int FS_filelength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}


/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
void	FS_CreatePath (char *path)
{
	char	*ofs;
	
	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
			Sys_Mkdir (path);
			*ofs = '/';
		}
	}
}


/*
==============
FS_FCloseFile

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FS_FCloseFile (FILE *f)
{
	fclose (f);
}


// RAFAEL
/*
	Developer_searchpath
*/
int	Developer_searchpath (int who)
{
	
	int		ch;
	// PMM - warning removal
//	char	*start;
	searchpath_t	*search;
	
	if (who == 1) // xatrix
		ch = 'x';
	else if (who == 2)
		ch = 'r';

	for (search = fs_searchpaths ; search ; search = search->next)
	{
		if (strstr (search->filename, "xatrix"))
			return 1;

		if (strstr (search->filename, "rogue"))
			return 2;
/*
		start = strchr (search->filename, ch);

		if (start == NULL)
			continue;

		if (strcmp (start ,"xatrix") == 0)
			return (1);
*/
	}
	return (0);

}


/*
===========
FS_FOpenFile

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
FILE *DecompressZIP(FILE *source, unzFile unzipfile, int insize,int outsize);
int file_from_pak = 0;
#ifndef NO_ADDONS
int FS_FOpenFile (char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	char			prefixedpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;
	filelink_t		*link;

	file_from_pak = 0;

	// check for links first
	for (link = fs_links ; link ; link=link->next)
	{
		if (!strncmp (filename, link->from, link->fromlength))
		{
			Com_sprintf (netpath, sizeof(netpath), "%s%s",link->to, filename+link->fromlength);
			//*file = fopen (netpath, "rb");
			//if (*file)
			{		
				Com_DPrintf ("link file: %s\n",netpath);
				filename = netpath;
				//return FS_filelength (*file);
			}
			return -1;
		}
	}

//
// search through the path, one element at a time
//
	Com_DPrintf("want to load:  %s\n", filename);
	for (search = fs_searchpaths ; search ; search = search->next)
	{
	// is the element a pak file?
		if (search->pack)
		{
		// look through all the pak file elements
			pak = search->pack;
			for (i = 0; i < pak->numfiles; i++)
			{
				strcpy_s(prefixedpath, sizeof(netpath), pak->prefix);
				strcat_s(prefixedpath, sizeof(netpath), pak->files[i].name);

				if (!Q_strfncmp(prefixedpath, filename) || !Q_strfncmp(pak->files[i].name, filename))
				{	// found it!
					file_from_pak = 1;

					// open a new file on the pakfile
					*file = fopen(pak->filename, "rb");
					if (!*file)
						Com_Error(ERR_FATAL, "Couldn't reopen %s", pak->filename);
					if (pak->is_zip) {

						zlib_filefunc_def filefuncs;
						qfill_fopen_filefunc(&filefuncs, *file);

						unzFile unzfile = unzOpen2("", &filefuncs);
						if (unzLocateFile(unzfile, pak->files[i].name, 1) != UNZ_OK) return -1;

						unz_file_info info;

						if (unzOpenCurrentFile(unzfile) != UNZ_OK) return -1;

						if (unzGetCurrentFileInfo(unzfile, &info, 0, 0, 0, 0, 0, 0) != UNZ_OK) return -1;
						*file = DecompressZIP(*file, unzfile, info.compressed_size, info.uncompressed_size);
						return info.uncompressed_size;
					}
					else
					{
						fseek(*file, pak->files[i].filepos, SEEK_SET);
						if (pak->files[i].compressed) {
							*file = DecompressANOXDATA(*file, pak->files[i].compressed, pak->files[i].filelen);
						}
					}

					return pak->files[i].filelen;
				}
			}
		}
		else
		{		
	// check a file in the directory tree
			
			Com_sprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
			
			*file = fopen (netpath, "rb");
			if (!*file)
				continue;
			
			Com_DPrintf ("FindFile: %s\n",netpath);

			return FS_filelength (*file);
		}
		
	}
	
	Com_DPrintf ("FindFile: can't find %s\n", filename);
	
	*file = NULL;
	return -1;
}

#else

// this is just for demos to prevent add on hacking

int FS_FOpenFile (char *filename, FILE **file)
{
	searchpath_t	*search;
	char			netpath[MAX_OSPATH];
	pack_t			*pak;
	int				i;

	file_from_pak = 0;

	// get config from directory, everything else from pak
	if (!strcmp(filename, "config.cfg") || !strncmp(filename, "players/", 8))
	{
		Com_sprintf (netpath, sizeof(netpath), "%s/%s",FS_Gamedir(), filename);
		
		*file = fopen (netpath, "rb");
		if (!*file)
			return -1;
		
		Com_DPrintf ("FindFile: %s\n",netpath);

		return FS_filelength (*file);
	}

	for (search = fs_searchpaths ; search ; search = search->next)
		if (search->pack)
			break;
	if (!search)
	{
		*file = NULL;
		return -1;
	}

	pak = search->pack;
	for (i=0 ; i<pak->numfiles ; i++)
		if (!Q_strcasecmp (pak->files[i].name, filename))
		{	// found it!
			file_from_pak = 1;
			Com_DPrintf ("PackFile: %s : %s\n",pak->filename, filename);
		// open a new file on the pakfile
			*file = fopen (pak->filename, "rb");
			if (!*file)
				Com_Error (ERR_FATAL, "Couldn't reopen %s", pak->filename);	
			fseek (*file, pak->files[i].filepos, SEEK_SET);
			return pak->files[i].filelen;
		}
	
	Com_DPrintf ("FindFile: can't find %s\n", filename);
	
	*file = NULL;
	return -1;
}

#endif


/*
=================
FS_ReadFile

Properly handles partial reads
=================
*/
void CDAudio_Stop(void);
#define	MAX_READ	0x100000		// read in blocks of 64k
void FS_Read (void *buffer, int len, FILE *f)
{
	size_t		block, remaining;
	size_t		read;
	byte	*buf;
	int		tries;

	buf = (byte *)buffer;

	// read in chunks for progress bar
	remaining = len;
	tries = 0;
	while (remaining)
	{
		block = remaining;
		if (block > MAX_READ)
			block = MAX_READ;
		read = fread (buf, 1, block, f);
		if (read == 0)
		{
			// we might have been trying to read from a CD
			if (!tries)
			{
				tries = 1;
				CDAudio_Stop();
			}
			else {
				Com_Printf( "FS_Read: 0 bytes read");
				return;
			}
		}

		if (read == -1) {
			Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
			return;
		}

		// do some progress bar thing here...

		remaining -= read;
		buf += read;
	}

}

/*
============
FS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
int FS_LoadFile (char *path, void **buffer)
{
	FILE	*h;
	byte	*buf;
	int		len;

	buf = NULL;	// quiet compiler warning

// look for it in the filesystem or pack files
	len = FS_FOpenFile (path, &h);
	if (!h)
	{
		if (buffer)
			*buffer = NULL;
		return -1;
	}
	
	if (!buffer)
	{
		fclose (h);
		return len;
	}

	buf = Z_Malloc(len);
	*buffer = buf;

	FS_Read (buf, len, h);

	fclose (h);

	return len;
}


/*
=============
FS_FreeFile
=============
*/
void FS_FreeFile (void *buffer)
{
	Z_Free (buffer);
}

/*
=================
FS_LoadPackFile

Takes an explicit (not game tree related) path to a pak file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
pack_t *FS_LoadPackFile (char *packfile)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	dpackfile_t		info[MAX_FILES_IN_PACK];
	unsigned		checksum;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	fread (&header, 1, sizeof(header), packhandle);
	if (LittleLong(header.ident) != IDPAKHEADER)
		Com_Error (ERR_FATAL, "%s is not a packfile", packfile);
	header.dirofs = LittleLong (header.dirofs);
	header.dirlen = LittleLong (header.dirlen);

	numpackfiles = header.dirlen / sizeof(dpackfile_t);

	if (numpackfiles > MAX_FILES_IN_PACK)
		Com_Error (ERR_FATAL, "%s has %i files", packfile, numpackfiles);

	newfiles = Z_Malloc (numpackfiles * sizeof(packfile_t));

	fseek (packhandle, header.dirofs, SEEK_SET);
	fread (info, 1, header.dirlen, packhandle);

// crc the directory to check for modifications
	checksum = Com_BlockChecksum ((void *)info, header.dirlen);

#ifdef NO_ADDONS
	if (checksum != PAK0_CHECKSUM)
		return NULL;
#endif
// parse the directory
	for (i=0 ; i<numpackfiles ; i++)
	{
		strcpy (newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
		newfiles[i].compressed = 0;
		Com_Printf("added: %s\n", newfiles[i].name);
	}

	pack = Z_Malloc (sizeof (pack_t));
	strcpy (pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	pack->is_zip = false;
	pack->prefix[0] = 0;

	Com_Printf ("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}

pack_t *FS_LoadDATFile(char *packfile)
{
	dpackheader_t	header;
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;
	adatfile_t		*info;
	unsigned		checksum;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	fread(&header, 1, sizeof(header), packhandle);
	if (LittleLong(header.ident) != ADATHEADER)
		Com_Error(ERR_FATAL, "%s is not a datfile", packfile);
	header.dirofs = LittleLong(header.dirofs);
	header.dirlen = LittleLong(header.dirlen);

	numpackfiles = header.dirlen / sizeof(adatfile_t);

	info = Z_Malloc(numpackfiles * sizeof(adatfile_t));
	newfiles = Z_Malloc(numpackfiles * sizeof(packfile_t));

	fseek(packhandle, header.dirofs, SEEK_SET);
	fread(info, 1, header.dirlen, packhandle);

	// crc the directory to check for modifications
	checksum = Com_BlockChecksum((void *)info, header.dirlen);

#ifdef NO_ADDONS
	if (checksum != PAK0_CHECKSUM)
		return NULL;
#endif
	// parse the directory
	pack = Z_Malloc(sizeof(pack_t));
	{


		for (i = 0; i < numpackfiles; i++)
		{
			int limit;
			char *bsl;
			info[i].name[127] = 0;
			strcpy_s(newfiles[i].name, sizeof(newfiles[i].name), info[i].name);
			newfiles[i].filepos = LittleLong(info[i].filepos);
			newfiles[i].filelen = LittleLong(info[i].uncompressed);
			newfiles[i].compressed = LittleLong(info[i].compressed);
			if (i < (numpackfiles - 1)) limit = info[i + 1].filepos;
			else limit = header.dirofs;
			if ((info[i].filepos + info[i].compressed) > limit) {
				Com_Printf("packfile %s file %i %s failed to read length correctly %X is too long should be %X\n", packfile, i, info[i].name, info[i].compressed, limit - newfiles[i].filepos);
			}
			while (bsl = strchr(newfiles[i].name, '\\'))
				*bsl = '/';
			//Com_Printf("added: %s\n", newfiles[i].name);
		}
	}

	strcpy_s(pack->filename, sizeof(pack->filename), packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	pack->is_zip = false;
	char *slash = strrchr(packfile, '/');
	char *bslash = strrchr(packfile, '\\');
	if (bslash > slash)
		slash = bslash;

	strcpy_s(pack->prefix, sizeof(pack->prefix), slash + 1);

	char*dot = strchr(pack->prefix, '.');
	*dot = 0;
	strcat_s(pack->prefix, sizeof(pack->prefix), "/");

	Com_Printf("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}
pack_t *FS_LoadZIPFile(char *packfile)
{
	int				i;
	packfile_t		*newfiles;
	int				numpackfiles;
	pack_t			*pack;
	FILE			*packhandle;	
	unsigned		checksum;
	zlib_filefunc_def filefuncs;
	unzFile unzfile;
	unz_global_info global_info;

	packhandle = fopen(packfile, "rb");
	if (!packhandle)
		return NULL;

	qfill_fopen_filefunc(&filefuncs, packhandle);

	unzfile = unzOpen2("", &filefuncs);

	if (!unzfile) 
		{
		Com_Error(ERR_FATAL, "%s is not a zipfile", packfile);
		unzClose(unzfile);
		return NULL;
	}
	unzGetGlobalInfo(unzfile, &global_info);
	numpackfiles = global_info.number_entry;

	newfiles = Z_Malloc(numpackfiles * sizeof(packfile_t));
	memset(newfiles, 0, numpackfiles * sizeof(packfile_t));

	// parse the directory
	{
		i = 0;
		for (unzGoToFirstFile(unzfile); unzGoToNextFile(unzfile) == UNZ_OK;i++)
		{
			char fn[256];
			unz_file_info file_info;		
			unzGetCurrentFileInfo(unzfile, &file_info, newfiles[i].name, sizeof(newfiles[i].name), 0, 0, 0, 0);
			int limit;
			char*dot, *bsl;
			newfiles[i].filepos = 0;
			newfiles[i].filelen = file_info.uncompressed_size;
			newfiles[i].compressed = file_info.compressed_size;
			while (bsl = strchr(newfiles[i].name, '\\'))
				*bsl = '/';
			Com_Printf("added: %s\n", newfiles[i].name);
		}
	}

	pack = Z_Malloc(sizeof(pack_t));
	strcpy_s(pack->filename, sizeof(pack->filename), packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	pack->is_zip = true;
	char *slash = strrchr(packfile, '/');
	char *bslash = strrchr(packfile, '\\');
	if (bslash > slash)
		slash = bslash;

	strcpy_s(pack->prefix,sizeof(pack->prefix), slash + 1);

	char*dot = strchr(pack->prefix, '.');
	*dot = 0;
	strcat_s(pack->prefix, sizeof(pack->prefix), "/");

	Com_Printf("Added packfile %s (%i files)\n", packfile, numpackfiles);
	return pack;
}
extern char **FS_ListFiles(char *, int *, unsigned, unsigned);
/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads and adds pak1.pak pak2.pak ... 
================
*/
void FS_AddGameDirectory (char *dir)
{
	int				i;
	searchpath_t	*search;
	pack_t			*pak;
	
		char	**dirnames;
	int ndirs;
	char			pakfile[MAX_OSPATH];

	strcpy (fs_gamedir, dir);

	//
	// add the directory to the search path
	//
	search = Z_Malloc (sizeof(searchpath_t));
	strcpy (search->filename, dir);
	search->next = fs_searchpaths;
	fs_searchpaths = search;

	//
	// add any pak files in the format pak0.pak pak1.pak, ...
	//
	for (i=0; i<10; i++)
	{
		Com_sprintf (pakfile, sizeof(pakfile), "%s/pak%i.pak", dir, i);
		pak = FS_LoadPackFile (pakfile);
		if (!pak)
			continue;
		search = Z_Malloc (sizeof(searchpath_t));
		search->pack = pak;
		search->next = fs_searchpaths;
		fs_searchpaths = search;		
	}

	Com_sprintf(pakfile, sizeof(pakfile), "%s/*.dat", dir, i);
	if ((dirnames = FS_ListFiles(pakfile, &ndirs, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM)) != 0)
	{
		int i;

		for (i = 0; i < ndirs - 1; i++)
		{
			//if (strrchr(dirnames[i], '/'))
				//Com_Printf("%s\n", strrchr(dirnames[i], '/') + 1);
			//else
			Com_Printf("%s\n", dirnames[i]);


			pak = FS_LoadDATFile(dirnames[i]);
			if (!pak)
				continue;
			search = Z_Malloc(sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;

			free(dirnames[i]);
		}
		free(dirnames);
	}
	Com_Printf("\n");	
	Com_sprintf(pakfile, sizeof(pakfile), "%s/*.zip", dir, i);
	if ((dirnames = FS_ListFiles(pakfile, &ndirs, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM)) != 0)
	{
		int i;

		for (i = 0; i < ndirs - 1; i++)
		{
			Com_Printf("%s\n", dirnames[i]);


			pak = FS_LoadZIPFile(dirnames[i]);
			if (!pak)
				continue;
			search = Z_Malloc(sizeof(searchpath_t));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;

			free(dirnames[i]);
		}
		free(dirnames);
	}
	Com_Printf("\n");

}

/*
============
FS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
char *FS_Gamedir (void)
{
	if (*fs_gamedir)
		return fs_gamedir;
	else
		return BASEDIRNAME;
}

/*
=============
FS_ExecAutoexec
=============
*/
void FS_ExecAutoexec (void)
{
	char *dir;
	char name [MAX_QPATH];

	dir = Cvar_VariableString("gamedir");
	if (*dir)
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, dir); 
	else
		Com_sprintf(name, sizeof(name), "%s/%s/autoexec.cfg", fs_basedir->string, BASEDIRNAME); 
	if (Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM))
		Cbuf_AddText ("exec autoexec.cfg\n");
	Sys_FindClose();
}


/*
================
FS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void FS_SetGamedir (char *dir)
{
	searchpath_t	*next;

	if (strstr(dir, "..") || strstr(dir, "/")
		|| strstr(dir, "\\") || strstr(dir, ":") )
	{
		Com_Printf ("Gamedir should be a single filename, not a path\n");
		return;
	}

	//
	// free up any current game dir info
	//
	while (fs_searchpaths != fs_base_searchpaths)
	{
		if (fs_searchpaths->pack)
		{
			fclose (fs_searchpaths->pack->handle);
			Z_Free (fs_searchpaths->pack->files);
			Z_Free (fs_searchpaths->pack);
		}
		next = fs_searchpaths->next;
		Z_Free (fs_searchpaths);
		fs_searchpaths = next;
	}

	//
	// flush all data, so it will be forced to reload
	//
	if (dedicated && !dedicated->value)
		Cbuf_AddText ("vid_restart\nsnd_restart\n");

	Com_sprintf (fs_gamedir, sizeof(fs_gamedir), "%s/%s", fs_basedir->string, dir);

	if (!strcmp(dir,BASEDIRNAME) || (*dir == 0))
	{
		Cvar_FullSet ("gamedir", "", CVAR_SERVERINFO|CVAR_NOSET);
		Cvar_FullSet ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	}
	else
	{
		Cvar_FullSet ("gamedir", dir, CVAR_SERVERINFO|CVAR_NOSET);
		if (fs_cddir->string[0])
			FS_AddGameDirectory (va("%s/%s", fs_cddir->string, dir) );
		FS_AddGameDirectory (va("%s/%s", fs_basedir->string, dir) );
	}
}


/*
================
FS_Link_f

Creates a filelink_t
================
*/
void FS_Link_f (void)
{
	filelink_t	*l, **prev;

	if (Cmd_Argc() != 3)
	{
		Com_Printf ("USAGE: link <from> <to>\n");
		return;
	}

	// see if the link already exists
	prev = &fs_links;
	for (l=fs_links ; l ; l=l->next)
	{
		if (!strcmp (l->from, Cmd_Argv(1)))
		{
			Z_Free (l->to);
			if (!strlen(Cmd_Argv(2)))
			{	// delete it
				*prev = l->next;
				Z_Free (l->from);
				Z_Free (l);
				return;
			}
			l->to = CopyString (Cmd_Argv(2));
			return;
		}
		prev = &l->next;
	}

	// create a new link
	l = Z_Malloc(sizeof(*l));
	l->next = fs_links;
	fs_links = l;
	l->from = CopyString(Cmd_Argv(1));
	l->fromlength =(int) strlen(l->from);
	l->to = CopyString(Cmd_Argv(2));
}

bool wildcardmatch(char const *needle, char const *haystack) {
	for (; *needle != '\0'; ++needle) {
		switch (*needle) {
		case '?':
			if (*haystack == '\0')
				return false;
			++haystack;
			break;
		case '*': {
			if (needle[1] == '\0')
				return true;
			size_t max = strlen(haystack);
			for (size_t i = 0; i < max; i++)
				if (wildcardmatch(needle + 1, haystack + i))
					return true;
			return false;
		}
		default:
			if (*haystack != *needle)
				return false;
			++haystack;
		}
	}
	return *haystack == '\0';
}

/*
** FS_ListFiles
*/
char **FS_ListFilesAll(char *findname, int *numfiles, unsigned musthave, unsigned canthave)
{
	searchpath_t *search;
	char *s;
	int nfiles = 0;
	char **list = 0;



	for (search = fs_searchpaths; search; search = search->next)
	{
		// is the element a pak file?
		if (search->pack)
		{
			// look through all the pak file elements
			pack_t* pak = search->pack;
			for (int i = 0; i < pak->numfiles; i++)
				if (wildcardmatch(findname, pak->files[i].name))
					nfiles++;
		}
	}
	nfiles++; // add space for a guard
	*numfiles = nfiles;

	if (!nfiles)
		return NULL;

	list = malloc(sizeof(char *) * nfiles);
	memset(list, 0, sizeof(char *) * nfiles);

	nfiles = 0;
	for (search = fs_searchpaths; search; search = search->next)
	{
		// is the element a pak file?
		if (search->pack)
		{
			// look through all the pak file elements
			pack_t* pak = search->pack;
			for (int i = 0; i < pak->numfiles; i++)
				if (wildcardmatch(findname, pak->files[i].name) )
				{
					list[nfiles] = strdup(pak->files[i].name);
					nfiles++;
				}
		
		}
	}
	return list;
}
char **FS_ListFiles(char *findname, int *numfiles, unsigned musthave, unsigned canthave)
{
	char *s;
	int nfiles = 0;
	char **list = 0;

	s = Sys_FindFirst(findname, musthave, canthave);
	while (s)
	{
		if (s[strlen(s) - 1] != '.')
			nfiles++;
		s = Sys_FindNext(musthave, canthave);
	}
	Sys_FindClose();

	if (!nfiles)
		return NULL;

	nfiles++; // add space for a guard
	*numfiles = nfiles;

	list = malloc(sizeof(char *) * nfiles);
	memset(list, 0, sizeof(char *) * nfiles);

	s = Sys_FindFirst(findname, musthave, canthave);
	nfiles = 0;
	while (s)
	{
		if (s[strlen(s) - 1] != '.')
		{
			list[nfiles] = strdup(s);
#ifdef _WIN32
			strlwr(list[nfiles]);
#endif
			nfiles++;
		}
		s = Sys_FindNext(musthave, canthave);
	}
	Sys_FindClose();

	return list;
}
extern char **FS_ListFiles(char *, int *, unsigned, unsigned);
/*
** FS_Dir_f
*/
void FS_Dir_f( void )
{
	char	*path = "";
	char	findname[1024];
	char	wildcard[1024] = "*.*";
	char	**dirnames;
	int		ndirs;

	if ( Cmd_Argc() != 1 )
	{
		strcpy( wildcard, Cmd_Argv( 1 ) );
	}

	//while ( ( path = FS_NextPath( path ) ) != NULL )
	{
		char *tmp = findname;

		Com_sprintf( findname, sizeof(findname), "%s",  wildcard );

		while ( *tmp != 0 )
		{
			if ( *tmp == '\\' ) 
				*tmp = '/';
			tmp++;
		}
		Com_Printf( "Directory of %s\n", findname );
		Com_Printf( "----\n" );

		if ( ( dirnames = FS_ListFilesAll( findname, &ndirs, 0, 0 ) ) != 0 )
		{
			int i;

			for ( i = 0; i < ndirs-1; i++ )
			{
				if ( strrchr( dirnames[i], '/' ) )
					Com_Printf( "%s\n", strrchr( dirnames[i], '/' ) + 1 );
				else
					Com_Printf( "%s\n", dirnames[i] );

				free( dirnames[i] );
			}
			free( dirnames );
		}
		Com_Printf( "\n" );
	};
}

/*
============
FS_Path_f

============
*/
void FS_Path_f (void)
{
	searchpath_t	*s;
	filelink_t		*l;

	Com_Printf ("Current search path:\n");
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s == fs_base_searchpaths)
			Com_Printf ("----------\n");
		if (s->pack)
			Com_Printf ("%s (%i files)\n", s->pack->filename, s->pack->numfiles);
		else
			Com_Printf ("%s\n", s->filename);
	}

	Com_Printf ("\nLinks:\n");
	for (l=fs_links ; l ; l=l->next)
		Com_Printf ("%s : %s\n", l->from, l->to);
}

/*
================
FS_NextPath

Allows enumerating all of the directories in the search path
================
*/
char *FS_NextPath (char *prevpath)
{
	searchpath_t	*s;
	char			*prev;

	if (!prevpath)
		return fs_gamedir;

	prev = fs_gamedir;
	for (s=fs_searchpaths ; s ; s=s->next)
	{
		if (s->pack)
			continue;
		if (prevpath == prev)
			return s->filename;
		prev = s->filename;
	}

	return NULL;
}


/*
================
FS_InitFilesystem
================
*/
void FS_InitFilesystem (void)
{
	Cmd_AddCommand ("path", FS_Path_f);
	Cmd_AddCommand ("link", FS_Link_f);
	Cmd_AddCommand ("dir", FS_Dir_f );

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
	fs_basedir = Cvar_Get ("basedir", ".", CVAR_NOSET);

	//
	// cddir <path>
	// Logically concatenates the cddir after the basedir for 
	// allows the game to run from outside the data tree
	//
	fs_cddir = Cvar_Get ("cddir", "", CVAR_NOSET);
	if (fs_cddir->string[0])
		FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_cddir->string) );

	//
	// start up with baseq2 by default
	//
	FS_AddGameDirectory (va("%s/"BASEDIRNAME, fs_basedir->string) );

	FS_AddGameDirectory(va("%s/anoxdata", fs_basedir->string) );

	// any set gamedirs will be freed up to here
	fs_base_searchpaths = fs_searchpaths;

	// check for game override
	fs_gamedirvar = Cvar_Get ("game", "", CVAR_LATCH|CVAR_SERVERINFO);
	if (fs_gamedirvar->string[0])
		FS_SetGamedir (fs_gamedirvar->string);
}



