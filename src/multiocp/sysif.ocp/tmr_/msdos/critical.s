;
;  SCCS revision: @(#)critical.s	1.1    Released: 2/4/88
;

		name	critical

		include	segment.mac

;
;  General routines called from Microsoft C.
;

		public	_ENTER
		public	_LEAVE

		DSEG

nest		db	0		; nesting of ENTERs
psw		dw	?		; saved processor status word

		ENDDS

		PSEG	MAIN_TEXT
;  void ENTER()
;
;  Enter a critical section: if this is the first ENTER in a
;  nest save the procesor-status-word and disable interrupts.
;

_ENTER		proc	far
		pushf			; save flags
		pop	ax
		cli			; disable interrupts

		cmp	nest, 0		; if nest == 0
		jne	en0		; psw = flags
		mov	psw, ax		; endif
en0:
		inc	nest		; nest++
		ret
_ENTER		endp

;
;  void LEAVE()
;
;  Exit a critical section: restore the processor
;  status word if this is the last LEAVE in a nest.
;  Called with interrupts disabled, by definition.
;

_LEAVE		proc	far
		dec	nest		; nest--
		jnz	le0			
		push	psw		; if nest == 0
		popf				; flags = psw
le0:					; endif
		ret
_LEAVE		endp

		ENDPS	MAIN_TEXT
		end

