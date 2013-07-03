#ifndef _winx68k_winui_h
#define _winx68k_winui_h

extern	BYTE	Debug_Text, Debug_Grp, Debug_Sp;
extern	DWORD	LastClock[4];

void WinUI_Init(void);
//LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifndef _winx68k_gtkui_h
#define _winx68k_gtkui_h

#include <gdk/gdk.h>
#include <gtk/gtk.h>

GtkWidget * create_menu(GtkWidget *w);

#endif //winx68k_gtkui_h

#endif //winx68k_winui_h
