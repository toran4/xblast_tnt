/*
 * file w32_keysym.c - create X11 like keysmbol names for virtual keys
 * 
 * $Id: sdl_keysym.c,v 1.5 2006/02/10 15:07:41 fzago Exp $
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

/*
 * local types
 */
typedef struct
{
	SDL_Keycode code;
	const char *name;
} KeyNameTable;

typedef struct
{
	XBAtom atom;
	SDL_Keycode code;
} AtomCodeTable;

/*
 * local variables
 */
static int numKeys = 0;
static int numCodes = 0;
static AtomCodeTable *atomTable = NULL;
static AtomCodeTable *codeTable = NULL;
static KeyNameTable keyTable[] = {
	{SDLK_BACKSPACE, "BackSpace"},
	{SDLK_TAB, "Tab"},
	{SDLK_RETURN, "Return"},
	{SDLK_LSHIFT, "Shift"},
	{SDLK_LCTRL, "Control"},
	{SDLK_PAUSE, "Pause"},
	{SDLK_CAPSLOCK, "Caps_Lock"},
	{SDLK_ESCAPE, "Escape"},
	{SDLK_SPACE, "space"},
	{SDLK_PAGEUP, "Prior"},
	{SDLK_PAGEDOWN, "Next"},
	{SDLK_END, "End"},
	{SDLK_HOME, "Home"},
	{SDLK_LEFT, "Left"},
	{SDLK_UP, "Up"},
	{SDLK_RIGHT, "Right"},
	{SDLK_DOWN, "Down"},
	{SDLK_PRINTSCREEN, "Print"},
	{SDLK_INSERT, "Insert"},
	{SDLK_DELETE, "Delete"},
	{SDLK_0, "0"},
	{SDLK_1, "1"},
	{SDLK_2, "2"},
	{SDLK_3, "3"},
	{SDLK_4, "4"},
	{SDLK_5, "5"},
	{SDLK_6, "6"},
	{SDLK_7, "7"},
	{SDLK_8, "8"},
	{SDLK_9, "9"},
	{SDLK_A, "A"},
	{SDLK_B, "B"},
	{SDLK_C, "C"},
	{SDLK_D, "D"},
	{SDLK_E, "E"},
	{SDLK_F, "F"},
	{SDLK_G, "G"},
	{SDLK_H, "H"},
	{SDLK_I, "I"},
	{SDLK_J, "J"},
	{SDLK_K, "K"},
	{SDLK_L, "L"},
	{SDLK_M, "M"},
	{SDLK_N, "N"},
	{SDLK_O, "O"},
	{SDLK_P, "P"},
	{SDLK_Q, "Q"},
	{SDLK_R, "R"},
	{SDLK_S, "S"},
	{SDLK_T, "T"},
	{SDLK_U, "U"},
	{SDLK_V, "V"},
	{SDLK_W, "W"},
	{SDLK_X, "X"},
	{SDLK_Y, "Y"},
	{SDLK_Z, "Z"},
	{SDLK_A, "a"},
	{SDLK_B, "b"},
	{SDLK_C, "c"},
	{SDLK_D, "d"},
	{SDLK_E, "e"},
	{SDLK_F, "f"},
	{SDLK_G, "g"},
	{SDLK_H, "h"},
	{SDLK_I, "i"},
	{SDLK_J, "j"},
	{SDLK_K, "k"},
	{SDLK_L, "l"},
	{SDLK_M, "m"},
	{SDLK_N, "n"},
	{SDLK_O, "o"},
	{SDLK_P, "p"},
	{SDLK_Q, "q"},
	{SDLK_R, "r"},
	{SDLK_S, "s"},
	{SDLK_T, "t"},
	{SDLK_U, "u"},
	{SDLK_V, "v"},
	{SDLK_W, "w"},
	{SDLK_X, "x"},
	{SDLK_Y, "y"},
	{SDLK_Z, "z"},
	{SDLK_KP_0, "KP_0"},
	{SDLK_KP_1, "KP_1"},
	{SDLK_KP_2, "KP_2"},
	{SDLK_KP_3, "KP_3"},
	{SDLK_KP_4, "KP_4"},
	{SDLK_KP_5, "KP_5"},
	{SDLK_KP_6, "KP_6"},
	{SDLK_KP_7, "KP_7"},
	{SDLK_KP_8, "KP_8"},
	{SDLK_KP_9, "KP_9"},
	{SDLK_KP_MULTIPLY, "KP_Multiply"},
	{SDLK_KP_PLUS, "KP_Add"},
	{SDLK_KP_MINUS, "KP_Subtract"},
	{SDLK_KP_PERIOD, "KP_Decimal"},
	{SDLK_KP_DIVIDE, "KP_Divide"},
	{SDLK_F1, "F1"},
	{SDLK_F2, "F2"},
	{SDLK_F3, "F3"},
	{SDLK_F4, "F4"},
	{SDLK_F5, "F5"},
	{SDLK_F6, "F6"},
	{SDLK_F7, "F7"},
	{SDLK_F8, "F8"},
	{SDLK_F9, "F9"},
	{SDLK_F10, "F10"},
	{SDLK_F11, "F11"},
	{SDLK_F12, "F12"},
	{SDLK_F13, "F13"},
	{SDLK_F14, "F14"},
	{SDLK_F15, "F15"},
	/* terminator */
	{0, NULL},
};

