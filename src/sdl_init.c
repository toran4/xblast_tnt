/*
 * file sdl_init.c - SDL3 video/display initialisation
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

#include "sdl_common.h"
#include "sdl_image.h"
#include "sdl_keysym.h"
#include "sdl_event.h"
#include "sdl_pixmap.h"
#include "sdl_sprite.h"
#include "sdl_text.h"
#include "sdl_tile.h"

/*
 * local variables
 */
static XBQuitFunction quitFunc = NULL;
static XBBool warnedPresent = XBFalse;

/*
 * compute where the back buffer is shown inside the window: scaled to fit,
 * keeping the aspect ratio and centered.
 */
void
GUI_GetScreenDestination (const SDL_Surface * winSurface, SDL_Rect * dst)
{
	if (winSurface->w <= 0 || winSurface->h <= 0 || screen->w <= 0 || screen->h <= 0) {
		dst->x = dst->y = dst->w = dst->h = 0;
		return;
	}
	if (winSurface->w * screen->h <= winSurface->h * screen->w) {
		dst->w = winSurface->w;
		dst->h = screen->h * dst->w / screen->w;
	}
	else {
		dst->h = winSurface->h;
		dst->w = screen->w * dst->h / screen->h;
	}
	dst->x = (winSurface->w - dst->w) / 2;
	dst->y = (winSurface->h - dst->h) / 2;
}								/* GetScreenDestination */

/*
 * draw a part of the backbuffer to the window.
 *
 * The game keeps drawing into the "screen" surface; it is copied (and, in
 * fullscreen modes, scaled to the window size) on every flush. If rect is
 * not NULL only that part of the backbuffer is presented, which is needed
 * for the fade in effect.
 */
void
GUI_PresentScreenRect (const SDL_Rect * rect)
{
	SDL_Surface *winSurface;
	SDL_Rect dst, part, src;

	winSurface = SDL_GetWindowSurface (sdlWindow);
	if (NULL == winSurface || NULL == screen) {
		if (!warnedPresent) {
			fprintf (stderr, "sdl_init: SDL_GetWindowSurface failed: %s\n", SDL_GetError ());
			warnedPresent = XBTrue;
		}
		return;
	}
	GUI_GetScreenDestination (winSurface, &dst);
	if (dst.w <= 0 || dst.h <= 0) {
		return;
	}

	if (NULL == rect) {
		if (winSurface->w == screen->w && winSurface->h == screen->h) {
			SDL_BlitSurface (screen, NULL, winSurface, NULL);
		}
		else {
			SDL_FillSurfaceRect (winSurface, NULL, SDL_MapSurfaceRGB (winSurface, 0, 0, 0));
			SDL_BlitSurfaceScaled (screen, NULL, winSurface, &dst, SDL_SCALEMODE_NEAREST);
		}
		if (!SDL_UpdateWindowSurface (sdlWindow)) {
			if (!warnedPresent) {
				fprintf (stderr, "sdl_init: SDL_UpdateWindowSurface failed: %s\n", SDL_GetError ());
				warnedPresent = XBTrue;
			}
		}
		return;
	}

	/* clip the source rectangle to the backbuffer */
	src = *rect;
	if (src.x < 0) {
		src.w += src.x;
		src.x = 0;
	}
	if (src.y < 0) {
		src.h += src.y;
		src.y = 0;
	}
	if (src.x + src.w > screen->w) {
		src.w = screen->w - src.x;
	}
	if (src.y + src.h > screen->h) {
		src.h = screen->h - src.y;
	}
	if (src.w <= 0 || src.h <= 0) {
		return;
	}

	/* map it to the corresponding part of the window, rounding up so that
	   neighbouring bands don't leave gaps */
	part.x = dst.x + src.x * dst.w / screen->w;
	part.y = dst.y + src.y * dst.h / screen->h;
	part.w = ((src.x + src.w) * dst.w + screen->w - 1) / screen->w
		- src.x * dst.w / screen->w;
	part.h = ((src.y + src.h) * dst.h + screen->h - 1) / screen->h
		- src.y * dst.h / screen->h;
	if (part.w < 1) {
		part.w = 1;
	}
	if (part.h < 1) {
		part.h = 1;
	}
	/* rounding up may reach one pixel into the letterbox */
	if (part.x + part.w > dst.x + dst.w) {
		part.w = dst.x + dst.w - part.x;
	}
	if (part.y + part.h > dst.y + dst.h) {
		part.h = dst.y + dst.h - part.y;
	}
	SDL_BlitSurfaceScaled (screen, &src, winSurface, &part, SDL_SCALEMODE_NEAREST);
	if (!SDL_UpdateWindowSurfaceRects (sdlWindow, &part, 1)) {
		if (!warnedPresent) {
			fprintf (stderr, "sdl_init: SDL_UpdateWindowSurfaceRects failed: %s\n", SDL_GetError ());
			warnedPresent = XBTrue;
		}
	}
}								/* GUI_PresentScreenRect */

