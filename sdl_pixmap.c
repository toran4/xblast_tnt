/*
 * file x11c_pixmap.c - double buffer for drawing
 *
 * $Id: sdl_pixmap.c,v 1.5 2006/03/28 11:50:25 fzago Exp $
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
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xblast.h"

#include "sdl_common.h"
#include "sdl_image.h"

/*
 * local constants
 */
#define CLEAR_WIDTH  (24*BASE_X)
#define CLEAR_HEIGHT (24*BASE_Y)
#define FADE_STEP 16

/*
 * local variables
 */

/* Changed by VVL (Chat) 12/11/99 */
#ifdef SMPF
static SDL_Rect xrec[MAZE_W * (MAZE_H + 3) + STAT_W * 4];
#else
static SDL_Rect xrec[MAZE_W * (MAZE_H + 2) + STAT_W * 4];
#endif
int counter = 0;
static SDL_Rect *xrecMax = xrec;

/* fading */
static XBFadeMode fadeMode;
static int fadeMax;				/* maximum y coordinate */
static int fadeStep;			/* step width between lines */

/* GC to use*/
static Uint32 fadeColor;
static SDL_Surface *clearPix;

/* This SDL_Surface is used as a work around to blit back the screen after a
   fading. The right fix would be to force an entire drawing after fading.
   TODO: remove this hack and force entire drawing where needed.
*/

static SDL_Surface *screen_copy;

/*
 *
 */
XBBool
InitPixmap (void)
{

	SDL_Surface *temp;
	temp =
		ReadCchPixmap (imgPathMisc, imgFileTitle, COLOR_BLACK, COLOR_GRAY_75, COLOR_MIDNIGHT_BLUE);
	if (!temp) {
		fprintf (stderr, "Error: file not found.\n");
		return XBFalse;
	}
	clearPix = SDL_ConvertSurface (temp, screen->format);
	SDL_DestroySurface (temp);
	if (clearPix == NULL) {
		fprintf (stderr, "ReadCchPixmap Failed in InitPixmap\n");
		return XBFalse;
	};

	screen_copy = SDL_CreateSurface (screen->w, screen->h, screen->format);
	if (screen_copy == NULL) {
		fprintf (stderr, "Error: failed to init screen_copy. Reason: %s\n", SDL_GetError ());
		return XBFalse;
	};

	return XBTrue;

}								/* InitPixmap */

/* 
 * Clear screen with clearpix
 */
void
GUI_ClearPixmap (void)
{

	SDL_Rect DstRect;

	DstRect.w = PIXW;
	for (DstRect.x = 0; DstRect.x < PIXW; DstRect.x += clearPix->w) {
		for (DstRect.y = 0; DstRect.y < PIXH + SCOREH; DstRect.y += clearPix->h) {
			SDL_BlitSurface (clearPix, NULL, screen, &DstRect);
		}
	}
	GUI_PresentScreen ();

}								/* GUI_ClearPixmap */

/*
 *
 */
void
GUI_AddMazeRectangle (int x, int y)
{
	xrecMax->x = BLOCK_WIDTH * x;
	xrecMax->y = BLOCK_HEIGHT * y;
	xrecMax->w = BLOCK_WIDTH;
	xrecMax->h = BLOCK_HEIGHT;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		if ((prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
		}
	}

	xrecMax++;
}								/* GUI_AddMazeRectangle */

/*
 *
 */
void
GUI_AddStatRectangle (int x, int y)
{
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = MAZE_H * BLOCK_HEIGHT + y * STAT_HEIGHT;
	xrecMax->w = STAT_WIDTH;
	xrecMax->h = (y < STAT_H) ? STAT_HEIGHT : LED_HEIGHT;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		/* try to join rectangles */
		if ((prev->y == xrecMax->y) && ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
			counter--;
		}
	}

	xrecMax++;
}								/* GUI_AddStatRectangle */

/* Added by VVL (Chat) 12/11/99 : Begin */
/* Added by VVL (Chat) 12/11/99 : Begin */
void
GUI_AddChatRectangle (int x, int y)
{
#ifdef SMPF
	int i = 2;
#else
	int i = 1;
#endif
	int j;
	j = MAZE_W * (MAZE_H + 1) + STAT_W * 4;
	xrecMax->h = i * STAT_HEIGHT + BLOCK_HEIGHT + 8;
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = (MAZE_H + i) * BLOCK_HEIGHT;
	xrecMax->w = STAT_WIDTH;

	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;
		if ((prev->y == xrecMax->y)
			&& ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;

		}
	}
	if (xrec + MAZE_W * (MAZE_H + 1) + STAT_W * 4 == xrecMax) {
		return;
	}
	xrecMax++;

}

/* Added by VVL (Chat) 12/11/99 : End */
/* Added by VVL (Chat) 12/11/99 : End */
/* Added by VVL (Chat) 12/11/99 : Begin */
/*
void 
AddTilesRectangle (int x, int y)
{  
} */

