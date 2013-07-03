;	$Id$

bits	32

extern	VLINEBG		; DWORD
extern	Sprite_Regs	; BYTE [0x800]

extern	BG		; BYTE [0x8000]
extern	BGCHR8		; BYTE [8*8*256]
extern	BGCHR16		; BYTE [16*16*256]
extern	BG_LineBuf	; WORD [1600]
extern	BG_PriBuf	; WORD [1600]
extern	BG_VLINE	; long
extern	BG_HAdjust	; long
extern	BG_CHRSIZE	; BYTE
extern	BG_Regs		; BYTE [0x12]

extern	BG_BG0TOP	; WORD
extern	BG0ScrollX	; DWORD
extern	BG0ScrollY	; DWORD
extern	BG_BG1TOP	; WORD
extern	BG1ScrollX	; DWORD
extern	BG1ScrollY	; DWORD

extern	TextDotX	; DWORD
extern	TextPal		; WORD [256]
extern	Text_TrFlag	; BYTE [1024]


%macro	BG_DrawLineMcr8	4
bg8_%1:
	mov	edx, dword [%4]
	add	edx, dword [VLINEBG]
	sub	edx, dword [BG_VLINE]
	mov	ebp, edx
	and	ebp, 7
	shl	bp, 3
	and	edx, 000001f8h
	shl	edx, 4
	add	dx, word [%2]
	mov	eax, dword [%3]
	sub	eax, dword [BG_HAdjust]
	mov	ecx, eax
	and	eax, 7
	xor	al, 15
	mov	edi, eax
	and	ecx, 000001f8h
	shr	cx, 2
	mov	ebx, dword [TextDotX]
	shl	ebx, 13
bg8_%1_linelp:
	movzx	esi, word [BG + ecx + edx]
	mov	bx, si
	and	si, 0000ff00h
	shr	si, 2
	cmp	bl, 40h
	jc	bg8_%1_flipx
	js	bg8_%1_fxflipy
	jge	bg8_%1_fxflipx
	add	esi, BGCHR8 + 00000038h
	sub	esi, ebp
	jmp	bg8_%1_flip
bg8_%1_fxflipy:
	add	esi, BGCHR8 + 0000003fh
	sub	esi, ebp
	std
	jmp	bg8_%1_out
bg8_%1_fxflipx:
	lea	esi, [BGCHR8 + esi + ebp + 7]
	std
	jmp	bg8_%1_out
bg8_%1_flipx:
	lea	esi, [BGCHR8 + esi + ebp]
bg8_%1_flip:
	cld
bg8_%1_out:
	mov	bh, 8
	shl	bl, 4
bg8_%1_outlp:
	lodsb
	or	al, bl
	and	ax, 00ffh
	je	bg8_%1_outed
	test	ax, 000fh
	mov	ax, word [TextPal + eax * 2]
	jne	bg8_%1_outcont
	test	byte [Text_TrFlag + edi + 1], 2
	jne	bg8_%1_outed
bg8_%1_outcont:
	mov	word [BG_LineBuf + 2 + edi * 2], ax
	or	byte [Text_TrFlag + 1 + edi], 2
bg8_%1_outed:
	inc	edi
	dec	bh
	jne	bg8_%1_outlp
	add	cl, 2
	and	cl, 7fh
	sub	ebx, 10000h
	jns	bg8_%1_linelp
%endmacro

%macro	BG_DrawLineMcr16	4
bg16_%1:
	mov	edx, [%4]
	add	edx, [VLINEBG]
	sub	edx, [BG_VLINE]
	mov	ebp, edx
	and	ebp, 15
	shl	bp, 4
	and	edx, 03f0h
	shl	edx, 3
	add	dx, [%2]
	mov	eax, [%3]
	sub	eax, [BG_HAdjust]
	mov	ecx, eax
	and	eax, 15
	xor	al, 15
	mov	edi, eax
	and	ecx, 03f0h
	shr	cx, 3
	mov	ebx, [TextDotX]
	shl	ebx, 12
