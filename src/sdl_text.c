/*
 * file x11c_text.c - draw text strings 
 *
 * $Id: sdl_text.c,v 1.12 2006/03/28 11:41:19 fzago Exp $
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

static const SDL_Color SDL_COLOR_BLACK = { 0x00, 0x00, 0x00, 0xFF };
static const SDL_Color SDL_COLOR_YELLOW = { 0xFF, 0xFF, 0x00, 0xFF };

/*
 * local variables
 */
static TTF_Font *fontStruct[NUM_FONTS];
static XBBool fontsInitialized = XBFalse;
static SDL_Surface *bgTextSurface;
static SDL_Surface *fgTextSurface;

//static Uint32 darkTextColor1;
//static Uint32 darkTextColor2;
//static Uint32 lightTextColor1;
//static Uint32 lightTextColor2;
SDL_Surface *RenderSolidText (TTF_Font * font, const char *text, SDL_Color textColor);
SDL_Surface *RenderOutlineText (TTF_Font * font, const char *text, SDL_Color textColor,
								SDL_Color outlineColor, int length);

/*
 * load fonts struct by font name
 */
static TTF_Font *
LoadFont (const char *fontName, int fontSize)
{
	static const char *fontPath[] = {
		"",
		GAME_DATADIR "/",
		"/usr/share/fonts/truetype/",
		"/usr/share/fonts/TTF/",
		"/usr/share/fonts/truetype/ttf-bitstream-vera/",
		"/usr/share/fonts/truetype/dejavu/",
		"/usr/share/fonts/dejavu/",
		"/usr/share/fonts/truetype/liberation/",
		"/usr/share/fonts/truetype/freefont/",
		"/usr/local/share/fonts/",
		NULL
	};
	/* used when the configured font is not installed */
	static const char *fontFallback[] = {
		"DejaVuSans.ttf",
		"FreeSans.ttf",
		"LiberationSans-Regular.ttf",
		NULL
	};
	char path[1024];
	TTF_Font *font;
	int i, f;

	/* search for the font in the usual places, with a couple of common
	   fallbacks if the configured one is not available */
	font = NULL;
	for (f = 0; NULL == font; f++) {
		const char *name;

		if (0 == f) {
			name = fontName;
		}
		else if (NULL != fontFallback[f - 1]) {
			name = fontFallback[f - 1];
		}
		else {
			break;
		}
		for (i = 0; NULL != fontPath[i] && NULL == font; i++) {
			SDL_snprintf (path, sizeof (path), "%s%s", fontPath[i], name);
			if (0 != access (path, R_OK)) {
				continue;
			}
			font = TTF_OpenFont (path, (float)fontSize);
		}
	}
	if (NULL == font) {
		fprintf (stderr, "xblast: %s:%d:%s: TTF_OpenFont(%s, %d) failed: %s\n",
				 __FILE__, __LINE__, __FUNCTION__, fontName, fontSize, SDL_GetError ());
	};
	return font;
}								/* LoadFont */

/*
 *
 */
XBBool
InitFonts (void)
{
	const CFGFont *cfgFont;
	const CFGColor *cfgColor;
	char *fontName = "Vera.ttf";

	/*
	 * Initialize SDL_ttf library
	 */
	Dbg_Out ("Initialising font...");

	/* Initialize the TTF library */
	if (!TTF_Init ()) {
		fprintf (stderr, "xblast: %s:%d:%s: TTF_Init() failed: %s\n", __FILE__, __LINE__,
				 __FUNCTION__, SDL_GetError ());
		return (XBFalse);
	}
	Dbg_Out ("OK!\n");
	fontsInitialized = XBTrue;
	/* get config */
	cfgFont = GetFontConfig ();
	assert (cfgFont != NULL);
	cfgColor = GetColorConfig ();
	assert (cfgColor != NULL);
	bgTextSurface =
		ReadCchPixmap (imgPathMisc, imgFileTextBg, COLOR_BLACK, cfgColor->darkText1,
					   cfgColor->darkText2);
	assert (bgTextSurface != NULL);
	fgTextSurface =
		ReadCchPixmap (imgPathMisc, imgFileTextFg, COLOR_BLACK, cfgColor->lightText1,
					   cfgColor->lightText2);
	assert (fgTextSurface != NULL);
	fontStruct[FF_Large] = LoadFont (fontName, 24);
	if (fontStruct[FF_Large] == NULL) {
		fprintf (stderr, "Error: unable to load font.\n" "Possible reason: %s\n", SDL_GetError ());
		return XBFalse;
	}

	fontStruct[FF_Medium] = LoadFont (fontName, 18);
	if (fontStruct[FF_Medium] == NULL) {
		fprintf (stderr, "Error: unable to load font.\n" "Possible reason: %s\n", SDL_GetError ());
		return XBFalse;
	}

	fontStruct[FF_Small] = LoadFont (fontName, 12);
	if (fontStruct[FF_Small] == NULL) {
		fprintf (stderr, "Error: unable to load font.\n" "Possible reason: %s\n", SDL_GetError ());
		return XBFalse;
	}

	return XBTrue;
}								/* InitFonts */