void
GUI_AddTilesRectangle (int x, int y)
{
#ifdef SMPF
	int i = 0;
#else
	int i = 0;
#endif
	xrecMax->h = i * STAT_HEIGHT;
	xrecMax->x = x * STAT_WIDTH;
	xrecMax->y = (MAZE_H + i) * BLOCK_HEIGHT;
	xrecMax->w = STAT_WIDTH;
	if (xrecMax != xrec) {
		SDL_Rect *prev = xrecMax - 1;

		if ((prev->y == xrecMax->y)
			&& ((xrecMax->x - prev->x) == prev->w)) {
			prev->w += BLOCK_WIDTH;
			xrecMax = prev;
		}
	}

	xrecMax++;

}

/* Added by VVL (Chat) 12/11/99 : End */

/*
 *
 */
void
GUI_FlushPixmap (XBBool flag)
{
	SDL_Surface *winSurface;
	int n;

	if (!flag) {
		GUI_PresentScreen ();
		return;
	}

	n = xrecMax - xrec;
	winSurface = (NULL != sdlWindow) ? SDL_GetWindowSurface (sdlWindow) : NULL;
	if (NULL == winSurface || NULL == screen || n <= 0) {
		GUI_PresentScreen ();
	}
	else {
		SDL_Rect dst;
		SDL_Rect rects[n];
		int i, k;

		GUI_GetScreenDestination (winSurface, &dst);
		if (dst.w <= 0 || dst.h <= 0) {
			GUI_PresentScreen ();
		}
		else {
			k = 0;
			for (i = 0; i < n; i++) {
				SDL_Rect src = xrec[i];
				SDL_Rect part;

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
					continue;
				}
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
				if (part.x + part.w > dst.x + dst.w) {
					part.w = dst.x + dst.w - part.x;
				}
				if (part.y + part.h > dst.y + dst.h) {
					part.h = dst.y + dst.h - part.y;
				}
				rects[k] = part;
				SDL_BlitSurfaceScaled (screen, &src, winSurface, &part, SDL_SCALEMODE_NEAREST);
				k++;
			}
			if (k > 0) {
				if (!SDL_UpdateWindowSurfaceRects (sdlWindow, rects, k)) {
					static XBBool warned = XBFalse;
					if (!warned) {
						fprintf (stderr, "sdl_pixmap: SDL_UpdateWindowSurfaceRects failed: %s\n", SDL_GetError ());
						warned = XBTrue;
					}
				}
			}
		}
	}

	if (flag) {
		counter = 0;
		xrecMax = xrec;
	}
}								/* GUI_FlushPixmap  */

/*
 *
 */
void
GUI_FlushScoreBoard (void)
{
}								/* GUI_FlushScoreBoard  */

/*
 *
 */
// maxLines are not used in this SDL implementation.
void
GUI_InitFade (XBFadeMode mode, int maxLines)
{
	fadeMax = maxLines;
	fadeStep = FADE_STEP;
	fadeMode = mode;
	switch (mode) {
	case XBFM_BLACK_OUT:
		fadeColor = SDL_MapSurfaceRGB (screen, 0, 0, 0);
		break;
	case XBFM_WHITE_OUT:
		fadeColor = SDL_MapSurfaceRGB (screen, 0xFF, 0xFF, 0xFF);
		break;
	case XBFM_IN:				// just to keep compiler happy
		break;
	}
	if (mode != XBFM_IN)		// save a copy of the current screen (before fade)
	{
		SDL_BlitSurface (screen, NULL, screen_copy, NULL);
	}
}								/* GUI_InitFade */

/*
 * 
 */

XBBool
GUI_DoFade (void)
{
	int y;
	SDL_Rect r;

	if (fadeStep <= 0) {
		if (fadeMode != XBFM_IN)
			SDL_FillSurfaceRect (screen, NULL, fadeColor);
		GUI_PresentScreen ();
		SDL_Delay (200);
		if (fadeMode != XBFM_IN)
			SDL_BlitSurface (screen_copy, NULL, screen, NULL);
		return XBFalse;
	}

	r.x = 0;
	r.w = screen->w;
	r.h = (FADE_STEP + 1) - fadeStep;
	for (y = 0; y < screen->h; y += FADE_STEP) {
		r.y = y;
		if (fadeMode == XBFM_IN)
			GUI_PresentScreenRect (&r);
		else					// fade out
			SDL_FillSurfaceRect (screen, &r, fadeColor);
	}

	if (fadeMode != XBFM_IN)
		GUI_PresentScreen ();

	/* preparing next fade. */
	fadeStep -= 2;

	/* thats all */
	return XBTrue;
}								/* GUI_FadeOut */

/*
 * release all pixmap resources
 */
void
FinishPixmap (void)
{
	if (NULL != clearPix) {
		SDL_DestroySurface (clearPix);
		clearPix = NULL;
	}
	if (NULL != screen_copy) {
		SDL_DestroySurface (screen_copy);
		screen_copy = NULL;
	}
}								/* FinishPixmap */

/*
 * end of file x11c_pixmap.c
 */
