// ---------------------------------------------------------------------------------------
//  COMMON - ɸ��إå�����COMMON.H�ˤȥ��顼��������ɽ���Ȥ�
// ---------------------------------------------------------------------------------------
#include	<windows.h>
#include	<stdio.h>

#include	"sstp.h"

extern HWND hWndMain;
extern const BYTE PrgTitle[];

void Error(const char* s)
{
	char title[80];
	sprintf(title, "%s ���顼\0", PrgTitle);

	SSTP_SendMes(SSTPMES_ERROR);

	MessageBox(hWndMain, s, title, MB_ICONERROR | MB_OK);
}

