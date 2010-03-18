
//**************************************************************************
//**
//** I_IBM.C
//**
//**************************************************************************

#include <avm2-libc/include/AS3.h>
#include <stdlib.h>
#include <stdarg.h>
#include "h2def.h"
#include "r_local.h"
#include "p_local.h"    // for P_AproxDistance
#include "sounds.h"
#include "i_sound.h"
#include "soundst.h"
#include "st_start.h"

// Macros

#define DEFAULT_ARCHIVEPATH     "o:\\sound\\archive\\"
#define PRIORITY_MAX_ADJUST 10
#define DIST_ADJUST (MAX_SND_DIST/PRIORITY_MAX_ADJUST)

#define DPMI_INT 0x31

#define SEQ_ADDR 0x3C4
#define SEQ_DATA 0x3C5
#define REG_MAPMASK 0x02

#define MASK_PLANE0 0x01
#define MASK_PLANE1 0x02
#define MASK_PLANE2 0x04
#define MASK_PLANE3 0x08

#define P0OFFSET 38400*0
#define P1OFFSET 38400*1
#define P2OFFSET 38400*2
#define P3OFFSET 38400*3

#define VID_INT 0x10

//#define NOKBD
//#define NOTIMER

// Public Data


boolean S_StopSoundID(int sound_id, int priority);

extern float soundBuffer[315*4*2];
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
	H2_PostEvent(&event);

	return AS3_Undefined();
}

AS3_Val F_KeyUp(void* self, AS3_Val args)
{
	int keyCode;
	AS3_ArrayValue(args, "IntType", &keyCode);

	event.type = ev_keyup;
	event.data1 = I_MapKey( keyCode );
	H2_PostEvent(&event);

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
	H2_PostEvent(&event);

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
	H2_GameLoop();
	return AS3_Undefined();
}

AS3_Val F_Render(void* self, AS3_Val args)
{
	extern void DrawAndBlit(void);

	DrawAndBlit();
	return AS3_Undefined();
}

void F_ShowLink(const char* url)
{
	AS3_CallTS("goToURL", thiz, "StrType", url);
}

static AS3_Val savegame_bytearray;
int F_OpenSaveGame(const char* name)
{
	AS3_Val lengthval;
	int length;

	savegame_bytearray = AS3_CallTS("getSaveGame", thiz, "StrType", name);

	lengthval = AS3_GetS(savegame_bytearray, "length");
	length = AS3_IntValue(lengthval);

	AS3_Release(lengthval);
	return length;
}

void F_SaveGameRead(void* buffer, int length)
{
	AS3_ByteArray_readBytes(buffer, savegame_bytearray, length);
}

void F_SaveGameWrite(void* buffer, int length)
{
	AS3_ByteArray_writeBytes(savegame_bytearray, buffer, length);
}

void F_CloseSaveGame()
{
	AS3_Release( savegame_bytearray );
}

int F_LoadSaveGame(const char* name, byte** buffer)
{
	int length;
	length = F_OpenSaveGame(name);
	if(length>0)
	{
		*buffer = Z_Malloc(length, PU_STATIC, NULL);
		F_SaveGameRead(*buffer, length);
	}
	F_CloseSaveGame();
	return length;
}

void F_StoreSaveGame(const char* name, byte* buffer, int length)
{
	F_OpenSaveGame(name);
	F_SaveGameWrite(buffer, length);
	F_CloseSaveGame();
}