bg16_%1_linelp:
	movzx	esi, word [BG + ecx + edx]
	mov	bx, si
	and	si, 0ff00h
	cmp	bl, 40h
	jc	bg16_%1_flipx
	js	bg16_%1_fxflipy
	jge	bg16_%1_fxflipx
	add	esi, BGCHR16 + 000000f0h
	sub	esi, ebp
	jmp	bg16_%1_flip
bg16_%1_fxflipy:
	add	esi, BGCHR16 + 000000ffh
	sub	esi, ebp
	std
	jmp	bg16_%1_out
bg16_%1_fxflipx:
	lea	esi, [BGCHR16 + esi + ebp + 15]
	std
	jmp	bg16_%1_out
bg16_%1_flipx:
	lea	esi, [BGCHR16 + esi + ebp]
bg16_%1_flip:
	cld
bg16_%1_out:
	mov	bh, 16
	shl	bl, 4
bg16_%1_outlp:
	lodsb
	or	al, bl
	and	ax, 00ffh
	je	bg16_%1_outed
	test	ax, 000fh
	mov	ax, word [TextPal + eax * 2]
	jne	bg16_%1_outcont
	test	byte [Text_TrFlag + 1 + edi], 2
	jne	bg16_%1_outed
bg16_%1_outcont:
	mov	word [BG_LineBuf + 2 + edi*2], ax
	or	byte [Text_TrFlag + 1 + edi], 2
bg16_%1_outed:
	inc	edi
	dec	bh
	jne	bg16_%1_outlp
	add	cl, 2
	and	cl, 7fh
	sub	ebx, 10000h
	jns	bg16_%1_linelp
%endmacro

%macro	BG_DrawLineMcr8_ng	4
bg8_ng_%1:
	mov	edx, [%4]
	add	edx, [VLINEBG]
	sub	edx, [BG_VLINE]
	mov	ebp, edx
	and	ebp, 7
	shl	bp, 3
	and	edx, 01f8h
	shl	edx, 4
	add	dx, [%2]
	mov	eax, [%3]
	sub	eax, [BG_HAdjust]
	mov	ecx, eax
	and	eax, 7
	xor	al, 15
	mov	edi, eax
	and	ecx, 01f8h
	shr	cx, 2
	mov	ebx, [TextDotX]
	shl	ebx, 13
bg8_ng_%1_linelp:
	movzx	esi, word [BG+ecx+edx]
	mov	bx, si
	and	si, 0ff00h
	shr	si, 2
	cmp	bl, 40h
	jc	bg8_ng_%1_flipx
	js	bg8_ng_%1_fxflipy
	jge	bg8_ng_%1_fxflipx
	add	esi, BGCHR8 + 00000038h
	sub	esi, ebp
	jmp	bg8_ng_%1_flip
bg8_ng_%1_fxflipy:
	add	esi, BGCHR8 + 0000003fh
	sub	esi, ebp
	std
	jmp	bg8_ng_%1_out
bg8_ng_%1_fxflipx:
	lea	esi, [BGCHR8+esi+ebp+7]
	std
	jmp	bg8_ng_%1_out
bg8_ng_%1_flipx:
	lea	esi, [BGCHR8+esi+ebp]
bg8_ng_%1_flip:
	cld
bg8_ng_%1_out:
	mov	bh, 8
	shl	bl, 4
bg8_ng_%1_outlp:
	lodsb
	and	ax, 000fh
	je	bg8_ng_%1_outed
	or	al, bl
	mov	ax, word [TextPal+eax*2]
	mov	word [BG_LineBuf+2+edi*2], ax
	or	byte [Text_TrFlag+1+edi], 2
bg8_ng_%1_outed:
	inc	edi
	dec	bh
	jne	bg8_ng_%1_outlp
	add	cl, 2
	and	cl, 7fh
	sub	ebx, 10000h
	jns	bg8_ng_%1_linelp
%endmacro

%macro	BG_DrawLineMcr16_ng	4
bg16_ng_%1:
	mov	edx, [%4]
	add	edx, [VLINEBG]
	sub	edx, [BG_VLINE]
	mov	ebp, edx
	and	ebp, 15
	shl	bp, 4
	and	edx, 03f0h
	shl	edx, 3
	add	dx, [%2]
	mov	eax, [%3]
