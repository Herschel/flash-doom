
// I_FLASH.C
#include <avm2-libc/include/AS3.h>
#include <stdlib.h>
#include <stdarg.h>
#include "DoomDef.h"
#include "R_local.h"
#include "sounds.h"
#include "i_sound.h"

// Public Data

// Code

boolean S_StopSoundID(int sound_id, int priority);

extern float soundBuffer[];
unsigned int gameScreen[SCREENWIDTH*SCREENHEIGHT];
unsigned int palette[256];

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
	extern int key_flyup, key_flydown, key_flycenter;
	extern int key_lookup, key_lookdown, key_lookcenter;
	extern int key_invleft, key_invright, key_useartifact;

	AS3_Val bindingsArray;

	AS3_ArrayValue(args, "AS3ValType", &bindingsArray);
	AS3_ArrayValue(bindingsArray, "IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType, IntType",
		&keys[0], &keys[1], &keys[2], &keys[3], &keys[4], &keys[5], &keys[6], &keys[7], &keys[8],
		&keys[9], &keys[10], &keys[11], &keys[12], &keys[13]);

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
	key_invleft = keys[10];
	key_invright = keys[11];
	key_useartifact = keys[12];
	key_jump = keys[13];

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
	didCheat = 0;

    return 0;
}


// SAVE GAME

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

void I_StartupNet (void);
void I_ShutdownNet (void);

void I_InitDiskFlash (void);

extern  int     usemouse, usejoystick;

extern void **lumpcache;

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

static channel_t channel[MAX_CHANNELS];

static int rs; //the current registered song.
int mus_song = -1;
int mus_lumpnum;
void *mus_sndptr;
byte *soundCurve;

extern sfxinfo_t S_sfx[];
extern musicinfo_t S_music[];

extern int snd_DesiredMusicDevice;
extern int snd_DesiredSfxDevice;
extern int snd_MaxVolume;
extern int snd_MusicVolume;
extern int snd_Channels;

extern int startepisode;
extern int startmap;

int AmbChan;

void S_Start(void)
{
	int i;

	S_StartSong((gameepisode-1)*9 + gamemap-1, true);

	//stop all sounds
	for(i=0; i < snd_Channels; i++)
	{
		if(channel[i].handle)
		{
			S_StopSound(channel[i].mo);
		}
	}
	memset(channel, 0, 8*sizeof(channel_t));
}

void S_StartSong(int song, boolean loop)
{
	if(song == mus_song)
	{ // don't replay an old song
		return;
	}
	if(rs)
	{
		I_StopSong(rs);
		I_UnRegisterSong(rs);
		Z_ChangeTag(lumpcache[mus_lumpnum], PU_CACHE);
		#ifdef __WATCOMC__
			_dpmi_unlockregion(mus_sndptr, lumpinfo[mus_lumpnum].size);
		#endif
	}
	if(song < mus_e1m1 || song > NUMMUSIC)
	{
		return;
	}
	mus_lumpnum = W_GetNumForName(S_music[song].name);
	mus_sndptr = W_CacheLumpNum(mus_lumpnum, PU_MUSIC);
	#ifdef __WATCOMC__
		_dpmi_lockregion(mus_sndptr, lumpinfo[mus_lumpnum].size);
	#endif
	rs = I_RegisterSong(mus_sndptr);
	I_PlaySong(rs, loop); //'true' denotes endless looping.
	mus_song = song;
}

