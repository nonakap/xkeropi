// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$fmgen-Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPN/OPNA
//	OPN/OPNA ���ɤ����������������벻����˥å�
//	
//	interface:
//	bool Init(uint clock, uint rate, bool, const char* path);
//		����������Υ��饹����Ѥ������ˤ��ʤ餺�Ƥ�Ǥ������ȡ�
//		OPNA �ξ��Ϥ��δؿ��ǥꥺ�ॵ��ץ���ɤ߹���
//
//		clock:	OPN/OPNA/OPNB �Υ���å����ȿ�(Hz)
//
//		rate:	�������� PCM ��ɸ�ܼ��ȿ�(Hz)
//
//		path:	�ꥺ�ॵ��ץ�Υѥ�(OPNA �Τ�ͭ��)
//				��ά���ϥ����ȥǥ��쥯�ȥ꤫���ɤ߹���
//				ʸ����������ˤ� '\' �� '/' �ʤɤ�Ĥ��뤳��
//
//		�֤���	���������������� true
//
//	bool LoadRhythmSample(const char* path)
//		(OPNA ONLY)
//		Rhythm ����ץ���ɤ�ľ����
//		path �� Init �� path ��Ʊ����
//		
//	bool SetRate(uint clock, uint rate, bool)
//		����å��� PCM �졼�Ȥ��ѹ�����
//		�������� Init �򻲾ȤΤ��ȡ�
//	
//	void Mix(FM_SAMPLETYPE* dest, int nsamples)
//		Stereo PCM �ǡ����� nsamples ʬ�������� dest �ǻϤޤ������
//		�ä���(�û�����)
//		��dest �ˤ� sample*2 ��ʬ���ΰ褬ɬ��
//		����Ǽ������ L, R, L, R... �Ȥʤ롥
//		�������ޤǲû��ʤΤǡ����餫��������򥼥��ꥢ����ɬ�פ�����
//		��FM_SAMPLETYPE �� short ���ξ�祯��åԥ󥰤��Ԥ���.
//		�����δؿ��ϲ��������Υ����ޡ��Ȥ���Ω���Ƥ��롥
//		  Timer �� Count �� GetNextEvent ������ɬ�פ����롥
//	
//	void Reset()
//		������ꥻ�å�(�����)����
//
//	void SetReg(uint reg, uint data)
//		�����Υ쥸���� reg �� data ��񤭹���
//	
//	uint GetReg(uint reg)
//		�����Υ쥸���� reg �����Ƥ��ɤ߽Ф�
//		�ɤ߹��ळ�Ȥ������쥸������ PSG, ADPCM �ΰ�����ID(0xff) �Ȥ�
//	
//	uint ReadStatus()/ReadStatusEx()
//		�����Υ��ơ������쥸�������ɤ߽Ф�
//		ReadStatusEx �ϳ�ĥ���ơ������쥸�������ɤ߽Ф�(OPNA)
//		busy �ե饰�Ͼ�� 0
//	
//	bool Count(uint32 t)
//		�����Υ����ޡ��� t [����] �ʤ�롥
//		�������������֤��Ѳ������ä���(timer �����С��ե�)
//		true ���֤�
//
//	uint32 GetNextEvent()
//		�����Υ����ޡ��Τɤ��餫�������С��ե�����ޤǤ�ɬ�פ�
//		����[����]���֤�
//		�����ޡ�����ߤ��Ƥ������ ULONG_MAX ���֤��� �Ȼפ�
//	
//	void SetVolumeFM(int db)/SetVolumePSG(int db) ...
//		�Ʋ����β��̤�ܡ�������Ĵ�᤹�롥ɸ���ͤ� 0.
//		ñ�̤��� 1/2 dB��ͭ���ϰϤξ�¤� 20 (10dB)
//
namespace FM
{
	//	OPN Base -------------------------------------------------------
	class OPNBase : public Timer
	{
	public:
		OPNBase();
		virtual ~OPNBase() {}
		
		bool	Init(uint c, uint r);
		virtual void Reset();
		