void F_ClearSaveGame(const char* name)
{
	int length;
	AS3_Val zero;

	zero = AS3_Int(0);

	length = F_OpenSaveGame(name);
	if(length > 0)
	{
		AS3_SetS(savegame_bytearray, "length", zero);
	}
	AS3_CallTS("clear", savegame_bytearray, "");
	F_CloseSaveGame();

	AS3_Release(zero);

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

    H2_Main (); 

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

// Code

void I_StartupNet (void);
void I_ShutdownNet (void);
void I_ReadExternDriver(void);

void I_ReadMouse (void);

extern  int     usemouse, usejoystick;

extern void **lumpcache;

int i_Vector;
externdata_t *i_ExternData;
boolean useexterndriver;

boolean i_CDMusic;
int i_CDTrack;
int i_CDCurrentTrack;
int i_CDMusicLength;
int oldTic;

/*
===============================================================================

		MUSIC & SFX API

===============================================================================
*/

//static channel_t channel[MAX_CHANNELS];

//static int rs; //the current registered song.
//int mus_song = -1;
//int mus_lumpnum;
//void *mus_sndptr;
//byte *soundCurve;

extern sfxinfo_t S_sfx[];
extern musicinfo_t S_music[];

static channel_t Channel[MAX_CHANNELS];
static int RegisteredSong; //the current registered song.
static int NextCleanup;
static boolean MusicPaused;
static int Mus_Song = -1;
static int Mus_LumpNum;
static void *Mus_SndPtr;
static byte *SoundCurve;

static boolean UseSndScript;
static char ArchivePath[128];

extern int snd_MusicDevice;
extern int snd_SfxDevice;
extern int snd_MaxVolume;
extern int snd_MusicVolume;
extern int snd_Channels;

extern int startepisode;
extern int startmap;

// int AmbChan;

//==========================================================================
//
// S_Start
//
//==========================================================================

void S_Start(void)
{
	S_StopAllSound();
	S_StartSong(gamemap, true);
}

//==========================================================================
//
// S_StartSong
//
//==========================================================================

void S_StartSong(int song, boolean loop)
{
	char *songLump;
	int track;

	if(i_CDMusic)
	{ // Play a CD track, instead
		if(i_CDTrack)
		{ // Default to the player-chosen track
			track = i_CDTrack;
		}
		else
		{
			track = P_GetMapCDTrack(gamemap);
		}
		if(track == i_CDCurrentTrack && i_CDMusicLength > 0)
		{
			return;
		}
		if(!I_CDMusPlay(track))
		{
			if(loop)
			{
				i_CDMusicLength = 35*I_CDMusTrackLength(track);
				oldTic = gametic;
			}
			else
			{
				i_CDMusicLength = -1;
			}
			i_CDCurrentTrack = track;
		}
	}
	else
	{
		if(song == Mus_Song)
		{ // don't replay an old song
			return;
		}
		if(RegisteredSong)
		{
			I_StopSong(RegisteredSong);
			I_UnRegisterSong(RegisteredSong);
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				Z_ChangeTag(lumpcache[Mus_LumpNum], PU_CACHE);
			}
			#ifdef __WATCOMC__
				_dpmi_unlockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
			#endif
			RegisteredSong = 0;
		}
		songLump = P_GetMapSongLump(song);
		if(!songLump)
		{
			return;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		#ifdef __WATCOMC__
			_dpmi_lockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
		#endif
		RegisteredSong = I_RegisterSong(Mus_SndPtr);
		I_PlaySong(RegisteredSong, loop); // 'true' denotes endless looping.
		Mus_Song = song;
	}
}

//==========================================================================
//
// S_StartSongName
//
//==========================================================================

void S_StartSongName(char *songLump, boolean loop)
{
	int cdTrack;

	if(!songLump)
	{
		return;
	}
	if(i_CDMusic)
	{
		cdTrack = 0;

		if(!strcmp(songLump, "hexen"))
		{
			cdTrack = P_GetCDTitleTrack();
		}
		else if(!strcmp(songLump, "hub"))
		{
			cdTrack = P_GetCDIntermissionTrack();
		}
		else if(!strcmp(songLump, "hall"))
		{
			cdTrack = P_GetCDEnd1Track();
		}
		else if(!strcmp(songLump, "orb"))
		{
			cdTrack = P_GetCDEnd2Track();
		}
		else if(!strcmp(songLump, "chess") && !i_CDTrack)
		{
			cdTrack = P_GetCDEnd3Track();
		}
/*	Uncomment this, if Kevin writes a specific song for startup
		else if(!strcmp(songLump, "start"))
		{
			cdTrack = P_GetCDStartTrack();
		}
*/
		if(!cdTrack || (cdTrack == i_CDCurrentTrack && i_CDMusicLength > 0))
		{
			return;
		}
		if(!I_CDMusPlay(cdTrack))
		{
			if(loop)
			{
				i_CDMusicLength = 35*I_CDMusTrackLength(cdTrack);
				oldTic = gametic;
			}
			else
			{
				i_CDMusicLength = -1;
			}
			i_CDCurrentTrack = cdTrack;
			i_CDTrack = false;
		}
	}
	else
	{
		if(RegisteredSong)
		{
			I_StopSong(RegisteredSong);
			I_UnRegisterSong(RegisteredSong);
			if(UseSndScript)
			{
				Z_Free(Mus_SndPtr);
			}
			else
			{
				Z_ChangeTag(lumpcache[Mus_LumpNum], PU_CACHE);
			}
			#ifdef __WATCOMC__
				_dpmi_unlockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
			#endif
			RegisteredSong = 0;
		}
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, songLump);
			M_ReadFile(name, (byte **)&Mus_SndPtr);
		}
		else
		{
			Mus_LumpNum = W_GetNumForName(songLump);
			Mus_SndPtr = W_CacheLumpNum(Mus_LumpNum, PU_MUSIC);
		}
		#ifdef __WATCOMC__
			_dpmi_lockregion(Mus_SndPtr, lumpinfo[Mus_LumpNum].size);
		#endif
		RegisteredSong = I_RegisterSong(Mus_SndPtr);
		I_PlaySong(RegisteredSong, loop); // 'true' denotes endless looping.
		Mus_Song = -1;
	}
}

