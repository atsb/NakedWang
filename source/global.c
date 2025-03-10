//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
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

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef __BEOS__
#include <endian.h>
#endif

#ifdef __sun
#include <sys/byteorder.h>
#endif

#include "mytypes.h"
#include "shadow.h"
#include "common.h"
#include "util_lib.h"

static char ApogeePath[256];

// portability stuff.  --ryan.
// A good portion of this was ripped from GPL'd Rise of the Triad.  --ryan.

void FixFilePath(char *filename)
{
#if defined(PLATFORM_UNIX) && !defined(DC)
    char *ptr;
    char *lastsep = filename;

    if ((!filename) || (*filename == '\0'))
        return;

    if (access(filename, F_OK) == 0)  /* File exists; we're good to go. */
        return;

    for (ptr = filename; 1; ptr++)
    {
        if (*ptr == '\\')
            *ptr = PATH_SEP_CHAR;

        if ((*ptr == PATH_SEP_CHAR) || (*ptr == '\0'))
        {
            char pch = *ptr;
            struct dirent *dent = NULL;
            DIR *dir;

            if ((pch == PATH_SEP_CHAR) && (*(ptr + 1) == '\0'))
                return; /* eos is pathsep; we're done. */

            if (lastsep == ptr)
                continue;  /* absolute path; skip to next one. */

            *ptr = '\0';
            if (lastsep == filename) {
                dir = opendir((*lastsep == PATH_SEP_CHAR) ? ROOTDIR : CURDIR);
                
                if (*lastsep == PATH_SEP_CHAR) {
                    lastsep++;
                }
            } 
            else
            {
                *lastsep = '\0';
                dir = opendir(filename);
                *lastsep = PATH_SEP_CHAR;
                lastsep++;
            }

            if (dir == NULL)
            {
                *ptr = PATH_SEP_CHAR;
                return;  /* maybe dir doesn't exist? give up. */
            }

            while ((dent = readdir(dir)) != NULL)
            {
                if (strcasecmp(dent->d_name, lastsep) == 0)
                {
                    /* found match; replace it. */
                    strcpy(lastsep, dent->d_name);
                    break;
                }
            }

            closedir(dir);
            *ptr = pch;
            lastsep = ptr;

            if (dent == NULL)
                return;  /* no match. oh well. */

            if (pch == '\0')  /* eos? */
                return;
        }
    }
#endif
}


#if PLATFORM_DOS
 /* no-op. */

#elif PLATFORM_WIN32
int _dos_findfirst(char *filename, int x, struct find_t *f)
{
    long rc = _findfirst(filename, &f->data);
    f->handle = rc;
    if (rc != -1)
    {
        strncpy(f->name, f->data.name, sizeof (f->name) - 1);
        f->name[sizeof (f->name) - 1] = '\0';
        return(0);
    }
    return(1);
}

int _dos_findnext(struct find_t *f)
{
    int rc = 0;
    if (f->handle == -1)
        return(1);   /* invalid handle. */

    rc = _findnext(f->handle, &f->data);
    if (rc == -1)
    {
        _findclose(f->handle);
        f->handle = -1;
        return(1);
    }

    strncpy(f->name, f->data.name, sizeof (f->name) - 1);
    f->name[sizeof (f->name) - 1] = '\0';
    return(0);
}

#elif PLATFORM_UNIX 
int _dos_findfirst(char *filename, int x, struct find_t *f)
{
    char *ptr;

    if (strlen(filename) >= sizeof (f->pattern))
        return(1);

    strcpy(f->pattern, filename);
    FixFilePath(f->pattern);
    ptr = strrchr(f->pattern, PATH_SEP_CHAR);

    if (ptr == NULL)
    {
        ptr = filename;
        f->dir = opendir(CURDIR);
    }
    else
    {
        *ptr = '\0';
        f->dir = opendir(f->pattern);
        memmove(f->pattern, ptr + 1, strlen(ptr + 1) + 1);
    }

    return(_dos_findnext(f));
}


