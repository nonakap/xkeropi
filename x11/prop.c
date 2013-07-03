// ---------------------------------------------------------------------------------------
//  PROP.C - 各種設定用プロパティシートと設定値管理
// ---------------------------------------------------------------------------------------

#include <sys/stat.h>

#include "common.h"
#include "winx68k.h"
#include "keyboard.h"
#include "fileio.h"
#include "prop.h"

BYTE	LastCode = 0;
BYTE	KEYCONFFILE[] = "keyconf.dat";

int	CurrentHDDNo = 0;

static const SSTPPorts[] = {11000, 9801, 0};

BYTE ini_title[] = "WinX68k";
static char X68KeyName[128][10] = {
	"         \0", "ESC\0      ", "1\0        ", "2\0        ", "3\0        ", "4\0        ", "5\0        ", "6\0        ", 
	"7\0        ", "8\0        ", "9\0        ", "0\0        ", "-\0        ", "^\0        ", "\\\0        ", "BS\0       ", 
	"TAB\0      ", "Q\0        ", "W\0        ", "E\0        ", "R\0        ", "T\0        ", "Y\0        ", "U\0        ", 
	"I\0        ", "O\0        ", "P\0        ", "@\0        ", "[\0        ", "ENTER\0    ", "A\0        ", "S\0        ", 
	"D\0        ", "F\0        ", "G\0        ", "H\0        ", "J\0        ", "K\0        ", "L\0        ", ";\0        ", 
	":\0        ", "]\0        ", "Z\0        ", "X\0        ", "C\0        ", "V\0        ", "B\0        ", "N\0        ", 
	"M\0        ", ",\0        ", ".\0        ", "/\0        ", "_\0        ", "         \0", "HOME\0     ", "DEL\0      ", 
	"ROLL UP\0  ", "ROLL DOWN\0", "UNDO\0     ", "←\0       ", "↑\0       ", "→\0       ", "↓\0       ", "CLR\0      ", 
	"/(10キー)\0", "*(10キー)\0", "-(10キー)\0", "7(10キー)\0", "8(10キー)\0", "9(10キー)\0", "+(10キー)\0", "4(10キー)\0", 
	"5(10キー)\0", "6(10キー)\0", "=(10キー)\0", "1(10キー)\0", "2(10キー)\0", "3(10キー)\0", "ENTER(10)\0", "0(10キー)\0", 
	",(10キー)\0", ".(10キー)\0", "記号入力\0 ", "登録\0     ", "HELP\0     ", "XF1\0      ", "XF2\0      ", "XF3\0      ", 
	"XF4\0      ", "XF5\0      ", "かな\0     ", "ローマ字\0 ", "カナ入力\0 ", "CAPS\0     ", "INS\0      ", "ひらがな\0 ", 
	"全角\0     ", "BREAK\0    ", "COPY\0     ", "F1\0       ", "F2\0       ", "F3\0       ", "F4\0       ", "F5\0       ", 
	"F6\0       ", "F7\0       ", "F8\0       ", "F9\0       ", "F10\0      ", "         \0", "         \0", "         \0", 
	"SHIFT\0    ", "CTRL\0     ", "OPT.1\0    ", "OPT.2\0    ", "         \0", "         \0", "         \0", "         \0", 
	"         \0", "         \0", "         \0", "         \0", "         \0", "         \0", "         \0", "         \0", 
};

static char MIDI_TYPE_NAME[4][7] = {
	"LA音源\0", "GM音源\0", "GS音源\0", "XG音源\0"
};

BYTE KeyTableBk[512];

Win68Conf Config;
Win68Conf ConfBk;

#ifndef MAX_BUTTON
#define MAX_BUTTON 32
#endif

extern char filepath[MAX_PATH];
extern char winx68k_ini[MAX_PATH];
extern int winx, winy;
#if 0
extern LPDIRECTINPUTDEVICE2 joy[2];
#endif
extern char joyname[2][MAX_PATH];
extern char joybtnname[2][MAX_BUTTON][MAX_PATH];
extern BYTE joybtnnum[2];
extern BYTE FrameRate;

#define CFGLEN MAX_PATH
#define BSTATE(b) (b ? BST_CHECKED : BST_UNCHECKED)

static long solveHEX(char *str) {

	long	ret;
	int		i;
	char	c;

	ret = 0;
	for (i=0; i<8; i++) {
		c = *str++;
		if ((c >= '0') && (c <= '9')) {
			c -= '0';
		}
		else if ((c >= 'A') && (c <= 'F')) {
			c -= '7';
		}
		else if ((c >= 'a') && (c <= 'f')) {
			c -= 'W';
		}
		else {
			break;
		}
		ret <<= 4;
		ret += (long) c;
	}
	return(ret);
}

static char *makeBOOL(BYTE value) {

	if (value) {
		return("true");
	}
	return("false");
}

static BYTE Aacmp(char *cmp, char *str) {

	char	p;

	while(*str) {
		p = *cmp++;
		if (!p) {
			break;
		}
		if (p >= 'a' && p <= 'z') {
			p -= 0x20;
		}
		if (p != *str++) {
			return(-1);
		}
	}
	return(0);
}

static BYTE solveBOOL(char *str) {

	if ((!Aacmp(str, "TRUE")) || (!Aacmp(str, "ON")) ||
		(!Aacmp(str, "+")) || (!Aacmp(str, "1")) ||
		(!Aacmp(str, "ENABLE"))) {
		return(1);
	}
	return(0);
}

int
set_modulepath(char *path, size_t len)
{
	struct stat sb;

	snprintf(path, len, "%s/%s", getenv("HOME"), ".keropi");
	if (stat(path, &sb) < 0) {
		if (mkdir(path, 0700) < 0) {
			perror(path);
			return 1;
		}
	} else {
		if ((sb.st_mode & S_IFDIR) == 0) {
			fprintf(stderr, "%s isn't directory.\n", path);
			return 1;
		}
	}
	snprintf(winx68k_ini, sizeof(winx68k_ini), "%s/%s", path, "config");
	if (stat(winx68k_ini, &sb) >= 0) {
		if (sb.st_mode & S_IFDIR) {
			fprintf(stderr, "%s is directory.\n", winx68k_ini);
			return 1;
		}
	}

#if 0
	snprintf(fontfile, sizeof(fontfile), "%s/%s", modulefile, "font.bmp");
	if (stat(fontfile, &sb) >= 0) {
		if (sb.st_mode & S_IFDIR) {
			fprintf(stderr, "%s is directory.\n", fontfile);
			return 1;
		}
	}
#endif
	return 0;
}