/*
 * draw simple textbox (in game)
 */
void
GUI_DrawSimpleTextbox (const char *text_in, unsigned flags, const BMRectangle * rect)
{
	SDL_Surface *text_surface = NULL;
	SDL_Surface *surface = NULL;
	TTF_Font *font = NULL;
	SDL_Rect rects;
	int x, y;
	const char *text;

	if (strlen (text_in) == 0)
		return;					// SDL_ttf doesn't like rendering 0 width text

	text = gettext (text_in);
	font = fontStruct[FM_Size & flags];

	text_surface = TTF_RenderText_Solid (font, text, 0, SDL_COLOR_YELLOW);
	if (NULL == text_surface) {
		fprintf (stderr, "GUI_DrawSimpleTextbox: TTF_RenderText_Solid failed: %s\n",
				 SDL_GetError ());
		return;
	}
	surface = SDL_CreateSurface (rect->w, rect->h, screen->format);
	if (NULL == surface) {
		fprintf (stderr, "GUI_DrawSimpleTextbox: could not create surface: %s\n",
				 SDL_GetError ());
		SDL_DestroySurface (text_surface);
		return;
	}
	SDL_FillSurfaceRect (surface, NULL, SDL_MapSurfaceRGB (surface, 0x0, 0x0, 0x0));
	rects.x = (surface->w - text_surface->w) / 2;
	rects.y = (surface->h - text_surface->h) / 2;
	rects.w = 0;
	rects.h = 0;
	if (!SDL_BlitSurface (text_surface, NULL, surface, &rects)) {
		fprintf (stderr, "Error: could blit!\n" "Reason: %s\n", SDL_GetError ());
	}

	//  SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 0xf, 0xf, 0xf));
	for (y = 0; y < surface->h; y++) {
		putpixel (surface, 0, y, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, 1, y, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, surface->w - 1, y, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, surface->w - 2, y, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
	}
	for (x = 0; x < surface->w; x++) {
		putpixel (surface, x, 0, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, x, 1, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, x, surface->h - 1, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
		putpixel (surface, x, surface->h - 2, SDL_MapSurfaceRGB (surface, 0xff, 0xff, 0x0));
	}
	rects.x = rect->x;
	rects.y = rect->y;
	SDL_BlitSurface (surface, NULL, screen, &rects);
	SDL_DestroySurface (text_surface);
	SDL_DestroySurface (surface);
}								/* GUI_DrawSimpleTextbox */

/*
 *
 */
static void
DrawAlignedText (const char *text, unsigned flags, SDL_Rect * rect, TTF_Font * font,
				 const SDL_Color * fgColor, const SDL_Color * bgColor)
{
	SDL_Surface *text_surface = NULL;
	SDL_Rect rects;
	if (strlen (text) == 0)
		return;					// SDL_ttf doesn't like rendering 0 width text
	rects.y = rect->y;
	if (bgColor == NULL) {
		text_surface = RenderSolidText (font, text, *fgColor);
		rects.y += 3;
	}

	else
		text_surface = RenderOutlineText (font, text, *fgColor, *bgColor, 3);
	if (text_surface == NULL) {
		fprintf (stderr, "xblast: %s:%d:%s: TTF_RenderUTF8_Solid failed: %s\n", __FILE__, __LINE__,
				 __FUNCTION__, SDL_GetError ());
		return;
	};
	switch (flags & FM_Align) {
	case FF_Left:
		rects.x = rect->x;
		break;
	case FF_Right:
		rects.x = rect->x + rect->w - text_surface->w;
		break;
	default:
		rects.x = (rect->w - text_surface->w);
		if (rects.x < 0) {
			rects.x = -rects.x;
		}
		rects.x = (rects.x >> 1) + rect->x;
		break;
	}
	rects.w = 0;
	rects.h = 0;
	{
		SDL_Rect src;

		src.x = 0;
		src.y = 0;
		src.w = text_surface->w;
		src.h = text_surface->h;
		SDL_BlitSurface (text_surface, &src, screen, &rects);
	}
	SDL_DestroySurface (text_surface);
}								/* DrawAlignedText */

/*
 *
 */
void
GUI_DrawTextbox (const char *text, unsigned flags, const BMRectangle * rect)
{
	TTF_Font *font;
	int offset, i;
	SDL_Surface *rect_surface;
	const SDL_Color *fgColor, *bgColor;
	SDL_Surface *TextImage;
	Uint32 BackGround;
	SDL_Rect rects;

	if (text == NULL)
		return;

	if (flags & FM_Color) {
		offset = 2;
		TextImage = bgTextSurface;
		fgColor = &SDL_COLOR_YELLOW;
		bgColor = &SDL_COLOR_BLACK;
	}
	else {
		offset = 3;
		TextImage = fgTextSurface;
		fgColor = &SDL_COLOR_BLACK;
		bgColor = &SDL_COLOR_YELLOW;
	}
	assert (((unsigned)(FM_Size & flags)) < NUM_FONTS);
	font = fontStruct[FM_Size & flags];
	assert (font != NULL);
	assert (text != NULL);

	rects.x = rect->x;
	rects.y = rect->y;
	rects.w = rect->w;
	rects.h = rect->h;
	if (flags & FM_Boxed) {
		SDL_Rect rect1;
		rect_surface = SDL_CreateSurface (rect->w, rect->h, screen->format);
		BackGround = SDL_MapSurfaceRGB (rect_surface, fgColor->r, fgColor->g, fgColor->b);
		SDL_FillSurfaceRect (rect_surface, NULL, BackGround);
		rect1.y = offset;
		rect1.x = offset;
		rect1.w = rect_surface->w - 2 * offset;
		rect1.h = rect_surface->h - 2 * offset;
		SDL_FillSurfaceRect (rect_surface, &rect1, SDL_MapSurfaceRGB (rect_surface, 0xFF, 0xFF, 0xFF));
		if (!(flags & FM_Transparent)) {
			int line, col;
			SDL_Rect bgImgRect;
			bgImgRect.w = TextImage->w;
			bgImgRect.h = TextImage->h;
			for (line = offset; line < rects.h - offset; line += TextImage->h) {
				bgImgRect.y = line;
				for (col = offset; col < rects.w - offset; col += TextImage->w) {
					bgImgRect.x = col;
					SDL_BlitSurface (TextImage, NULL, rect_surface, &bgImgRect);
				}
			}
			for (line = offset; line < rects.w; line++) {
				for (i = 1; i <= offset; i++)
					putpixel (rect_surface, line, rect_surface->h - i, BackGround);
			}
			for (col = offset; col < rects.h; col++) {
				for (i = 1; i <= offset; i++)
					putpixel (rect_surface, rect_surface->w - i, col, BackGround);
			}
		}
		else {
			int x, y;
			SDL_SetSurfaceColorKey (rect_surface, true,
							 SDL_MapSurfaceRGB (rect_surface, 0xFF, 0xFF, 0xFF));
			for (y = 2; y < rect_surface->h - 2; y++) {
				for (x = 2; x < rect_surface->w - 2; x++) {
					if (y % 2 == 0)
						putpixel (rect_surface, x, y,
								  SDL_MapSurfaceRGB (rect_surface, 0x0, 0x0, 0x0));
				}
			}
		}
		SDL_BlitSurface (rect_surface, NULL, screen, &rects);
		SDL_DestroySurface (rect_surface);
	}
	if (*text != 0) {
		if (flags & FM_Color)
			DrawAlignedText (gettext (text), flags, &rects, font, fgColor, NULL);

		else
			DrawAlignedText (gettext (text), flags, &rects, font, fgColor, bgColor);
	}

	else {
		if (flags & FM_Color)
			DrawAlignedText ("", flags, &rects, font, fgColor, NULL);

		else
			DrawAlignedText ("", flags, &rects, font, fgColor, bgColor);
	}
}								/* GUI_DrawTextbox */

/*
 * returns a surface with plain solid rendered text
 *
 */
SDL_Surface *
RenderSolidText (TTF_Font * font, const char *text, SDL_Color textColor)
{
	SDL_Surface *text_surface;
	SDL_Surface *surface;

	text_surface = TTF_RenderText_Solid (font, text, 0, textColor);
	if (text_surface == NULL) {
		fprintf (stderr, "Error: TTF could not render font!\n" "Reason: %s\n", SDL_GetError ());
		return NULL;
	}
	surface = SDL_CreateSurface (text_surface->w, text_surface->h, screen->format);
	SDL_BlitSurface (text_surface, NULL, surface, NULL);
	SDL_SetSurfaceColorKey (surface, true, 0);
	SDL_DestroySurface (text_surface);

	// the reason why we create a new blit is to make pallete just like the
	// screen, so that the colors from both text rendered by RenderSolidText()
	// and RenderOutlineText() looks the same.

	// It should be possible to make this more elegant and fast.
	return surface;
}								/* RenderSolidText */

/*
 * returns a surface with outline rendered text
 *
 */
SDL_Surface *
RenderOutlineText (TTF_Font * font, const char *text, SDL_Color textColor,
				   SDL_Color outlineColor, int length)
{
	int x, y, l;
	SDL_Surface *text_surface = NULL;
	SDL_Surface *surface = NULL;
	SDL_Rect rect;

	// color cannot be exactly 0, since that's the colorkey
	if (textColor.r == 0 && textColor.g == 0 && textColor.b == 0)
		textColor.r = textColor.g = textColor.b = 1;
	if (outlineColor.r == 0 && outlineColor.g == 0 && outlineColor.b == 0)
		outlineColor.r = outlineColor.g = outlineColor.b = 1;
	text_surface = TTF_RenderText_Solid (font, text, 0, textColor);
	if (text_surface == NULL) {
		fprintf (stderr, "Error: TTF could not render font!\n" "Reason: %s\n", SDL_GetError ());
		return NULL;
	}
	rect.x = rect.y = length;
	surface = SDL_CreateSurface (text_surface->w + 2 * length,
								 text_surface->h + 2 * length, screen->format);
	if (surface == NULL) {
		fprintf (stderr, "Error: could not create surface!\n" "Reason: %s\n", SDL_GetError ());
		return NULL;
	}

	/* make grey transparent */
	SDL_FillSurfaceRect (surface, NULL, SDL_MapSurfaceRGB (surface, 0xf, 0xf, 0xf));
	if (!SDL_BlitSurface (text_surface, NULL, surface, &rect)) {
		fprintf (stderr, "Error: could blit!\n" "Reason: %s\n", SDL_GetError ());
	}
	SDL_SetSurfaceColorKey (surface, true, SDL_MapSurfaceRGB (surface, 0xf, 0xf, 0xf));
	SDL_DestroySurface (text_surface);
	if (length > 0) {
		Uint32 tcolor = SDL_MapSurfaceRGB (surface, textColor.r, textColor.g, textColor.b);
		Uint32 ocolor =
			SDL_MapSurfaceRGB (surface, outlineColor.r, outlineColor.g, outlineColor.b);
		SDL_LockSurface (surface);
		for (y = 0; y < surface->h; y++)
			for (x = 0; x < surface->w; x++) {
				if (getpixel (surface, x, y) == tcolor) {

					// filling pixels to the right
					for (l = 0; l < length && x + l < surface->w; l++)
						if (getpixel (surface, x + l, y) != tcolor)
							putpixel (surface, x + l, y, ocolor);

					// filling pixels to the left
					for (l = 0; l < length && x - l > 0; l++)
						if (getpixel (surface, x - l, y) != tcolor)
							putpixel (surface, x - l, y, ocolor);

					// filling pixels to down
					for (l = 0; l < length && y + l < surface->h; l++)
						if (getpixel (surface, x, y + l) != tcolor)
							putpixel (surface, x, y + l, ocolor);

					// filling pixels to up
					for (l = 0; l < length && y - l > 0; l++)
						if (getpixel (surface, x, y - l) != tcolor)
							putpixel (surface, x, y - l, ocolor);
				}
			}
		SDL_UnlockSurface (surface);
	}
	return surface;
}								/* RenderOutlineText */

/*
 * local helpers for filled polygons and lines (replacing SDL_gfx)
 */
static void
DrawLineColor (SDL_Surface * surface, int x1, int y1, int x2, int y2, Uint32 pixel)
{
	int dx = abs (x2 - x1);
	int sx = (x1 < x2) ? 1 : -1;
	int dy = -abs (y2 - y1);
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx + dy;

	for (;;) {
		if (x1 >= 0 && y1 >= 0 && x1 < surface->w && y1 < surface->h) {
			putpixel (surface, x1, y1, pixel);
		}
		if (x1 == x2 && y1 == y2) {
			break;
		}
		{
			int e2 = 2 * err;
			if (e2 >= dy) {
				err += dy;
				x1 += sx;
			}
			if (e2 <= dx) {
				err += dx;
				y1 += sy;
			}
		}
	}
}								/* DrawLineColor */

static void
FillPolygonColor (SDL_Surface * surface, const Sint16 * xv, const Sint16 * yv, int n,
				  Uint32 pixel)
{
	int minY = yv[0];
	int maxY = yv[0];
	int i, y;

	for (i = 1; i < n; i++) {
		if (yv[i] < minY) {
			minY = yv[i];
		}
		if (yv[i] > maxY) {
			maxY = yv[i];
		}
	}
	if (minY < 0) {
		minY = 0;
	}
	if (maxY >= surface->h) {
		maxY = surface->h - 1;
	}

	for (y = minY; y <= maxY; y++) {
		int nodes[64];
		int numNodes = 0;
		int x;

		for (i = 0; i < n; i++) {
			int j = (i + 1) % n;
			if (((yv[i] <= y) && (yv[j] > y)) || ((yv[j] <= y) && (yv[i] > y))) {
				if (numNodes < (int)(sizeof (nodes) / sizeof (nodes[0]))) {
					nodes[numNodes++] = xv[i] + (int)((y - yv[i]) * (double)(xv[j] - xv[i])
													  / (double)(yv[j] - yv[i]));
				}
			}
		}
		/* sort intersections */
		for (i = 1; i < numNodes; i++) {
			int key = nodes[i];
			int k = i - 1;
			while (k >= 0 && nodes[k] > key) {
				nodes[k + 1] = nodes[k];
				k--;
			}
			nodes[k + 1] = key;
		}
		for (i = 0; i + 1 < numNodes; i += 2) {
			int x0 = nodes[i];
			int x1 = nodes[i + 1];
			if (x0 < 0) {
				x0 = 0;
			}
			if (x1 >= surface->w) {
				x1 = surface->w - 1;
			}
			for (x = x0; x <= x1; x++) {
				putpixel (surface, x, y, pixel);
			}
		}
	}
}								/* FillPolygonColor */

/*
 * draw a filled and outlined polygon
 */
void
GUI_DrawPolygon (int x, int y, int w, int h, int lw, const BMPoint * points, int npoints,
				 XBBool black_white)
{
	int i;

	/* convert and scale to xpoints */
	Sint16 xv[npoints + 1], yv[npoints + 1];
	Uint32 blackPixel, yellowPixel;
	for (i = 0; i < npoints; i++) {
		xv[i] = (int)(x + w * points[i].x);
		yv[i] = (int)(y + h * points[i].y);
	} xv[npoints] = xv[0];
	yv[npoints] = yv[0];

	blackPixel = SDL_MapSurfaceRGB (screen, SDL_COLOR_BLACK.r, SDL_COLOR_BLACK.g,
									SDL_COLOR_BLACK.b);
	yellowPixel = SDL_MapSurfaceRGB (screen, SDL_COLOR_YELLOW.r, SDL_COLOR_YELLOW.g,
									 SDL_COLOR_YELLOW.b);

	SDL_LockSurface (screen);
	/* now draw it */
	FillPolygonColor (screen, xv, yv, npoints, blackPixel);
	if (!black_white) {
		int l;
		for (i = 0; i < npoints; i++) {
			for (l = 0; l < lw; l++) {
				DrawLineColor (screen, xv[i] + l, yv[i], xv[i + 1] + l, yv[i + 1], yellowPixel);
				DrawLineColor (screen, xv[i], yv[i] + l, xv[i + 1], yv[i + 1] + l, yellowPixel);
			}
		}
	}
	SDL_UnlockSurface (screen);

	/* that's all */

#if 0							// old X11
	XPoint *xp;
	int i;
	XGCValues xgcv;

	/* convert and scale to xpoints */
	xp = (XPoint *) calloc (sizeof (XPoint), npoints + 1);
	for (i = 0; i < npoints; i++) {
		xp[i].x = (int)(x + w * points[i].x);
		xp[i].y = (int)(y + h * points[i].y);
	} xp[npoints] = xp[0];

	/* set line width */
	xgcv.line_width = lw;

	/* now draw it */
	if (black_white) {
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
		XFillPolygon (dpy, pix, gcTextBlack, xp, npoints, Complex, CoordModeOrigin);
		XDrawLines (dpy, pix, gcTextWhite, xp, npoints + 1, CoordModeOrigin);
		xgcv.line_width = 2;
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
	}
	else {
		XChangeGC (dpy, gcTextBlack, GCLineWidth, &xgcv);
		XFillPolygon (dpy, pix, gcTextWhite, xp, npoints, Complex, CoordModeOrigin);
		XDrawLines (dpy, pix, gcTextBlack, xp, npoints + 1, CoordModeOrigin);
		xgcv.line_width = 2;
		XChangeGC (dpy, gcTextWhite, GCLineWidth, &xgcv);
	}

	/* that's all */
	free (xp);

#endif /*  */
}								/* GUI_DrawPolygon */

/* Follows a couple of SDL functions to get and put pixels */

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 *
 * Taken from SDL documentation.
 */
Uint32
getpixel (SDL_Surface * surface, int x, int y)
{
	int bpp = SDL_BYTESPERPIXEL (surface->format);

	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;
	switch (bpp) {
	case 1:
		return *p;
	case 2:
		return *(Uint16 *) p;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];

		else
			return p[0] | p[1] << 8 | p[2] << 16;
	case 4:
		return *(Uint32 *) p;
	default:
		return 0;				/* shouldn't happen, but avoids warnings */
	}
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 *
 * Taken from SDL documentation.
 */
void
putpixel (SDL_Surface * surface, int x, int y, Uint32 pixel)
{
	int bpp = SDL_BYTESPERPIXEL (surface->format);

	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;
	switch (bpp) {
	case 1:
		*p = pixel;
		break;
	case 2:
		*(Uint16 *) p = pixel;
		break;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		}
		else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;
	case 4:
		*(Uint32 *) p = pixel;
		break;
	}
}

/*
 * release all font resources
 */
void
FinishFonts (void)
{
	int i;

	if (!fontsInitialized) {
		return;
	}
	for (i = 0; i < NUM_FONTS; i++) {
		if (NULL != fontStruct[i]) {
			TTF_CloseFont (fontStruct[i]);
			fontStruct[i] = NULL;
		}
	}
	TTF_Quit ();
	fontsInitialized = XBFalse;
}								/* FinishFonts */

/*
 * end of file sdl_text.c
 */
