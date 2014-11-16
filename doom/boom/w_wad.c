/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _MSC_VER
#include <stddef.h>
#include <io.h>
#endif
#include <fcntl.h>


#ifdef NORMALUNIX
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloca.h>
#define O_BINARY		0
#endif

#include "../doomtype.h"
#include "../m_swap.h"
#include "../i_system.h"
#include "z_zone.h"

#include "../doomstat.h"
#include "../d_net.h"
#include "../doomtype.h"
#include "../i_system.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "../w_wad.h"

void**			lumpcache;
int			reloadlump;
char*			reloadname;
//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t *lumpinfo;
int        numlumps;         // killough

void ExtractFileBase (const char *path, char *dest)
{
  const char *src = path + strlen(path) - 1;
  int length;

  // back up until a \ or the start
  while (src != path && src[-1] != ':' // killough 3/22/98: allow c:filename
         && *(src-1) != '\\'
         && *(src-1) != '/')
  {
    src--;
  }

  // copy up to eight characters
  memset(dest,0,8);
  length = 0;

  while ((*src) && (*src != '.') && (++length<9))
  {
    *dest++ = toupper(*src);
    *src++;
  }
  /* cph - length check removed, just truncate at 8 chars.
   * If there are 8 or more chars, we'll copy 8, and no zero termination
   */
}

//
// 1/18/98 killough: adds a default extension to a path
// Note: Backslashes are treated specially, for MS-DOS.
//