void LoadConfig(void)
{
	int	i, j;
	char	buf[CFGLEN];
	FILEH fp;

#if 0
	winx = GetPrivateProfileInt(ini_title, "WinPosX", CW_USEDEFAULT, winx68k_ini);
	winy = GetPrivateProfileInt(ini_title, "WinPosY", CW_USEDEFAULT, winx68k_ini);
#endif
	FrameRate = (BYTE)GetPrivateProfileInt(ini_title, "FrameRate", 7, winx68k_ini);
	if (!FrameRate) FrameRate=7;
	GetPrivateProfileString(ini_title, "StartDir", "", buf, MAX_PATH, winx68k_ini);
	if (buf[0] != 0)
		strcpy(filepath, buf);
	else
		filepath[0] = 0;

	Config.OPM_VOL = GetPrivateProfileInt(ini_title, "OPM_Volume", 12, winx68k_ini);
	Config.PCM_VOL = GetPrivateProfileInt(ini_title, "PCM_Volume", 15, winx68k_ini);
	Config.MCR_VOL = GetPrivateProfileInt(ini_title, "MCR_Volume", 13, winx68k_ini);
	Config.SampleRate = GetPrivateProfileInt(ini_title, "SampleRate", 22050, winx68k_ini);
	Config.BufferSize = GetPrivateProfileInt(ini_title, "BufferSize", 50, winx68k_ini);

	Config.MouseSpeed = GetPrivateProfileInt(ini_title, "MouseSpeed", 10, winx68k_ini);

	GetPrivateProfileString(ini_title, "FDDStatWin", "1", buf, CFGLEN, winx68k_ini);
	Config.WindowFDDStat = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "FDDStatFullScr", "1", buf, CFGLEN, winx68k_ini);
	Config.FullScrFDDStat = solveBOOL(buf);

	GetPrivateProfileString(ini_title, "DSAlert", "1", buf, CFGLEN, winx68k_ini);
	Config.DSAlert = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "SoundLPF", "1", buf, CFGLEN, winx68k_ini);
	Config.Sound_LPF = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "UseRomeo", "0", buf, CFGLEN, winx68k_ini);
	Config.SoundROMEO = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "MIDI_SW", "1", buf, CFGLEN, winx68k_ini);
	Config.MIDI_SW = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "MIDI_Reset", "0", buf, CFGLEN, winx68k_ini);
	Config.MIDI_Reset = solveBOOL(buf);
	Config.MIDI_Type = GetPrivateProfileInt(ini_title, "MIDI_Type", 1, winx68k_ini);

	GetPrivateProfileString(ini_title, "JoySwap", "0", buf, CFGLEN, winx68k_ini);
	Config.JoySwap = solveBOOL(buf);

	GetPrivateProfileString(ini_title, "JoyKey", "0", buf, CFGLEN, winx68k_ini);
	Config.JoyKey = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "JoyKeyReverse", "0", buf, CFGLEN, winx68k_ini);
	Config.JoyKeyReverse = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "JoyKeyJoy2", "0", buf, CFGLEN, winx68k_ini);
	Config.JoyKeyJoy2 = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "SRAMBootWarning", "1", buf, CFGLEN, winx68k_ini);
	Config.SRAMWarning = solveBOOL(buf);

	GetPrivateProfileString(ini_title, "WinDrvLFN", "1", buf, CFGLEN, winx68k_ini);
	Config.LongFileName = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "WinDrvFDD", "1", buf, CFGLEN, winx68k_ini);
	Config.WinDrvFD = solveBOOL(buf);

	Config.WinStrech = GetPrivateProfileInt(ini_title, "WinStretch", 1, winx68k_ini);

	GetPrivateProfileString(ini_title, "DSMixing", "0", buf, CFGLEN, winx68k_ini);
	Config.DSMixing = solveBOOL(buf);

	Config.XVIMode = (BYTE)GetPrivateProfileInt(ini_title, "XVIMode", 0, winx68k_ini);

	GetPrivateProfileString(ini_title, "CDROM_ASPI", "1", buf, CFGLEN, winx68k_ini);
	Config.CDROM_ASPI = solveBOOL(buf);
	Config.CDROM_SCSIID = (BYTE)GetPrivateProfileInt(ini_title, "CDROM_SCSIID", 6, winx68k_ini);
	Config.CDROM_ASPI_Drive = (BYTE)GetPrivateProfileInt(ini_title, "CDROM_ASPIDrv", 0, winx68k_ini);
	Config.CDROM_IOCTRL_Drive = (BYTE)GetPrivateProfileInt(ini_title, "CDROM_CTRLDrv", 16, winx68k_ini);
	GetPrivateProfileString(ini_title, "CDROM_Enable", "1", buf, CFGLEN, winx68k_ini);
	Config.CDROM_Enable = solveBOOL(buf);

	GetPrivateProfileString(ini_title, "SSTP_Enable", "0", buf, CFGLEN, winx68k_ini);
	Config.SSTP_Enable = solveBOOL(buf);
	Config.SSTP_Port = GetPrivateProfileInt(ini_title, "SSTP_Port", 11000, winx68k_ini);

	GetPrivateProfileString(ini_title, "ToneMapping", "0", buf, CFGLEN, winx68k_ini);
	Config.ToneMap = solveBOOL(buf);
	GetPrivateProfileString(ini_title, "ToneMapFile", "", buf, MAX_PATH, winx68k_ini);
	if (buf[0] != 0)
		strcpy(Config.ToneMapFile, buf);
	else
		Config.ToneMapFile[0] = 0;

	Config.MIDIDelay = GetPrivateProfileInt(ini_title, "MIDIDelay", Config.BufferSize*5, winx68k_ini);
	Config.MIDIAutoDelay = GetPrivateProfileInt(ini_title, "MIDIAutoDelay", 1, winx68k_ini);

	for (i=0; i<2; i++)
	{
		for (j=0; j<8; j++)
		{
			sprintf(buf, "Joy%dButton%d", i+1, j+1);
			Config.JOY_BTN[i][j] = GetPrivateProfileInt(ini_title, buf, j, winx68k_ini);
		}
	}

	for (i=0; i<16; i++)
	{
		sprintf(buf, "HDD%d", i);
		GetPrivateProfileString(ini_title, buf, "", Config.HDImage[i], MAX_PATH, winx68k_ini);
	}

	fp = File_OpenCurDir(KEYCONFFILE);
	if (fp)
	{
		File_Read(fp, KeyTable, 512);
		File_Close(fp);
	}
}