static int check_pattern_nocase(const char *x, const char *y)
{
    if ((x == NULL) || (y == NULL))
        return(0);  /* not a match. */

    while ((*x) && (*y))
    {
        if (*x == '*')
        {
            x++;
            while (*y != '\0')
            {
                if (toupper((int) *x) == toupper((int) *y))
                    break;
                y++;
            }
        }

        else if (*x == '?')
        {
            if (*y == '\0')
                return(0);  /* anything but EOS is okay. */
        }

        else
        {
            if (toupper((int) *x) != toupper((int) *y))
                return(0);  /* not a match. */
        }

        x++;
        y++;
    }

    return(*x == *y);  /* it's a match (both should be EOS). */
}

int _dos_findnext(struct find_t *f)
{
    struct dirent *dent;

    if (f->dir == NULL)
        return(1);  /* no such dir or we're just done searching. */

    while ((dent = readdir(f->dir)) != NULL)
    {
        if (check_pattern_nocase(f->pattern, dent->d_name))
        {
            if (strlen(dent->d_name) < sizeof (f->name))
            {
                strcpy(f->name, dent->d_name);
                return(0);  /* match. */
            }
        }
    }

    closedir(f->dir);
    f->dir = NULL;
    return(1);  /* no match in whole directory. */
}
#else
#error please define for your platform.
#endif


#if !PLATFORM_DOS
void _dos_getdate(struct dosdate_t *date)
{
	time_t curtime = time(NULL);
	struct tm *tm;
	
	if (date == NULL) {
		return;
	}
	
	memset(date, 0, sizeof(struct dosdate_t));
	
	if ((tm = localtime(&curtime)) != NULL) {
		date->day = tm->tm_mday;
		date->month = tm->tm_mon + 1;
		date->year = tm->tm_year + 1900;
		date->dayofweek = tm->tm_wday + 1;
	}
}
#endif


int FindDistance2D(int ix, int iy)
{
  int   t;

  ix= abs(ix);        /* absolute values */
  iy= abs(iy);

  if (ix<iy)
  {
     int tmp = ix;
     ix = iy;
     iy = tmp;
  }

  t = iy + (iy>>1);

  return (ix - (ix>>5) - (ix>>7)  + (t>>2) + (t>>6));
}

int FindDistance3D(int ix, int iy, int iz)
{
   int   t;

   ix= abs(ix);           /* absolute values */
   iy= abs(iy);
   iz= abs(iz);

   if (ix<iy)
   {
     int tmp = ix;
     ix = iy;
     iy = tmp;
   }

   if (ix<iz)
   {
     int tmp = ix;
     ix = iz;
     iz = tmp;
   }

   t = iy + iz;

   return (ix - (ix>>4) + (t>>2) + (t>>3));
}

void Error (char *error, ...)
{
   char msgbuf[300];
   va_list argptr;
   static int inerror = 0;

   inerror++;
   if (inerror > 1)
      return;

   #if USE_SDL
   SDL_Quit();
   #endif

   va_start (argptr, error);
   vfprintf(stderr, error, argptr);
   va_end (argptr);

   exit (1);
}


int32 SafeOpenAppend (const char *_filename, int32 filetype)
{
	int	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_APPEND
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening for append %s: %s",filename,strerror(errno));

	return handle;
}

boolean SafeFileExists ( const char * _filename )
{
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

    return(access(filename, F_OK) == 0);
}


int32 SafeOpenWrite (const char *_filename, int32 filetype)
{
	int	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}

int32 SafeOpenRead (const char *_filename, int32 filetype)
{
	int	handle;
    char filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	handle = open(filename,O_RDONLY | O_BINARY);

	if (handle == -1)
		Error ("Error opening %s: %s",filename,strerror(errno));

	return handle;
}


void SafeRead (int32 handle, void *buffer, int32 count)
{
        unsigned iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (read (handle,buffer,iocount) != (int)iocount)
			Error ("File read failure reading %ld bytes",count);
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
}