/*
 * draw the whole backbuffer to the window
 */
void
GUI_PresentScreen (void)
{
	GUI_PresentScreenRect (NULL);
}								/* GUI_PresentScreen */

/*
 * convert window coordinates to backbuffer coordinates.
 *
 * Returns XBFalse if the point is not inside the (possibly letterboxed)
 * game area, so that clicks in the black bars are ignored.
 */
XBBool
GUI_WindowToScreen (float wx, float wy, int *sx, int *sy)
{
	SDL_Surface *winSurface;
	SDL_Rect dst;
	int x, y;

	if (NULL == screen || NULL == sdlWindow) {
		return XBFalse;
	}
	winSurface = SDL_GetWindowSurface (sdlWindow);
	if (NULL == winSurface) {
		return XBFalse;
	}
	GUI_GetScreenDestination (winSurface, &dst);
	if (dst.w <= 0 || dst.h <= 0) {
		return XBFalse;
	}
	x = (int)wx - dst.x;
	y = (int)wy - dst.y;
	if (x < 0 || y < 0 || x >= dst.w || y >= dst.h) {
		return XBFalse;
	}
	*sx = x * screen->w / dst.w;
	*sy = y * screen->h / dst.h;
	return XBTrue;
}								/* GUI_WindowToScreen */

/*
 * apply video setup from configuration
 */
void
SetupVideo (CFGVideoSetup * video)
{
	XBBool isFull;

	isFull = (SDL_GetWindowFlags (sdlWindow) & SDL_WINDOW_FULLSCREEN) ? XBTrue : XBFalse;

	if ((isFull && video->mode == XBVM_Windowed) ||
		(!isFull && video->mode == XBVM_Full)) {
		if (!SDL_SetWindowFullscreen (sdlWindow, video->mode == XBVM_Full)) {
			fprintf (stderr, "sdl_init: SDL_SetWindowFullscreen failed: %s\n", SDL_GetError ());
		}
		else {
			SDL_SyncWindow (sdlWindow);
		}
	}
}								/* SetupVideo */

static void
HotKey_ToggleFullScreen (void)
{
	XBBool isFull;

	isFull = (SDL_GetWindowFlags (sdlWindow) & SDL_WINDOW_FULLSCREEN) ? XBTrue : XBFalse;
	if (SDL_SetWindowFullscreen (sdlWindow, !isFull)) {
		SDL_SyncWindow (sdlWindow);
		Dbg_Out ("Toggled fullscreen mode - now %s\n", isFull ? "windowed" : "fullscreen");
	}
	else {
		fprintf (stderr, "sdl_init: SDL_SetWindowFullscreen failed: %s\n", SDL_GetError ());
	}
}								/* HotKey_ToggleFullScreen */

static void
HotKey_ToggleGrab (void)
{
	XBBool grab;

	grab = SDL_GetWindowMouseGrab (sdlWindow);
	if (!SDL_SetWindowMouseGrab (sdlWindow, !grab) ||
	    !SDL_SetWindowKeyboardGrab (sdlWindow, !grab)) {
		fprintf (stderr, "sdl_init: unable to change input grab: %s\n", SDL_GetError ());
	}
	else {
		Dbg_Out ("Ctrl-G: grab is now %s\n", grab ? "off" : "on");
	}
}								/* HotKey_ToggleGrab */

static void
HotKey_Iconify (void)
{
	Dbg_Out ("Ctrl-Z: iconifying window!\n");
	if (!SDL_MinimizeWindow (sdlWindow)) {
		fprintf (stderr, "sdl_init: SDL_MinimizeWindow failed: %s\n", SDL_GetError ());
	}
}								/* HotKey_Iconify */

/*
 * quit through the callback registered by the main program, if any
 */
void
GUI_Quit (void)
{
	GUI_Finish ();
	if (NULL != quitFunc) {
		(*quitFunc) ();
	}
	exit (0);
}								/* GUI_Quit */

/*
 * handle global hot keys. Returns XBTrue if the key has been consumed.
 *
 * This must run on the main thread (unlike the event filter), since it
 * changes window state.
 */
XBBool
GUI_HandleHotKey (const SDL_KeyboardEvent * key)
{
	if (SDLK_ESCAPE == key->key) {
		GUI_Quit ();
		return XBTrue;
	}
	if (key->mod & SDL_KMOD_CTRL) {
		if (SDLK_G == key->key) {
			HotKey_ToggleGrab ();
			return XBTrue;
		}
		if (SDLK_Z == key->key) {
			HotKey_Iconify ();
			return XBTrue;
		}
	}
	if ((key->mod & SDL_KMOD_ALT) && (SDLK_RETURN == key->key)) {
		HotKey_ToggleFullScreen ();
		return XBTrue;
	}
	return XBFalse;
}								/* GUI_HandleHotKey */