void SaveConfig(void)
{
	int	i, j;
	char	buf[CFGLEN], buf2[CFGLEN];
	FILEH fp;

	wsprintf(buf, "%d", winx);
	WritePrivateProfileString(ini_title, "WinPosX", buf, winx68k_ini);
	wsprintf(buf, "%d", winy);
	WritePrivateProfileString(ini_title, "WinPosY", buf, winx68k_ini);
	wsprintf(buf, "%d", FrameRate);
	WritePrivateProfileString(ini_title, "FrameRate", buf, winx68k_ini);
	WritePrivateProfileString(ini_title, "StartDir", filepath, winx68k_ini);

	wsprintf(buf, "%d", Config.OPM_VOL);
	WritePrivateProfileString(ini_title, "OPM_Volume", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.PCM_VOL);
	WritePrivateProfileString(ini_title, "PCM_Volume", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.MCR_VOL);
	WritePrivateProfileString(ini_title, "MCR_Volume", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.SampleRate);
	WritePrivateProfileString(ini_title, "SampleRate", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.BufferSize);
	WritePrivateProfileString(ini_title, "BufferSize", buf, winx68k_ini);

	wsprintf(buf, "%d", Config.MouseSpeed);
	WritePrivateProfileString(ini_title, "MouseSpeed", buf, winx68k_ini);

	WritePrivateProfileString(ini_title, "FDDStatWin", makeBOOL((BYTE)Config.WindowFDDStat), winx68k_ini);
	WritePrivateProfileString(ini_title, "FDDStatFullScr", makeBOOL((BYTE)Config.FullScrFDDStat), winx68k_ini);

	WritePrivateProfileString(ini_title, "DSAlert", makeBOOL((BYTE)Config.DSAlert), winx68k_ini);
	WritePrivateProfileString(ini_title, "SoundLPF", makeBOOL((BYTE)Config.Sound_LPF), winx68k_ini);
	WritePrivateProfileString(ini_title, "UseRomeo", makeBOOL((BYTE)Config.SoundROMEO), winx68k_ini);
	WritePrivateProfileString(ini_title, "MIDI_SW", makeBOOL((BYTE)Config.MIDI_SW), winx68k_ini);
	WritePrivateProfileString(ini_title, "MIDI_Reset", makeBOOL((BYTE)Config.MIDI_Reset), winx68k_ini);
	wsprintf(buf, "%d", Config.MIDI_Type);
	WritePrivateProfileString(ini_title, "MIDI_Type", buf, winx68k_ini);

	WritePrivateProfileString(ini_title, "JoySwap", makeBOOL((BYTE)Config.JoySwap), winx68k_ini);

	WritePrivateProfileString(ini_title, "JoyKey", makeBOOL((BYTE)Config.JoyKey), winx68k_ini);
	WritePrivateProfileString(ini_title, "JoyKeyReverse", makeBOOL((BYTE)Config.JoyKeyReverse), winx68k_ini);
	WritePrivateProfileString(ini_title, "JoyKeyJoy2", makeBOOL((BYTE)Config.JoyKeyJoy2), winx68k_ini);
	WritePrivateProfileString(ini_title, "SRAMBootWarning", makeBOOL((BYTE)Config.SRAMWarning), winx68k_ini);

	WritePrivateProfileString(ini_title, "WinDrvLFN", makeBOOL((BYTE)Config.LongFileName), winx68k_ini);
	WritePrivateProfileString(ini_title, "WinDrvFDD", makeBOOL((BYTE)Config.WinDrvFD), winx68k_ini);

	wsprintf(buf, "%d", Config.WinStrech);
	WritePrivateProfileString(ini_title, "WinStretch", buf, winx68k_ini);

	WritePrivateProfileString(ini_title, "DSMixing", makeBOOL((BYTE)Config.DSMixing), winx68k_ini);

	wsprintf(buf, "%d", Config.XVIMode);
	WritePrivateProfileString(ini_title, "XVIMode", buf, winx68k_ini);

	WritePrivateProfileString(ini_title, "CDROM_ASPI", makeBOOL((BYTE)Config.CDROM_ASPI), winx68k_ini);
	wsprintf(buf, "%d", Config.CDROM_SCSIID);
	WritePrivateProfileString(ini_title, "CDROM_SCSIID", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.CDROM_ASPI_Drive);
	WritePrivateProfileString(ini_title, "CDROM_ASPIDrv", buf, winx68k_ini);
	wsprintf(buf, "%d", Config.CDROM_IOCTRL_Drive);
	WritePrivateProfileString(ini_title, "CDROM_CTRLDrv", buf, winx68k_ini);
	WritePrivateProfileString(ini_title, "CDROM_Enable", makeBOOL((BYTE)Config.CDROM_Enable), winx68k_ini);

	WritePrivateProfileString(ini_title, "SSTP_Enable", makeBOOL((BYTE)Config.SSTP_Enable), winx68k_ini);
	wsprintf(buf, "%d", Config.SSTP_Port);
	WritePrivateProfileString(ini_title, "SSTP_Port", buf, winx68k_ini);

	WritePrivateProfileString(ini_title, "ToneMapping", makeBOOL((BYTE)Config.ToneMap), winx68k_ini);
	WritePrivateProfileString(ini_title, "ToneMapFile", Config.ToneMapFile, winx68k_ini);

	wsprintf(buf, "%d", Config.MIDIDelay);
	WritePrivateProfileString(ini_title, "MIDIDelay", buf, winx68k_ini);
	WritePrivateProfileString(ini_title, "MIDIAutoDelay", makeBOOL((BYTE)Config.MIDIAutoDelay), winx68k_ini);

	for (i=0; i<2; i++)
	{
		for (j=0; j<8; j++)
		{
			sprintf(buf, "Joy%dButton%d", i+1, j+1);
			wsprintf(buf2, "%d", Config.JOY_BTN[i][j]);
			WritePrivateProfileString(ini_title, buf, buf2, winx68k_ini);
		}
	}

	for (i=0; i<16; i++)
	{
		sprintf(buf, "HDD%d", i);
		WritePrivateProfileString(ini_title, buf, Config.HDImage[i], winx68k_ini);
	}

	fp = File_OpenCurDir(KEYCONFFILE);
	if (!fp)
		fp = File_CreateCurDir(KEYCONFFILE, FTYPE_TEXT);
	if (fp)
	{
		File_Write(fp, KeyTable, 512);
		File_Close(fp);
	}
}


#if 0
static LRESULT CALLBACK PropDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
void PropPage_Init(void);

extern HINSTANCE hInst;
extern HWND hWndMain;

#define NUM_PROPSHEETS  8

static DWORD dwDlgId[] = {
	IDD_PROP_SOUND,
	IDD_PROP_MIDI,
	IDD_PROP_KEYBOARD,
	IDD_PROP_JOYSTICK1,
	IDD_PROP_JOYSTICK2,
	IDD_PROP_MOUSE,
	IDD_PROP_SCSI,
	IDD_PROP_OTHERS,
};
#endif

