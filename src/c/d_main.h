// Emacs style mode select   -*- C++ -*- 
/*-----------------------------------------------------------------------------
 *  FlashDoom
 * 
 *  based on Linux DOOM 1.10
 *  Copyright (C) 1999 by
 *  id Software
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
 *  DESCRIPTION:
 *  High-level game routines
 *  AUTHOR: Mike Welsh
 *-----------------------------------------------------------------------------
 */

#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include <avm2-libc/include/AS3.h>

#ifdef __GNUG__
#pragma interface
#endif



#define MAXWADFILES             20
extern char*		wadfiles[MAXWADFILES];

void D_AddFile (char *file);

// MIKE 11/08
extern AS3_Val thiz;
AS3_Val getSaveGame(int i, int clear);


// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
// If not overrided by user input, calls N_AdvanceDemo.
//
void D_DoomMain (void);
AS3_Val D_DoomLoop (void *data, AS3_Val args);

// Called by IO functions when input is detected.
void D_PostEvent (event_t* ev);


//
// BASE LEVEL
//
void D_PageTicker (void);
void D_PageDrawer (void);
void D_AdvanceDemo (void);
void D_StartTitle (void);

#endif
