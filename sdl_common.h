/*
 * file SDL_common.h -  global variables for Win32 engine
 *
 * $Id: sdl_common.h,v 1.7 2006/03/28 11:49:10 fzago Exp $
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

#ifndef XBLAST_SDL_COMMON_H
#define XBLAST_SDL_COMMON_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "sdl_event.h"
#include "sdl_keysym.h"
#include "sdl_joystick.h"
#include "sdl_image.h"
#include "sdl_config.h"
#include "sdl_text.h"
#include "sdl_sprite.h"
#include "sdl_socket.h"
#include "sdl_tile.h"
#include "sdl_pixmap.h"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define PPM_PIXELFORMAT SDL_PIXELFORMAT_RGB24
#else
#define PPM_PIXELFORMAT SDL_PIXELFORMAT_BGR24
#endif

/*
 * global constants
 */
#define NUM_FONTS 3

/*
 * global variables
 */
extern SDL_Surface *screen;
extern SDL_Surface *pix;
extern SDL_Window *sdlWindow;

extern const char *xblastClass;

/*
 * global prototypes
 */
extern void GUI_PresentScreen (void);
extern void GUI_PresentScreenRect (const SDL_Rect * rect);
extern void GUI_GetScreenDestination (const SDL_Surface * winSurface, SDL_Rect * dst);
extern XBBool GUI_WindowToScreen (float wx, float wy, int *sx, int *sy);
extern XBBool GUI_HandleHotKey (const SDL_KeyboardEvent * key);
extern void GUI_Quit (void);

#endif
/*
 * end of file SDL_common.h
 */