void PropPage_Init(void)
{
#if 0
	PROPSHEETPAGE	pspage[NUM_PROPSHEETS];
	PROPSHEETHEADER	pshead;
	int i;

	pshead.hwndParent     = hWndMain;
	pshead.dwSize         = sizeof(PROPSHEETHEADER);
	pshead.dwFlags        = PSH_PROPSHEETPAGE | PSH_USEICONID ;
	pshead.hInstance      = hInst;
	pshead.pszCaption     = "けろぴーの行動設定";
	pshead.nPages         = NUM_PROPSHEETS;
	pshead.nStartPage     = 0;
	pshead.pszIcon        = MAKEINTRESOURCE(IDI_MAIN_ICON);
	pshead.ppsp           = pspage;

	for (i = 0; i < NUM_PROPSHEETS; i++)
	{
		pspage[i].dwSize      = sizeof(PROPSHEETPAGE);
		pspage[i].dwFlags     = 0;
		pspage[i].hInstance   = hInst;
		pspage[i].pszTemplate = MAKEINTRESOURCE(dwDlgId[i]);
		pspage[i].pfnCallback = NULL;
		pspage[i].lParam      = 0;
		pspage[i].pfnDlgProc  = PropDialogProc;
	}

	if (PropertySheet(&pshead) == -1)
		Error("プロパティシートの作成に失敗しました。");
#endif
}


void Setup_JoystickPage(HWND hDlg)
{
#if 0
	int i;
	char *nobutton="なし";

	if (joy[0])
	{
		SetDlgItemText(hDlg, IDC_JOY1_NAME, joyname[0]);

		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN1), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN2), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN3), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN4), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN5), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN6), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN7), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN8), TRUE);

		SendDlgItemMessage(hDlg, IDC_JOY1_BTN1, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN2, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN3, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN4, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_ADDSTRING, 0, (long)nobutton);
		for (i=0; i<joybtnnum[0]; i++)
		{
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN1, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN2, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN3, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN4, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_ADDSTRING, 0, (long)joybtnname[0][i]);
		}
		if (Config.JOY_BTN[0][0]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN1, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][0]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN1, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][1]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN2, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][1]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN2, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][2]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN3, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][2]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN3, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][3]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN4, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][3]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN4, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][4]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][4]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][5]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][5]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][6]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][6]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[0][7]<joybtnnum[0])
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_SELECTSTRING, 0, (long)joybtnname[0][Config.JOY_BTN[0][7]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_SELECTSTRING, 0, (long)nobutton);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN1), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN2), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN3), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN4), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN5), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN6), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN7), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY1_BTN8), FALSE);
	}

	if (joy[1])
	{
		SetDlgItemText(hDlg, IDC_JOY2_NAME, joyname[1]);

		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN1), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN2), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN3), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN4), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN5), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN6), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN7), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN8), TRUE);

		SendDlgItemMessage(hDlg, IDC_JOY2_BTN1, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN2, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN3, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN4, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN5, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN6, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN7, CB_ADDSTRING, 0, (long)nobutton);
		SendDlgItemMessage(hDlg, IDC_JOY2_BTN8, CB_ADDSTRING, 0, (long)nobutton);
		for (i=0; i<joybtnnum[1]; i++)
		{
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN1, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN2, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN3, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN4, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN5, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN6, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN7, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN8, CB_ADDSTRING, 0, (long)joybtnname[1][i]);
		}
		if (Config.JOY_BTN[1][0]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN1, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][0]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN1, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][1]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN2, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][1]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN2, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][2]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN3, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][2]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN3, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][3]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN4, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][3]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN4, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][4]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN5, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][4]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN5, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][5]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN6, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][51]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN6, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][6]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN7, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][6]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN7, CB_SELECTSTRING, 0, (long)nobutton);
		if (Config.JOY_BTN[1][7]<joybtnnum[1])
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN8, CB_SELECTSTRING, 0, (long)joybtnname[1][Config.JOY_BTN[1][7]]);
		else
			SendDlgItemMessage(hDlg, IDC_JOY2_BTN8, CB_SELECTSTRING, 0, (long)nobutton);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN1), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN2), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN3), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN4), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN5), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN6), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN7), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_JOY2_BTN8), FALSE);
	}
#endif
}


// キーアサイン変更
void Prop_KeyCodeSet(HWND hDlg, BYTE key)
{
#if 0
//	int i;
	WORD code;
	char buf[200], keyname[20];

	sprintf(KeyConfMessage, "X68kの「%s」キーに割り当てたいキーを押してください。\n\n（F12キーでキャンセル）\0", X68KeyName[key]);
	SetDlgItemText(hDlg, IDC_KEYCONF_TEXT, KeyConfMessage);
//	SetDlgItemText(hDlg, IDC_KEYCONF_DLG, KeyConfMessage);

	hWndKeyConf = (HWND)DialogBox(hInst, MAKEINTRESOURCE(IDD_KEYCONF), hDlg, &KeyConfProc);
	if (!KeyConf_CodeW)
	{
		sprintf(buf, "キャンセルしました。\0");
		SetDlgItemText(hDlg, IDC_KEYCONF_TEXT, buf);
		return;			// 変更なし
	}

	code = (WORD)((KeyConf_CodeW&0xff)|((KeyConf_CodeL>>16)&0x0100));
/*
	for (i=0; i<512; i++)
	{
		if ( (KeyTable[i]==key) && (i!=code) )
		{
		}
	}
*/
	KeyTable[code] = key;

	GetKeyNameText(KeyConf_CodeL, keyname, 20);
	sprintf(buf, "X68kの「%s」キーとして、Windowsの「%s」キーを割り当てました。", X68KeyName[key], keyname);
	SetDlgItemText(hDlg, IDC_KEYCONF_TEXT, buf);
#endif
}


FILEH MakeHDDFile = NULL;
int MakeHDDSize = 0;
int MakeHDDCurSize = 0;


#if 0
static LRESULT CALLBACK MakeHDDProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
	int i, j;
	char* hddbuf, hddbuf2;
	char buf[200];
	switch (Msg)
	{
	case WM_INITDIALOG :
		SendDlgItemMessage(hDlg, IDC_PROGBAR, PBM_SETRANGE, 0, MAKELPARAM(0,MakeHDDSize));
		MakeHDDCurSize = 0;

		if (MakeHDDFile)
			SetTimer(hDlg,0,1,NULL);
		else
			EndDialog(hDlg,0);
			
		return 1;

	case WM_TIMER :
		for (j=0; j<10; j++)
		{
			if (MakeHDDCurSize != MakeHDDSize)
			{
				sprintf(buf, "Making HDD Image ...\n(%d/%d)", MakeHDDCurSize, MakeHDDSize);
			        SetDlgItemText( hDlg, IDC_PROGTEXT, buf );
				hddbuf = malloc(0x1000);
				if (hddbuf)
				{
					ZeroMemory(hddbuf, 0x1000);
					File_Write(MakeHDDFile, hddbuf, 0x1000);
					free(hddbuf);
				}
				else
				{
					hddbuf2 = 0;
					for (i=0; i<0x1000; i++) File_Write(MakeHDDFile, &hddbuf2, 1);
				}
				MakeHDDCurSize++;
				SendDlgItemMessage(hDlg, IDC_PROGBAR, PBM_SETPOS, MakeHDDCurSize, 0);
			}
			else
			{
				KillTimer(hDlg,0);
				EndDialog(hDlg,0);
				break;
			}
		}
		break;
  
	case WM_COMMAND :
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		//	EndDialog(hDlg,0);
			break;
		}
		return 1;
	}
	return 0;
}


