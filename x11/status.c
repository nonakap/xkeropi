// -----------------------------------------------------------------------
//   FDD状態表示用ステータスバー
// -----------------------------------------------------------------------

#include "common.h"
#include "windraw.h"
#include "winx68k.h"
#include "prop.h"
#include "status.h"
#include "fileio.h"
#include "fdd.h"

HWND	hWndStat;
RECT	rectStat;

typedef struct {
	int insert;
	int access;
	int blink;
	int timer;
	char file[MAX_PATH];
} FDDLED;

static FDDLED FddLed[2];
static int HddLed = 0, HddLedTmp = 0;
int	heightStat;

static DWORD color[3] = { RGB(1, 1, 1), RGB(0, 192, 0), RGB(192, 0, 0) };


void StatBar_Redraw(void)
{
	if ( !Config.FullScrFDDStat ) return;

	StatBar_FDD(0, 0, FddLed[0].access);
	StatBar_FDD(0, 1, FddLed[0].insert);
	StatBar_FDD(1, 0, FddLed[1].access);
	StatBar_FDD(1, 1, FddLed[1].insert);
	StatBar_FDName(0, FddLed[0].file);
	StatBar_FDName(1, FddLed[1].file);
}


void StatBar_Show(int sw)
{
#if 0
	int	widths[4] = {200, 400, 450, -1};
//	char buf[255];
	HBRUSH hbrush, oldbr;
	HPEN hpen, oldpen;
	HDC dc;

	if ( sw ) {
		if ( !hWndStat ) {
			hWndStat = CreateStatusWindow(WS_CHILD/*|WS_VISIBLE*/, 0, hWndMain, 1);
			GetWindowRect(hWndStat, &rectStat);
			heightStat = rectStat.bottom-rectStat.top;
			winh += heightStat;
			MoveWindow(hWndMain, winx, winy, winw, winh, TRUE);
			MoveWindow(hWndStat, 0, winh-heightStat, rectStat.right-rectStat.left, winh, TRUE);
			SendMessage(hWndStat, SB_SETPARTS, 4, (LPARAM)widths);
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|0, 0);
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|1, 0);
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|2, 0);
			ShowWindow(hWndStat, SW_SHOW);
		} else {
			GetWindowRect(hWndStat, &rectStat);
			MoveWindow(hWndStat, 0, winh-heightStat, rectStat.right-rectStat.left, winh, TRUE);
		}
	} else {
		if ( hWndStat ) {
			winh -= heightStat;
			DestroyWindow(hWndStat);
			hWndStat = 0;
			MoveWindow(hWndMain, winx, winy, winw, winh, TRUE);
		}
	}
#endif
}


void StatBar_Draw(DRAWITEMSTRUCT* dis)
{
#if 0
	char buf[MAX_PATH];
	HBRUSH hbrs0, hbrs1, oldbrs;
	HPEN hpen0, hpen1, oldpen;
	int c;

	switch ( dis->itemID ) {
	case 0:
	case 1:
		c = ((FddLed[dis->itemID].blink)&&(!FddLed[dis->itemID].file[0]))?FddLed[dis->itemID].timer:FddLed[dis->itemID].access;
		hbrs0 = CreateSolidBrush(color[c]);
		hpen0 = CreatePen(PS_SOLID, 1, color[c]);
		hbrs1 = CreateSolidBrush(color[FddLed[dis->itemID].insert]);
		hpen1 = CreatePen(PS_SOLID, 1, color[FddLed[dis->itemID].insert]);
		oldbrs = (HBRUSH)SelectObject(dis->hDC, hbrs0);
		oldpen = (HPEN)  SelectObject(dis->hDC, hpen0);
		Ellipse(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+7, dis->rcItem.left+8, dis->rcItem.top+13);
		SelectObject(dis->hDC, hbrs1);
		SelectObject(dis->hDC, hpen1);
		Rectangle(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+3, dis->rcItem.left+8, dis->rcItem.top+5);
		SelectObject(dis->hDC, oldbrs);
		SelectObject(dis->hDC, oldpen);
		DeleteObject(hbrs0);
		DeleteObject(hpen0);
		DeleteObject(hbrs1);
		DeleteObject(hpen1);
		SetBkColor(dis->hDC, GetSysColor(COLOR_3DFACE));
		sprintf(buf, "%d:%s", dis->itemID, FddLed[dis->itemID].file);
		TextOut(dis->hDC, dis->rcItem.left+12, dis->rcItem.top+4, buf, strlen(buf));
		break;
	case 2:
		hbrs0 = CreateSolidBrush(color[HddLed]);
		hpen0 = CreatePen(PS_SOLID, 1, color[HddLed]);
		oldbrs = (HBRUSH)SelectObject(dis->hDC, hbrs0);
		oldpen = (HPEN)  SelectObject(dis->hDC, hpen0);
		Rectangle(dis->hDC, dis->rcItem.left+2, dis->rcItem.top+3, dis->rcItem.left+8, dis->rcItem.top+6);
		SelectObject(dis->hDC, oldbrs);
		SelectObject(dis->hDC, oldpen);
		DeleteObject(hbrs0);
		DeleteObject(hpen0);
		SetBkColor(dis->hDC, GetSysColor(COLOR_3DFACE));
		TextOut(dis->hDC, dis->rcItem.left+12, dis->rcItem.top+4, "HDD", 3);
		break;
	}
#endif
}


