/*	$Id$	*/

// MOUSE.C - �ޥ�������

/* 
 * Copyright (c) 2003 NONAKA Kimihiro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by NONAKA Kimihiro.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"
#include "winx68k.h"
#include "prop.h"
#include "scc.h"
#include "crtc.h"
#include "mouse.h"

int	MousePosX = 0;
int	MousePosY = 0;
BYTE	MouseStat = 0;
BYTE	MouseSW = 0;

POINT	CursorPos;
int	mousex=0, mousey=0;

static GdkPixmap *cursor_pixmap;
static GdkCursor *cursor;

static void getmaincenter(GtkWidget *w, POINT *p);
void gdk_window_set_pointer(GdkWindow *window, gint x, gint y);

void Mouse_Init(void)
{
	static gchar hide_cursor[16*16/8] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	static int inited = 0;

	if (!inited) {
		GtkWidget *w = window;

		inited = 1;

		cursor_pixmap = gdk_pixmap_create_from_data(w->window,
		    hide_cursor, 16, 16, 1, &w->style->black, &w->style->black);
		cursor = gdk_cursor_new_from_pixmap(cursor_pixmap,
		    cursor_pixmap, &w->style->black, &w->style->black, 0, 0);
	}

	MousePosX = (TextDotX / 2);
	MousePosY = (TextDotY / 2);
	MouseStat = 0;
}


// ----------------------------------
//	Mouse Event Occured
// ----------------------------------
void Mouse_Event(DWORD wparam, DWORD lparam)
{

	if (MouseSW) {
		switch (wparam) {
		case 0:	// �ޥ�����ư
			MousePosX = (lparam >> 16) & 0xffff;
			MousePosY = (lparam & 0xffff);
			break;

		case 1:	// ���ܥ���
			if (lparam == TRUE)
				MouseStat |= 1;
			else
				MouseStat &= 0xfe;
			break;

		case 2:	// ���ܥ���
			if (lparam == TRUE)
				MouseStat |= 2;
			else
				MouseStat &= 0xfd;
			break;

		default:
			break;
		}
	}
}


// ----------------------------------
//	Mouse Data send to SCC
// ----------------------------------
void Mouse_SetData(void)
{
	POINT pt;
	int x, y;

	if (MouseSW) {
		mousex += (MousePosX - (TextDotX / 2)) * Config.MouseSpeed;
		mousey += (MousePosY - (TextDotY / 2)) * Config.MouseSpeed;
		x = mousex / 10;
		y = mousey / 10;
		mousex -= x * 10;
		mousey -= y * 10;
		MouseSt = MouseStat;

		if (x > 127) {
			MouseSt |= 0x10;
			MouseX = 127;
		} else if (x < -128) {
			MouseSt |= 0x20;
			MouseX = -128;
		} else
			MouseX = (signed char)x;

		if (y > 127) {
			MouseSt |= 0x40;
			MouseY = 127;
		} else if (y < -128) {
			MouseSt |= 0x80;
			MouseY = -128;
		} else
			MouseY = (signed char)y;

		MousePosX = (TextDotX / 2);
		MousePosY = (TextDotY / 2);
		getmaincenter(window, &pt);
		gdk_window_set_pointer(window->window, pt.x, pt.y);
	} else {
		MouseSt = 0;
		MouseX = 0;
		MouseY = 0;
	}
}


// ----------------------------------
//	Start Capture
// ----------------------------------
void Mouse_StartCapture(int flag)
{
	GtkWidget *w = window;
	RECT rect;
	POINT pt;

	if (flag && !MouseSW) {
		int cx, cy;

		gdk_window_get_pointer(w->window, &cx, &cy, NULL);
		gdk_pointer_grab(w->window, TRUE, 0, w->window, cursor, 0);
		getmaincenter(w, &pt);
		gdk_window_set_pointer(w->window, pt.x, pt.y);

		CursorPos.x = (WORD)cx;
		CursorPos.y = (WORD)cy;
		MouseSW = 1;
	}

	if (!flag && MouseSW) {
		gdk_window_set_pointer(w->window, CursorPos.x, CursorPos.y);
		gdk_pointer_ungrab(0);

		MouseSW = 0;
	}
}


void Mouse_ChangePos(void)
{

	if (MouseSW) {
		POINT pt;

		getmaincenter(window, &pt);
		gdk_window_set_pointer(window->window, pt.x, pt.y);
	}
}

static void
getmaincenter(GtkWidget *w, POINT *p)
{

	p->x = w->allocation.x + w->allocation.width / 2;
	p->y = w->allocation.y + w->allocation.height / 2;
}

#if GTK_MAJOR_VERSION == 2
#include <gdk/x11/gdkx.h>
#include <gdk/x11/gdkdrawable-x11.h>
#include <gdk/x11/gdkscreen-x11.h>
#include <gdk/x11/gdkwindow-x11.h>

void
gdk_window_set_pointer(GdkWindow *window, gint x, gint y)
{
	GdkScreen *screen;
	GdkScreenX11 *screen_x11;

	if (window == 0) {
		screen = gdk_screen_get_default();
		window = gdk_screen_get_root_window(screen);
	} else
		screen = gdk_drawable_get_screen(window);
	if (GDK_WINDOW_DESTROYED(window))
		return;

	screen_x11 = GDK_SCREEN_X11(screen);
	XWarpPointer(screen_x11->xdisplay, None, GDK_WINDOW_XID(window),
	    0, 0, 0, 0, x, y);
}
#elif GTK_MAJOR_VERSION == 1
#include <gdk/gdkprivate.h>

void
gdk_window_set_pointer(GdkWindow *window, gint x, gint y)
{ 
	GdkWindowPrivate *private;

	if (!window)
		window = (GdkWindow *)&gdk_root_parent;

	private = (GdkWindowPrivate *)window;
	if (private->destroyed)
		return;

	XWarpPointer(private->xdisplay, None, private->xwindow,
	    0, 0, 0, 0, x, y);
}
#endif