static bool
FilterEvents (void *userdata, SDL_Event * event)
{
	(void) userdata;

	switch (event->type) {

	case SDL_EVENT_WINDOW_EXPOSED:
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		/* pass through, forces a redraw (close requests are handled in
		   HandleSDLEvent() on the main thread) */
		return true;

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		/* drop, the game doesn't care */
		return false;

		/* Pass various input events. The hot keys are handled later in
		   HandleSDLEvent(), on the main thread. */
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		return true;

	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
	case SDL_EVENT_JOYSTICK_BALL_MOTION:
	case SDL_EVENT_JOYSTICK_HAT_MOTION:
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		return true;

		/* This is important!  Queue it if we want to quit. */
	case SDL_EVENT_QUIT:
		Dbg_Out ("Quit demanded\n");
		return true;

		/* Drop all other events */
	default:
		return false;
	}
}								/* FilterEvents */

static XBBool
InitDisplay (void)
{
	char *title;
	CFGVideoSetup video;
	SDL_WindowFlags windowFlags = 0;
	SDL_Surface *winSurface;

	if (!SDL_Init (SDL_INIT_VIDEO)) {
		fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
		exit (1);
	}
	/* audio is optional, the sound layer degrades gracefully */
	if (!SDL_InitSubSystem (SDL_INIT_AUDIO)) {
		fprintf (stderr, "Warning: no audio device available: %s\n", SDL_GetError ());
	}
	atexit (SDL_Quit);

	/* Set the title bar */
	title = "XBlast TNT " VERSION_STRING;

	if (RetrieveVideoSetup (&video)) {
		if (video.mode == XBVM_Full) {
			windowFlags |= SDL_WINDOW_FULLSCREEN;
		}
	}

	sdlWindow = SDL_CreateWindow (title, PIXW, PIXH + SCOREH, windowFlags);
	if (NULL == sdlWindow) {
		fprintf (stderr, "Couldn't create window: %s\n", SDL_GetError ());
		return XBFalse;
	}

	winSurface = SDL_GetWindowSurface (sdlWindow);
	if (NULL == winSurface) {
		fprintf (stderr, "Couldn't get window surface: %s\n", SDL_GetError ());
		return XBFalse;
	}

	/* Create the back buffer all drawing goes to */
	screen = SDL_CreateSurface (PIXW, PIXH + SCOREH, winSurface->format);
	if (NULL == screen) {
		fprintf (stderr, "Couldn't create back buffer: %s\n", SDL_GetError ());
		return XBFalse;
	}

	/* Set an event filter. */
	SDL_SetEventFilter (FilterEvents, NULL);

	return XBTrue;
}								/* InitDisplay */

XBBool
GUI_Init (int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	if (!InitDisplay ()) {
		GUI_Finish ();
		return XBFalse;
	}
	/* joystick support is optional, never abort startup for it */
	(void)InitJoystick ();
	/* now setup image loading */
	/* if (! InitImage () ) {
	   return XBFalse;
	   } */
	if (!InitKeysym ()) {
		GUI_Finish ();
		return XBFalse;
	}
	if (!InitEvent ()) {
		GUI_Finish ();
		return XBFalse;
	}
	/* now create pixmap for double bufferung */
	if (!InitPixmap ()) {
		GUI_Finish ();
		return XBFalse;
	}
	/* now create pixmap for double bufferung */
	if (!InitSprites ()) {
		GUI_Finish ();
		return XBFalse;
	}
	if (!InitFonts ()) {
		GUI_Finish ();
		return XBFalse;
	}
	/* initalisize tile drawing and scoreboard */
	if (!InitTiles ()) {
		GUI_Finish ();
		return XBFalse;
	}

	return XBTrue;
}								/* GUI_Init */

void
GUI_Finish (void)
{
	FinishSprites ();
	FinishTiles ();
	FinishFonts ();
	FinishPixmap ();
	FinishJoystick ();
	FinishEvent ();
	if (NULL != screen) {
		SDL_DestroySurface (screen);
		screen = NULL;
	}
	if (NULL != sdlWindow) {
		SDL_DestroyWindow (sdlWindow);
		sdlWindow = NULL;
	}
}								/* GUI_Finish */

void
GUI_OnQuit (XBQuitFunction _quitFunc)
{
	assert (NULL != _quitFunc);

	quitFunc = _quitFunc;
}								/* GUI_OnQuit */

/*
 * end of file sdl_init.c
 */