void S_StartSound(mobj_t *origin, int sound_id)
{
	int dist, vol;
	int i;
	int sound;
	int priority;
	int sep;
	int angle;
	int absx;
	int absy;

	static int sndcount = 0;
	int chan;

	if(sound_id==0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[consoleplayer].mo;
	}

// calculate the distance before other stuff so that we can throw out
// sounds that are beyond the hearing range.
	absx = abs(origin->x-players[consoleplayer].mo->x);
	absy = abs(origin->y-players[consoleplayer].mo->y);
	dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
	dist >>= FRACBITS;
//  dist = P_AproxDistance(origin->x-viewx, origin->y-viewy)>>FRACBITS;

	if(dist >= MAX_SND_DIST)
	{
//      dist = MAX_SND_DIST - 1;
	  return; //sound is beyond the hearing range...
	}
	if(dist < 0)
	{
		dist = 0;
	}
	priority = S_sfx[sound_id].priority;
	priority *= (10 - (dist/160));
	if(!S_StopSoundID(sound_id, priority))
	{
		return; // other sounds have greater priority
	}
	for(i=0; i<snd_Channels; i++)
	{
		if(origin->player)
		{
			i = snd_Channels;
			break; // let the player have more than one sound.
		}
		if(origin == channel[i].mo)
		{ // only allow other mobjs one sound
			S_StopSound(channel[i].mo);
			break;
		}
	}
	if(i >= snd_Channels)
	{
		if(sound_id >= sfx_wind)
		{
			if(AmbChan != -1 && S_sfx[sound_id].priority <=
				S_sfx[channel[AmbChan].sound_id].priority)
			{
				return; //ambient channel already in use
			}
			else
			{
				AmbChan = -1;
			}
		}
		for(i=0; i<snd_Channels; i++)
		{
			if(channel[i].mo == NULL)
			{
				break;
			}
		}
		if(i >= snd_Channels)
		{
			//look for a lower priority sound to replace.
			sndcount++;
			if(sndcount >= snd_Channels)
			{
				sndcount = 0;
			}
			for(chan=0; chan < snd_Channels; chan++)
			{
				i = (sndcount+chan)%snd_Channels;
				if(priority >= channel[i].priority)
				{
					chan = -1; //denote that sound should be replaced.
					break;
				}
			}
			if(chan != -1)
			{
				return; //no free channels.
			}
			else //replace the lower priority sound.
			{
				if(channel[i].handle)
				{
					if(I_SoundIsPlaying(channel[i].handle))
					{
						I_StopSound(channel[i].handle);
					}
					if(S_sfx[channel[i].sound_id].usefulness > 0)
					{
						S_sfx[channel[i].sound_id].usefulness--;
					}

					if(AmbChan == i)
					{
						AmbChan = -1;
					}
				}
			}
		}
	}
	if(S_sfx[sound_id].lumpnum == 0)
	{
		S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
	}
	if(S_sfx[sound_id].snd_ptr == NULL)
	{
		S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
			PU_SOUND);
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}

	// calculate the volume based upon the distance from the sound origin.
//      vol = (snd_MaxVolume*16 + dist*(-snd_MaxVolume*16)/MAX_SND_DIST)>>9;
	vol = soundCurve[dist];

	if(origin == players[consoleplayer].mo)
	{
		sep = 128;
	}
	else
	{
		/*angle = R_PointToAngle2(players[consoleplayer].mo->x,
			players[consoleplayer].mo->y, channel[i].mo->x, channel[i].mo->y);
		angle = (angle-viewangle)>>24;
		sep = angle*2-128;
		if(sep < 64)
			sep = -sep;
		if(sep > 192)
			sep = 512-sep;*/

#define S_STEREO_SWING		(96*0x10000)

		// angle of source to listener
		angle = R_PointToAngle2(players[consoleplayer].mo->x,
					players[consoleplayer].mo->y,
					channel[i].mo->x,
					channel[i].mo->y);

		if (angle > players[consoleplayer].mo->angle)
		angle = angle - players[consoleplayer].mo->angle;
		else
		angle = angle + (0xffffffff - players[consoleplayer].mo->angle);

		angle >>= ANGLETOFINESHIFT;

		// stereo separation
		sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);
	}

	channel[i].pitch = (byte)(127+(M_Random()&7)-(M_Random()&7));
	channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, vol, sep, channel[i].pitch, 0);
	channel[i].mo = origin;
	channel[i].sound_id = sound_id;
	channel[i].priority = priority;
	if(sound_id >= sfx_wind)
	{
		AmbChan = i;
	}
	if(S_sfx[sound_id].usefulness == -1)
	{
		S_sfx[sound_id].usefulness = 1;
	}
	else
	{
		S_sfx[sound_id].usefulness++;
	}
}