;	sub	eax, 0 /*BG_HAdjust*/
	mov	ecx, eax
	and	eax, 15
	xor	al, 15
	mov	edi, eax
	and	ecx, 03f0h
	shr	cx, 3
	mov	ebx, [TextDotX]
	shl	ebx, 12
bg16_ng_%1_linelp:
	movzx	esi, word [BG+ecx+edx]
	mov	bx, si
	and	si, 0ff00h
	cmp	bl, 40h
	jc	bg16_ng_%1_flipx
	js	bg16_ng_%1_fxflipy
	jge	bg16_ng_%1_fxflipx
	add	esi, BGCHR16 + 000000f0h
	sub	esi, ebp
	jmp	bg16_ng_%1_flip
bg16_ng_%1_fxflipy:
	add	esi, BGCHR16 + 000000ffh
	sub	esi, ebp
	std
	jmp	bg16_ng_%1_out
bg16_ng_%1_fxflipx:
	lea	esi, [BGCHR16+esi+ebp+15]
	std
	jmp	bg16_ng_%1_out
bg16_ng_%1_flipx:
	lea	esi, [BGCHR16+esi+ebp]
bg16_ng_%1_flip:
	cld
bg16_ng_%1_out:
	mov	bh, 16
	shl	bl, 4
bg16_ng_%1_outlp:
	lodsb
	and	ax, 000fh
	je	bg16_ng_%1_outed
	or	al, bl
	mov	ax, word [TextPal+eax*2]
	mov	word [BG_LineBuf+2+edi*2], ax
	or	byte [Text_TrFlag+1+edi], 2
bg16_ng_%1_outed:
	inc	edi
	dec	bh
	jne	bg16_ng_%1_outlp
	add	cl, 2
	and	cl, 7fh
	sub	ebx, 10000h
	jns	bg16_ng_%1_linelp
%endmacro

struc	spritectrltbl
	.sprite_posx	resw	1
	.sprite_posy	resw	1
	.sprite_ctrl	resb	1
	.sprite_ply	resb	1
endstruc

%macro	Sprite_DrawLineMcr	2
	mov	ebp, [TextDotX]
	add	ebp, 16
	mov	edx, 127*8
spline_%1_lp:
	mov	al, byte [Sprite_Regs + edx + spritectrltbl.sprite_ply]
	and	al, 3
	cmp	al, %2
	je	spline_%1_plyhit
spline_%1_lpcnt:
	sub	dx, 8
	jns	spline_%1_lp
	jmp	spline_%1_ed
spline_%1_plyhit:
	movzx	edi, word [Sprite_Regs + edx + spritectrltbl.sprite_posx]
	add	edi, [BG_HAdjust]
	and	di, 03ffh
	cmp	edi, ebp
	jnc	spline_%1_lpcnt
	movzx	eax, word [Sprite_Regs + edx + spritectrltbl.sprite_posy]
	and	ax, 03ffh
	sub	eax, [VLINEBG]
	add	eax, [BG_VLINE]
	neg	eax
	add	eax, 16
	jnc	spline_%1_lpcnt
	shl	al, 4
	movzx	esi, byte [Sprite_Regs + edx + spritectrltbl.sprite_ctrl]
	mov	bx, si
	shl	si, 8
	cmp	bh, 40h
	jc	spline_%1_flipx
	js	spline_%1_fxflipy
	jge	spline_%1_fxflipx
	xor	al, 0f0h
spline_%1_flipx:
	lea	esi, [BGCHR16 + esi + eax]
	cld
spline_%1_out:
	shl	bh, 4
	mov	ecx, 16
spline_%1_outlp:
	lodsb
	and	ax, 000fh
	je	spline_%1_trans
	or	al, bh
	cmp	word [BG_PriBuf+edi*2], dx
	jc	spline_%1_trans
	mov	ax, word [TextPal+eax*2]
	mov	word [BG_LineBuf+edi*2], ax
	or	byte [Text_TrFlag+edi], 2
	mov	word [BG_PriBuf+edi*2], dx
spline_%1_trans:
	inc	edi
	loop	spline_%1_outlp
	jmp	spline_%1_lpcnt
spline_%1_fxflipy:
	xor	al, 0f0h
