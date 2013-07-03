#ifndef winx68k_statusbar_h
#define winx68k_statusbar_h

#ifdef __cplusplus
extern "C" {
#endif

extern HWND	hWndStat;
extern RECT	rectStat;
extern int	heightStat;

void StatBar_Redraw(void);
void StatBar_Show(int sw);
void StatBar_Draw(DRAWITEMSTRUCT* dis);
void StatBar_FDName(int drv, char* name);
void StatBar_FDD(int drv, int led, int col);
void StatBar_UpdateTimer(void);
void StatBar_SetFDD(int drv, char* file);
void StatBar_ParamFDD(int drv, int access, int insert, int blink);
void StatBar_HDD(int sw);

#ifdef __cplusplus
};
#endif

#endif //winx68k_statusbar_h
