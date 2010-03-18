#ifndef __SOUND__
#define __SOUND__

void I_PauseSong(int handle);
void I_ResumeSong(int handle);
void I_SetMusicVolume(int volume);
void I_SetSfxVolume(int volume);
int I_RegisterSong(void *data);
void I_UnRegisterSong(int handle);
int I_QrySongPlaying(int handle);
void I_StopSong(int handle);
void I_PlaySong(int handle, int looping);
int I_GetSfxLumpNum(sfxinfo_t *sound);
int I_StartSound (int id, void *data, int vol, int sep, int pitch, int priority);
void I_StopSound(int handle);
int I_SoundIsPlaying(int handle);
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);
void I_sndArbitrateCards(void);
void I_StartupSound (void);
void I_ShutdownSound (void);
void I_SetChannels(int channels);

#endif

#ifndef __ICDMUS__
#define __ICDMUS__

#define CDERR_NOTINSTALLED   10		// MSCDEX not installed
#define CDERR_NOAUDIOSUPPORT 11		// CD-ROM Doesn't support audio
#define CDERR_NOAUDIOTRACKS  12		// Current CD has no audio tracks
#define CDERR_BADDRIVE       20		// Bad drive number
#define CDERR_BADTRACK       21		// Bad track number
#define CDERR_IOCTLBUFFMEM   22		// Not enough low memory for IOCTL
#define CDERR_DEVREQBASE     100	// DevReq errors

extern int cd_Error;

int I_CDMusInit(void);
int I_CDMusPlay(int track);
int I_CDMusStop(void);
int I_CDMusResume(void);
int I_CDMusSetVolume(int volume);
int I_CDMusFirstTrack(void);
int I_CDMusLastTrack(void);
int I_CDMusTrackLength(int track);

#endif

