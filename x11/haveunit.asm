
;	for new GOGO-no-coda (1999/09)
;	Copyright (C) 1999 shigeo
;	modified by Keiichi SAKAI, URURI, Hash

;	99/07/29
;	99/09/06	�ե饰�ɲ�
;	99/09/07	SSE�ν������̤ˤ���
;	99/09/29	Cx586�к�
;	99/11/10	Cxrix�к�

;	int haveUNIT(void);

;	486̤����CPU�ޤ���FPU����ܤ��Ƥ��ʤ����0
;	����ʳ��ϼ����ͤ�������
;	PIII�Ǥϴݤ����ͼθ����⡼�ɤˤ���(�餷��)

;	haveunit.h�Ȥ����������

%include "x11/nasm.h"

tFPU	equ	(1<<0)
tMMX	equ	(1<<1)
t3DN	equ	(1<<2)
tSSE	equ	(1<<3)
tCMOV	equ	(1<<4)
tE3DN	equ	(1<<5)	;/* Athlon�� (extend 3D Now!)*/
tEMMX   equ	(1<<6)  ;/* EMMX=E3DNow!_INT=SSE_INT */

tSPC1	equ	(1<<16)	;/* ���̤ʥ����å� */
tSPC2	equ	(1<<17)	;/* ���ӤϷ�ޤäƤʤ� */

tINTEL	equ	(1<<8)
tAMD	equ	(1<<9)
tCYRIX	equ	(1<<10)
tIDT	equ	(1<<11)
tUNKNOWN	equ	(1<<15)	;�٥������ʬ����ʤ�

tFAMILY4	equ	(1<<20)	;/* 486 ���λ��٥����Ƚ������Ƥˤʤ�ʤ� */
tFAMILY5	equ	(1<<21)	;/* 586 (P5, P5-MMX, K6, K6-2, K6-III) */
tFAMILY6	equ	(1<<22)	;/* 686�ʹ� P-Pro, P-II, P-III, Athlon */

		globaldef	_haveUNIT
		globaldef	setPIII_round
		globaldef	maskFPU_exception

	segment_data
	segment_code

ACflag equ (1<<18)
IDflag equ (1<<21)

		align	16
haveUNIT:
		push	ebx
		push	esi
		xor		esi,esi
		call	near haveFPU
		jnz		near .Lexit
		or		esi,tFPU
		pushfd						;flag��¸
		pushfd
		pop		eax					;eax=flag
		or		eax,ACflag			;eax=flag|ACflag
		push	eax
		popfd						;flag=eax
		pushfd
		pop		eax					;eax=flag
		popfd						;flag����
		test	eax,ACflag			;ACflag���Ѳ���������
		jz		near .Lexit
;486�ʹ�
		pushfd						;flag��¸
		pushfd
		pop		eax					;eax=flag
		or		eax,IDflag			;eax=flag|IDflag
		push	eax
		popfd						;flag=eax
		pushfd
		pop		eax					;eax=flag
		popfd						;flag����
		test	eax,IDflag
;		jz		short .Lexit
		jnz		.L586

%if 1
	;Cyrix 486CPU check Cyrix �� HP �ˤ��ä���� by Hash

        xor   ax, ax         ; clear ax
        sahf                 ; clear flags, bit 1 is always 1 in flags
        mov   ax, 5          
        mov   bx, 2
        div   bl             ; do an operation that does not change flags
        lahf                 ; get flags
        cmp   ah, 2          ; check for change in flags
        jne   .L486intel     ; flags changed not Cyrix        
        or    esi,tCYRIX     ; TRUE Cyrix CPU 
        jmp   .L486

.L486intel:
		or		esi,tINTEL
.L486:
		or		esi,tFAMILY4
		jmp		.Lexit

%else
	; check for Cyrix 486DLC -- based on check routine
	; documented in "Cx486SLC/e SMM Programmer's Guide"
		xor		dx,dx
		cmp		dx,dx
		pushf
		pop		cx
		mov		ax,0FFFFh
		mov		bx,4
		div		bx
		pushf
		pop		ax
		and		ax,8D5h
		and		cx,8D5h
		cmp		ax,cx
		jnz		.L486intel
		or		esi,tCYRIX
		jmp		.L486
