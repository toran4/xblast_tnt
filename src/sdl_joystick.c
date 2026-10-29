/*
 * file sdl_joystick.c - joystick support for linux
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

#include <SDL3/SDL.h>

#include "sdl_joystick.h"

#define JOYDIR_NONE  0x00
#define JOYDIR_UP    0x01
#define JOYDIR_DOWN  0x02
#define JOYDIR_Y     0x03
#define JOYDIR_LEFT  0x04
#define JOYDIR_RIGHT 0x08
#define JOYDIR_X     0x0C

#define THRESHOLD    4096
// old value: 8192

/*
 * local types
 */
typedef struct
{
	SDL_Joystick *joy;
	SDL_JoystickID id;			/* SDL instance id, -1 if unused */
	XBEventCode event;
	unsigned dir;
} XBJoystick;

/*
 * local variables
 */
static XBJoystick joystick[NUM_JOYSTICKS] = {
	{NULL, -1, XBE_NONE, 0},
	{NULL, -1, XBE_NONE, 0},
	{NULL, -1, XBE_NONE, 0},
	{NULL, -1, XBE_NONE, 0},
	{NULL, -1, XBE_NONE, 0},
	{NULL, -1, XBE_NONE, 0},
};

/*
 * translate an SDL joystick instance id to our slot index
 */
static int
JoystickIndex (SDL_JoystickID id)
{
	int i;

	for (i = 0; i < NUM_JOYSTICKS; i++) {
		if (NULL != joystick[i].joy && joystick[i].id == id) {
			return i;
		}
	}
	return -1;
}								/* JoystickIndex */

/*
 * global function: GUI_NumJoysticks
 * description:     Query number of connected joysticks
 * return value:    # joysticks
 */
int
GUI_NumJoysticks (void)
{
	SDL_JoystickID *joys;
	int count = 0;

	joys = SDL_GetJoysticks (&count);
	if (NULL != joys) {
		SDL_free (joys);
	}
	/* there are only event codes for four joystick players */
	if (count > 4) {
		count = 4;
	}
	return count;
}								/* GUI_NumJoysticks */

/*
 * Initialize 
 */
XBBool
InitJoystick (void)
{
	SDL_JoystickID *joys;
	int count, i, j;

	if (!SDL_InitSubSystem (SDL_INIT_JOYSTICK)) {
		/* joysticks are optional, the game is perfectly playable without */
		fprintf (stderr, "Warning: no joystick support: %s\n", SDL_GetError ());
		return XBTrue;
	}

	joys = SDL_GetJoysticks (&count);
	if (NULL == joys) {
		count = 0;
	}
	for (i = 0, j = 0; i < count && j < NUM_JOYSTICKS && j < 4; i++) {
		joystick[j].joy = SDL_OpenJoystick (joys[i]);
		if (joystick[j].joy) {
			joystick[j].id = joys[i];
			joystick[j].event = XBE_JOYST_1 + j;
			printf ("Using joystick %d\n", j);
			j++;
		}
		else
			printf ("Couldn't open joystick %d\n", i);
	}
	if (NULL != joys) {
		SDL_free (joys);
	}

	return XBTrue;
}								/* InitJoystick */

/*
 * shutdown
 */
void
FinishJoystick (void)
{
	int i;
	for (i = 0; i < NUM_JOYSTICKS; i++) {
		if (joystick[i].joy != NULL) {
			printf ("Closing joystick %d\n", i);
			SDL_CloseJoystick (joystick[i].joy);
			joystick[i].joy = NULL;
			joystick[i].id = -1;
		}
	}
}								/* FinishJoystick */

/*
 * calculate new joystick direction from axis-event
 */
static unsigned
EvalJoystickMove (unsigned dir, unsigned axis, int value)
{
	unsigned newDir = dir;

	switch (axis) {
		/* x-Axis */
	case 0:
		newDir &= ~JOYDIR_X;
		if (value > THRESHOLD)
			newDir |= JOYDIR_RIGHT;
		else if (value < -THRESHOLD)
			newDir |= JOYDIR_LEFT;
		break;
		/* y-axis */
	case 1:
		newDir &= ~JOYDIR_Y;
		if (value > THRESHOLD)
			newDir |= JOYDIR_DOWN;
		else if (value < -THRESHOLD)
			newDir |= JOYDIR_UP;
		break;
	default:
		break;
	}
#ifdef DEBUG_JOYSTICK
	Dbg_Out ("joy move: axis=%u value=%d => %02x\n", axis, value, newDir);
#endif
	return newDir;
}								/* EvalJoystickMove */

/*
 * handle new joystick event during game
 */