char *AddDefaultExtension(char *path, const char *ext)
{
  char *p = path;
  while (*p++);
  while (p-->path && *p!='/' && *p!='\\')
    if (*p=='.')
      return path;
  if (*ext!='.')
    strcat(path,".");
  return strcat(path,ext);
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// Reload hack removed by Lee Killough
// CPhipps - source is an enum
//
// proff - changed using pointer to wadfile_info_t
static void W_AddFile(char *name)
// killough 1/31/98: static, const
{
  int         handle;
  wadinfo_t   header;
  lumpinfo_t* lump_p;
  unsigned    i;
  int         length;
  int         startlump;
  filelump_t  *fileinfo, *fileinfo2free=NULL; //killough
  filelump_t  singleinfo;

    // BEGIN from id Doom.
    // handle reload indicator.
    if (*name == '~')
    {
	name++;
	reloadname = name;
	reloadlump = numlumps;
    }
    // END from id Doom.


  // open the file and add to directory

  handle = open(name, O_RDONLY | O_BINARY);

#ifdef HAVE_NET
  if (handle == -1 && D_NetGetWad(name)) // CPhipps
    handle = open(name,O_RDONLY | O_BINARY);
#endif

  if (handle == -1)
    {
      if (  strlen(name)<=4 ||      // add error check -- killough
	         (strcasecmp(name+strlen(name)-4 , ".lmp" ) &&
	          strcasecmp(name+strlen(name)-4 , ".gwa" ) )
         )
	I_Error("W_AddFile: couldn't open %s",name);
      return;
    }

  //jff 8/3/98 use logical output routine
  printf("%s adding %s\n",  __func__, name);
  startlump = numlumps;

  if (strlen(name)<=4 ||
	      (
          strcasecmp(name+strlen(name)-4,".wad") &&
	        strcasecmp(name+strlen(name)-4,".gwa")
        )
     )
    {
      // single lump file
      fileinfo = &singleinfo;
      singleinfo.filepos = 0;
      singleinfo.size = lseek(handle, 0, SEEK_END);
      lseek(handle, 0, SEEK_SET);
      ExtractFileBase(name, singleinfo.name);
      numlumps++;
    }
  else
    {
      // WAD file
      read(handle, &header, sizeof(header));
      if (strncmp(header.identification,"IWAD",4) &&
          strncmp(header.identification,"PWAD",4))
        printf("W_AddFile: Wad file %s doesn't have IWAD or PWAD id", name);
      header.numlumps = LONG(header.numlumps);
      header.infotableofs = LONG(header.infotableofs);
      length = header.numlumps*sizeof(filelump_t);
      fileinfo2free = fileinfo = malloc(length);    // killough
      lseek(handle, header.infotableofs, SEEK_SET);
      read(handle, fileinfo, length);
      numlumps += header.numlumps;
    }

    // Fill in lumpinfo
    lumpinfo = realloc(lumpinfo, numlumps*sizeof(lumpinfo_t));

    lump_p = &lumpinfo[startlump];
/*
    char	name[255];
    int		handle;
    int		position;
    int		size;
*/
    for (i=startlump ; (int)i<numlumps ; i++,lump_p++, fileinfo++)
      {
        lump_p->position = LONG(fileinfo->filepos);
        lump_p->size = LONG(fileinfo->size);
        strncpy(lump_p->name, fileinfo->name, MAX_WAD_NAME_LENGTH);
      }

    free(fileinfo2free);      // killough
}

// jff 1/23/98 Create routines to reorder the master directory
// putting all flats into one marked block, and all sprites into another.
// This will allow loading of sprites and flats from a PWAD with no
// other changes to code, particularly fast hashes of the lumps.
//
// killough 1/24/98 modified routines to be a little faster and smaller

static int IsMarker(const char *marker, const char *name)
{
  return !strncasecmp(name, marker, MAX_WAD_NAME_LENGTH) ||
    (*name == *marker && !strncasecmp(name+1, marker, MAX_WAD_NAME_LENGTH-1));
}

// killough 4/17/98: add namespace tags

static void W_CoalesceMarkedResource(const char *start_marker,
                                     const char *end_marker, int li_namespace)
{
  lumpinfo_t *marked = malloc(sizeof(*marked) * numlumps);
  size_t i, num_marked = 0, num_unmarked = 0;
  int is_marked = 0, mark_end = 0;
  lumpinfo_t *lump = lumpinfo;

  for (i=numlumps; i--; lump++)
    if (IsMarker(start_marker, lump->name))       // start marker found
      { // If this is the first start marker, add start marker to marked lumps
        if (!num_marked)
          {
            strncpy(marked->name, start_marker, MAX_WAD_NAME_LENGTH);
            marked->size = 0;  // killough 3/20/98: force size to be 0
            num_marked = 1;
          }
        is_marked = 1;                            // start marking lumps
      }
    else
      if (IsMarker(end_marker, lump->name))       // end marker found
        {
          mark_end = 1;                           // add end marker below
          is_marked = 0;                          // stop marking lumps
        }
      else
        if (is_marked)                            // if we are marking lumps,
          {                                       // move lump to marked list
            marked[num_marked] = *lump;
          }
        else
          lumpinfo[num_unmarked++] = *lump;       // else move down THIS list

  // Append marked list to end of unmarked list
  memcpy(lumpinfo + num_unmarked, marked, num_marked * sizeof(*marked));

  free(marked);                                   // free marked list

  numlumps = num_unmarked + num_marked;           // new total number of lumps

  if (mark_end)                                   // add end marker
    {
      lumpinfo[numlumps].size = 0;  // killough 3/20/98: force size to be 0
      strncpy(lumpinfo[numlumps++].name, end_marker, MAX_WAD_NAME_LENGTH);
    }
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough

unsigned W_LumpNameHash(const char *s)
{
  unsigned hash;
  (void) ((hash =        toupper(s[0]), s[1]) &&
          (hash = hash*3+toupper(s[1]), s[2]) &&
          (hash = hash*2+toupper(s[2]), s[3]) &&
          (hash = hash*2+toupper(s[3]), s[4]) &&
          (hash = hash*2+toupper(s[4]), s[5]) &&
          (hash = hash*2+toupper(s[5]), s[6]) &&
          (hash = hash*2+toupper(s[6]),
           hash = hash*2+toupper(s[7]))
         );
  return hash;
}


int W_CheckNumForName (const char* name)
{
    union {
	char s[MAX_WAD_NAME_LENGTH+1];
	int	x[MAX_WAD_NAME_LENGTH/sizeof(int)];

    } name8;

    int		v1;
    int		v2;
    lumpinfo_t*	lump_p;

    // make the name into two integers for easy compares
    strncpy(name8.s,name,MAX_WAD_NAME_LENGTH);

    // in case the name was a fill 8 chars
    name8.s[MAX_WAD_NAME_LENGTH] = 0;

    // case insensitive

    for(int i = 0; i<MAX_WAD_NAME_LENGTH; i++){
        if(name8.s[i]=='\0')
          break;
        name8.s[i] = toupper(name8.s[i]);
    }

    v1 = name8.x[0];
    v2 = name8.x[1];


    // scan backwards so patch lump files take precedence
    lump_p = lumpinfo + numlumps;

    while (lump_p-- != lumpinfo)
    {
	if ( *(int *)lump_p->name == v1
	     && *(int *)&lump_p->name[4] == v2)
	{
	    return lump_p - lumpinfo;
	}
    }

    // TFB. Not found.
    return -1;
}




//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName (const char* name)
{
    int	i;

    i = W_CheckNumForName (name);

    if (i == -1)
      fprintf(stderr, "W_GetNumForName: %s not found!", name);

    return i;
}


// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump)
{
  if (lump >= numlumps)
    I_Error ("W_LumpLength: %i >= numlumps",lump);
  return lumpinfo[lump].size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//

void W_ReadLump(int lump, void *dest)
{
  lumpinfo_t *l = lumpinfo + lump;

#ifdef RANGECHECK
  if (lump >= numlumps)
    I_Error ("W_ReadLump: %i >= numlumps",lump);
#endif

    {
      if (l->handle)
      {
        lseek(l->handle, l->position, SEEK_SET);
        read(l->handle, dest, l->size);
      }
    }
}



//
// W_CacheLumpNum
//
void*
W_CacheLumpNum
( int		lump,
  int		tag )
{
    byte*	ptr;

    if ((unsigned)lump >= numlumps)
        fprintf(stderr, "WARNING W_CacheLumpNum: %i >= numlumps\n",lump);
//	I_Error ("W_CacheLumpNum: %i >= numlumps",lump);

    if (!lumpcache[lump])
    {
	// read the lump in

	//printf ("cache miss on lump %i\n",lump);
	ptr = Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
	W_ReadLump (lump, lumpcache[lump]);
    }
    else
    {
	//printf ("cache hit on lump %i\n",lump);
	Z_ChangeTag (lumpcache[lump],tag);
    }

    return lumpcache[lump];
}



//
// W_CacheLumpName
//
void*
W_CacheLumpName
( char*		name,
  int		tag )
{
    return W_CacheLumpNum (W_GetNumForName(name), tag);
}


//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
void W_InitMultipleFiles (char** filenames)
{
    int		size;

    // open all the files, load headers, and count lumps
    numlumps = 0;

    // will be realloced as lumps are added
    lumpinfo = malloc(1);

    for ( ; *filenames ; filenames++)
	W_AddFile (*filenames);

    if (!numlumps)
	I_Error ("W_InitFiles: no files found");

    // set up caching
    size = numlumps * sizeof(*lumpcache);
    lumpcache = malloc (size);

    if (!lumpcache)
	I_Error ("Couldn't allocate lumpcache");

    memset (lumpcache,0, size);
}

//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload (void)
{
    wadinfo_t		header;
    int			lumpcount;
    lumpinfo_t*		lump_p;
    unsigned		i;
    int			handle;
    int			length;
    filelump_t*		fileinfo;

    if (!reloadname)
	return;

    if ( (handle = open (reloadname,O_RDONLY | O_BINARY)) == -1)
	I_Error ("W_Reload: couldn't open %s",reloadname);

    read (handle, &header, sizeof(header));
    lumpcount = LONG(header.numlumps);
    header.infotableofs = LONG(header.infotableofs);
    length = lumpcount*sizeof(filelump_t);
    fileinfo = alloca (length);
    lseek (handle, header.infotableofs, SEEK_SET);
    read (handle, fileinfo, length);

    // Fill in lumpinfo
    lump_p = &lumpinfo[reloadlump];

    for (i=reloadlump ;
	 i<reloadlump+lumpcount ;
	 i++,lump_p++, fileinfo++)
    {
	if (lumpcache[i])
	    Z_Free (lumpcache[i]);

	lump_p->position = LONG(fileinfo->filepos);
	lump_p->size = LONG(fileinfo->size);
    }

    close (handle);
}