static LRESULT CALLBACK PropDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	char buf[255];
	char keyname[20];
	int temp;
	int i, j;
	WORD wp;
	unsigned int id, code;
	DWORD keycode;
	OPENFILENAME ofn;
	int isopen, openflag = 0;
	char filename[MAX_PATH];
	FILEH fp;

/*{
FILE *fp;
fp=fopen("_mes.txt", "a");
fprintf(fp, "Message - Mes:%08X  WP:%04X  LP:%04X\n", Msg, wParam, lParam);
fclose(fp);
}*/
	switch (Msg)
	{
	case WM_INITDIALOG:
		{
			HWND hWnd = PropSheet_GetTabControl(GetParent(hDlg));
			DWORD tabStyle = (GetWindowLong(hWnd,GWL_STYLE) & ~TCS_MULTILINE);
			SetWindowLong(hWnd,GWL_STYLE,tabStyle | TCS_SINGLELINE);

			memcpy(&ConfBk, &Config, sizeof(Win68Conf));
			memcpy(KeyTableBk, KeyTable, 512);

			CheckDlgButton(hDlg, IDC_RATE_NON, BSTATE((Config.SampleRate==0)));
			CheckDlgButton(hDlg, IDC_RATE_48,  BSTATE((Config.SampleRate==48000)));
			CheckDlgButton(hDlg, IDC_RATE_44,  BSTATE((Config.SampleRate==44100)));
			CheckDlgButton(hDlg, IDC_RATE_22,  BSTATE((Config.SampleRate==22050)));
			CheckDlgButton(hDlg, IDC_RATE_11,  BSTATE((Config.SampleRate==11025)));

			SendDlgItemMessage(hDlg, IDC_OPMVOL, TBM_SETRANGE, TRUE, MAKELONG(0, 16));
			SendDlgItemMessage(hDlg, IDC_OPMVOL, TBM_SETPOS, TRUE, Config.OPM_VOL);
			SetDlgItemInt(hDlg, IDC_OPMVOL_TEXT, Config.OPM_VOL, FALSE);
			SendDlgItemMessage(hDlg, IDC_PCMVOL, TBM_SETRANGE, TRUE, MAKELONG(0, 16));
			SendDlgItemMessage(hDlg, IDC_PCMVOL, TBM_SETPOS, TRUE, Config.PCM_VOL);
			SetDlgItemInt(hDlg, IDC_PCMVOL_TEXT, Config.PCM_VOL, FALSE);
			SendDlgItemMessage(hDlg, IDC_MCRVOL, TBM_SETRANGE, TRUE, MAKELONG(0, 16));
			SendDlgItemMessage(hDlg, IDC_MCRVOL, TBM_SETPOS, TRUE, Config.MCR_VOL);
			SetDlgItemInt(hDlg, IDC_MCRVOL_TEXT, Config.MCR_VOL, FALSE);
			SendDlgItemMessage(hDlg, IDC_BUFSIZE, TBM_SETRANGE, TRUE, MAKELONG(10, 200));
			SendDlgItemMessage(hDlg, IDC_BUFSIZE, TBM_SETPOS, TRUE, Config.BufferSize);
			SetDlgItemInt(hDlg, IDC_BUFSIZE_TEXT, Config.BufferSize, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), (Config.SampleRate!=0));
			EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), (Config.SampleRate!=0));

			if ( Config.MIDIAutoDelay ) {
				Config.MIDIDelay = Config.BufferSize*5;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_MIDIDELAY), !Config.MIDIAutoDelay);
			CheckDlgButton(hDlg, IDC_MIDIDELAYAUTO, BSTATE(Config.MIDIAutoDelay));
			SendDlgItemMessage(hDlg, IDC_MIDIDELAY, TBM_SETRANGE, TRUE, MAKELONG(0, 1000));
			SendDlgItemMessage(hDlg, IDC_MIDIDELAY, TBM_SETPOS, TRUE, Config.MIDIDelay);
			wsprintf(buf, "%dms", Config.MIDIDelay);
			SetDlgItemText(hDlg, IDC_MIDIDELAYTEXT, buf);

			SendDlgItemMessage(hDlg, IDC_MOUSESPEED, TBM_SETRANGE, TRUE, MAKELONG(1, 20));
			SendDlgItemMessage(hDlg, IDC_MOUSESPEED, TBM_SETPOS, TRUE, Config.MouseSpeed);

			CheckDlgButton(hDlg, IDC_STATUS, BSTATE(Config.WindowFDDStat));
			CheckDlgButton(hDlg, IDC_FULLSCREENFDD, BSTATE(Config.FullScrFDDStat));

			CheckDlgButton(hDlg, IDC_DSALERT, BSTATE(Config.DSAlert));
			CheckDlgButton(hDlg, IDC_SOUNDLPF, BSTATE(Config.Sound_LPF));
			CheckDlgButton(hDlg, IDC_SOUNDROMEO, BSTATE(Config.SoundROMEO));
			CheckDlgButton(hDlg, IDC_MIDI, BSTATE(Config.MIDI_SW));

			CheckDlgButton(hDlg, IDC_MIDIRESET, BSTATE(Config.MIDI_Reset));

			CheckDlgButton(hDlg, IDC_SRAMWARNING, BSTATE(Config.SRAMWarning));

			CheckDlgButton(hDlg, IDC_DSMIXING, BSTATE(Config.DSMixing));

			CheckDlgButton(hDlg, IDC_LONGFILENAME, BSTATE(Config.LongFileName));
			CheckDlgButton(hDlg, IDC_WINDRVFD, BSTATE(Config.WinDrvFD));

			CheckDlgButton(hDlg, IDC_JOYSWAP, BSTATE(Config.JoySwap));

			CheckDlgButton(hDlg, IDC_SSTP, BSTATE(Config.SSTP_Enable));
			for (i=0; SSTPPorts[i]; i++) {
				sprintf(buf, "%d", SSTPPorts[i]);
				SendDlgItemMessage(hDlg, IDC_SSTPPORT, CB_ADDSTRING, 0, (long)buf);
			}
			sprintf(buf, "%d", Config.SSTP_Port);
			SendDlgItemMessage(hDlg, IDC_SSTPPORT, CB_SELECTSTRING, 0, (long)buf);
			EnableWindow(GetDlgItem(hDlg, IDC_SSTPPORT), Config.SSTP_Enable);
			EnableWindow(GetDlgItem(hDlg, IDC_SSTPPORT_TEXT), Config.SSTP_Enable);

			for (i=0; i<7; i++)
			{
				buf[0] = i+0x30;
				buf[1] = 0;
				SendDlgItemMessage(hDlg, IDC_CDROM_SCSIID, CB_ADDSTRING, 0, (long)buf);
			}
			buf[0] = Config.CDROM_SCSIID+0x30;
			SendDlgItemMessage(hDlg, IDC_CDROM_SCSIID, CB_SELECTSTRING, 0, (long)buf);
			CheckDlgButton(hDlg, IDC_CDROM_ASPI, BSTATE(Config.CDROM_ASPI));
			CheckDlgButton(hDlg, IDC_CDROM_IOCTRL, BSTATE(!Config.CDROM_ASPI));
			for (i=0; i<CDASPI_CDNum; i++)
				SendDlgItemMessage(hDlg, IDC_CDROM_ASPIID, CB_ADDSTRING, 0, (LPARAM)CDASPI_CDName[i]);
			SendDlgItemMessage(hDlg, IDC_CDROM_ASPIID, CB_SELECTSTRING, 0, (LPARAM)CDASPI_CDName[Config.CDROM_ASPI_Drive]);
			for (i=0; i<26; i++)
			{
				sprintf(buf, "%c:", i+0x41);
				SendDlgItemMessage(hDlg, IDC_CDROM_CTRLID, CB_ADDSTRING, 0, (LPARAM)buf);
			}
			sprintf(buf, "%c:", Config.CDROM_IOCTRL_Drive+0x41);
			SendDlgItemMessage(hDlg, IDC_CDROM_CTRLID, CB_SELECTSTRING, 0, (LPARAM)buf);

			EnableWindow(GetDlgItem(hDlg, IDC_SCSIID_TEXT), Config.CDROM_Enable);
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_SCSIID), Config.CDROM_Enable);
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_IOCTRL), Config.CDROM_Enable);
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPI), Config.CDROM_Enable);
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID), (Config.CDROM_Enable)&&(!Config.CDROM_ASPI));
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID_TEXT), (Config.CDROM_Enable)&&(!Config.CDROM_ASPI));
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID), (Config.CDROM_Enable)&&(Config.CDROM_ASPI));
			EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID_TEXT), (Config.CDROM_Enable)&&(Config.CDROM_ASPI));
			CheckDlgButton(hDlg, IDC_CDROM_ENABLE, BSTATE(Config.CDROM_Enable));

			for (i=0; i<4; i++)
				SendDlgItemMessage(hDlg, IDC_MIDITYPE, CB_ADDSTRING, 0, (long)MIDI_TYPE_NAME[i]);
			SendDlgItemMessage(hDlg, IDC_MIDITYPE, CB_SELECTSTRING, 0, (long)MIDI_TYPE_NAME[Config.MIDI_Type]);
			if (Config.MIDI_SW)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITYPE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITEXT), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDIRESET), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIMAP), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILESEL), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITYPE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITEXT), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDIRESET), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIMAP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILESEL), FALSE);
			}

			for (i=0; i<16; i++)
			{
				sprintf(buf, "%d", i);
				SendDlgItemMessage(hDlg, IDC_HDDNO, CB_ADDSTRING, 0, (LPARAM)buf);
			}
			SendDlgItemMessage(hDlg, IDC_HDDNO, CB_SELECTSTRING, 0, (LPARAM)"0");
			SetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[0]);
			CurrentHDDNo = 0;

			CheckDlgButton(hDlg, IDC_MIMPIMAP, BSTATE(Config.ToneMap));
			SetDlgItemText(hDlg, IDC_MIMPIFILE, Config.ToneMapFile);

			EnableWindow(GetDlgItem(hDlg, IDC_SOUNDROMEO), juliet_YM2151IsEnable());
			if ( Config.SoundROMEO ) {
				EnableWindow(GetDlgItem(hDlg, IDC_OPMVOL), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_OPMVOL_TEXT), 0);
			}

			Setup_JoystickPage(hDlg);
		}

		ShowWindow(hDlg,SW_SHOW);
		return 1;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			wp = LOWORD(wParam);
			if ((wp>IDC_KEY_TOP)&&(wp<IDC_KEY_END))
			{
				Prop_KeyCodeSet(hDlg, (BYTE)(wp-IDC_KEY_TOP));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			}
			else switch (wp)
			{
			case IDC_KEY_RESET:
				memcpy(KeyTable, KeyTableMaster, sizeof(KeyTable));
				sprintf(buf, "設定をクリアして初期設定に戻しました。");
				SetDlgItemText(hDlg, IDC_KEYCONF_TEXT, buf);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_RATE_NON:
				Config.SampleRate=0; 
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), FALSE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_RATE_48:
				Config.SampleRate=48000;
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), TRUE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_RATE_44:
				Config.SampleRate=44100;
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), TRUE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_RATE_22:
				Config.SampleRate=22050;
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), TRUE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_RATE_11:
				Config.SampleRate=11025;
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUFSIZE_TEXT), TRUE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_STATUS:
				Config.WindowFDDStat=(!(Config.WindowFDDStat));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_FULLSCREENFDD:
				Config.FullScrFDDStat=(!(Config.FullScrFDDStat));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_DSALERT:
				Config.DSAlert=(!(Config.DSAlert));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

