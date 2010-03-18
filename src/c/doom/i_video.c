// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
//#include <sys/shm.h>
//
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/keysym.h>
//
//#include <X11/extensions/XShm.h>
// Had to dig up XShm.c for this one.
// It is in the libXext, but not in the XFree86 headers.
#ifdef LINUX
int XShmGetEventBase( Display* dpy ); // problems with g++?
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
//#include <errnos.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

unsigned int gameScreen[SCREENWIDTH*SCREENHEIGHT];

void I_InitGraphics(void)
{
}

void I_ShutdownGraphics(void)
{
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

void I_GetEvent(void)
{

}

//
// I_StartTic
//
void I_StartTic (void)
{
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//

unsigned int palette[256];

void I_FinishUpdate (void)
{
	int i;
	byte* ptr;
	unsigned int* gameScreenPtr = gameScreen;
	ptr = screens[0];
	for(i=0; i<SCREENWIDTH*SCREENHEIGHT; i++)
	{
		*gameScreenPtr = palette[*ptr];
		gameScreenPtr++;
		ptr++;
	}
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

//
// I_SetPalette
//

void I_SetPalette (byte* palData)
{
    //UploadNewPalette(X_cmap, palette);
	int i;

	for(i=0; i<256; i++)
	{
		palette[i] = gammatable[usegamma][*palData++] << 16;
		palette[i] |= gammatable[usegamma][*palData++] << 8;
		palette[i] |= gammatable[usegamma][*palData++];
	}
}


