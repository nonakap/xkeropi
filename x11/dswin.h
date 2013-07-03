/*	$Id: dswin.h,v 1.1.1.1 2003/04/28 18:06:56 nonaka Exp $	*/

#ifndef dswin_h__
#define dswin_h__

#include "common.h"

int DSound_Init(unsigned long rate, unsigned long length);
int DSound_Cleanup(void);

void DSound_Play(void);
void DSound_Stop(void);
void FASTCALL DSound_Send0(long clock);
void FASTCALL DSound_Send(void);

void DS_SetVolumeOPM(long vol);
void DS_SetVolumeADPCM(long vol);
void DS_SetVolumeMercury(long vol);

extern DWORD ratebase1000;

#endif /* dswin_h__ */