void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume)
{
	int dist;
	int i;
	int sep;

	static int sndcount;
	int chan;

	if(sound_id == 0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[consoleplayer].mo;
	}

	if(volume == 0)
	{
		return;
	}
	volume = (volume*(snd_MaxVolume+1)*8)>>7;

// no priority checking, as ambient sounds would be the LOWEST.
	for(i=0; i<snd_Channels; i++)
	{
		if(channel[i].mo == NULL)
		{
			break;
		}
	}
	if(i >= snd_Channels)
	{
		return;
	}
	if(S_sfx[sound_id].lumpnum == 0)
	{
		S_sfx[sound_id].lumpnum = I_GetSfxLumpNum(&S_sfx[sound_id]);
	}
	if(S_sfx[sound_id].snd_ptr == NULL)
	{
		S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
			PU_SOUND);
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}
	channel[i].pitch = (byte)(127-(M_Random()&3)+(M_Random()&3));
	channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, volume, 128, channel[i].pitch, 0);
	channel[i].mo = origin;
	channel[i].sound_id = sound_id;
	channel[i].priority = 1; //super low priority.
	if(S_sfx[sound_id].usefulness == -1)
	{
		S_sfx[sound_id].usefulness = 1;
	}
	else
	{
		S_sfx[sound_id].usefulness++;
	}
}

boolean S_StopSoundID(int sound_id, int priority)
{
	int i;
	int lp; //least priority
	int found;

	if(S_sfx[sound_id].numchannels == -1)
	{
		return(true);
	}
	lp = -1; //denote the argument sound_id
	found = 0;
	for(i=0; i<snd_Channels; i++)
	{
		if(channel[i].sound_id == sound_id && channel[i].mo)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= channel[i].priority)
			{ // if we're gonna kill one, then this'll be it
				lp = i;
				priority = channel[i].priority;
			}
		}
	}
	if(found < S_sfx[sound_id].numchannels)
	{
		return(true);
	}
	else if(lp == -1)
	{
		return(false); // don't replace any sounds
	}
	if(channel[lp].handle)
	{
		if(I_SoundIsPlaying(channel[lp].handle))
		{
			I_StopSound(channel[lp].handle);
		}
		if(S_sfx[channel[i].sound_id].usefulness > 0)
		{
			S_sfx[channel[i].sound_id].usefulness--;
		}
		channel[lp].mo = NULL;
	}
	return(true);
}

void S_StopSound(mobj_t *origin)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(channel[i].mo == origin)
		{
			I_StopSound(channel[i].handle);
			if(S_sfx[channel[i].sound_id].usefulness > 0)
			{
				S_sfx[channel[i].sound_id].usefulness--;
			}
			channel[i].handle = 0;
			channel[i].mo = NULL;
			if(AmbChan == i)
			{
				AmbChan = -1;
			}
		}
	}
}

void S_SoundLink(mobj_t *oldactor, mobj_t *newactor)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(channel[i].mo == oldactor)
			channel[i].mo = newactor;
	}
}

void S_PauseSound(void)
{
	I_PauseSong(rs);
}

void S_ResumeSound(void)
{
	I_ResumeSong(rs);
}

static int nextcleanup;