spline_%1_fxflipx:
	lea	esi, [BGCHR16 + esi + eax + 15]
	std
	jmp	spline_%1_out
spline_%1_ed:
%endmacro

	section	.text

;
; LABEL void FASTCALL BG_DrawLine(int opaq, int gd)
;
	global	BG_DrawLine
	align	16
BG_DrawLine:
	mov	ecx, [esp + 4]	; opaq
	mov	edx, [esp + 8]	; gd

	pushf
	push	ebx
	push	esi
	push	edi
	push	edx
	push	ebp

	mov	ax, word [TextPal]
	shl	eax, 16
	mov	ax, word [TextPal]
	mov	ebx, 0xffffffff
	mov	edi, 16*2
	or	ecx, ecx			; ecx = opaq
	jz	noclrloop
	mov	ecx, [TextDotX]
	shr	ecx, 1
BGLineClr_lp:
	mov	dword [BG_LineBuf + edi], eax
	mov	dword [BG_PriBuf + edi], ebx	; SP間のプライオリティ情報初期化
	add	edi, 4
	loop	BGLineClr_lp
	jmp	bgclrloopend

noclrloop:
	mov	ecx, [TextDotX]
	shr	ecx, 1
BGLineClr_lp2:
	mov	dword [BG_PriBuf + edi], ebx	; SP間のプライオリティ情報初期化
	add	edi, 4
	loop	BGLineClr_lp2

bgclrloopend:
	or	edx, edx			; edx = gd
	je	near BG_NOGRP

	cmp	byte [BG_CHRSIZE], 8
	jne	near BG16

	Sprite_DrawLineMcr 81, 1
;	test	byte [BG_Regs + 9], 8
;	je	near BG8_1skiped
;	BG_DrawLineMcr8 1, BG_BG1TOP, BG1ScrollX, BG1ScrollY
;BG8_1skiped:
	Sprite_DrawLineMcr 82, 2
	test	byte [BG_Regs + 9], 1
	je	near BG_0skiped
	BG_DrawLineMcr8 0, BG_BG0TOP, BG0ScrollX, BG0ScrollY 
	jmp	BG_0skiped

BG16:
	Sprite_DrawLineMcr 161, 1
;	test	byte [BG_Regs + 9], 8
;	je	near BG16_1skiped
;	BG_DrawLineMcr16 1, BG_BG1TOP, BG1ScrollX, BG1ScrollY
;BG16_1skiped:
	Sprite_DrawLineMcr 162, 2
	test	byte [BG_Regs + 9], 1
	je	near BG_0skiped
	BG_DrawLineMcr16 0, BG_BG0TOP, BG0ScrollX, BG0ScrollY
	jmp	BG_0skiped

BG_NOGRP:
	cmp	byte [BG_CHRSIZE], 8
	jne	near BG16_ng

	Sprite_DrawLineMcr ng81, 1
	test	byte [BG_Regs + 9], 8
	je	near BG8_ng_1skiped
	BG_DrawLineMcr8_ng 1, BG_BG1TOP, BG1ScrollX, BG1ScrollY
BG8_ng_1skiped:
	Sprite_DrawLineMcr ng82, 2
	test	byte [BG_Regs + 9], 1
	je	near BG_0skiped
	BG_DrawLineMcr8_ng 0, BG_BG0TOP, BG0ScrollX, BG0ScrollY
	jmp	BG_0skiped

BG16_ng:
	Sprite_DrawLineMcr ng161, 1
;	test	byte [BG_Regs + 9], 8
;	je	near BG16_ng_1skiped
;	BG_DrawLineMcr16_ng 1, BG_BG1TOP, BG1ScrollX, BG1ScrollY
;BG16_ng_1skiped:
	Sprite_DrawLineMcr ng162, 2
	test	byte [BG_Regs + 9], 1
	je	near BG_0skiped
	BG_DrawLineMcr16_ng 0, BG_BG0TOP, BG0ScrollX, BG0ScrollY
BG_0skiped:
	Sprite_DrawLineMcr 163, 3

	pop	ebp
	pop	edx
	pop	edi
	pop	esi
	pop	ebx
	popf
	ret
