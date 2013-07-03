// ---------------------------------------------------------------------------------------
//  SRAM.C - SRAM (16kb) �ΰ�
// ---------------------------------------------------------------------------------------

#include	"common.h"
#include	"fileio.h"
#include	"prop.h"
#include	"winx68k.h"
#include	"sysport.h"
#include	"memory.h"
#include	"sram.h"

	BYTE	SRAM[0x4000];
	BYTE	SRAMFILE[] = "sram.dat";


// -----------------------------------------------------------------------
//   ���Ω���ʤ������뤹�����å�
// -----------------------------------------------------------------------
void SRAM_VirusCheck(void)
{
	//int i, ret;

	if (!Config.SRAMWarning) return;				// Warningȯ���⡼�ɤǤʤ���е���

	if ( (cpu_readmem24_dword(0xed3f60)==0x60000002)
	   &&(cpu_readmem24_dword(0xed0010)==0x00ed3f60) )		// ���ꤦ���뤹�ˤ��������ʤ����
	{
#if 0 /* XXX */
		ret = MessageBox(hWndMain,
			"����SRAM�ǡ����ϥ����륹�˴������Ƥ����ǽ��������ޤ���\n�����Ľ�Υ��꡼�󥢥åפ�Ԥ��ޤ�����",
			"����ԡ�����ηٹ�", MB_ICONWARNING | MB_YESNO);
		if (ret == IDYES)
		{
			for (i=0x3c00; i<0x4000; i++)
				SRAM[i] = 0;
			SRAM[0x11] = 0x00;
			SRAM[0x10] = 0xed;
			SRAM[0x13] = 0x01;
			SRAM[0x12] = 0x00;
			SRAM[0x19] = 0x00;
		}
#endif /* XXX */
		SRAM_Cleanup();
		SRAM_Init();			// Virus���꡼�󥢥å׸�Υǡ�����񤭹���Ǥ���
	}
}


// -----------------------------------------------------------------------
//   �����
// -----------------------------------------------------------------------
void SRAM_Init(void)
{
	int i;
	BYTE tmp;
	FILEH fp;

	for (i=0; i<0x4000; i++)
		SRAM[i] = 0;

	fp = File_OpenCurDir(SRAMFILE);
	if (fp)
	{
		File_Read(fp, SRAM, 0x4000);
		File_Close(fp);
		for (i=0; i<0x4000; i+=2)
		{
			tmp = SRAM[i];
			SRAM[i] = SRAM[i+1];
			SRAM[i+1] = tmp;
		}
	}
}


// -----------------------------------------------------------------------
//   ű����
// -----------------------------------------------------------------------
void SRAM_Cleanup(void)
{
	int i;
	BYTE tmp;
	FILEH fp;

	for (i=0; i<0x4000; i+=2)
	{
		tmp = SRAM[i];
		SRAM[i] = SRAM[i+1];
		SRAM[i+1] = tmp;
	}

	fp = File_OpenCurDir(SRAMFILE);
	if (!fp)
		fp = File_CreateCurDir(SRAMFILE, FTYPE_SRAM);
	if (fp)
	{
		File_Write(fp, SRAM, 0x4000);
		File_Close(fp);
	}
}


// -----------------------------------------------------------------------
//   �꡼��
// -----------------------------------------------------------------------
BYTE FASTCALL SRAM_Read(DWORD adr)
{
	adr &= 0xffff;
	adr ^= 1;
	if (adr<0x4000)
		return SRAM[adr];
	else
		return 0xff;
}


// -----------------------------------------------------------------------
//   �餤��
// -----------------------------------------------------------------------
void FASTCALL SRAM_Write(DWORD adr, BYTE data)
{
	//int ret;

	if ( (SysPort[5]==0x31)&&(adr<0xed4000) )
	{
		if ((adr==0xed0018)&&(data==0xb0))	// SRAM��ư�ؤ��ڤ��ؤ��ʴ�ñ�ʥ����륹�к���
		{
			if (Config.SRAMWarning)		// Warningȯ���⡼�ɡʥǥե���ȡ�
			{
#if 0 /* XXX */
				ret = MessageBox(hWndMain,
					"SRAM�֡��Ȥ��ڤ��ؤ��褦�Ȥ��Ƥ��ޤ���\n�����륹�δ����ʤ������ǧ���Ƥ���������\nSRAM�֡��Ȥ��ڤ��ؤ�����³���ޤ�����",
					"����ԡ�����ηٹ�", MB_ICONWARNING | MB_YESNO);
				if (ret != IDYES)
				{
					data = 0;	// STD�֡��Ȥˤ���
				}
#endif /* XXX */
			}
		}
		adr &= 0xffff;
		adr ^= 1;
		SRAM[adr] = data;
	}
}