void S_UpdateSounds(mobj_t *listener)
{
	int i, dist, vol;
	int angle;
	int sep;
	int priority;
	int absx;
	int absy;

	listener = players[consoleplayer].mo;
	if(snd_MaxVolume == 0)
	{
		return;
	}
	/*if(nextcleanup < gametic)
	{
		for(i=0; i < NUMSFX; i++)
		{
			if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
			{
				if(lumpcache[S_sfx[i].lumpnum])
				{
					if(((memblock_t *)((byte *)(lumpcache[S_sfx[i].lumpnum])-
						sizeof(memblock_t)))->id == 0x1d4a11)
					{ // taken directly from the Z_ChangeTag macro
						Z_ChangeTag2(lumpcache[S_sfx[i].lumpnum], PU_CACHE);
						#ifdef __WATCOMC__
							_dpmi_unlockregion(S_sfx[i].snd_ptr, lumpinfo[S_sfx[i].lumpnum].size);
						#endif
					}
				}
				S_sfx[i].usefulness = -1;
				S_sfx[i].snd_ptr = NULL;
			}
		}
		nextcleanup = gametic+35; //CLEANUP DEBUG cleans every second
	}*/
	for(i=0;i<snd_Channels;i++)
	{
		if(!channel[i].handle || S_sfx[channel[i].sound_id].usefulness == -1)
		{
			continue;
		}
		if(!I_SoundIsPlaying(channel[i].handle))
		{
			if(S_sfx[channel[i].sound_id].usefulness > 0)
			{
				S_sfx[channel[i].sound_id].usefulness--;
			}
			channel[i].handle = 0;
			channel[i].mo = NULL;
			channel[i].sound_id = 0;
			if(AmbChan == i)
			{
				AmbChan = -1;
			}
		}
		if(channel[i].mo == NULL || channel[i].sound_id == 0
			|| channel[i].mo == players[consoleplayer].mo)
		{
			continue;
		}
		else
		{
			absx = abs(channel[i].mo->x-players[consoleplayer].mo->x);
			absy = abs(channel[i].mo->y-players[consoleplayer].mo->y);
			dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
			dist >>= FRACBITS;
//          dist = P_AproxDistance(channel[i].mo->x-listener->x, channel[i].mo->y-listener->y)>>FRACBITS;

			if(dist >= MAX_SND_DIST)
			{
				S_StopSound(channel[i].mo);
				continue;
			}
			if(dist < 0)
				dist = 0;

// calculate the volume based upon the distance from the sound origin.
//          vol = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE)+dist)*(snd_MaxVolume*8))>>7;
			vol = soundCurve[dist];

			angle = R_PointToAngle2(players[consoleplayer].mo->x,
				players[consoleplayer].mo->y, channel[i].mo->x, channel[i].mo->y);
			angle = (angle-viewangle)>>24;
			sep = angle*2-128;
			if(sep < 64)
				sep = -sep;
			if(sep > 192)
				sep = 512-sep;
			I_UpdateSoundParams(channel[i].handle, vol, sep, channel[i].pitch);
			priority = S_sfx[channel[i].sound_id].priority;
			priority *= (10 - (dist>>8));
			channel[i].priority = priority;
		}
	}
}

void S_Init(void)
{
	soundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);
	I_StartupSound();
	snd_Channels = 8;
	I_SetChannels(snd_Channels);
	I_SetMusicVolume(snd_MusicVolume);
	S_SetMaxVolume(true);
}

void S_GetChannelInfo(SoundInfo_t *s)
{
	int i;
	ChanInfo_t *c;

	s->channelCount = snd_Channels;
	s->musicVolume = snd_MusicVolume;
	s->soundVolume = snd_MaxVolume;
	for(i = 0; i < snd_Channels; i++)
	{
		c = &s->chan[i];
		c->id = channel[i].sound_id;
		c->priority = channel[i].priority;
		c->name = S_sfx[c->id].name;
		c->mo = channel[i].mo;
		c->distance = P_AproxDistance(c->mo->x-viewx, c->mo->y-viewy)
			>>FRACBITS;
	}
}

void S_SetMaxVolume(boolean fullprocess)
{
	int i;

	if(!fullprocess)
	{
		soundCurve[0] = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE))*(snd_MaxVolume*8))>>7;
	}
	else
	{
		for(i = 0; i < MAX_SND_DIST; i++)
		{
			soundCurve[i] = (*((byte *)W_CacheLumpName("SNDCURVE", PU_CACHE)+i)*(snd_MaxVolume*8))>>7;
		}
	}
}

static boolean musicPaused;
void S_SetMusicVolume(void)
{
	I_SetMusicVolume(snd_MusicVolume);
	if(snd_MusicVolume == 0)
	{
		I_PauseSong(rs);
		musicPaused = true;
	}
	else if(musicPaused)
	{
		musicPaused = false;
		I_ResumeSong(rs);
	}
}