/*
 * compare by code
 */
static int
CompareByCode (const void *a, const void *b)
{
	return ((const AtomCodeTable *) a)->code - ((const AtomCodeTable *) b)->code;
}								/* CompareByCode */

/*
 * compare by atom
 */
static int
CompareByAtom (const void *a, const void *b)
{
	return ((const AtomCodeTable *) a)->atom - ((const AtomCodeTable *) b)->atom;
}								/* CompareByAtom */

/*
 * normalize right-hand modifier keys to their left-hand counterparts,
 * since the key name table only has one "Shift" and one "Control"
 */
SDL_Keycode
NormalizeKey (SDL_Keycode code)
{
	switch (code) {
	case SDLK_RSHIFT:
		return SDLK_LSHIFT;
	case SDLK_RCTRL:
		return SDLK_LCTRL;
	default:
		return code;
	}
}								/* NormalizeKey */

/*
 * init keysyms
 */
XBBool
InitKeysym (void)
{
	int i;

	/* count keys */
	for (numKeys = 0; keyTable[numKeys].name != NULL; numKeys++)
		continue;
	/* alloc tables */
	atomTable = calloc (numKeys, sizeof (AtomCodeTable));
	assert (NULL != atomTable);
	codeTable = calloc (numKeys, sizeof (AtomCodeTable));
	assert (NULL != codeTable);
	/* fill the atom table with all names, so that both "A" and "a" can be
	   parsed when reading a configuration */
	for (i = 0; i < numKeys; i++) {
		XBAtom atom = GUI_StringToAtom (keyTable[i].name);
		atomTable[i].code = NormalizeKey (keyTable[i].code);
		atomTable[i].atom = atom;
	}
	qsort (atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
	/* fill the code table without duplicate keycodes: SDL uses the same
	   code for "A" and "a" (and NormalizeKey() maps left and right
	   modifiers together), so the first name in keyTable is the
	   canonical one for a key */
	numCodes = 0;
	for (i = 0; i < numKeys; i++) {
		SDL_Keycode code = NormalizeKey (keyTable[i].code);
		int j;

		for (j = 0; j < numCodes; j++) {
			if (codeTable[j].code == code) {
				break;
			}
		}
		if (j == numCodes) {
			codeTable[numCodes].code = code;
			codeTable[numCodes].atom = GUI_StringToAtom (keyTable[i].name);
			numCodes++;
		}
	}
	qsort (codeTable, numCodes, sizeof (AtomCodeTable), CompareByCode);
	/* that's all */
	return XBTrue;
}								/* InitKeysym */

/*
 *
 */
void
FinishKeysym (void)
{
	if (NULL != atomTable) {
		free (atomTable);
		atomTable = NULL;
	}
	if (NULL != codeTable) {
		free (codeTable);
		codeTable = NULL;
	}
	numKeys = 0;
	numCodes = 0;
}								/* FinishKeysym */

/*
 * convert string to keycode
 */
SDL_Keycode
StringToVirtualKey (const char *name)
{
	AtomCodeTable key;
	AtomCodeTable *result;
	XBAtom atom;

	atom = GUI_StringToAtom (name);
	key.atom = atom;
	result = bsearch (&key, atomTable, numKeys, sizeof (AtomCodeTable), CompareByAtom);
	if (NULL == result) {
		return SDLK_UNKNOWN;
	}
	return result->code;
}								/* StringToVirtualKey */

/*
 * convert keycode to string
 */
XBAtom
VirtualKeyToAtom (SDL_Keycode code)
{
	AtomCodeTable key;
	AtomCodeTable *result;
	XBAtom atom;

	key.code = NormalizeKey (code);
	result = bsearch (&key, codeTable, numCodes, sizeof (AtomCodeTable), CompareByCode);
	if (NULL == result) {
		return ATOM_INVALID;
	}
	atom = result->atom;
	return atom;
}								/* VirtualKeyToAtom */

/*
 * end of file w32_keysym.c
 */
