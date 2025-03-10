//-------------------------------------------------------------------------
/*
Copyright (C) 1997, 2005 - 3D Realms Entertainment

This file is part of Shadow Warrior version 1.2

Shadow Warrior is free software; you can redistribute it and/or
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

Original Source: 1994 - Jim Dose
Prepared for public release: 03/28/2005 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

/**********************************************************************
   module: USRHOOKS.C

   author: James R. Dos�
   phone:  (214)-XXX-XXXX Ext #221
   date:   July 26, 1994

   This module contains cover functions for operations the library
   needs that may be restricted by the calling program.  This code
   is left public for you to modify.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "shadow.h" // added for unix port.  --ryan.
#include "keys.h"
#include "game.h"
#include "usrhooks.h"
#include "def.h"

/*---------------------------------------------------------------------
   Function: USRHOOKS_GetMem

   Allocates the requested amount of memory and returns a pointer to
   its location, or NULL if an error occurs.  NOTE: pointer is assumed
   to be dword aligned.
---------------------------------------------------------------------*/

int USRHOOKS_GetMem
   (
   void **ptr,
   unsigned long size
   )

   {
   void *memory;

   //memory = AllocMem( size );
   memory = malloc( size );

   //LoadFile routine allocs memory without this routine so
   //cant do AllocMem

   if ( memory == NULL )
      {
      TerminateGame();
      printf("USRHOOKS_GetMem out of memory");
      exit(0);
      }

   *ptr = memory;

    ////DSPRINTF(ds,"User Hooks %d, %ud",size,*ptr);
    //MONO_PRINT(ds);
   

   return( USRHOOKS_Ok );
   }


/*---------------------------------------------------------------------
   Function: USRHOOKS_FreeMem

   Deallocates the memory associated with the specified pointer.
---------------------------------------------------------------------*/

int USRHOOKS_FreeMem
   (
   void *ptr
   )

   {
   if ( ptr == NULL )
      {
      TerminateGame();
      printf("USRHOOKS_FreeMem freed a null pointer");
      exit(0);
      }

    ////DSPRINTF(ds,"Free Hooks %ud",ptr);
    //MONO_PRINT(ds);
      
   //FreeMem( ptr );
   free( ptr );

   return( USRHOOKS_Ok );
   }