void
HandleXBlastJoystick (SDL_Event * event)
{
	unsigned newDir;
	int value;
	int which;

	switch (event->type) {
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
		which = JoystickIndex (event->jaxis.which);
		if (which < 0) {
			break;
		}
		newDir = EvalJoystickMove (joystick[which].dir,
								   event->jaxis.axis, event->jaxis.value);
		switch (newDir) {
		case JOYDIR_UP:
			value = XBGK_GO_UP;
			break;
		case JOYDIR_DOWN:
			value = XBGK_GO_DOWN;
			break;
		case JOYDIR_LEFT:
			value = XBGK_GO_LEFT;
			break;
		case JOYDIR_RIGHT:
			value = XBGK_GO_RIGHT;
			break;
		case JOYDIR_NONE:
			value = XBGK_STOP_ALL;
			break;
		default:
			value = XBGK_NONE;
			break;
		}
		if (value != XBGK_NONE && newDir != joystick[which].dir) {
			QueueEventValue (joystick[which].event, value);
			joystick[which].dir = newDir;
		}
		break;

	case SDL_EVENT_JOYSTICK_HAT_MOTION:
		which = JoystickIndex (event->jhat.which);
		if (which < 0) {
			break;
		}
		if (event->jhat.value & SDL_HAT_UP) {
			QueueEventValue (joystick[which].event, XBGK_GO_UP);
			joystick[which].dir |= JOYDIR_UP;
		}
		if (event->jhat.value & SDL_HAT_DOWN) {
			QueueEventValue (joystick[which].event, XBGK_GO_DOWN);
			joystick[which].dir |= JOYDIR_DOWN;
		}
		if (event->jhat.value & SDL_HAT_RIGHT) {
			QueueEventValue (joystick[which].event, XBGK_GO_RIGHT);
			joystick[which].dir |= JOYDIR_RIGHT;
		}
		if (event->jhat.value & SDL_HAT_LEFT) {
			QueueEventValue (joystick[which].event, XBGK_GO_LEFT);
			joystick[which].dir |= JOYDIR_LEFT;
		}
		if (SDL_HAT_CENTERED == event->jhat.value) {
			QueueEventValue (joystick[which].event, XBGK_STOP_ALL);
			joystick[which].dir = JOYDIR_NONE;
		}
		break;

	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		which = JoystickIndex (event->jbutton.which);
		if (which < 0) {
			break;
		}
		if (event->jbutton.button == 0)
			QueueEventValue (joystick[which].event, XBGK_BOMB);
		if (event->jbutton.button == 1)
			QueueEventValue (joystick[which].event, XBGK_SPECIAL);
		break;
	}
}								/* HandleJoystick */

/*
 * handle new joystick event in menus
 */
void
HandleMenuJoystick (SDL_Event * event)
{
	unsigned newDir;
	int which;

	switch (event->type) {
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
		which = JoystickIndex (event->jaxis.which);
		if (which < 0) {
			break;
		}
		newDir = EvalJoystickMove (joystick[which].dir,
								   event->jaxis.axis, event->jaxis.value);
		/* test changes in y dir */
		if ((newDir & JOYDIR_Y) != (joystick[which].dir & JOYDIR_Y)) {
			switch (newDir & JOYDIR_Y) {
			case JOYDIR_UP:
				QueueEventValue (XBE_MENU, XBMK_UP);
				break;
			case JOYDIR_DOWN:
				QueueEventValue (XBE_MENU, XBMK_DOWN);
				break;
			default:
				break;
			}
		}
		/* test changes in x dir */
		if ((newDir & JOYDIR_X) != (joystick[which].dir & JOYDIR_X)) {
			switch (newDir & JOYDIR_X) {
			case JOYDIR_LEFT:
				QueueEventValue (XBE_MENU, XBMK_LEFT);
				break;
			case JOYDIR_RIGHT:
				QueueEventValue (XBE_MENU, XBMK_RIGHT);
				break;
			default:
				break;
			}
		}
		joystick[which].dir = newDir;
		break;

	case SDL_EVENT_JOYSTICK_HAT_MOTION:
		which = JoystickIndex (event->jhat.which);
		if (which < 0) {
			break;
		}
		if (event->jhat.value & SDL_HAT_UP)
			QueueEventValue (XBE_MENU, XBMK_UP);
		if (event->jhat.value & SDL_HAT_DOWN)
			QueueEventValue (XBE_MENU, XBMK_DOWN);
		if (event->jhat.value & SDL_HAT_RIGHT)
			QueueEventValue (XBE_MENU, XBMK_RIGHT);
		if (event->jhat.value & SDL_HAT_LEFT)
			QueueEventValue (XBE_MENU, XBMK_LEFT);
		break;

	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		if (JoystickIndex (event->jbutton.which) < 0) {
			break;
		}
		QueueEventValue (XBE_MENU, XBMK_SELECT);
		break;
	}
}								/* HandleMenuJoystick */

/*
 * end of file sdl_joystick.c
 */