		void	SetVolumeFM(int db);
		void	SetVolumePSG(int db);
		void	SetLPFCutoff(uint freq) {}	// obsolete

	protected:
		void	SetParameter(Channel4* ch, uint addr, uint data);
		void	SetPrescaler(uint p);
		void	RebuildTimeTable();
		
		int		fmvolume;
		
		uint	clock;				// OPN ����å�
		uint	rate;				// FM ���������졼��
		uint	psgrate;			// FMGen  ���ϥ졼��
		uint	status;
		Channel4* csmch;
		

		static  uint32 lfotable[8];
	
	private:
		void	TimerA();
		uint8	prescale;
		
	protected:
		Chip	chip;
		PSG		psg;
	};

	//	OPN2 Base ------------------------------------------------------
	class OPNABase : public OPNBase
	{
	public:
		OPNABase();
		virtual ~OPNABase();
		
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx();
		void	SetChannelMask(uint mask);
	
	private:
		virtual void Intr(bool) {}

		void	MakeTable2();
	
	protected:
		bool	Init(uint c, uint r, bool);
		bool	SetRate(uint c, uint r, bool);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		void	SetADPCMBReg(uint reg, uint data);
		uint	GetReg(uint addr);	
	
	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);
		
		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(Sample* dest, uint count);

