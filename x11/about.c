// ---------------------------------------------------------------------------------------
//  ABOUT.C - ABOUTダイアログ
// ---------------------------------------------------------------------------------------
#include	<windows.h>
#include	<windowsx.h>
#include	<shlobj.h>
#include	"common.h"
#include	"resource.h"
#include	"version.h"

LRESULT CALLBACK AboutDialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	char buf[4096];
	
	switch (msg)
	{
	case WM_INITDIALOG:
#ifndef WIN68DEBUG
		wsprintf(buf, "けろぴー (WinX68k)\nSHARP X680x0 series emulator\n"
					  "Version " APP_VER_STRING " w/ SSTP1.0\n"
					  "Copyright (C) 2000-02 Kenjo\n"
					  "\nUsing \"FM Sound Generator\" (C) cisc");
#else
		wsprintf(buf, "けろぴー 人柱版\nSHARP X680x0 series emulator\n"
					  "Version " APP_VER_STRING " w/ SSTP1.0\n"
					  "Copyright (C) 2000-02 Kenjo\n"
					  "\nUsing \"FM Sound Generator\" (C) cisc");
#endif
		SetDlgItemText(hdlg, IDC_ABOUT_TEXT, buf);

		SetFocus(GetDlgItem(hdlg, IDOK));

		return 0;

	case WM_COMMAND:
		if (IDOK == LOWORD(wp))
		{
			EndDialog(hdlg, TRUE);
			break;
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hdlg, FALSE);
		return TRUE;

	default:
		return FALSE;
	}
	return FALSE;
}