void SafeWrite (int32 handle, void *buffer, int32 count)
{
	unsigned	iocount;

	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,iocount) != (int)iocount)
			Error ("File write failure writing %ld bytes",count);
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
}


void GetPathFromEnvironment( char *fullname, int32 length, const char *filename )
{
	snprintf(fullname, length-1, "%s%s", ApogeePath, filename);
}

void SafeWriteString (int handle, char * buffer)
{
	unsigned	iocount;

   iocount=strlen(buffer);
	if (write (handle,buffer,iocount) != (int)iocount)
			Error ("File write string failure writing %s\n",buffer);
}

void *SafeMalloc (long size)
{
	void *ptr;

#if 0
   if (zonememorystarted==false)
      Error("Called SafeMalloc without starting zone memory\n");
	ptr = Z_Malloc (size,PU_STATIC,NULL);
#else
    ptr = malloc(size);
#endif

	if (!ptr)
      Error ("SafeMalloc failure for %lu bytes",size);

	return ptr;
}

void SafeRealloc (void **x, int32 size)
{
	void *ptr;

#if 0
   if (zonememorystarted==false)
      Error("Called SafeMalloc without starting zone memory\n");
	ptr = Z_Malloc (size,PU_STATIC,NULL);
#else
    ptr = realloc(*x, size);
#endif

	if (!ptr)
      Error ("SafeRealloc failure for %lu bytes",size);

    *x = ptr;
}

void *SafeLevelMalloc (long size)
{
	void *ptr;

#if 0
   if (zonememorystarted==false)
      Error("Called SafeLevelMalloc without starting zone memory\n");
   ptr = Z_LevelMalloc (size,PU_STATIC,NULL);
#else
    ptr = malloc(size);
#endif

	if (!ptr)
      Error ("SafeLevelMalloc failure for %lu bytes",size);

	return ptr;
}

void SafeFree (void * ptr)
{
   if ( ptr == NULL )
      Error ("SafeFree : Tried to free a freed pointer\n");

#if 0
	Z_Free (ptr);
#else
    free(ptr);
#endif
}


#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#if PLATFORM_DOS
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#ifdef PLATFORM_WIN32
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#ifdef _LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN)
#define BYTE_ORDER BIG_ENDIAN
#endif

#ifndef BYTE_ORDER
#error Please define your platform.
#endif

#if (BYTE_ORDER == LITTLE_ENDIAN)
#define KeepShort IntelShort
#define SwapShort MotoShort
#define KeepLong IntelLong
#define SwapLong MotoLong
#else
#define KeepShort MotoShort
#define SwapShort IntelShort
#define KeepLong MotoLong
#define SwapLong IntelLong
#endif

