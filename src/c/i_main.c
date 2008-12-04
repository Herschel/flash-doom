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
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";

#include "doomdef.h"
#include "d_main.h"
#include "v_video.h"
#include "i_sound.h"
#include "m_argv.h"
#include <avm2-libc/include/AS3.h>

AS3_Val thiz;

AS3_Val getSaveGame(int i, int clear)
{
	AS3_Val boolVal = clear ? AS3_True() : AS3_False();
	AS3_Val result = AS3_CallTS("getSaveGame", thiz, "IntType, AS3ValType", i, boolVal);
	AS3_Release(boolVal);
	return result;
}

AS3_Val getScreen(void* data, AS3_Val args)
{
	return AS3_Ptr( (void*)&gameScreen[0] );
}

AS3_Val getSoundData(void* self, AS3_Val args)
{
	return AS3_Ptr( (void*)soundBuffer );
}

AS3_Val keyDown(void* self, AS3_Val args)
{
	int keyCode = 0;
	AS3_ArrayValue(args, "IntType", &keyCode);
	keyStates[keyCode] = -1;

	return AS3_Undefined();
}

AS3_Val keyUp(void* self, AS3_Val args)
{
	int keyCode = 0;
	AS3_ArrayValue(args, "IntType", &keyCode);
	keyStates[keyCode] = 1;

	return AS3_Undefined();
}

AS3_Val getRam(void* self, AS3_Val args)
{
	return AS3_Ram();
}

AS3_Val setThiz(void* self, AS3_Val args)
{
	AS3_ArrayValue(args, "AS3ValType", &thiz);
	return AS3_Undefined();
}

int
main
(  ) 
{ 
	int i;
    //myargc = argc; 
	myargc = 0; 
    //myargv = argv; 
	
	memset(gameScreen, 0, SCREENWIDTH*SCREENHEIGHT*4);

    D_DoomMain (); 

 	AS3_Val tickMethod = AS3_Function( NULL, D_DoomLoop );
	AS3_Val getScreenMethod = AS3_Function( NULL, getScreen );
	AS3_Val getSoundDataMethod = AS3_Function( NULL, getSoundData );
	AS3_Val keyDownMethod = AS3_Function( NULL, keyDown );
	AS3_Val keyUpMethod = AS3_Function( NULL, keyUp );
	AS3_Val getRamMethod = AS3_Function( NULL, getRam );
	AS3_Val setThizMethod = AS3_Function( NULL, setThiz );

	// construct an object that holds references to the functions
	AS3_Val result = AS3_Object( "tick: AS3ValType, getFrameBuffer: AS3ValType, getSoundData: AS3ValType, keyDown: AS3ValType, keyUp: AS3ValType, getRam: AS3ValType, setThiz: AS3ValType", tickMethod, getScreenMethod, getSoundDataMethod, keyDownMethod, keyUpMethod, getRamMethod, setThizMethod );

	// Release
	AS3_Release( tickMethod );
	AS3_Release( getScreenMethod );
	AS3_Release( getSoundDataMethod );
	AS3_Release( keyDownMethod );
	AS3_Release( keyUpMethod );
	AS3_Release( getRamMethod );
	AS3_Release( setThizMethod );

	for(i=0; i<256; i++)
		keyStates[i] = 0;
	
	AS3_LibInit( result );

    return 0;
}