
// I_SOUND.C

#include <stdio.h>
#include "h2def.h"
#include "sounds.h"
#include "i_sound.h"

#define SAMPLERATE			11025
#define SAMPLECOUNT			315//(SAMPLERATE/TICRATE)
#define SOUNDBUFFERSIZE		(SAMPLECOUNT*2*4)
#define NUM_CHANNELS		8

#define SOUND_AMPLIFY		(0.5f/32768.0f)

#define ntohl(x) \
        ((unsigned long int)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                             (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                             (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                             (((unsigned long int)(x) & 0xff000000U) >> 24)))

const char snd_prefixen[] = { 'P', 'P', 'A', 'S', 'S', 'S', 'M',
  'M', 'M', 'S' };

int snd_Channels;
int snd_DesiredMusicDevice, snd_DesiredSfxDevice;
int snd_MusicDevice,    // current music card # (index to dmxCodes)
	snd_SfxDevice,      // current sfx card # (index to dmxCodes)
	snd_MaxVolume,      // maximum volume for sound
	snd_MusicVolume;    // maximum volume for music

int		volume_lookup[128][256];
int		audiohandle;

float soundBuffer[SOUNDBUFFERSIZE];

extern sfxinfo_t S_sfx[];

int steptable[256];

typedef struct
{
	boolean	active;

	int		handle;
	unsigned char*	data;
	unsigned char*	data_end;

	int		step;
	int		step_remainder;

	int		id;
	int		volume;
	int		separation;
	int		pitch;
	int		priority;

	int		leftvolume;
	int		rightvolume;
} audiochannel_t;

#define NUM_CHANNELS 8

audiochannel_t audiochannels[NUM_CHANNELS];

void I_PauseSong(int handle)
{
}

void I_ResumeSong(int handle)
{
}

void I_SetMusicVolume(int volume)
{
	snd_MusicVolume = volume;
}

void I_SetSfxVolume(int volume)
{
	snd_MaxVolume = volume;
}

/*
 *
 *                              SONG API
 *
 */

int I_RegisterSong(void *data)
{
  return 0;
}

void I_UnRegisterSong(int handle)
{
}

int I_QrySongPlaying(int handle)
{
  return 0;
}

// Stops a song.  MUST be called before I_UnregisterSong().

void I_StopSong(int handle)
{
}

void I_PlaySong(int handle, int looping)
{

}

/*
 *
 *                                 SOUND FX API
 *
 */

// Gets lump nums of the named sound.  Returns pointer which will be
// passed to I_StartSound() when you want to start an SFX.  Must be
// sure to pass this to UngetSoundEffect() so that they can be
// freed!


static char sfxname[10];

int I_GetSfxLumpNum(sfxinfo_t *sound)
{
	if ( W_CheckNumForName(sound->lumpname) == -1 )
		return W_GetNumForName("default");

	return W_GetNumForName(sound->lumpname);
}

void I_CalculateSoundParameters(audiochannel_t* audiochannel)
{
	int sep = audiochannel->separation+1;

	// sanity: sep from 1-256
	if(sep<1)sep = 1;
	if(sep>256)sep = 256;

    audiochannel->leftvolume = audiochannel->volume - ((audiochannel->volume*sep*sep) >> 16);
    sep -= 257;
    audiochannel->rightvolume = audiochannel->volume - ((audiochannel->volume*sep*sep) >> 16);	
}

int I_StartSound (int id, void *data, int vol, int sep, int pitch, int priority)
{
/*
  // hacks out certain PC sounds
  if (snd_SfxDevice == PC
	&& (data == S_sfx[sfx_posact].data
	||  data == S_sfx[sfx_bgact].data
	||  data == S_sfx[sfx_dmact].data
	||  data == S_sfx[sfx_dmpain].data
	||  data == S_sfx[sfx_popain].data
	||  data == S_sfx[sfx_sawidl].data)) return -1;

  else
		*/

	if(id >= W_NumLumps())return 0;

	int i;
	int lowpri = MAXINT;
	int i_lowpri;
	for(i=0; i<NUM_CHANNELS; i++)
	{
		if(!audiochannels[i].active)
			break;

		if(audiochannels[i].priority < lowpri)
		{
			i_lowpri = i;
			lowpri = audiochannels[i].priority;
		}
	}

	if(i >= NUM_CHANNELS)
		i = i_lowpri;

	audiochannels[i].active = true;
	audiochannels[i].id = id;
	audiochannels[i].data = data+8;
	audiochannels[i].volume = vol;
	audiochannels[i].separation = sep;
	audiochannels[i].pitch = pitch;
	audiochannels[i].priority = priority;
	audiochannels[i].step = steptable[pitch];
	audiochannels[i].step_remainder = 0;
	audiochannels[i].handle = audiohandle++;

	audiochannels[i].data_end = data+W_LumpLength( S_sfx[id].lumpnum );

	I_CalculateSoundParameters(&audiochannels[i]);

	return audiochannels[i].handle;

}

void I_StopSound(int handle)
{
	int i;
	for(i=0; i<NUM_CHANNELS; i++)
	{
		if(audiochannels[i].handle == handle)
		{
			audiochannels[i].active = false;
			return;
		}
	}
}

int I_SoundIsPlaying(int handle)
{
	int i;
	for(i=0; i<NUM_CHANNELS; i++)
	{
		if(audiochannels[i].handle == handle && audiochannels[i].active)
		{
			return 1;
		}
	}

  return 0;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	int i;
  for(i=0; i<NUM_CHANNELS; i++)
	{
		if(audiochannels[i].handle == handle && audiochannels[i].active)
		{
			audiochannels[i].volume = vol;
			audiochannels[i].separation = sep;
			audiochannels[i].pitch = pitch;

			I_CalculateSoundParameters(&audiochannels[i]);

			return;
		}
	}
}

void I_UpdateSound( void )
{ 

  // Mix current sound data.
  // Data, from raw sound, for right and left.
  register unsigned char	sample;
  register int		dl;
  register int		dr;
  float fl;
  float fr;
int v;
  float*		leftout;
  float*		rightout;
  float*		leftend;

  unsigned int* flp;
  unsigned int* frp;

  // Mixing channel index.
  int				i;

    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = soundBuffer;
    rightout = soundBuffer+1;

    leftend = soundBuffer + SOUNDBUFFERSIZE;

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT,
    //  that is 512 values for two channels.
    while (leftout != leftend)
    {
		// Reset left/right value. 
		dl = 0;
		dr = 0;

		// Love thy L2 chache - made this a loop.
		// Now more channels could be set at compile time
		//  as well. Thus loop those  channels.
		for (i = 0; i < NUM_CHANNELS; i++ )
		{
			if (audiochannels[i].active)
			{
				sample = *audiochannels[ i ].data;
				// Add left and right part
				//  for this channel (sound)
				//  to the current data.
				// Adjust volume accordingly.

				dl += volume_lookup[audiochannels[i].leftvolume][sample];
				dr += volume_lookup[audiochannels[i].rightvolume][sample];

				audiochannels[i].step_remainder += audiochannels[i].step;
				audiochannels[i].data += audiochannels[i].step_remainder >> 16;
				audiochannels[i].step_remainder &= 0xffff;

				if(audiochannels[i].data >= audiochannels[i].data_end)
					audiochannels[i].active = false;
			}
		}

		// amplify
		fl = SOUND_AMPLIFY * (float)dl;
		if (fl > 1.0f)
			fl = 1.0f;
		else if(fl < -1.0f)
			fl = -1.0f;

		fr = SOUND_AMPLIFY*(float)dr;
		if(fr > 1.0f)
			fr = 1.0f;
		else if(fr < -1.0f)
			fr = -1.0f;

		flp = (unsigned int*)&fl;
		frp = (unsigned int*)&fr;

		*flp = ntohl(*flp);
		*frp = ntohl(*frp);

		// MIKE
		for(i=0; i<4; i++)
		{
			*leftout = fl;
			*rightout = fr;

			// Increment current pointers in mixbuffer.
			leftout += 2;
			rightout += 2;
		}
	}
}

/*
 *
 *                                                      SOUND STARTUP STUFF
 *
 *
 */

// inits all sound stuff

void I_StartupSound (void)
{
	int i, j;
	for (i=0 ; i<128 ; i++)
		for (j=0 ; j<256 ; j++)
			volume_lookup[i][j] = (i*(j-128)*256)/127;

	
	for (i=0 ; i<256 ; i++)
		steptable[i] = (int)(pow(2.0, ((i-127)/64.0))*65536.0);

	audiohandle = 0;

	snd_Channels = 8;
}

// shuts down all sound stuff

void I_ShutdownSound (void)
{
}

void I_SetChannels(int channels)
{
}

int I_CDMusInit(void)
{
	return 0;
}

int I_CDMusPlay(int track)
{
	return 0;
}

int I_CDMusStop(void)
{
	return 0;
}

int I_CDMusResume(void)
{
	return 0;
}

int I_CDMusSetVolume(int volume)
{
	return 0;
}

int I_CDMusFirstTrack(void)
{
	return 0;
}

int I_CDMusLastTrack(void)
{
	return 0;
}

int I_CDMusTrackLength(int track)
{
	return 0;
}