//==========================================================================
//
// S_GetSoundID
//
//==========================================================================

int S_GetSoundID(char *name)
{
	int i;

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].tagName, name))
		{
			return i;
		}
	}
	return 0;
}

//==========================================================================
//
// S_StartSound
//
//==========================================================================

void S_StartSound(mobj_t *origin, int sound_id)
{
	S_StartSoundAtVolume(origin, sound_id, 127);
}

//==========================================================================
//
// S_StartSoundAtVolume
//
//==========================================================================

void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume)
{
	int dist, vol;
	int i;
	int priority;
	int sep;
	int angle;
	int absx;
	int absy;

	static int sndcount = 0;
	int chan;

	if(sound_id == 0 || snd_MaxVolume == 0)
		return;
	if(origin == NULL)
	{
		origin = players[displayplayer].mo;
	}
	if(volume == 0)
	{
		return;
	}

	// calculate the distance before other stuff so that we can throw out
	// sounds that are beyond the hearing range.
	absx = abs(origin->x-players[displayplayer].mo->x);
	absy = abs(origin->y-players[displayplayer].mo->y);
	dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
	dist >>= FRACBITS;
	if(dist >= MAX_SND_DIST)
	{
	  return; // sound is beyond the hearing range...
	}
	if(dist < 0)
	{
		dist = 0;
	}
	priority = S_sfx[sound_id].priority;
	priority *= (PRIORITY_MAX_ADJUST-(dist/DIST_ADJUST));
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
		if(origin == Channel[i].mo)
		{ // only allow other mobjs one sound
			S_StopSound(Channel[i].mo);
			break;
		}
	}
	if(i >= snd_Channels)
	{
		for(i = 0; i < snd_Channels; i++)
		{
			if(Channel[i].mo == NULL)
			{
				break;
			}
		}
		if(i >= snd_Channels)
		{
			// look for a lower priority sound to replace.
			sndcount++;
			if(sndcount >= snd_Channels)
			{
				sndcount = 0;
			}
			for(chan = 0; chan < snd_Channels; chan++)
			{
				i = (sndcount+chan)%snd_Channels;
				if(priority >= Channel[i].priority)
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
				if(Channel[i].handle)
				{
					if(I_SoundIsPlaying(Channel[i].handle))
					{
						I_StopSound(Channel[i].handle);
					}
					if(S_sfx[Channel[i].sound_id].usefulness > 0)
					{
						S_sfx[Channel[i].sound_id].usefulness--;
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
		if(UseSndScript)
		{
			char name[128];
			sprintf(name, "%s%s.lmp", ArchivePath, S_sfx[sound_id].lumpname);
			M_ReadFile(name, (byte **)&S_sfx[sound_id].snd_ptr);
		}
		else
		{
			S_sfx[sound_id].snd_ptr = W_CacheLumpNum(S_sfx[sound_id].lumpnum,
				PU_SOUND);
		}
		#ifdef __WATCOMC__
		_dpmi_lockregion(S_sfx[sound_id].snd_ptr,
			lumpinfo[S_sfx[sound_id].lumpnum].size);
		#endif
	}

	vol = (SoundCurve[dist]*(snd_MaxVolume*8)*volume)>>14;
	if(origin == players[displayplayer].mo)
	{
		sep = 128;
//              vol = (volume*(snd_MaxVolume+1)*8)>>7;
	}
	else
	{
		/*angle = R_PointToAngle2(players[displayplayer].mo->x,
			players[displayplayer].mo->y, Channel[i].mo->x, Channel[i].mo->y);
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
					Channel[i].mo->x,
					Channel[i].mo->y);

		if (angle > players[consoleplayer].mo->angle)
		angle = angle - players[consoleplayer].mo->angle;
		else
		angle = angle + (0xffffffff - players[consoleplayer].mo->angle);

		angle >>= ANGLETOFINESHIFT;

		// stereo separation
		sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

//              vol = SoundCurve[dist];
	}

	if(S_sfx[sound_id].changePitch)
	{
		Channel[i].pitch = (byte)(127+(M_Random()&7)-(M_Random()&7));
	}
	else
	{
		Channel[i].pitch = 127;
	}
	Channel[i].handle = I_StartSound(sound_id, S_sfx[sound_id].snd_ptr, vol,
		sep, Channel[i].pitch, 0);
	Channel[i].mo = origin;
	Channel[i].sound_id = sound_id;
	Channel[i].priority = priority;
	Channel[i].volume = volume;
	if(S_sfx[sound_id].usefulness < 0)
	{
		S_sfx[sound_id].usefulness = 1;
	}
	else
	{
		S_sfx[sound_id].usefulness++;
	}
}

//==========================================================================
//
// S_StopSoundID
//
//==========================================================================

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
		if(Channel[i].sound_id == sound_id && Channel[i].mo)
		{
			found++; //found one.  Now, should we replace it??
			if(priority >= Channel[i].priority)
			{ // if we're gonna kill one, then this'll be it
				lp = i;
				priority = Channel[i].priority;
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
	if(Channel[lp].handle)
	{
		if(I_SoundIsPlaying(Channel[lp].handle))
		{
			I_StopSound(Channel[lp].handle);
		}
		if(S_sfx[Channel[lp].sound_id].usefulness > 0)
		{
			S_sfx[Channel[lp].sound_id].usefulness--;
		}
		Channel[lp].mo = NULL;
	}
	return(true);
}

//==========================================================================
//
// S_StopSound
//
//==========================================================================

void S_StopSound(mobj_t *origin)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(Channel[i].mo == origin)
		{
			I_StopSound(Channel[i].handle);
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
			{
				S_sfx[Channel[i].sound_id].usefulness--;
			}
			Channel[i].handle = 0;
			Channel[i].mo = NULL;
		}
	}
}

//==========================================================================
//
// S_StopAllSound
//
//==========================================================================

void S_StopAllSound(void)
{
	int i;

	//stop all sounds
	for(i=0; i < snd_Channels; i++)
	{
		if(Channel[i].handle)
		{
			S_StopSound(Channel[i].mo);
		}
	}
	memset(Channel, 0, 8*sizeof(channel_t));
}

//==========================================================================
//
// S_SoundLink
//
//==========================================================================

void S_SoundLink(mobj_t *oldactor, mobj_t *newactor)
{
	int i;

	for(i=0;i<snd_Channels;i++)
	{
		if(Channel[i].mo == oldactor)
			Channel[i].mo = newactor;
	}
}

//==========================================================================
//
// S_PauseSound
//
//==========================================================================

void S_PauseSound(void)
{
	if(i_CDMusic)
	{
		I_CDMusStop();
	}
	else
	{
		I_PauseSong(RegisteredSong);
	}
}

//==========================================================================
//
// S_ResumeSound
//
//==========================================================================

void S_ResumeSound(void)
{
	if(i_CDMusic)
	{
		I_CDMusResume();
	}
	else
	{
		I_ResumeSong(RegisteredSong);
	}
}

//==========================================================================
//
// S_UpdateSounds
//
//==========================================================================

void S_UpdateSounds(mobj_t *listener)
{
	int i, dist, vol;
	int angle;
	int sep;
	int priority;
	int absx;
	int absy;

	if(i_CDMusic)
	{
		I_UpdateCDMusic();
	}
	if(snd_MaxVolume == 0)
	{
		return;
	}

	// Update any Sequences
	SN_UpdateActiveSequences();

	/*if(NextCleanup < gametic)
	{
		if(UseSndScript)
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					S_sfx[i].usefulness = -1;
				}
			}
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(S_sfx[i].usefulness == 0 && S_sfx[i].snd_ptr)
				{
					if(lumpcache[S_sfx[i].lumpnum])
					{
						if(((memblock_t *)((byte*)
							(lumpcache[S_sfx[i].lumpnum])-
							sizeof(memblock_t)))->id == 0x1d4a11)
						{ // taken directly from the Z_ChangeTag macro
							Z_ChangeTag2(lumpcache[S_sfx[i].lumpnum],
								PU_CACHE);
#ifdef __WATCOMC__
								_dpmi_unlockregion(S_sfx[i].snd_ptr,
									lumpinfo[S_sfx[i].lumpnum].size);
#endif
						}
					}
					S_sfx[i].usefulness = -1;
					S_sfx[i].snd_ptr = NULL;
				}
			}
		}
		NextCleanup = gametic+35*30; // every 30 seconds
	}*/
	for(i=0;i<snd_Channels;i++)
	{
		if(!Channel[i].handle || S_sfx[Channel[i].sound_id].usefulness == -1)
		{
			continue;
		}
		if(!I_SoundIsPlaying(Channel[i].handle))
		{
			if(S_sfx[Channel[i].sound_id].usefulness > 0)
			{
				S_sfx[Channel[i].sound_id].usefulness--;
			}
			Channel[i].handle = 0;
			Channel[i].mo = NULL;
			Channel[i].sound_id = 0;
		}
		if(Channel[i].mo == NULL || Channel[i].sound_id == 0
			|| Channel[i].mo == listener)
		{
			continue;
		}
		else
		{
			absx = abs(Channel[i].mo->x-listener->x);
			absy = abs(Channel[i].mo->y-listener->y);
			dist = absx+absy-(absx > absy ? absy>>1 : absx>>1);
			dist >>= FRACBITS;

			if(dist >= MAX_SND_DIST)
			{
				S_StopSound(Channel[i].mo);
				continue;
			}
			if(dist < 0)
			{
				dist = 0;
			}
			//vol = SoundCurve[dist];
			vol = (SoundCurve[dist]*(snd_MaxVolume*8)*Channel[i].volume)>>14;
			if(Channel[i].mo == listener)
			{
				sep = 128;
			}
			else
			{
				angle = R_PointToAngle2(listener->x, listener->y,
								Channel[i].mo->x, Channel[i].mo->y);
				angle = (angle-viewangle)>>24;
				sep = angle*2-128;
				if(sep < 64)
					sep = -sep;
				if(sep > 192)
					sep = 512-sep;
			}
			I_UpdateSoundParams(Channel[i].handle, vol, sep,
				Channel[i].pitch);
			priority = S_sfx[Channel[i].sound_id].priority;
			priority *= PRIORITY_MAX_ADJUST-(dist/DIST_ADJUST);
			Channel[i].priority = priority;
		}
	}
}

//==========================================================================
//
// S_Init
//
//==========================================================================

void S_Init(void)
{
	SoundCurve = W_CacheLumpName("SNDCURVE", PU_STATIC);
//      SoundCurve = Z_Malloc(MAX_SND_DIST, PU_STATIC, NULL);
	I_StartupSound();
	if(snd_Channels > 8)
	{
		snd_Channels = 8;
	}
	I_SetChannels(snd_Channels);
	I_SetMusicVolume(snd_MusicVolume);

	// Attempt to setup CD music
	/*if(snd_MusicDevice == snd_CDMUSIC)
	{
	   	ST_Message("    Attempting to initialize CD Music: ");
		if(!cdrom)
		{
			i_CDMusic = (I_CDMusInit() != -1);
		}
		else
		{ // The user is trying to use the cdrom for both game and music
			i_CDMusic = false;
		}
	   	if(i_CDMusic)
	   	{
	   		ST_Message("initialized.\n");
	   	}
	   	else
	   	{
	   		ST_Message("failed.\n");
	   	}
	} MIKE*/
}

//==========================================================================
//
// S_GetChannelInfo
//
//==========================================================================

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
		c->id = Channel[i].sound_id;
		c->priority = Channel[i].priority;
		c->name = S_sfx[c->id].lumpname;
		c->mo = Channel[i].mo;
		c->distance = P_AproxDistance(c->mo->x-viewx, c->mo->y-viewy)
			>>FRACBITS;
	}
}

//==========================================================================
//
// S_GetSoundPlayingInfo
//
//==========================================================================

boolean S_GetSoundPlayingInfo(mobj_t *mobj, int sound_id)
{
	int i;

	for(i = 0; i < snd_Channels; i++)
	{
		if(Channel[i].sound_id == sound_id && Channel[i].mo == mobj)
		{
			if(I_SoundIsPlaying(Channel[i].handle))
			{
				return true;
			}
		}
	}
	return false;
}

//==========================================================================
//
// S_SetMusicVolume
//
//==========================================================================

void S_SetMusicVolume(void)
{
	if(i_CDMusic)
	{
		I_CDMusSetVolume(snd_MusicVolume*16); // 0-255
	}
	else
	{
		I_SetMusicVolume(snd_MusicVolume);
	}
	if(snd_MusicVolume == 0)
	{
		if(!i_CDMusic)
		{
			I_PauseSong(RegisteredSong);
		}
		MusicPaused = true;
	}
	else if(MusicPaused)
	{
		if(!i_CDMusic)
		{
			I_ResumeSong(RegisteredSong);
		}
		MusicPaused = false;
	}
}

//==========================================================================
//
// S_ShutDown
//
//==========================================================================

void S_ShutDown(void)
{
	extern int tsm_ID;
	if(tsm_ID != -1)
	{
		I_StopSong(RegisteredSong);
		I_UnRegisterSong(RegisteredSong);
		I_ShutdownSound();
	}
	if(i_CDMusic)
	{
		I_CDMusStop();
	}
}

//==========================================================================
//
// S_InitScript
//
//==========================================================================

void S_InitScript(void)
{
	int p;
	int i;

	strcpy(ArchivePath, DEFAULT_ARCHIVEPATH);
	if(!(p = M_CheckParm("-devsnd")))
	{
		UseSndScript = false;
		SC_OpenLump("sndinfo");
	}
	else
	{
		UseSndScript = true;
		SC_OpenFile(myargv[p+1]);
	}
	while(SC_GetString())
	{
		if(*sc_String == '$')
		{
			if(!stricmp(sc_String, "$ARCHIVEPATH"))
			{
				SC_MustGetString();
				strcpy(ArchivePath, sc_String);
			}
			else if(!stricmp(sc_String, "$MAP"))
			{
				SC_MustGetNumber();
				SC_MustGetString();
				if(sc_Number)
				{
					P_PutMapSongLump(sc_Number, sc_String);
				}
			}
			continue;
		}
		else
		{
			for(i = 0; i < NUMSFX; i++)
			{
				if(!strcmp(S_sfx[i].tagName, sc_String))
				{
					SC_MustGetString();
					if(*sc_String != '?')
					{
						strcpy(S_sfx[i].lumpname, sc_String);
					}
					else
					{
						strcpy(S_sfx[i].lumpname, "default");
					}
					break;
				}
			}
			if(i == NUMSFX)
			{
				SC_MustGetString();
			}
		}
	}
	SC_Close();

	for(i = 0; i < NUMSFX; i++)
	{
		if(!strcmp(S_sfx[i].lumpname, ""))
		{
			strcpy(S_sfx[i].lumpname, "default");
		}
	}
}

//==========================================================================
//
// I_UpdateCDMusic
//
// Updates playing time for current track, and restarts the track, if
// needed
//
//==========================================================================

void I_UpdateCDMusic(void)
{
	extern boolean MenuActive;

	if(MusicPaused || i_CDMusicLength < 0
	|| (paused && !MenuActive))
	{ // Non-looping song/song paused
		return;
	}
	i_CDMusicLength -= gametic-oldTic;
	oldTic = gametic;
	if(i_CDMusicLength <= 0)
	{
		S_StartSong(gamemap, true);
	}
}

/*
============================================================================

							CONSTANTS

============================================================================
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

// boolean grmode;

//==================================================
//
// joystick vars
//
//==================================================

boolean         joystickpresent;
extern  unsigned        joystickx, joysticky;
boolean I_ReadJoystick (void);          // returns false if not connected


//==================================================

#define VBLCOUNTER              34000           // hardware tics to a frame


#define TIMERINT 8
#define KEYBOARDINT 9


#define MOUSEB1 1
#define MOUSEB2 2
#define MOUSEB3 4

boolean mousepresent;
//static  int tsm_ID = -1; // tsm init flag

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

byte        scantokey[128] =
					{
//  0           1       2       3       4       5       6       7
//  8           9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    KEY_BACKSPACE, 9, // 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13 ,    KEY_RCTRL,'a',  's',      // 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	39 ,    '`',    KEY_LSHIFT,92,  'z',    'x',    'c',    'v',      // 2
	'b',    'n',    'm',    ',',    '.',    '/',    KEY_RSHIFT,'*',
	KEY_RALT,' ',   0  ,    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,   // 3
	KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,0  ,    0  , KEY_HOME,
	KEY_UPARROW,KEY_PGUP,'-',KEY_LEFTARROW,'5',KEY_RIGHTARROW,'+',KEY_END, //4
	KEY_DOWNARROW,KEY_PGDN,KEY_INS,KEY_DEL,0,0,             0,              KEY_F11,
	KEY_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7
					};

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

/*
void I_ColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 63);
	}
}
*/

//--------------------------------------------------------------------------
//
// PROC I_UnColorBorder
//
//--------------------------------------------------------------------------

/*
void I_UnColorBorder(void)
{
	int i;

	I_WaitVBL(1);
	_outbyte(PEL_WRITE_ADR, 0);
	for(i = 0; i < 3; i++)
	{
		_outbyte(PEL_DATA, 0);
	}
}
*/

/*
============================================================================

								USER INPUT

============================================================================
*/

//--------------------------------------------------------------------------
//
// PROC I_SetPalette
//
// Palette source must use 8 bit RGB elements.
//
//--------------------------------------------------------------------------

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

/*
============================================================================

							GRAPHICS MODE

============================================================================
*/

/*
==============
=
= I_Update
=
==============
*/

int UpdateState;
extern int screenblocks;

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

/*
void I_ReadScreen(byte *scr)
{
	memcpy(scr, screen, SCREENWIDTH*SCREENHEIGHT);
}
*/

//===========================================================================

/*
===================
=
= I_StartTic
=
// called by D_DoomLoop
// called before processing each tic in a frame
// can call H2_PostEvent
// asyncronous interrupt functions should maintain private ques that are
// read by the syncronous functions to be converted into events
===================
*/

/*
 OLD STARTTIC STUFF

void   I_StartTic (void)
{
	int             k;
	event_t ev;


	I_ReadMouse ();

//
// keyboard events
//
	while (kbdtail < kbdhead)
	{
		k = keyboardque[kbdtail&(KBDQUESIZE-1)];

//      if (k==14)
//              I_Error ("exited");

		kbdtail++;

		// extended keyboard shift key bullshit
		if ( (k&0x7f)==KEY_RSHIFT )
		{
			if ( keyboardque[(kbdtail-2)&(KBDQUESIZE-1)]==0xe0 )
				continue;
			k &= 0x80;
			k |= KEY_RSHIFT;
		}

		if (k==0xe0)
			continue;               // special / pause keys
		if (keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k==0xc5 && keyboardque[(kbdtail-2)&(KBDQUESIZE-1)] == 0x9d)
		{
			ev.type = ev_keydown;
			ev.data1 = KEY_PAUSE;
			H2_PostEvent (&ev);
			continue;
		}

		if (k&0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;
		k &= 0x7f;

		ev.data1 = k;
		//ev.data1 = scantokey[k];

		H2_PostEvent (&ev);
	}
}
*/

#define SC_UPARROW              0x48
#define SC_DOWNARROW    0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW   0x4d

void   I_StartTic (void)
{
}


/*
void   I_ReadKeys (void)
{
	int             k;


	while (1)
	{
	   while (kbdtail < kbdhead)
	   {
		   k = keyboardque[kbdtail&(KBDQUESIZE-1)];
		   kbdtail++;
		   printf ("0x%x\n",k);
		   if (k == 1)
			   I_Quit ();
	   }
	}
}
*/

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
============================================================================

							MOUSE

============================================================================
*/


int I_ResetMouse (void)
{
	return 0;
}



/*
================
=
= StartupMouse
=
================
*/

void I_StartupCyberMan(void);

void I_StartupMouse (void)
{
	mousepresent = 1;
}


/*
================
=
= ShutdownMouse
=
================
*/

void I_ShutdownMouse (void)
{
	if (!mousepresent)
	  return;

	I_ResetMouse ();
}


/*
================
=
= I_ReadMouse
=
================
*/

void I_ReadMouse (void)
{
}

/*
===============
=
= I_StartupJoystick
=
===============
*/

int             basejoyx, basejoyy;

void I_StartupJoystick (void)
{
}

/*
===============
=
= I_JoystickEvents
=
===============
*/

void I_JoystickEvents (void)
{
}

//===========================================================================


/*
===============
=
= I_Init
=
= hook interrupts and set graphics mode
=
===============
*/

void I_Init (void)
{
	extern void I_StartupTimer(void);

//	novideo = M_CheckParm("novideo"); MIKE
	ST_Message("  I_StartupMouse ");
	I_StartupMouse();
//	tprintf("I_StartupJoystick ",1);
//	I_StartupJoystick();
//	tprintf("I_StartupKeyboard ",1);
//	I_StartupKeyboard();
	ST_Message("  S_Init... ");
	S_Init();
	//IO_StartupTimer();
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
	S_ShutDown ();
	I_ShutdownMouse ();
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
	printf ("\n");
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
	D_QuitNetGame();
	M_SaveDefaults();
	I_Shutdown();

//	scr = (byte *)W_CacheLumpName("ENDTEXT", PU_CACHE);
/*
	memcpy((void *)0xb8000, scr, 80*25*2);
	regs.w.ax = 0x0200;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int386(0x10, (const union REGS *)&regs, &regs); // Set text pos
	_settextposition(24, 1);
*/
	printf("\nHexen: Beyond Heretic\n");

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

/* // FUCKED LINES
typedef struct
{
	long    id;
	short   intnum;                 // DOOM executes an int to execute commands

// communication between DOOM and the driver
	short   command;                // CMD_SEND or CMD_GET
	short   remotenode;             // dest for send, set by get (-1 = no packet)
	short   datalength;             // bytes in doomdata to be sent

// info common to all nodes
	short   numnodes;               // console is allways node 0
	short   ticdup;                 // 1 = no duplication, 2-5 = dup for slow nets
	short   extratics;              // 1 = send a backup tic in every packet
	short   deathmatch;             // 1 = deathmatch
	short   savegame;               // -1 = new game, 0-5 = load savegame
	short   episode;                // 1-3
	short   map;                    // 1-9
	short   skill;                  // 1-5

// info specific to this node
	short   consoleplayer;
	short   numplayers;
	short   angleoffset;    // 1 = left, 0 = center, -1 = right
	short   drone;                  // 1 = drone

// packet data to be sent
	doomdata_t      data;
} doomcom_t;
*/ // FUCKED LINES

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
	DPMIInt (doomcom->intnum);
}

//=========================================================================
//
// I_CheckExternDriver
//
//		Checks to see if a vector, and an address for an external driver
//			have been passed.
//=========================================================================

void I_CheckExternDriver(void)
{
	int i;

	if(!(i = M_CheckParm("-externdriver")))
	{
		return;
	}
	i_ExternData = (externdata_t *)atoi(myargv[i+1]);
	i_Vector = i_ExternData->vector;

	useexterndriver = true;
}

//=========================================================================
//=========================================================================
// Hi-Res (mode 12) stuff
//=========================================================================
//=========================================================================


//==========================================================================
//
// SetVideoModeHR - Set video mode to 640x480x16
//
//==========================================================================


void SetVideoModeHR(void)
{
}


//==========================================================================
//
// ClearScreenHR - Clear the screen to color 0
//
//==========================================================================

void ClearScreenHR(void)
{
}


//==========================================================================
//
// SlamHR - copy 4-plane buffer to screen
//
//==========================================================================

void SlamHR(char *buffer)
{

}


//==========================================================================
//
// SlamHR - copy 4-plane buffer to screen
//
// X and Width should be a multiple of 8
// src should be 4 planes of block size, back to back
//==========================================================================

void SlamBlockHR(int x, int y, int w, int h, char *src)
{
	
}

//==========================================================================
//
// InitPaletteHR
//
//==========================================================================

void InitPaletteHR(void)
{
}


//==========================================================================
//
// SetPaletteHR - Set the HR palette
//
//==========================================================================

void SetPaletteHR(byte *palette)
{
}


//==========================================================================
//
// GetPaletteHR - Get the HR palette
//
//==========================================================================

void GetPaletteHR(byte *palette)
{
}


//==========================================================================
//
// FadeToPaletteHR
//
//==========================================================================

void FadeToPaletteHR(byte *palette)
{
}


//==========================================================================
//
// FadeToBlackHR - Fades the palette out to black
//
//==========================================================================

/*
void FadeToBlackHR(void)
{
	char work[16*3];
	char base[16*3];
	int i,j,steps=70;

	GetPaletteHR(base);
	for (i=0; i<steps; i++)
	{
		for (j=0; j<16*3; j++)
		{
			work[j] = base[j]-(base[j]*i/steps);
		}
		VB_SYNC;
		SetPaletteHR(work);
	}
	memset(work,0,16*3);
	SetPaletteHR(work);
}
*/

//==========================================================================
//
// BlackPaletteHR - Instantly blacks out the palette
//
//==========================================================================

void BlackPaletteHR(void)
{
}

//==========================================================================
//
//
// I_StartupReadKeys
//
//
//==========================================================================

void I_StartupReadKeys(void)
{
}


void I_ReadCyberCmd (ticcmd_t *cmd)
{

}


void I_Tactile (int on, int off, int total)
{
}
