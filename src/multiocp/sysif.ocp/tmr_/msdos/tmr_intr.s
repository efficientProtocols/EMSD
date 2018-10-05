	name	tmr_intr


;
;  SCCS revision: @(#)tmr_intr.s	1.1    Released: 2/4/88
;

;
;  Segment definitions.
;

_DATA	SEGMENT  WORD PUBLIC 'DATA'
	extrn	_origClockIntr:dword
_DATA	ENDS

CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS

_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS

DGROUP	GROUP	CONST,	_BSS,	_DATA

ASSUME DS:DGROUP

	extrn	_TMR_clock:far

MAIN_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
	public	_myClockIntr

_myClockIntr  proc	far
	sub	sp,4	; space for chaining
	push	bp
	mov	bp,sp
	push	ds
	push	es
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di

	mov	ax,SEG DGROUP	; default data segment
	mov	ds,ax

	; variable _origClockIntr is in the default data segment
	les	ax,_origClockIntr	; setup return to previous handler
	mov	[bp+4],es
	mov	[bp+2],ax

	mov	ax,SEG DGROUP; default data segment
	mov	ds,ax

	call	_TMR_clock

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	es
	pop	ds
	pop	bp
	ret

_myClockIntr	endp

MAIN_TEXT	ENDS
	end
