/*
 * file sdl_sound.c - sound via SDL3_mixer or bell
 *
 * Program XBLAST 
 * (C) by Oliver Vogel (e-mail: m.vogel@ndh.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will be entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xblast.h"

#include <SDL3_mixer/SDL_mixer.h>

/*
 * local constants
 */

/* values to be used in acknowledge pipe */
#define SND_ACK_OK      0
#define SND_ACK_ERROR   1

typedef short s16;

static struct _sound_name
{
	int sound_id;				/* the sound's id to refer to it */
	const char *name;			/* raw samples data file name */
	uint8_t *samples;			/* pointer to samples memory */
	int length;					/* length in samples of the sound */
	int repeat;					/* repeat flag to play sound endlessly */
	int mono;					/* mono flag indicating mono sounds */
} sound_name[] = {
	{
	SND_BAD, "xb_bad.raw", NULL, 0, XBFalse, XBFalse},	/* got a skull */
	{
	SND_DROP, "xb_drop.raw", NULL, 0, XBFalse, XBTrue},	/* dropped a bomb */
	{
	SND_NEWBOMB, "xbnbmb.raw", NULL, 0, XBFalse, XBTrue},	/* got an extra bomb */
	{
	SND_NEWKICK, "xbnkick.raw", NULL, 0, XBFalse, XBTrue},	/* got kick extra */
	{
	SND_NEWPUMP, "xbnpmp.raw", NULL, 0, XBFalse, XBTrue},	/* got pump extra */
	{
	SND_NEWRC, "xbnrc.raw", NULL, 0, XBFalse, XBTrue},	/* got rem. control */
	{
	SND_MOREFIRE, "xbfire.raw", NULL, 0, XBFalse, XBTrue},	/* got more range */
	{
	SND_DEAD, "xb_dead.raw", NULL, 0, XBFalse, XBFalse},	/* player died */
	{
	SND_EXPL, "xb_expl.raw", NULL, 0, XBFalse, XBTrue},	/* normal explosion */
	{
	SND_KICK, "xb_kick.raw", NULL, 0, XBFalse, XBTrue},	/* kick a bomb */
	{
	SND_PUMP, "xb_pump.raw", NULL, 0, XBFalse, XBTrue},	/* pump a bomb */
	{
	SND_OUCH, "xb_ouch.raw", NULL, 0, XBFalse, XBFalse},	/* player lost life */
	{
	SND_INTRO, "xb_intro.raw", NULL, 0, XBFalse, XBFalse},	/* intro fanfare */
	{
	SND_APPL, "xb_appl.raw", NULL, 0, XBFalse, XBFalse},	/* applause */
	{
	SND_APPL2, "xb_app2.raw", NULL, 0, XBFalse, XBFalse},	/* applause */
	{
	SND_BUTT, "xb_butt.raw", NULL, 0, XBFalse, XBTrue},	/* triggered button */
	{
	SND_SHOOT, "xb_shoot.raw", NULL, 0, XBFalse, XBFalse},	/* using rem. ctrl. */
	{
	SND_INVIS, "xb_nvis.raw", NULL, 0, XBFalse, XBFalse},	/* player invisible */
	{
	SND_INVINC, "xb_nvnc.raw", NULL, 0, XBFalse, XBFalse},	/* player invincible */
	{
	SND_NEWTELE, "xbntel.raw", NULL, 0, XBFalse, XBTrue},	/* player got telep. */
	{
	SND_TELE, "xbtele.raw", NULL, 0, XBFalse, XBTrue},	/* player uses tele. */
	{
	SND_INJ, "xbinj.raw", NULL, 0, XBFalse, XBFalse},	/* player got junkie */
	{
	SND_MINIBOMB, "xbmbmb.raw", NULL, 0, XBFalse, XBTrue},	/* small bomb expl. */
	{
	SND_WON, "xb_won.raw", NULL, 0, XBFalse, XBFalse},	/* player won */
	{
	SND_HAUNT, "xb_haunt.raw", NULL, 0, XBFalse, XBFalse},	/* haunting bomb */
	{
	SND_SPIRAL, "xb_spir.raw", NULL, 0, XBFalse, XBTrue},	/* spiral shrinking */
	{
	SND_SPBOMB, "xb_spbmb.raw", NULL, 0, XBFalse, XBTrue},	/* got special bomb */
	{
	SND_SLIDE, "xbslide.raw", NULL, 0, XBFalse, XBTrue},	/* bomb slide sound */
	{
	SND_FINALE, "xbfin.raw", NULL, 0, XBFalse, XBFalse},	/* final fanfare */
	{
	SND_WARN, "xb_warn.raw", NULL, 0, XBFalse, XBFalse},	/* shrink warn sound */
	{
	SND_STUN, "xb_stun.raw", NULL, 0, XBFalse, XBFalse},	/* player stun sound */
	{
	SND_WHIRL, "xb_whrl.raw", NULL, 0, XBTrue, XBFalse},	/* intro whirl */
	{
	SND_COMPOUND, "xb_cmpnd.raw", NULL, 0, XBFalse, XBFalse},	/* compound shrink */
	{
	SND_TELE1, "xbtele1.raw", NULL, 0, XBFalse, XBTrue},	/* teleport start */
	{
	SND_TELE2, "xbtele2.raw", NULL, 0, XBFalse, XBTrue},	/* teleport end */
	{
	SND_HOLY, "xbholy.raw", NULL, 0, XBFalse, XBFalse},	/* holy grail extra */
	{
	SND_ENCLOAK, "xbcloak.raw", NULL, 0, XBFalse, XBTrue},	/* encloak sound */
	{
	SND_DECLOAK, "xbdcloak.raw", NULL, 0, XBFalse, XBTrue},	/* decloak sound */
	{
	SND_FAST, "xbfast.raw", NULL, 0, XBFalse, XBTrue},	/* speed up extra */
	{
	SND_SLOW, "xbslow.raw", NULL, 0, XBFalse, XBTrue},	/* slow down extra */
	{
	SND_SLAY, "xbslay.raw", NULL, 0, XBFalse, XBTrue},	/* slay extra */
	{
	SND_LIFE, "xblife.raw", NULL, 0, XBFalse, XBTrue},	/* extra life */
	{
	SND_NEWCLOAK, "xbcloakx.raw", NULL, 0, XBFalse, XBTrue},	/* new cloak extra */
	{
	SND_BOMBMORPH, "xb_bombmorph.raw", NULL, 0, XBFalse, XBTrue},	/* bomb morph */
	{
	SND_STEP1, "xbstep1.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #1 */
	{
	SND_STEP2, "xbstep2.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #2 */
	{
	SND_STEP3, "xbstep3.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #3 */
	{
	SND_STEP4, "xbstep4.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #4 */
	{
	SND_STEP5, "xbstep5.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #5 */
	{
	SND_STEP6, "xbstep6.raw", NULL, 0, XBFalse, XBTrue},	/* Backgr. song #6 */
	{
	SND_SNG1, "xbsng1.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #1 */
	{
	SND_SNG2, "xbsng2.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #2 */
	{
	SND_SNG3, "xbsng3.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #3 */
	{
	SND_SNG4, "xbsng4.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #4 */
	{
	SND_SNG5, "xbsng5.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #5 */
	{
	SND_SNG6, "xbsng6.raw", NULL, 0, XBTrue, XBFalse},	/* Backgr. song #6 */
	{
	SND_MAX, NULL, NULL, 0, XBFalse, XBFalse}
};

/*
 * The game can play the same sound several times at once (many bombs, all
 * players walking, ...), so keep a pool of tracks, like the old
 * Mix_AllocateChannels(20) did.
 */
#define SND_CHANNELS 20

/*
 * local variables
 */
static MIX_Mixer *mixer = NULL;
static MIX_Audio *sound_audio[SND_MAX];
static MIX_Track *sound_track[SND_CHANNELS];
static SND_Id sound_owner[SND_CHANNELS];
static unsigned sound_next = 0;

/*
 * audio format used for the mixer and for loading the raw sounds; the
 * number of channels depends on the mono/stereo setting (see SND_Init)
 */
static SDL_AudioSpec rawSpec = {
	SDL_AUDIO_U8, 2, 22050
};

static int mono_mode = XBFalse;

static XBBool isActive = XBFalse;
static XBBool boom = XBFalse;

static XBBool soundInitialized = XBFalse;

/*
 * Open Audio 
 */
XBBool
SND_Init (const CFGSoundSetup * setup)
{
	int i;

	assert (NULL != setup);

	/* mode selection */
	switch (setup->mode) {

	case XBSM_Waveout:
		isActive = XBTrue;
		break;

	case XBSM_Beep:
		isActive = XBTrue;
		return XBTrue;

	case XBSM_None:
		/* no sound at all */
		isActive = XBFalse;
		return XBTrue;

	default:
		return XBFalse;
	}

	/* select mono or stereo output; the raw samples are converted to
	   match this in SND_Load() */
	mono_mode = setup->stereo ? XBFalse : XBTrue;
	rawSpec.channels = mono_mode ? 1 : 2;

	/* Initialize SDL_mixer stuff */
	if (!MIX_Init ()) {
		fprintf (stderr, "Error: Couldn't initialize SDL_mixer\n"
				 "Possible reason: %s\n", SDL_GetError ());
		return XBFalse;
	}

	mixer = MIX_CreateMixerDevice (SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &rawSpec);
	if (NULL == mixer) {
		fprintf (stderr, "Error: Couldn't initializate audio\n"
				 "Possible reason: %s\n", SDL_GetError ());
		MIX_Quit ();
		return XBFalse;
	}

	/* reset sound stuff */
	for (i = 0; i < SND_MAX; i++) {
		sound_audio[i] = NULL;
	}
	for (i = 0; i < SND_CHANNELS; i++) {
		sound_track[i] = MIX_CreateTrack (mixer);
		sound_owner[i] = SND_MAX;
	}
	sound_next = 0;

	soundInitialized = XBTrue;

	return XBTrue;

}								/* SND_Init */

/*
 * stop all tracks that currently play sound id
 */
static void
StopSoundTracks (SND_Id id)
{
	int i;

	for (i = 0; i < SND_CHANNELS; i++) {
		if (NULL == sound_track[i]) {
			continue;
		}
		if (sound_owner[i] == id) {
			MIX_StopTrack (sound_track[i], 0);
			MIX_SetTrackAudio (sound_track[i], NULL);
			sound_owner[i] = SND_MAX;
		}
	}
}								/* StopSoundTracks */

/*
 * Stop Audio
 */
XBBool
SND_Stop (SND_Id id)
{
	int i;

	if (soundInitialized) {
		if (id == STOP_ALL_SOUNDS) {
			MIX_StopAllTracks (mixer, 0);
			for (i = 0; i < SND_CHANNELS; i++) {
				sound_owner[i] = SND_MAX;
			}
		}
		else {					// stop specific sound
			StopSoundTracks (id);
		}
	}
	return XBTrue;
}								/* SND_Stop */

/*
 * Play sound and set right position,
 */
XBBool
SND_Play (SND_Id id, int position)
{
	if (soundInitialized && NULL != sound_audio[id]) {

		// do a stereo effect
		int col1 = 255 - ((position * 255) / MAX_SOUND_POSITION);
		int col2 = (position * 255) / MAX_SOUND_POSITION;
		MIX_StereoGains gains;
		int slot = -1;
		int i;

		if (NULL == mixer) {
			return XBTrue;
		}
		/* look for a track that is free or has finished playing */
		for (i = 0; i < SND_CHANNELS; i++) {
			if (NULL != sound_track[i] && !MIX_TrackPlaying (sound_track[i])) {
				slot = i;
				break;
			}
		}
		/* all tracks are busy: steal one, round robin */
		if (slot < 0) {
			slot = (int)sound_next;
			sound_next = (sound_next + 1) % SND_CHANNELS;
			if (NULL == sound_track[slot]) {
				return XBTrue;
			}
			MIX_StopTrack (sound_track[slot], 0);
		}

		gains.left = (float)col2 / 255.0f;
		gains.right = (float)col1 / 255.0f;

		/* Panning must be after the audio is set, since we don't
		 * know the track properties before */
		if (!MIX_SetTrackAudio (sound_track[slot], sound_audio[id])) {
			fprintf (stderr, "MIX_SetTrackAudio(%d) failed: %s\n", id, SDL_GetError ());
			return XBTrue;
		}
		if (!MIX_SetTrackStereo (sound_track[slot], &gains)) {
			fprintf (stderr, "MIX_SetTrackStereo(%d, %d, %d) failed!\n", id, col1, col2);
			fprintf (stderr, "Reason: [%s].\n", SDL_GetError ());
		}
		if (!MIX_PlayTrack (sound_track[slot], 0)) {
			fprintf (stderr, "MIX_PlayTrack(%d) failed: %s\n", id, SDL_GetError ());
			return XBTrue;
		}
		sound_owner[slot] = id;
	}
	else {
		/* beep at next frame */
		if (SND_MINIBOMB == id || SND_EXPL == id) {
			boom = XBTrue;
		}
	}
	return XBTrue;
}								/* SND_Play */

/*
 * beep once
 */
void
SND_Beep (void)
{
	/* TODO 
	   XBell (dpy, 80);
	 */
}								/* SND_Beep */

/*
 * Load sound
 */
XBBool
SND_Load (SND_Id id)
{
	if (soundInitialized) {
		/* First load RAW into memory */
		char fname[1000];
		int i, f;
		static char *path_list[3] = {
			NULL,
			GAME_DATADIR "/",
			".",
		};

		/* a previous version of this sound must be released first */
		StopSoundTracks (id);
		if (NULL != sound_audio[id]) {
			MIX_DestroyAudio (sound_audio[id]);
			sound_audio[id] = NULL;
		}
		if (NULL != sound_name[id].samples) {
			free (sound_name[id].samples);
			sound_name[id].samples = NULL;
			sound_name[id].length = 0;
		}

		/* check environment for xblast search path */
		path_list[0] = getenv ("XBLASTDIR");

		for (i = 0; i < 3; i++) {
			if (path_list[i] != NULL) {
				sprintf (fname, "%s/%s/%s", path_list[i], "sounds", sound_name[id].name);
				if ((f = open (fname, O_RDONLY)) >= 0) {
					int sound_size;
					uint8_t *sb, *sb1;
					struct stat snd_stat;

#ifdef DEBUG
					fprintf (stderr, "Opened file \"%s\".\n", fname);
#endif
					(void)fstat (f, &snd_stat);
					sound_size = snd_stat.st_size / sizeof (uint8_t);

					if ((sb = malloc (sound_size * sizeof (uint8_t))) == NULL) {
						close (f);
						return (-1);
					}
					else {
						read (f, sb, sound_size * sizeof (uint8_t));
						close (f);
#if defined(SERVER_STATISTICS)
						total_loaded++;
#endif
						/* make sound Stereo although mono... blame SDL_mixer
						   same problem as the 2 channels issue
						 */
						if (mono_mode != XBTrue && sound_name[id].mono == XBTrue) {
							int k;
							if ((sb1 = malloc (2 * sound_size * sizeof (uint8_t))) == NULL) {
								free (sb);
								return (-1);
							}
							for (k = 0; k < sound_size; k++) {
								sb1[2 * k] = sb1[2 * k + 1] = sb[k];

							}
							/* we free sb afterwards so we need
							   to pass sb1 to sb */
							free (sb);
							sb = sb1;
							sound_size *= 2;
						}
						sound_name[id].samples = sb;
						sound_name[id].length = sound_size;
						/*
						 * convert stereo samples to mono if running in mono mode 
						 */

						if (mono_mode == XBTrue && sound_name[id].mono == XBFalse) {
							int k;
							uint8_t *m, *s;
							s16 sum;

							m = s = sound_name[id].samples;

							sound_name[id].length >>= 1;
							for (k = 0; k < sound_name[id].length; k++) {
								sum = *s + *(s + 1);
								*m++ = sum >> 1;
								s += 2;
							}
						}
						/* the mixer does not take ownership of the samples,
						   they are freed in SND_Unload */
						sound_audio[id] =
							MIX_LoadRawAudioNoCopy (mixer, sb, sound_name[id].length,
													&rawSpec, false);
						if (NULL == sound_audio[id]) {
							fprintf (stderr, "Warning: Could not open RAW from memory: %s\n",
									 SDL_GetError ());
						}

						break;
					}
				}
				else {
					fprintf (stderr, "Couldnt open file \"%s\".\n", fname);
				}
			}
		}

	}
	return XBTrue;
}								/* SND_Load */

/*
 *
 */
XBBool
SND_Unload (SND_Id id)
{
	if (soundInitialized) {
		StopSoundTracks (id);
		if (NULL != sound_audio[id]) {
			MIX_DestroyAudio (sound_audio[id]);
			sound_audio[id] = NULL;
		}
		if (NULL != sound_name[id].samples) {
			free (sound_name[id].samples);
			sound_name[id].samples = NULL;
			sound_name[id].length = 0;
		}
	}
	return XBTrue;
}								/* SND_Unload */

/*
 *
 */
void
SND_Flush (void)
{
	if (isActive && boom) {
		boom = XBFalse;
	}
}								/* SND_Flush */

/*
 *
 */
void
SND_Finish (void)
{
	int i;

	if (NULL != mixer) {
		for (i = 0; i < SND_CHANNELS; i++) {
			if (NULL != sound_track[i]) {
				MIX_DestroyTrack (sound_track[i]);
				sound_track[i] = NULL;
			}
			sound_owner[i] = SND_MAX;
		}
		for (i = 0; i < SND_MAX; i++) {
			if (NULL != sound_audio[i]) {
				MIX_DestroyAudio (sound_audio[i]);
				sound_audio[i] = NULL;
			}
			if (NULL != sound_name[i].samples) {
				free (sound_name[i].samples);
				sound_name[i].samples = NULL;
				sound_name[i].length = 0;
			}
		}
		MIX_DestroyMixer (mixer);
		mixer = NULL;
	}
	MIX_Quit ();

	soundInitialized = XBFalse;
}								/* SND_Finish */

/*
 * end of file sdl_sound.c
 */