		void	WriteRAM(uint data);
		uint	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint);
		
	// FM �����ط�
		uint8	pan[6];
		uint8	fnum2[9];
		
		uint8	reg22;
		uint	reg29;		// OPNA only?
		
		uint	stmask;
		uint	statusnext;

		uint32	lfocount;
		uint32	lfodcount;
		
		uint	fnum[6];
		uint	fnum3[3];
		
	// ADPCM �ط�
		uint8*	adpcmbuf;		// ADPCM RAM
		uint	adpcmmask;		// ���ꥢ�ɥ쥹���Ф���ӥåȥޥ���
		uint	adpcmnotice;	// ADPCM ������λ���ˤ��ĥӥå�
		uint	startaddr;		// Start address
		uint	stopaddr;		// Stop address
		uint	memaddr;		// �����楢�ɥ쥹
		uint	limitaddr;		// Limit address/mask
		int		adpcmlevel;		// ADPCM ����
		int		adpcmvolume;
		int		adpcmvol;
		uint	deltan;			// ��N
		int		adplc;			// ���ȿ��Ѵ����ѿ�
		int		adpld;			// ���ȿ��Ѵ����ѿ���ʬ��
		uint	adplbase;		// adpld �θ�
		int		adpcmx;			// ADPCM ������ x
		int		adpcmd;			// ADPCM ������ ��
		int		adpcmout;		// ADPCM ������ν���
		int		apout0;			// out(t-2)+out(t-1)
		int		apout1;			// out(t-1)+out(t)

		uint	adpcmreadbuf;	// ADPCM �꡼���ѥХåե�
		bool	adpcmplay;		// ADPCM ������
		int8	granuality;		
		bool	adpcmmask_;

		uint8	control1;		// ADPCM ����ȥ���쥸������
		uint8	control2;		// ADPCM ����ȥ���쥸������
		uint8	adpcmreg[8];	// ADPCM �쥸�����ΰ���ʬ

		int		rhythmmask_;

		Channel4 ch[6];

		static void	BuildLFOTable();
		static int amtable[FM_LFOENTS];
		static int pmtable[FM_LFOENTS];
		static int32 tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};

	//	YM2203(OPN) ----------------------------------------------------
	class OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}
		
		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool=false);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint mask);
		
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }
	
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		
		uint	fnum[3];
		uint	fnum3[3];
		uint8	fnum2[6];
		
		Channel4 ch[3];
	};

	//	YM2608(OPNA) ---------------------------------------------------
	class OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();
		
		bool	Init(uint c, uint r, bool  = false, const char* rhythmpath=0);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);

		void	SetVolumeADPCM(int db);
		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		uint8*	GetADPCMBuffer() { return adpcmbuf; }

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		
	private:
		struct Rhythm
		{
			uint8	pan;		// �Ѥ�
			int8	level;		// �����礦
			int		volume;		// �����礦���äƤ�
			int16*	sample;		// ����פ�
			uint	size;		// ������
			uint	pos;		// ����
			uint	step;		// ���Ƥäפ�
			uint	rate;		// ����פ�Τ졼��
		};
	
		void	RhythmMix(Sample* buffer, uint count);

	// �ꥺ�಻���ط�
		Rhythm	rhythm[6];
		int8	rhythmtl;		// �ꥺ�����Τβ���
		int		rhythmtvol;		
		uint8	rhythmkey;		// �ꥺ��Υ���
	};

	//	YM2610/B(OPNB) ---------------------------------------------------
	class OPNB : public OPNABase
	{
	public:
		OPNB();
		virtual ~OPNB();
		
		bool	Init(uint c, uint r, bool = false,
					 uint8 *_adpcma = 0, int _adpcma_size = 0,
					 uint8 *_adpcmb = 0, int _adpcmb_size = 0);
	
		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatusEx();

		void	SetVolumeADPCMATotal(int db);
		void	SetVolumeADPCMA(int index, int db);
		void	SetVolumeADPCMB(int db);

//		void	SetChannelMask(uint mask);
		
	private:
		struct ADPCMA
		{
			uint8	pan;		// �Ѥ�
			int8	level;		// �����礦
			int		volume;		// �����礦���äƤ�
			uint	pos;		// ����
			uint	step;		// ���Ƥäפ�

			uint	start;		// ����
			uint	stop;		// ��λ
			uint	nibble;		// ���� 4 bit
			int		adpcmx;		// �Ѵ���
			int		adpcmd;		// �Ѵ���
		};
	
		int		DecodeADPCMASample(uint);
		void	ADPCMAMix(Sample* buffer, uint count);
		static void InitADPCMATable();
		
	// ADPCMA �ط�
		uint8*	adpcmabuf;		// ADPCMA ROM
		int		adpcmasize;
		ADPCMA	adpcma[6];
		int8	adpcmatl;		// ADPCMA ���Τβ���
		int		adpcmatvol;		
		uint8	adpcmakey;		// ADPCMA �Υ���
		int		adpcmastep;
		uint8	adpcmareg[32];
 
		static int jedi_table[(48+1)*16];

		Channel4 ch[6];
	};

	//	YM2612/3438(OPN2) ----------------------------------------------------
	class OPN2 : public OPNBase
	{
	public:
		OPN2();
		virtual ~OPN2() {}
		
		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint mask);
		
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		
		uint	fnum[3];
		uint	fnum3[3];
		uint8	fnum2[6];
		
	// ��������ѥ��
		int32	mixc, mixc1;
		
		Channel4 ch[3];
	};

	//	YMF288 ---------------------------------------------------
	class Y288 : public OPNABase
	{
	public:
		Y288();
		virtual ~Y288();
		
		bool	Init(uint c, uint r, bool ipflag = false, const char* =0);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint c, uint r, bool ipflag = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);

	private:
		struct Rhythm
		{
			uint8	pan;		// �Ѥ�
			int8	level;		// �����礦
			int		volume;		// �����礦���äƤ�
			int16*	sample;		// ����פ�
			uint	size;		// ������
			uint	pos;		// ����
			uint	step;		// ���Ƥäפ�
			uint	rate;		// ����פ�Τ졼��
		};
	
		void	RhythmMix(Sample* buffer, uint count);

	// �ꥺ�಻���ط�
		Rhythm	rhythm[6];
		int8	rhythmtl;		// �ꥺ�����Τβ���
		int		rhythmtvol;		
		uint8	rhythmkey;		// �ꥺ��Υ���

		int	mode288;		// 288/2608�⡼�ɥե饰
	};
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = (uint8)-1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db)
{
	psg.SetVolume(db);
}

#endif // FM_OPNA_H