#if 1
			case IDC_SOUNDLPF:
				Config.Sound_LPF=(!(Config.Sound_LPF));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
#endif

			case IDC_SOUNDROMEO:
				Config.SoundROMEO=(!(Config.SoundROMEO));
				EnableWindow(GetDlgItem(hDlg, IDC_OPMVOL), !Config.SoundROMEO);
				EnableWindow(GetDlgItem(hDlg, IDC_OPMVOL_TEXT), !Config.SoundROMEO);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_SRAMWARNING:
				Config.SRAMWarning=(!(Config.SRAMWarning));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_LONGFILENAME:
				Config.LongFileName=(!(Config.LongFileName));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_WINDRVFD:
				Config.WinDrvFD=(!(Config.WinDrvFD));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_SSTP:
				Config.SSTP_Enable=(!(Config.SSTP_Enable));
				EnableWindow(GetDlgItem(hDlg, IDC_SSTPPORT), Config.SSTP_Enable);
				EnableWindow(GetDlgItem(hDlg, IDC_SSTPPORT_TEXT), Config.SSTP_Enable);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_ASPI:
				Config.CDROM_ASPI = 1;
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID_TEXT), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID_TEXT), TRUE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_IOCTRL:
				Config.CDROM_ASPI = 0;
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID_TEXT), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID_TEXT), FALSE);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_ENABLE:
				Config.CDROM_Enable = (!Config.CDROM_Enable);
				EnableWindow(GetDlgItem(hDlg, IDC_SCSIID_TEXT), Config.CDROM_Enable);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_SCSIID), Config.CDROM_Enable);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_IOCTRL), Config.CDROM_Enable);
				EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPI), Config.CDROM_Enable);
				if (CDROM_ASPIChecked)
				{
					EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID), (Config.CDROM_Enable)&&(!Config.CDROM_ASPI));
					EnableWindow(GetDlgItem(hDlg, IDC_CDROM_CTRLID_TEXT), (Config.CDROM_Enable)&&(!Config.CDROM_ASPI));
					EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID), (Config.CDROM_Enable)&&(Config.CDROM_ASPI));
					EnableWindow(GetDlgItem(hDlg, IDC_CDROM_ASPIID_TEXT), (Config.CDROM_Enable)&&(Config.CDROM_ASPI));
				}
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_MIDIDELAYAUTO:
				Config.MIDIAutoDelay = (!(Config.MIDIAutoDelay));
				if ( Config.MIDIAutoDelay ) {
					Config.MIDIDelay = Config.BufferSize*5;
				}
				EnableWindow(GetDlgItem(hDlg, IDC_MIDIDELAY), !Config.MIDIAutoDelay);
				CheckDlgButton(hDlg, IDC_MIDIDELAYAUTO, BSTATE(Config.MIDIAutoDelay));
				SendDlgItemMessage(hDlg, IDC_MIDIDELAY, TBM_SETPOS, TRUE, Config.MIDIDelay);
				wsprintf(buf, "%dms", Config.MIDIDelay);
				SetDlgItemText(hDlg, IDC_MIDIDELAYTEXT, buf);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_MIDI:
				Config.MIDI_SW=(!(Config.MIDI_SW));
				if (Config.MIDI_SW)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITYPE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITEXT), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDIRESET), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIMAP), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILE), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILESEL), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITYPE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDITEXT), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIDIRESET), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIMAP), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILE), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_MIMPIFILESEL), FALSE);
			}
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_MIDIRESET:
				Config.MIDI_Reset=(!(Config.MIDI_Reset));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_MIMPIMAP:
				Config.ToneMap=(!(Config.ToneMap));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_JOYSWAP:
				Config.JoySwap=(!(Config.JoySwap));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_DSMIXING:
				Config.DSMixing=(!(Config.DSMixing));
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_MIMPIFILESEL:
				memset(&ofn, 0, sizeof(ofn));
				filename[0] = 0;
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = "MIMPI Tone Mapping File (*.def)\0*.def\0"
							  "All Files (*.*)\0*.*\0";
				ofn.lpstrFile = filename;
				ofn.lpstrInitialDir = winx68k_dir;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_SHAREAWARE;
				ofn.lpstrDefExt = "hdf";
				ofn.lpstrTitle = "音色設定ファイルの選択";
	
				isopen = !!GetOpenFileName(&ofn);

				if (isopen)
				{
					strncpy(Config.ToneMapFile, filename, MAX_PATH);
					SetDlgItemText(hDlg, IDC_MIMPIFILE, Config.ToneMapFile);
			                PropSheet_Changed(GetParent(hDlg), hDlg);
				}
				return 1;

			case IDC_HDDEJECT:
				memset(Config.HDImage[CurrentHDDNo], 0, MAX_PATH);
				SetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[CurrentHDDNo]);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_HDDNEW:
				memset(&ofn, 0, sizeof(ofn));
				filename[0] = 0;
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = "X68k HardDisk Image (*.hdf)\0*.hdf\0"
							  "All Files (*.*)\0*.*\0";
				ofn.lpstrFile = filename;
				ofn.lpstrInitialDir = winx68k_dir;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = /*OFN_CREATEPROMPT | */OFN_HIDEREADONLY | OFN_SHAREAWARE;
				ofn.lpstrDefExt = "hdf";
				ofn.lpstrTitle = "新規HDイメージ作成";
	
				isopen = !!GetOpenFileName(&ofn);
				openflag = 0;

				if (isopen)
				{
					fp = File_Open(filename);
					if (!fp) {
						if (GetLastError()==ERROR_FILE_NOT_FOUND) {
							fp = File_Create(filename);
							if (fp)
							{
								MakeHDDFile = fp;
								MakeHDDSize = 0x2793; // 40Mb/0x1000
								DialogBox(hInst, MAKEINTRESOURCE(IDD_PROGBAR),
									hDlg, (DLGPROC)MakeHDDProc);
								openflag = 1;
								File_Close(fp);
							}
							else
							{
								Error("HDDイメージが作成できませんでした。");
							}
						}

					} else {
						Error("同名のファイルが既に存在します。");
						File_Close(fp);
					}
					if (openflag)
					{
						strncpy(Config.HDImage[CurrentHDDNo], filename, MAX_PATH);
						SetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[CurrentHDDNo]);
				                PropSheet_Changed(GetParent(hDlg), hDlg);
					}
				}
				return 1;

			case IDC_HDDSELECT:
				memset(&ofn, 0, sizeof(ofn));
				filename[0] = 0;
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFilter = "X68k HardDisk Image (*.hdf)\0*.hdf\0"
							  "All Files (*.*)\0*.*\0";
				ofn.lpstrFile = filename;
				ofn.lpstrInitialDir = winx68k_dir;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = /*OFN_CREATEPROMPT | */OFN_HIDEREADONLY | OFN_SHAREAWARE;
				ofn.lpstrDefExt = "hdf";
				ofn.lpstrTitle = "X68k HDイメージの選択";
	
				isopen = !!GetOpenFileName(&ofn);
				openflag = 0;

				if (isopen)
				{
					fp = File_Open(filename);
					if (fp) {
						openflag = 1;
						File_Close(fp);
					}
					if (openflag)
					{
						strncpy(Config.HDImage[CurrentHDDNo], filename, MAX_PATH);
						SetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[CurrentHDDNo]);
				                PropSheet_Changed(GetParent(hDlg), hDlg);
					}
				}
				return 1;
			}
		} else {
			switch (LOWORD(wParam))
			{
			case IDC_JOY1_BTN1:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN1, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][0] = temp-1;
				else Config.JOY_BTN[0][0] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN2:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN2, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][1] = temp-1;
				else Config.JOY_BTN[0][1] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN3:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN3, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][2] = temp-1;
				else Config.JOY_BTN[0][2] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN4:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN4, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][3] = temp-1;
				else Config.JOY_BTN[0][3] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN5:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][4] = temp-1;
				else Config.JOY_BTN[0][4] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN6:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][5] = temp-1;
				else Config.JOY_BTN[0][5] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN7:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][6] = temp-1;
				else Config.JOY_BTN[0][6] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY1_BTN8:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[0][7] = temp-1;
				else Config.JOY_BTN[0][7] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_JOY2_BTN1:
				temp = SendDlgItemMessage(hDlg, IDC_JOY2_BTN1, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][0] = temp-1;
				else Config.JOY_BTN[1][0] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN2:
				temp = SendDlgItemMessage(hDlg, IDC_JOY2_BTN2, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][1] = temp-1;
				else Config.JOY_BTN[1][1] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN3:
				temp = SendDlgItemMessage(hDlg, IDC_JOY2_BTN3, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][2] = temp-1;
				else Config.JOY_BTN[1][2] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN4:
				temp = SendDlgItemMessage(hDlg, IDC_JOY2_BTN4, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][3] = temp-1;
				else Config.JOY_BTN[1][3] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN5:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN5, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][4] = temp-1;
				else Config.JOY_BTN[1][4] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN6:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN6, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][5] = temp-1;
				else Config.JOY_BTN[1][5] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN7:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN7, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][6] = temp-1;
				else Config.JOY_BTN[1][6] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_JOY2_BTN8:
				temp = SendDlgItemMessage(hDlg, IDC_JOY1_BTN8, CB_GETCURSEL, 0, 0);
				if (temp) Config.JOY_BTN[1][7] = temp-1;
				else Config.JOY_BTN[1][7] = MAX_BUTTON;
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_HDDNO:
				CurrentHDDNo = SendDlgItemMessage(hDlg, IDC_HDDNO, CB_GETCURSEL, 0, 0);
				SetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[CurrentHDDNo]);
				return 1;

			case IDC_HDDFILE:
				GetDlgItemText(hDlg, IDC_HDDFILE, Config.HDImage[CurrentHDDNo], MAX_PATH);
				return 1;

			case IDC_MIDITYPE:
				Config.MIDI_Type = SendDlgItemMessage(hDlg, IDC_MIDITYPE, CB_GETCURSEL, 0, 0);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_SCSIID:
				Config.CDROM_SCSIID = SendDlgItemMessage(hDlg, IDC_CDROM_SCSIID, CB_GETCURSEL, 0, 0);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_ASPIID:
				Config.CDROM_ASPI_Drive = SendDlgItemMessage(hDlg, IDC_CDROM_ASPIID, CB_GETCURSEL, 0, 0);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;

			case IDC_CDROM_CTRLID:
				Config.CDROM_IOCTRL_Drive = SendDlgItemMessage(hDlg, IDC_CDROM_CTRLID, CB_GETCURSEL, 0, 0);
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			case IDC_SSTPPORT:
				temp = SendDlgItemMessage(hDlg, IDC_SSTPPORT, CB_GETCURSEL, 0, 0);
				Config.SSTP_Port = SSTPPorts[temp];
		                PropSheet_Changed(GetParent(hDlg), hDlg);
				return 1;
			}
		}
		return 0;

	case WM_HSCROLL:
		switch (GetDlgCtrlID((HWND)lParam))
		{
		case IDC_BUFSIZE:
			Config.BufferSize = SendDlgItemMessage(hDlg, IDC_BUFSIZE, TBM_GETPOS, 0, 0);
			wsprintf(buf, "%d", Config.BufferSize);
			SetDlgItemText(hDlg, IDC_BUFSIZE_TEXT, buf);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;

		case IDC_OPMVOL:
			Config.OPM_VOL = SendDlgItemMessage(hDlg, IDC_OPMVOL, TBM_GETPOS, 0, 0);
			wsprintf(buf, "%d", Config.OPM_VOL);
			SetDlgItemText(hDlg, IDC_OPMVOL_TEXT, buf);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;

		case IDC_PCMVOL:
			Config.PCM_VOL = SendDlgItemMessage(hDlg, IDC_PCMVOL, TBM_GETPOS, 0, 0);
			wsprintf(buf, "%d", Config.PCM_VOL);
			SetDlgItemText(hDlg, IDC_PCMVOL_TEXT, buf);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;

		case IDC_MCRVOL:
			Config.MCR_VOL = SendDlgItemMessage(hDlg, IDC_MCRVOL, TBM_GETPOS, 0, 0);
			wsprintf(buf, "%d", Config.MCR_VOL);
			SetDlgItemText(hDlg, IDC_MCRVOL_TEXT, buf);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;

		case IDC_MOUSESPEED:
			Config.MouseSpeed = SendDlgItemMessage(hDlg, IDC_MOUSESPEED, TBM_GETPOS, 0, 0);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;

		case IDC_MIDIDELAY:
			Config.MIDIDelay = SendDlgItemMessage(hDlg, IDC_MIDIDELAY, TBM_GETPOS, 0, 0);
			wsprintf(buf, "%dms", Config.MIDIDelay);
			SetDlgItemText(hDlg, IDC_MIDIDELAYTEXT, buf);
	                PropSheet_Changed(GetParent(hDlg), hDlg);
			return 1;
		}
		return 0;

	case WM_SETCURSOR:
		id =  GetDlgCtrlID((HWND)wParam);

		if ( (id>IDC_KEY_TOP)&&(id<IDC_KEY_END) )
		{
			code = id-IDC_KEY_TOP;
			if (LastCode == code) break;
			LastCode = code;
			sprintf(buf, "「%s」に割り当てられているキー：\n\n", X68KeyName[code]);
			for (i=0, j=0; i<512; i++)
			{
				if ( (KeyTable[i]==code) )
				{
					j=1;
					keycode=(MapVirtualKey(i&255, 0)&255);
					keycode <<= 16;
					if (i&0x100) keycode |= 0x01000000;
					GetKeyNameText(keycode, keyname, 20);
					strcat(buf, "「");
					if (!keyname[0])
						strcat(buf, "(名称不明)");
					else
						strcat(buf, keyname);
					strcat(buf, "」");
				}
			}
			if (!j) strcat(buf, "ありません。");
			SetDlgItemText(hDlg, IDC_KEYCONF_TEXT, buf);
		}
		break;


	case WM_NOTIFY:
		id =   (((NMHDR*) lParam)->idFrom);
		code = (((NMHDR*) lParam)->code);

		switch (code)
		{
		case PSN_SETACTIVE:
			break;

		case PSN_APPLY:
			memcpy(&ConfBk, &Config, sizeof(Win68Conf));
			memcpy(KeyTableBk, KeyTable, 512);
			PropSheet_UnChanged(GetParent(hDlg), hDlg);
			return PSNRET_NOERROR;
		
		case PSN_QUERYCANCEL:
			memcpy(&Config, &ConfBk, sizeof(Win68Conf));
			memcpy(KeyTable, KeyTableBk, 512);
			return FALSE;		
		}
		return TRUE;

	}
	return 0;
}
#endif