short	SwapShort (short l)
{
	byte	b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	KeepShort (short l)
{
	return l;
}


long	SwapLong (long l)
{
	byte	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((long)b1<<24) + ((long)b2<<16) + ((long)b3<<8) + b4;
}

long	KeepLong (long l)
{
	return l;
}


#undef KeepShort
#undef KeepLong
#undef SwapShort
#undef SwapLong

void SwapIntelLong(long *l)
{
    *l = IntelLong(*l);
}

void SwapIntelShort(short *s)
{
    *s = IntelShort(*s);
}

void SwapIntelLongArray(long *l, int num)
{
    while (num--) {
        SwapIntelLong(l);
        l++;
    }
}

void SwapIntelShortArray(short *s, int num)
{
    while (num--) {
        SwapIntelShort(s);
        s++;
    }
}


/* 
  Copied over from Wolf3D Linux: http://www.icculus.org/wolf3d/
  Modified for ROTT.
  Stolen for Duke3D, too.
 */
 
#if PLATFORM_UNIX
char *strlwr(char *s)
{
	char *p = s;
	
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	
	return s;
}

char *strupr(char *s)
{
	char *p = s;
	
	while (*p) {
		*p = toupper(*p);
		p++;
	}
	
	return s;
}
	
char *itoa(int value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%d", value);
			break;
		case 16:
			sprintf(string, "%x", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}

char *ltoa(long value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%ld", value);
			break;
		case 16:
			sprintf(string, "%lx", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%lu", value);
			break;
		case 16:
			sprintf(string, "%lux", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}
#endif


int setup_homedir (void)
{
#ifdef DC
	strcpy (ApogeePath, "/ram/");
#elif PLATFORM_UNIX
    char *cfgpath;
	int err;

#if PLATFORM_MACOSX
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/", getenv ("HOME"));
	mkdir (ApogeePath, S_IRWXU);
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/Application Support/", getenv ("HOME"));
	mkdir (ApogeePath, S_IRWXU);
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/Library/Application Support/Shadow Warrior/", getenv ("HOME"));
#else
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.shadowwarrior/", getenv ("HOME"));
#endif

	err = mkdir (ApogeePath, S_IRWXU);
	if (err == -1 && errno != EEXIST)
	{
		fprintf (stderr, "Couldn't create preferences directory: %s\n", 
				strerror (errno));
		return -1;
	}

    /* copy duke3d.cfg to prefpath if it doesn't exist... */
    cfgpath = alloca(strlen(ApogeePath) + strlen(SETUPFILENAME) + 1);
    strcpy(cfgpath, ApogeePath);
    strcat(cfgpath, SETUPFILENAME);
    if (access(cfgpath, F_OK) == -1)
    {
        FILE *in = fopen(SETUPFILENAME, "rb");
        if (in)
        {
            FILE *out = fopen(cfgpath, "wb");
            if (out)
            {
                int ch;
                while ((ch = fgetc(in)) != EOF)
                    fputc(ch, out);
                fclose(out);
            }
            fclose(in);
        }
    }
#else
    sprintf(ApogeePath, ".%s", PATH_SEP_STR);
#endif

	return 0;
}


int dukescreencapture(char *str, char inverseit)
{
// dos screencapture wants string to be in NAME0000.pcx format.
#ifndef PLATFORM_DOS
    // respect prefpath...
    const char *SCREENSHOTDIR = "Screenshots";
    size_t slen = strlen(ApogeePath) + strlen(str) +
                  strlen(PATH_SEP_STR) + strlen(SCREENSHOTDIR) + 1;
    char *path = alloca(slen);
    strcpy(path, ApogeePath);
    strcat(path, SCREENSHOTDIR);
    mkdir(path, "screenshot");
    strcat(path, PATH_SEP_STR);
    strcat(path, str);
    str = path;
#endif

    return(screencapture(str, inverseit));
}


char   CheckParm (char *check)
{
    int i;
    for (i = 1; i < _argc; i++)
    {
        if ((*(_argv[i]) == '-') && (strcmpi(_argv[i] + 1, check) == 0))
            return(i);
    }

    return(0);
}


static void (*shutdown_func)(void) = NULL;

void RegisterShutdownFunction( void (* shutdown) (void) )
{
    shutdown_func = shutdown;
}

void Shutdown(void)
{
    if (shutdown_func != NULL)
    {
        shutdown_func();
        shutdown_func = NULL;
    }
}


/*
 * ANSI escape sequences...ph34r.  --ryan.
 */
void _settextposition(int x, int y)
{
    //printf("\e[%d;%dH", x, y);
} /* gotoxy */


void _outtext(const char *str)
{
    puts(str);
} /* gotoxy */


// !!! FIXME: What IS this thing?
int _nullarea = 0;
int __begtext = 0;


/*
 * From Ryan's buildengine CHANGELOG:
 *  Removed global var: cachedebug in engine.c, and put #define
 *  BUILD_CACHEDEBUG 0 at the top of the source. Flip it to 1 if you ever
 *  need to tinker in the cache code.
 */
char cachedebug = 0;

