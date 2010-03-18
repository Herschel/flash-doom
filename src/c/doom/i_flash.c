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

static AS3_Val thiz;

// FLASH HOOKS
AS3_Val F_GetScreen(void* data, AS3_Val args)
{
	return AS3_Ptr( gameScreen );
}

AS3_Val F_GetSoundData(void* self, AS3_Val args)
{
	return AS3_Ptr( soundBuffer );
}

static event_t event;

int I_MapKey(int keyCode)
{
	switch(keyCode)
	{
		case 8:		return KEY_BACKSPACE;
		case 16:	return KEY_RSHIFT;
		case 18:	return KEY_RALT;
		case 17:	return KEY_RCTRL;
		case 37:	return KEY_LEFTARROW;
		case 38:	return KEY_UPARROW;
		case 39:	return KEY_RIGHTARROW;
		case 40:	return KEY_DOWNARROW;
		case 112:	return KEY_F1;
		case 113:	return KEY_F2;
		case 114:	return KEY_F3;
		case 115:	return KEY_F4;
		case 116:	return KEY_F5;
		case 117:	return KEY_F6;
		case 118:	return KEY_F7;
		case 119:	return KEY_F8;
		case 120:	return KEY_F9;
		case 121:	return KEY_F10;
		case 122:	return KEY_F11;
		case 123:	return KEY_F12;
		case 187:	return KEY_EQUALS;
		case 189:	return KEY_MINUS;
		case 219:	return '[';
		case 221:	return ']';
	}

	if( keyCode >= 65 && keyCode <= 90 )
		keyCode -= ('A'-'a');
	
	return keyCode;
}

AS3_Val F_KeyDown(void* self, AS3_Val args)
{
	int keyCode;
	AS3_ArrayValue(args, "IntType", &keyCode);

	event.type = ev_keydown;
	event.data1 = I_MapKey( keyCode );
	D_PostEvent(&event);

	return AS3_Undefined();
}

AS3_Val F_KeyUp(void* self, AS3_Val args)
{
	int keyCode;
	AS3_ArrayValue(args, "IntType", &keyCode);

	event.type = ev_keyup;
	event.data1 = I_MapKey( keyCode );
	D_PostEvent(&event);

	return AS3_Undefined();
}

AS3_Val F_MouseMove(void* self, AS3_Val args)
{
	int dx, dy;
	AS3_ArrayValue(args, "IntType, IntType", &dx, &dy);

	event.type = ev_mouse;
	event.data1 = 0;
	event.data2 = dx;
	event.data3 = dy;
	D_PostEvent(&event);

	return AS3_Undefined();
}

AS3_Val F_SetKeyBindings(void* self, AS3_Val args)
{
	int keys[14];
	int i;

	extern int key_right, key_left, key_up, key_down;
	extern int key_strafeleft, key_straferight, key_jump;
	extern int key_fire, key_use, key_strafe, key_speed;
	//extern int key_flyup, key_flydown, key_flycenter;
	//extern int key_lookup, key_lookdown, key_lookcenter;
	//extern int key_invleft, key_invright, key_useartifact;

	AS3_Val bindingsArray;

	AS3_ArrayValue(args, "AS3ValType", &bindingsArray);
	AS3_ArrayValue(bindingsArray, "IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType",
		&keys[0], &keys[1], &keys[2], &keys[3], &keys[4], &keys[5], &keys[6], &keys[7], &keys[8],
		&keys[9]/*, &keys[10], &keys[11], &keys[12], &keys[13]*/);

	for(i=0; i<14; i++)
	{
		keys[i] = I_MapKey( keys[i] );
	}

	key_up = keys[0];
	key_down = keys[1];
	key_right = keys[2];
	key_left = keys[3];
	key_fire = keys[4];
	key_use = keys[5];
	key_strafeleft = keys[6];
	key_straferight = keys[7];
	key_strafe = keys[8];
	key_speed = keys[9];
/*	key_invleft = keys[10];
	key_invright = keys[11];
	key_useartifact = keys[12];
	key_jump = keys[13];*/

	return AS3_Undefined();
}

AS3_Val F_GetRam(void* self, AS3_Val args)
{
	return AS3_Ram();
}

AS3_Val F_SetThiz(void* self, AS3_Val args)
{
	AS3_ArrayValue(args, "AS3ValType", &thiz);
	return AS3_Undefined();
}

AS3_Val F_TickGame(void* self, AS3_Val args)
{
	D_DoomLoop();
	return AS3_Undefined();
}

AS3_Val F_Render(void* self, AS3_Val args)
{
	extern void D_Display();
	D_Display();
	return AS3_Undefined();
}