void S_ShutDown(void)
{
	I_StopSong(rs);
	I_UnRegisterSong(rs);
	I_ShutdownSound();
}

/*
=============================================================================

							CONSTANTS

=============================================================================
*/

#define SC_INDEX                0x3C4
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_H_TOTAL    0
#define CRTC_H_DISPEND  1
#define CRTC_H_BLANK    2
#define CRTC_H_ENDBLANK 3
#define CRTC_H_RETRACE  4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL    6
#define CRTC_OVERFLOW   7
#define CRTC_ROWSCAN    8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND  11
#define CRTC_STARTHIGH  12
#define CRTC_STARTLOW   13
#define CRTC_CURSORHIGH 14
#define CRTC_CURSORLOW  15
#define CRTC_V_RETRACE  16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND  18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE  20
#define CRTC_V_BLANK    21
#define CRTC_V_ENDBLANK 22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE 24


#define GC_INDEX                0x3CE
#define GC_SETRESET             0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE 2
#define GC_DATAROTATE   3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN    17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN              19
#define ATR_COLORSELECT 20

#define STATUS_REGISTER_1    0x3da

#define PEL_WRITE_ADR   0x3c8
#define PEL_READ_ADR    0x3c7
#define PEL_DATA                0x3c9
#define PEL_MASK                0x3c6

boolean grmode;

//==================================================
//
// joystick vars
//
//==================================================

boolean         joystickpresent;
extern  unsigned        joystickx, joysticky;
boolean I_ReadJoystick (void);          // returns false if not connected

//===============================

int             ticcount;

#define KEY_LSHIFT      0xfe

#define KEY_INS         (0x80+0x52)
#define KEY_DEL         (0x80+0x53)
#define KEY_PGUP        (0x80+0x49)
#define KEY_PGDN        (0x80+0x51)
#define KEY_HOME        (0x80+0x47)
#define KEY_END         (0x80+0x4f)

#define SC_RSHIFT       0x36
#define SC_LSHIFT       0x2a

//==========================================================================

//--------------------------------------------------------------------------
//
// FUNC I_GetTime
//
// Returns time in 1/35th second tics.
//
//--------------------------------------------------------------------------

int I_GetTime (void)
{
	ticcount++;
	return(ticcount);
}

//--------------------------------------------------------------------------
//
// PROC I_ColorBorder
//
//--------------------------------------------------------------------------

void I_ColorBorder(void)
{
}

//--------------------------------------------------------------------------
//
// PROC I_UnColorBorder
//
//--------------------------------------------------------------------------

void I_UnColorBorder(void)
{
}

void I_WaitVBL(int vbls)
{
}

void I_SetPalette(byte *palData)
{
	int i;

	for(i=0; i<256; i++)
	{
		palette[i] = gammatable[usegamma][*palData++] << 16;
		palette[i] |= gammatable[usegamma][*palData++] << 8;
		palette[i] |= gammatable[usegamma][*palData++];
	}
}

void I_Update (void)
{
	int i;
	byte* ptr;
	unsigned int* gameScreenPtr = gameScreen;
	ptr = screen;
	for(i=0; i<SCREENWIDTH*SCREENHEIGHT; i++)
	{
		*gameScreenPtr = palette[*ptr];
		gameScreenPtr++;
		ptr++;
	}
}

//--------------------------------------------------------------------------
//
// PROC I_InitGraphics
//
//--------------------------------------------------------------------------

void I_InitGraphics(void)
{
	I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
	I_InitDiskFlash();
}

//--------------------------------------------------------------------------
//
// PROC I_ShutdownGraphics
//
//--------------------------------------------------------------------------

void I_ShutdownGraphics(void)
{
}

//--------------------------------------------------------------------------
//
// PROC I_ReadScreen
//
// Reads the screen currently displayed into a linear buffer.
//
//--------------------------------------------------------------------------

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screen, SCREENWIDTH*SCREENHEIGHT);
}