.L486intel:
		or		esi,tINTEL
.L486:
		or		esi,tFAMILY4
		jmp		.Lexit
%endif


.L586:

;cpuid �� eax,ebx,ecx,edx���˲�����Τ���ա�����

		xor		eax,eax
		cpuid
;		cmp		ecx,"letn"
		cmp		ecx,"ntel"	;�����NASM��ȿ�Ф����֤�����
		jne		.F00
		or		esi,tINTEL
		jmp		.F09
.F00:
;		cmp		ecx,"DMAc"
		cmp		ecx,"cAMD"
		jne		.F01
		or		esi,tAMD
		jmp		.F09
.F01:
;		cmp		ecx,"daet"
		cmp		ecx,"tead"
		jne		.F02
		or		esi,tCYRIX
		jmp		.F09
.F02:
;		cmp		ecx,"slua"
		cmp		ecx,"auls"
		jne		.F03
		or		esi,tIDT
		jmp		.F09
.F03:
		or		esi,tUNKNOWN
		jmp		.F09
.F09:
		mov		eax,1
		cpuid
		cmp		ah,4
		jne		.F10
		or		esi,tFAMILY4
		jmp		.Lexit
.F10:
		cmp		ah,5
		jne		.F11
		or		esi,tFAMILY5
		jmp		.F19
.F11:
		cmp		ah,6
		jne		.F12
		or		esi,tFAMILY6
		jmp		.F19
.F12:
		or		esi,tFAMILY6	; 7�ʾ��6�ȸ��ʤ�
.F19:

		;for AMD, IDT
		mov		eax,80000001h
		cpuid
		test	edx,(1 << 31)
		jz		.F20
		or		esi,t3DN
.F20:
		test	edx,(1 << 15)	;CMOVcc
		jz		.F21
;		test	edx,(1 << 16)	;FCMOVcc ;K7�����ѹ� by URURI
;		jz		.F21
		or		esi,tCMOV
.F21:
		test	edx,(1 << 30)	;��ĥ 3D Now!
		jz		.F22
%ifdef USE_E3DN
		or		esi,tE3DN
%endif
.F22:
		test	edx,(1 << 22)	;AMD MMX Ext
		jz		.F23
		or		esi,tEMMX
.F23:
	;Intel��
		mov		eax,1
		cpuid
		test	edx,(1 << 23)
		jz		.F30
		or		esi,tMMX
.F30:
		test	edx,(1 << 15)		;CMOVcc and FCMOV if FPU=1
		jz		.F31
		or		esi,tCMOV
.F31:
		test	edx,(1 << 25)
		jz		short .Lexit
		or		esi,tEMMX
%ifndef UNAVAILABLE_SSE
		or		esi,tSSE
%endif
.Lexit:
		mov		eax,esi
		pop		esi
		pop		ebx
		ret

;	  in:none
;	 out:ZF FPU����=1, �ʤ�=0
;	dest:eax

		align	16
haveFPU:
		mov		al,1
		fninit
		fnstsw	ax
		cmp		al,0
		jne		short .LhF_exit
		sub		esp,4
		fnstcw	word [esp]
		mov		ax,[esp]
		add		esp,4
		and		ax,103Fh
		cmp		ax,3Fh
.LhF_exit:
		ret

setPIII_round:
		;P-III��SSE��μ¤˻ͼθ����⡼�ɤ�
;		mov		eax,0x1f80	; default mode
		mov		eax,0x9f80	; flush to ZERO mode
		push	eax
		ldmxcsr	[esp]			; setup MXCSR
		pop		eax
		ret

;	for Win32 Delphi
maskFPU_exception:
		push		eax
		fnstcw	word [esp]
		and	word [esp],0xFFF2
		fldcw	word [esp]
		pop		eax
		ret

		end