void F_ShowLink(const char* url)
{
	AS3_CallTS("goToURL", thiz, "StrType", url);
}

int didCheat;

void F_AwardMedal(const int medalId)
{
	if(!didCheat)
		AS3_CallTS("awardMedal", thiz, "IntType", medalId);
}

void F_QuitGame()
{
	AS3_CallTS("quitGame", thiz, "");
}

int
main
( int argc, char** argv ) 
{ 
	int i;
    //myargc = argc; 
	myargc = 0; 
    //myargv = argv; 
	
	memset(gameScreen, 0, SCREENWIDTH*SCREENHEIGHT*4);

//	memset(key_right, 0, sizeof(int)*10*2);

    D_DoomMain (); 

 	AS3_Val tickMethod = AS3_Function( NULL, F_TickGame );
	AS3_Val getScreenMethod = AS3_Function( NULL, F_GetScreen );
	AS3_Val getSoundDataMethod = AS3_Function( NULL, F_GetSoundData );
	AS3_Val keyDownMethod = AS3_Function( NULL, F_KeyDown );
	AS3_Val keyUpMethod = AS3_Function( NULL, F_KeyUp );
	AS3_Val mouseMoveMethod = AS3_Function( NULL, F_MouseMove );
	AS3_Val getRamMethod = AS3_Function( NULL, F_GetRam );
	AS3_Val setThizMethod = AS3_Function( NULL, F_SetThiz );
	AS3_Val setKeyBindingsMethod = AS3_Function( NULL, F_SetKeyBindings );
	AS3_Val renderMethod = AS3_Function( NULL, F_Render );

	// construct an object that holds references to the functions
	AS3_Val result =	AS3_Object( 
		"tick: AS3ValType, getFrameBuffer: AS3ValType, getSoundData: AS3ValType, keyDown: AS3ValType, keyUp: AS3ValType, mouseMove: AS3ValType, getRam: AS3ValType, setThiz: AS3ValType, setKeyBindings: AS3ValType, render: AS3ValType",
						tickMethod, getScreenMethod, getSoundDataMethod, keyDownMethod, keyUpMethod, mouseMoveMethod, getRamMethod, setThizMethod, setKeyBindingsMethod, renderMethod
						);

	// Release
	AS3_Release( tickMethod );
	AS3_Release( getScreenMethod );
	AS3_Release( getSoundDataMethod );
	AS3_Release( keyDownMethod );
	AS3_Release( keyUpMethod );
	AS3_Release( mouseMoveMethod );
	AS3_Release( getRamMethod );
	AS3_Release( setThizMethod );
	AS3_Release( setKeyBindingsMethod );
	AS3_Release( renderMethod );

	AS3_LibInit( result );

	maketic = gametic = 0;
	singletics = true;
	didCheat = false;

    return 0;
}


// SAVE GAME

#define SAVEGAMESIZE	0x2c000	// TODO repeated
#define SAVESTRINGSIZE	24

boolean F_GetSaveGameName(int slot, char* dst)
{
	AS3_Val savegame_bytearray; // MIKE 11/08
	AS3_Val lengthval; // MIKE
	int length;

	savegame_bytearray = AS3_CallTS("getSaveGame", thiz, "IntType", slot);

	lengthval = AS3_GetS(savegame_bytearray, "length");
	length = AS3_IntValue(lengthval);

	if( length == 0 )
	{
		strcpy(dst,"");
	}
	else
	{
		AS3_ByteArray_readBytes(dst, savegame_bytearray, SAVESTRINGSIZE);
	}

	AS3_Release( lengthval );
	AS3_Release( savegame_bytearray );

	return length > 0;
}

void F_WriteSaveGame(int slot, byte* buffer, int length)
{
	AS3_Val savegame_bytearray;
	savegame_bytearray = AS3_CallTS("getSaveGame", thiz, "IntType", slot);

	AS3_CallTS("clear", savegame_bytearray, "");
	AS3_ByteArray_writeBytes( savegame_bytearray, buffer, length );

	AS3_Release( savegame_bytearray );
}

void F_ReadSaveGame(int slot, byte* buffer)
{
	AS3_Val savegame_bytearray;
	AS3_Val lengthval;
	int length;

	savegame_bytearray = AS3_CallTS("getSaveGame", thiz, "IntType", slot);

	lengthval = AS3_GetS(savegame_bytearray, "length");
	length = AS3_IntValue(lengthval);

	AS3_ByteArray_readBytes(buffer, savegame_bytearray, length);

	AS3_Release( lengthval );
	AS3_Release( savegame_bytearray );
}