//===========================================================================

/*
===================
=
= I_StartTic
=
// called by D_DoomLoop
// called before processing each tic in a frame
// can call D_PostEvent
// asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events
===================
*/


#define SC_UPARROW              0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW   0x4d

void   I_StartTic (void)
{
}

/*
===============
=
= I_StartFrame
=
===============
*/

void I_StartFrame (void)
{
}

/*
============================================================================

						KEYBOARD

============================================================================
*/

/*
===============
=
= I_StartupKeyboard
=
===============
*/

void I_StartupKeyboard (void)
{
}


void I_ShutdownKeyboard (void)
{
}

/*
===============
=
= I_StartupJoystick
=
===============
*/


void I_StartupJoystick (void)
{
}


/*
===============
=
= I_Init
=
= hook interrupts and set graphics mode
=
===============
*/

int isCyberPresent;

void I_Init (void)
{
	isCyberPresent = 0;

	tprintf("I_StartupMouse ",1);
	//I_StartupMouse();
	tprintf("I_StartupJoystick ",1);
	I_StartupJoystick();
	tprintf("I_StartupKeyboard ",1);
	I_StartupKeyboard();
	tprintf("S_Init... ",1);
	S_Init();
	S_Start();
}


/*
===============
=
= I_Shutdown
=
= return to default system state
=
===============
*/

void I_Shutdown (void)
{
	I_ShutdownGraphics ();
	//S_ShutDown ();
	//I_ShutdownMouse ();
	I_ShutdownKeyboard ();
}


/*
================
=
= I_Error
=
================
*/

void I_Error (char *error, ...)
{
	va_list argptr;

	D_QuitNetGame ();
	I_Shutdown ();
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	sztrace(error);
}

//--------------------------------------------------------------------------
//
// I_Quit
//
// Shuts down net game, saves defaults, prints the exit text message,
// goes to text mode, and exits.
//
//--------------------------------------------------------------------------

void I_Quit(void)
{
	F_QuitGame();
}

/*
===============
=
= I_ZoneBase
=
===============
*/

int	mb_used = 6;
byte* I_ZoneBase (int*	size)
{
    *size = mb_used*1024*1024;
    return (byte *) malloc (*size);
}

/*
=============================================================================

					DISK ICON FLASHING

=============================================================================
*/

void I_InitDiskFlash (void)
{
}

// draw disk icon
void I_BeginRead (void)
{
}

// erase disk icon
void I_EndRead (void)
{
}



/*
=============
=
= I_AllocLow
=
=============
*/

byte *I_AllocLow (int length)
{
	byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}

/*
============================================================================

						NETWORKING

============================================================================
*/

/* // FUCKED LINES
typedef struct
{
	char    priv[508];
} doomdata_t;
*/ // FUCKED LINES

#define DOOMCOM_ID              0x12345678l

extern  doomcom_t               *doomcom;

/*
====================
=
= I_InitNetwork
=
====================
*/

void I_InitNetwork (void)
{
	int             i;

	i = M_CheckParm ("-net");
	if (!i)
	{
	//
	// single player game
	//
		doomcom = malloc (sizeof (*doomcom) );
		memset (doomcom, 0, sizeof(*doomcom) );
		netgame = false;
		doomcom->id = DOOMCOM_ID;
		doomcom->numplayers = doomcom->numnodes = 1;
		doomcom->deathmatch = false;
		doomcom->consoleplayer = 0;
		doomcom->ticdup = 1;
		doomcom->extratics = 0;
		return;
	}

	netgame = true;
	doomcom = (doomcom_t *)atoi(myargv[i+1]);
//DEBUG
doomcom->skill = startskill;
doomcom->episode = startepisode;
doomcom->map = startmap;
doomcom->deathmatch = deathmatch;

}

void I_NetCmd (void)
{
	if (!netgame)
		I_Error ("I_NetCmd when not in netgame");
}

void I_ReadCyberCmd (ticcmd_t *cmd)
{

}


void I_Tactile (int on, int off, int total)
{
}
