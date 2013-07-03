#ifndef winx1_dsound_h
#define winx1_dsound_h

#include	"common.h"

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

#endif //winx1_dsound_h