void StatBar_UpdateTimer(void)
{
	FddLed[0].timer ^= 1;
	FddLed[1].timer ^= 1;
	if ( ((FddLed[0].blink)&&(!FddLed[0].file[0]))||((FddLed[1].blink)&&(!FddLed[1].file[0])) ) {
		if ( hWndStat ) {
#if 0
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|0, (LPARAM)0);
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|1, (LPARAM)0);
#endif
		}
	}
	if ( HddLed!=HddLedTmp ) {
		HddLed = HddLedTmp;
		if ( hWndStat ) {
#if 0
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|2, (LPARAM)0);
#endif
		}
	}
	StatBar_Redraw();
}


void StatBar_HDD(int hd)
{
	HddLedTmp = hd;
	if ( (HddLed!=hd)&&(hd) ) {
#if 0
		PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|2, (LPARAM)0);
#endif
		HddLed = hd;
	}
	StatBar_Redraw();
}


void StatBar_SetFDD(int drv, char* file)
{
	char *f;
	if ( (drv<0)||(drv>1) ) return;
	f = (char *)getFileName(file);
	strcpy(FddLed[drv].file, f);
	if ( f[0] ) {
		FddLed[drv].access = 1;
		FddLed[drv].insert = 1;
	} else {
		FddLed[drv].access = 0;
		FddLed[drv].insert = 0;
	}
#if 0
	PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|drv, (LPARAM)0);
#endif
	StatBar_Redraw();
}


void StatBar_ParamFDD(int drv, int access, int insert, int blink)
{
	int update = 0;
	if ( (drv<0)||(drv>1) ) return;
	if ( FddLed[drv].access!=access ) {
		FddLed[drv].access = access;
		update = 1;
	}
	if ( FddLed[drv].insert!=insert ) {
		FddLed[drv].insert = insert;
		update = 1;
	}
	if ( FddLed[drv].blink!=blink ) {
		FddLed[drv].blink = blink;
		update = 1;
	}
	if ( update ) {
		if ( hWndStat ) {
#if 0
			PostMessage(hWndStat, SB_SETTEXT, SBT_OWNERDRAW|drv, (LPARAM)0);
#endif
		}
		StatBar_Redraw();
	}
}


void StatBar_FDName(int drv, char* name)
{
#if 0
	HDC dc;
	COLORREF oldcol, oldbk;
	HFONT font, oldfont;
	char buf[255];

	if ( !FullScreenFlag ) return;
	if ( !Config.FullScrFDDStat ) return;

	dc = GetDC(hWndMain);
	font = CreateFont(10, 5, 0, 0, FW_THIN, 0, 0, 0, SHIFTJIS_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "ＭＳ ゴシック");
	oldfont = (HFONT) SelectObject(dc, font);
	oldcol = SetTextColor(dc, RGB(128, 128, 128));
	oldbk  = SetBkColor(  dc, RGB(  0,   0,   0));
	sprintf(buf, "%d:%s", drv, getFileName(name));
	TextOut(dc, 28, drv*16+FULLSCREEN_POSY+512+2, buf, strlen(buf));
	SetTextColor(dc, oldcol);
	SetBkColor(dc, oldbk);
	SelectObject(dc, oldfont);
	DeleteObject(font);
	ReleaseDC(hWndMain, dc);
#endif
}


void StatBar_FDD(int drv, int led, int col)
{
#if 0
	HDC dc;
	HBRUSH hbrush, oldbr;
	HPEN hpen, oldpen;

	if ( !FullScreenFlag ) return;
	if ( !Config.FullScrFDDStat ) return;

	dc = GetDC(hWndMain);
	hbrush = CreateSolidBrush(color[col]);
	hpen   = CreatePen(PS_SOLID, 1, color[col]);
	oldbr  = (HBRUSH) SelectObject(dc, hbrush);
	oldpen = (HPEN)   SelectObject(dc, hpen);
	if ( led ) {
		Rectangle(dc, 14, FULLSCREEN_POSY+512+6+(drv*16), 20, FULLSCREEN_POSY+512+8+(drv*16));
	} else {
		Ellipse(dc, 4, FULLSCREEN_POSY+512+4+(drv*16), 10, FULLSCREEN_POSY+512+10+(drv*16));
	}
	SelectObject(dc, oldbr);
	SelectObject(dc, oldpen);
	DeleteObject(hbrush);
	DeleteObject(hpen);
	ReleaseDC(hWndMain, dc);
#endif
}
