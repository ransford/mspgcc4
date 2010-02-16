/*  -*- Mode: Asm -*-  */


	.section .text.libgcc, "ax", @progbits

#if defined(MSP430_HAS_HWMUL)
#if defined(__MSP430X2__)
__MPY=0x4c0
#else
__MPY=0x130
#endif	
__MPYS=(__MPY   + 0x02)
__MAC=(__MPY    + 0x04)
__MACS=(__MPY   + 0x06)
__OP2=(__MPY    + 0x08)
__RESLO=(__MPY  + 0x0a)
__RESHI=(__MPY  + 0x0c)
__SUMEXT=(__MPY + 0x0e)
#endif

#if defined (L_cmpdi2)
	
	.global	__cmpdi2
	.func	__cmpdi2
__cmpdi2:
	sub     2(r1), r12	; a = a-b;
	subc    4(r1), r13
	subc    6(r1), r14
	subc    8(r1), r15
	
	tst     r15		; c<0 ? return -1;
	jge     .L2
	
	mov     #-1, r15	; yes, return -1
	ret
.L2:
	bis     r12, r14	; check if zero
	bis     r13, r15
	bis     r14, r15
	tst     r15
	jeq     .L4		; test result or or'ing all nibbles
	
	mov     #1, r15		; no, positive, return 1
	ret
.L4:
	mov     #0, r15		; return 0
	ret
.endfunc
#endif

#if defined (L_cmpsf2)
	.global __cmpsf2
	.func	__cmpsf2
__cmpsf2:
/* prologue: frame size = 0; addenum 0; alloca:0, varargs:0 , fpr:0*/
.L__FrameSize___cmpsf2=0x0
.L__FrameOffset___cmpsf2=0x4
/* prologue end (size=2) */
        cmp     r12, r14	;  11  cmpsi   [length = 3]
        jne     .L2
        cmp     r13, r15
        jne     .L2      	;  12  bne     [length = 1]
        mov     #llo(0), r15	;  15  *movhi3/7       [length = 1]
	ret
.L2:
        tst     r15		;  20  tstsi   [length = 1]
        jge     .L3		;  21  bge     [length = 1]
        tst     r13		;  22  tstsi   [length = 1]
        jge     .L3		;  23  bge     [length = 1]
        xor     #lhi(-2147483648), r15		;  27  *xorsi3_3       [length = 2]
        xor     #lhi(-2147483648), r13		;  29  *xorsi3_3       [length = 2]
.L3:
        sub     r14, r12	;  64  *subsi3_3       [length = 2]
        subc    r15, r13
        jge     .L4		;  33  bge     [length = 1]
        mov     #llo(1), r15	;  36  *movhi3/7       [length = 1]
	ret
.L4:
        mov     #llo(-1), r15	;  43  *movhi3/7       [length = 1]
.L1:
/* epilogue: frame size=0 */
        ret
/* epilogue end (size=3) */
/* function __cmpsf2 size 25 (20) */

.endfunc

#endif



/*******************************************************
               Multiplication  8 x 8
*******************************************************/
#if defined (L_mulqi3)
/* 
	a = reg:qi 10	clobber
	b = reg:qi 12	clobber
	res = reg:qi 14
*/

	.global	__mulqi3
	.func	__mulqi3
__mulqi3:
	clr	r14
.L__mulqiloop:
	tst.b	r10
	jz	.L__mulqiexit
	clrc
	rrc.b	r12
	jnc	+2
	add.b	r10, r14
	rla.b	r10
	tst.b	r12
	jne	.L__mulqiloop
.L__mulqiexit:
	ret
	.endfunc
#endif 	/* defined (L_mulqi3) */


#if defined (L_mulqihi3)
	.global	__mulqihi3
	.func	__mulqihi3
__mulqihi3:
	sxt	r10
	sxt	r12
	br	#__mulhi3
.endfunc
#endif /* defined (L_mulqihi3) */

#if defined (L_umulqihi3)
	.global	__umulqihi3
	.func	__umulqihi3
__umulqihi3:
	and.b	#-1, r10
	and.b	#-1, r12
	br	#__mulhi3
	.endfunc
#endif /* defined (L_umulqihi3) */

/*******************************************************
               Multiplication  16 x 16
*******************************************************/
#if defined (L_mulhi3)
/* 
	a = reg:hi 10	clobber
	b = reg:hi 12	clobber
	res = reg:hi 14
*/

	.global	__mulhi3
	.func	__mulhi3
__mulhi3:
	clr	r14
.L__mulhiloop:
	tst	r10
	jz	.L__mulhiexit
	clrc
	rrc	r12
	jnc	+2
	add	r10, r14
	rla	r10
	tst     r12
	jne	.L__mulhiloop
.L__mulhiexit:
	ret
	.endfunc
#endif /* defined (L_mulhi3) */

#if defined (L_mulhisi3)
/* clobber r11, r13 */
	.global	__mulhisi3
	.func	__mulhisi3
__mulhisi3:
	br	#__mulsi3
	.endfunc
#endif /* defined (L_mulhisi3) */

#if defined (L_umulhisi3)
	.global	__umulhisi3
	.func	__umulhisi3
__umulhisi3:
	br	#__mulsi3
	.endfunc
#endif /* defined (L_umulhisi3) */

#if defined (L_mulsi3)
/*******************************************************
               Multiplication  32 x 32
*******************************************************/
/*
res = a*b
	a - reg:SI 10 clobber
	b - reg:SI 12 clobber
	res - reg: SI 14
*/
	.global	__mulsi3
	.func	__mulsi3

__mulsi3:
	clr	r14
	clr	r15
	jmp	.L__mulsi3st
.L__mulsi3loop:
	clrc
	rrc	r13		;	b >>= 1
	rrc 	r12
	jnc	+4		;	
	add	r10, r14	;	res = res + a
	addc	r11, r15
	rla	r10
	rlc	r11		;	a <<= 1
.L__mulsi3st:
	tst	r12		; if b ne 0 goto L__mulsi3loop
	jne	.L__mulsi3loop
	tst	r13
	jne	.L__mulsi3loop
	ret
	.endfunc

#endif

#if defined (L_mulsi3hw)
	
	.global	__umulsi3hw
	.func	__umulsi3hw
__umulsi3hw:
	mov	r12, &__MPY
	mov	r10, &__OP2
	mov	r12, &__MAC
	mov	&__RESLO, r14
	mov	&__RESHI, &__RESLO
	mov	r11, &__OP2
	mov	r13, &__MAC
	mov	r10, &__OP2
	mov	&__RESLO, r15
	ret
.endfunc

#endif

	
/*******************************************************
       Division 8 / 8 => (result + remainder)
*******************************************************/

#define r_rem   r14	/* remainder */
#define r_arg1  r12	/* dividend, quotient */
#define r_arg2  r10	/* divisor */
#define r_cnt   r11	/* loop count */
#define r_tmp	r13	/* save carry flag */


#if defined (L_udivmodqi4)
	.global	__udivmodqi4
	.func	__udivmodqi4
__udivmodqi4:
	xor.b	r_rem, r_rem		; clear reminder and carry
	mov.b	#9, r_cnt
	jmp	.L__udivmodqi4_ep
.L__udivmodqi4_loop:	
	rrc	r_tmp			; restore carry bit
	rlc.b	r_rem
	cmp.b	r_arg2, r_rem
	jlo	.L__udivmodqi4_ep
	sub.b	r_arg2, r_rem		; FIXME: will this clobber carry ?
.L__udivmodqi4_ep:
	rlc.b	r_arg1			; shift divident
	rlc	r_tmp			; save carry bit
	dec.b	r_cnt			; this clobbers C bit.
	jnz	.L__udivmodqi4_loop
	ret
	.endfunc
#endif /* defined (L_udivmodqi4) */


#if defined (L_divmodqi4)
	.global	__divmodqi4
	.func	__divmodqi4
__divmodqi4:
	clr	r_tmp
	bit	#0x80, r_arg1		; save divident sign
	jnc	.L__divmodqi4arg1pos
	inv.b	r_arg1			; negate
	inc.b	r_arg1
	bis	#4, r_tmp

.L__divmodqi4arg1pos:
	bit	#0x80, r_arg2		; check divisor sign
	jnc	.L__divmodqi4arg2pos
	inv.b	r_arg2			; negate
	inc.b	r_arg2
	bis	#8, r_tmp

.L__divmodqi4arg2pos:
	
	call	#__udivmodqi4		; do unsigned division
	rrc	r_tmp			; restore carry and sign bits

	bit	#4, r_tmp		; is divident < 0 ?
	jnc	.L__divmodqi4rem	; no. skip
	inv.b	r_rem			; negate remainder
	inc.b	r_rem

;;	bit	#8, r_tmp
;;	jc	.L__divmodqi4end
	inv.b	r_arg1			; negate quotient
	inc.b	r_arg1

.L__divmodqi4rem:
	bit	#8, r_tmp
	jnc	.L__divmodqi4end
	inv.b	r_arg1
	inc.b	r_arg1

.L__divmodqi4end:
	ret

	.endfunc
#endif /* defined (L_divmodqi4) */

#undef r_rem
#undef r_arg1
#undef r_arg2
#undef r_cnt 
#undef r_tmp


/*******************************************************
       Division 16 / 16 => (result + remainder)
*******************************************************/

#define r_rem   r14	/* remainder */
#define r_arg1  r12	/* dividend, quotient */
#define r_arg2  r10	/* divisor */
#define r_cnt   r11	/* loop count */
#define r_tmp	r13


#if defined (L_udivmodhi4)
	.global	__udivmodhi4
	.func	__udivmodhi4
__udivmodhi4:
	xor	r_rem, r_rem		; clear reminder and carry
	mov	#17, r_cnt
	jmp	.L__udivmodhi4_ep
.L__udivmodhi4_loop:	
	rrc	r_tmp			; restore carry bit
	rlc	r_rem
	cmp	r_arg2, r_rem
	jlo	.L__udivmodhi4_ep
	sub	r_arg2, r_rem
.L__udivmodhi4_ep:
	rlc	r_arg1
	rlc	r_tmp			; save carry bit
	dec	r_cnt			; this clobbers C bit.
	jnz	.L__udivmodhi4_loop
	ret
	.endfunc
#endif /* defined (L_udivmodhi4) */


#if defined (L_divmodhi4)
#define r_rem   r14     /* remainder */
#define r_arg1  r12     /* dividend, quotient */
#define r_arg2  r10     /* divisor */   
#define r_cnt   r11     /* loop count */
#define r_tmp   r13


	.global	__divmodhi4
	.func	__divmodhi4
__divmodhi4:
	clr	r_tmp			; clear reg is cheaper than clr 2 bits.
	bit	#0x8000, r_arg1		; save divident sign
	jnc	.L__divmodhi4arg1pos
	inv	r_arg1			; negate
	inc	r_arg1
	bis	#4, r_tmp

.L__divmodhi4arg1pos:
	bit	#0x8000, r_arg2		; check divisor sign
	jnc	.L__divmodhi4arg2pos
	inv	r_arg2			; negate
	inc	r_arg2
	bis	#8, r_tmp

.L__divmodhi4arg2pos:	
	call	#__udivmodhi4		; do unsigned division
	rrc	r_tmp			; restore carry and sign bits

	bit	#4, r_tmp		; is divident < 0 ?
	jnc	.L__divmodhi4rem	; no. skip
	inv	r_rem			; negate remainder
	inc	r_rem

;;	bit	#8, r_tmp
;;	jc	.L__divmodhi4end
	inv	r_arg1			; negate quotient
	inc	r_arg1

.L__divmodhi4rem:
	bit	#8, r_tmp
	jnc	.L__divmodhi4end
	inv	r_arg1
	inc	r_arg1

.L__divmodhi4end:
	ret
	.endfunc
#endif /* defined (L_divmodhi4) */

#undef r_rem
#undef r_arg1
#undef r_arg2
#undef r_cnt 
#undef r_tmp

/*******************************************************
       Division 32 / 32 => (result + remainder)
*******************************************************/

#if defined (L_udivmodsi4)

#define r_remh  r15  
#define r_reml  r14	/* remainder */
#define r_arg1h r13
#define r_arg1l r12	/* dividend, quotient */
#define r_arg2h r11   
#define r_arg2l r10	/* divisor */
#define r_cnt   r9	/* loop count */
#define r_tmp   r8

	.global	__udivmodsi4
	.func	__udivmodsi4
__udivmodsi4:
	xor	r_remh, r_remh		; clear reminder and carry
	xor	r_reml, r_reml
	mov	#33, r_cnt
	jmp	.L__udivmodsi4_ep
.L__udivmodsi4_loop:	
	rrc	r_tmp			; restore carry bit
	rlc	r_reml
	rlc	r_remh

	cmp	r_arg2h, r_remh		; is reminder < divisor ?
	jlo	.L__udivmodsi4_ep	; yes, skip correction
	jne	+4
					; they equal. check LSBytes
	cmp	r_arg2l, r_reml
	jlo	.L__udivmodsi4_ep	; is reminder still < divisor ?

	sub	r_arg2l, r_reml		; adjust reminder
	subc	r_arg2h, r_remh

.L__udivmodsi4_ep:
	rlc	r_arg1l
	rlc	r_arg1h
	rlc	r_tmp
	dec	r_cnt			; this clobbers C bit.
	jnz	.L__udivmodsi4_loop
	ret
	.endfunc

#undef r_remh
#undef r_reml  
#undef r_arg1h
#undef r_arg1l
#undef r_arg2h
#undef r_arg2l

#undef r_cnt
#undef r_tmp

#endif /* defined (L_udivmodsi4) */


#if defined (L_divmodsi4)
#define r_remh  r15  
#define r_reml  r14     /* remainder */
#define r_arg1h r13
#define r_arg1l r12     /* dividend, quotient */
#define r_arg2h r11   
#define r_arg2l r10     /* divisor */   
#define r_cnt   r9      /* loop count */
#define r_tmp   r8

	.global	__divmodsi4
	.func	__divmodsi4
__divmodsi4:
	clr	r_tmp			; clear reg is cheaper than clr 2 bits.
	bit	#0x8000, r_arg1h		; save divident sign
	jz	.L__divmodsi4arg1pos
	inv	r_arg1h			; negate
	inv	r_arg1l
	inc	r_arg1l
	adc	r_arg1h
	bis	#4, r_tmp

.L__divmodsi4arg1pos:
	bit	#0x8000, r_arg2h		; check divisor sign
	jz	.L__divmodsi4arg2pos
	inv	r_arg2h			; negate
	inv	r_arg2l
	inc	r_arg2l
	adc	r_arg2h
	bis	#8, r_tmp		; save divisor sign

.L__divmodsi4arg2pos:
	
	call	#__udivmodsi4		; do unsigned division
	rrc	r_tmp			; restore carry and sign bits

	bit	#4, r_tmp		; is divident < 0 ?
	jz	.L__divmodsi4rem	; no. skip
	inv	r_reml			; negate remainder
	inv	r_remh
	inc	r_reml
	adc	r_remh

;;	bit	#8, r_tmp
;;	jc	.L__divmodsi4end
	inv	r_arg1l			; negate quotient
	inv	r_arg1h
	inc	r_arg1l
	adc	r_arg1h

.L__divmodsi4rem:
	bit	#8, r_tmp
	jz	.L__divmodsi4end
	inv	r_arg1l
	inv	r_arg1h
	inc	r_arg1l
	adc	r_arg1h

.L__divmodsi4end:
	ret
	.endfunc

#undef r_remh  
#undef r_reml  
#undef r_arg1h 
#undef r_arg1l
#undef r_arg2h
#undef r_arg2l

#undef r_cnt
#undef r_tmp

#endif /* defined (L_divmodsi4) */


/******* CRT support functions *********/

#if defined(L_reset_vector__)
/*****************************************************************
 * Program starts here.
 *   overwriting this label in the user program
 *   causes removing all strtup code except __do_global_ctors
 *****************************************************************/
	.section .init0, "ax", @progbits

	.global	_reset_vector__
	.weak   _reset_vector__

	.func   _reset_vector__

_reset_vector__:

    /* link following functions if library _reset_vector__ used */

;    stack can be initialized in main() prologue,
;    but setting stack pointer here allows to call subroutines 
;    from startup code and call constructors of statically allocated C++ objects.
;    Stack pointer will have the same value entering main() as here,
;    so -mno-stack-init can be used to reduce code size.
;    initial stack value can be set in ld script as __stack symbol 
;    (end of RAM by default), or via -defsym __stack=<address> ld option
;    or via -Wl,defsym,__stack=<address> gcc option, or by redefining
;    __init_stack function as fololws:
;
;#if defined (__cplusplus)
;extern "C" 
;#endif
;__attribute__((__naked__)) __attribute__((section(".init2"))) void __init_stack()
;{
;    asm volatile("\tmov\t#stack_addr, r1\n");
;}
;

	.global	__init_stack

	.global	__low_level_init
	.global	__do_copy_data
	.global	__do_clear_bss
	.global	__jump_to_main

	.endfunc
#endif  /* defined(L_reset_vector__) */
    
#if defined(L__init_stack)
/*****************************************************************
 * Set stack pointer
 * can be overwriten
 *****************************************************************/
	.section .init2, "ax", @progbits

	.global	__init_stack
	.weak   __init_stack

	.func   __init_stack

__init_stack:
	mov     #__stack, r1

	.endfunc
#endif

#if defined(L__low_level_init)
/*****************************************************************
 * Initialize peripherial, particularly disable watchdog
 * can be overwriten
 *****************************************************************/
	.section .init3, "ax", @progbits

	.global	__low_level_init
	.weak   __low_level_init

	.func   __low_level_init

__low_level_init:
	mov     #0x5a80, &0x120

	.endfunc
#endif

#if defined(L_copy_data)
/*****************************************************************
 * Initialize data: copy data 
 * from __data_load_start ( = _etext) to __data_start
 * can be overwriten
 *****************************************************************/
	.section .init4, "ax", @progbits

	.global __do_copy_data
	.weak   __do_copy_data

	.func   __do_copy_data

__do_copy_data:
	mov     #__data_size, r15
	tst     r15
	jz      .L__copy_data_end
.L__copy_data_loop:
	decd    r15
	mov.w   __data_load_start(r15), __data_start(r15)    ; data section is word-aligned, so word transfer is acceptable
	jne     .L__copy_data_loop
.L__copy_data_end:

	.endfunc
#endif /* defined(L_copy_data) */
    
#if defined(L_clear_bss)
/*****************************************************************
 * Initialize data: clear .bss
 * can be overwriten
 *****************************************************************/
	.section .init4, "ax", @progbits

	.global __do_clear_bss
	.weak   __do_clear_bss

	.func   __do_clear_bss

__do_clear_bss:
	mov     #__bss_size, r15
	tst     r15
	jz      .L__clear_bss_end
.L__clear_bss_loop:
	dec     r15
	clr.b   __bss_start(r15)
	jne     .L__clear_bss_loop
.L__clear_bss_end:

	.endfunc
#endif  /* defined(L_clear_bss) */

#if defined(L_ctors)
/*****************************************************************
 * Call C++ global and static objects constructors
 * can be overwriten
 *****************************************************************/
	.section .init6, "ax", @progbits
	.global __do_global_ctors
	.weak   __do_global_ctors
    
	.func   __do_global_ctors

__do_global_ctors:
	mov     #__ctors_start, r11
	mov     #__ctors_end,   r10
.L__ctors_loop:
	call	@r11+	; call constructor
	cmp     r10, r11
	jne     .L__ctors_loop

	.endfunc
#endif
    
#if defined(L__jump_to_main)
/*****************************************************************
 * jump to main.
 * can be overwriten
 *****************************************************************/
	.section .init9, "ax", @progbits

	.global	__jump_to_main
	.weak   __jump_to_main

	.func   __jump_to_main

__jump_to_main:
	br      #main
	.endfunc
#endif

#if defined(L__stop_progExec__)
/*****************************************************************
 * return from main.
 * can be overwriten
 *****************************************************************/
	.section .fini9, "ax", @progbits
	.global __stop_progExec__
	.weak	__stop_progExec__

	.func  __stop_progExec__

__stop_progExec__:

	.endfunc
#endif

#if defined(L_dtors)
/*****************************************************************
 * Call C++ global and static objects destructors
 * can be overwriten
 *****************************************************************/
	.section .fini6,"ax",@progbits
	.global __do_global_dtors
	.weak   __do_global_dtors
    
	.func   _dtors

__do_global_dtors:
	mov     #__dtors_start, r11
	mov     #__dtors_end, r10
.L__dtors_loop:
	call    @r11+
	cmp     r10, r11
	jne     .L__dtors_loop

	.endfunc
#endif

#if defined(L__stop_progExec__)
/*****************************************************************
 * endless loop
 * can be overwriten
 *****************************************************************/
	.section .fini0, "ax", @progbits

	.func   _endless_loop__
1:
	jmp  1b

	.endfunc
#endif

/********* PROLOGE / EPILOGUE aux routines ******************/
#if defined (L__prologue_saver)
	.global __prologue_saver
	.func	__prologue_saver
__prologue_saver:
	mov	r4, 0(r1)
	mov	r5, 2(r1)
	mov	r6, 4(r1)
	mov	r7, 6(r1)
	mov	r8, 8(r1)
	mov	r9, 10(r1)
	mov	r10, 12(r1)
	mov	r11, 14(r1)
	br	r12	; now jump to the function body
.endfunc

#endif


#if defined (L__epilogue_restorer)
	.global __epilogue_restorer
	.func	__epilogue_restorer
__epilogue_restorer:
	pop	r4
	pop	r5
	pop	r6
	pop	r7
	pop	r8
	pop	r9
	pop	r10
	pop	r11
	ret
.endfunc

#endif


#if defined (L__epilogue_restorer_intr)
	.global __epilogue_restorer_intr
	.func	__epilogue_restorer_intr
__epilogue_restorer_intr:
	pop	r4
	pop	r5
	pop	r6
	pop	r7
	pop	r8
	pop	r9
	pop	r10
	pop	r11
	pop	r12
	pop	r13
	pop	r14
	pop	r15
	reti
.endfunc

#endif

/******************************************
 * quot/rem = 64/64
 ******************************************/

#if defined (L_udivmoddi3_parts) || defined (L_udivdi3) || defined (L_umoddi3) || defined (L_divdi3) || defined (L_moddi3)

#define r_remhh  r11  	/* remainder */
#define r_remhl  r10
#define r_remlh	 r9
#define r_remll	 r8

#define r_arg1hh r15	/* dividend, quotient */
#define r_arg1hl r14
#define r_arg1lh r13
#define r_arg1ll r12

#define r_arg2hh r7	/* divisor */
#define r_arg2hl r6
#define r_arg2lh r5
#define r_arg2ll r4

#define r_cnt   2(r1)	/* loop count */
#define r_tmp   0(r1)	/* we'll save carry and signs here */

#endif


#if defined (L_udivmoddi3_parts)

	.global	__udivmoddi3_parts
	.func	__udivmoddi3_parts
__udivmoddi3_parts:
	xor	r_remhh, r_remhh	; clear reminder and carry
	xor	r_remhl, r_remhl 
	xor	r_remlh, r_remlh
	xor	r_remll, r_remll
	
	mov	#65, 2+r_cnt
	jmp	.L__udivmoddi3_ep

.L__udivmoddi3_loop:	
	rrc	2+r_tmp			; restore carry bit
	
	rlc	r_remll			; shift carry in.
	rlc	r_remlh
	rlc	r_remhl
	rlc	r_remhh
	
	cmp     r_arg2hh, r_remhh       ; is reminder < divisor ?
	jlo     .L__udivmoddi3_ep       ; yes, skip correction
	jne     .L_udmdcrt
					; they equal. check LSBytes
	cmp     r_arg2hl, r_remhl
	jlo     .L__udivmoddi3_ep       ; is reminder still < divisor ?
	jne     .L_udmdcrt

	cmp     r_arg2lh, r_remlh
	jlo     .L__udivmoddi3_ep
	jne     .L_udmdcrt

	cmp     r_arg2ll, r_remll
	jlo     .L__udivmoddi3_ep
	jne     .L_udmdcrt

.L_udmdcrt:
	sub	r_arg2ll, r_remll	; adjust reminder
	subc	r_arg2lh, r_remlh
	subc	r_arg2hl, r_remhl
	subc	r_arg2hh, r_remhh
	
.L__udivmoddi3_ep:
	rlc	r_arg1ll		; shift carry into arg1
	rlc	r_arg1lh
	rlc	r_arg1hl
	rlc	r_arg1hh
	
	rlc	2+r_tmp			; save carry
	dec	2+r_cnt			; this clobbers C bit.
	jnz	.L__udivmoddi3_loop
	
	ret
	.endfunc

#endif /* defined (L_udivmoddi3_parts) */


#if defined (L_udivdi3)

;;  First arg will be in r15:r12 
;;  next on stack
;;	return in r15:r12
;; rearrange them as:
;;	r15:r12 	-> 	r_arg1hh:r_arg1ll
;;	stack+8:stack+2 -> 	r_arg2hh:r_arg2ll

	.global	__udivdi3
	.func	__udivdi3
__udivdi3:
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	
	mov	18+0(r1), r_arg2ll	; 18 is a stack offset
	mov	18+2(r1), r_arg2lh	; so move arg 2 in.
	mov	18+4(r1), r_arg2hl
	mov	18+6(r1), r_arg2hh
	
	sub	#4, r1
	call	#__udivmoddi3_parts
	add	#4, r1
	
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	ret
	.endfunc
#endif


#if defined (L_umoddi3)
	.global	__umoddi3
	.func	__umoddi3
__umoddi3:
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	
	mov	18+0(r1), r_arg2ll	
	mov	18+2(r1), r_arg2lh
	mov	18+4(r1), r_arg2hl
	mov	18+6(r1), r_arg2hh

	sub	#4, r1
	call    #__udivmoddi3_parts
	add	#4, r1
	
	mov	r_remhh, r15	; move reminder to (reg:DI 12)
	mov	r_remhl, r14
	mov	r_remlh, r13
	mov	r_remll, r12
	
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	ret
	.endfunc
#endif


#if defined (L_divdi3)
	.global	__divdi3
	.func	__divdi3
__divdi3:
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	
	mov	18+0(r1), r_arg2ll	
	mov	18+2(r1), r_arg2lh
	mov	18+4(r1), r_arg2hl
	mov	18+6(r1), r_arg2hh
	
	sub	#4, r1
	
	clr	r_tmp
	bit     #0x8000, r_arg1hh
	jnc     .L__divdi3rempos
	inv	r_arg1hh
	inv     r_arg1hl
	inv     r_arg1lh
	inv     r_arg1ll
	inc	r_arg1ll
	adc     r_arg1lh
	adc     r_arg1hl
	adc     r_arg1hh
	bis     #4, r_tmp
	
.L__divdi3rempos:
	bit     #0x8000, r_arg2hh
	jnc     .L__divdi3arg2pos	
	inv	r_arg2hh
	inv     r_arg2hl
	inv     r_arg2lh
	inv     r_arg2ll
	inc	r_arg2ll
	adc     r_arg2lh
	adc     r_arg2hl
	adc     r_arg2hh
	xor     #4, r_tmp	; this is a trick - invert bit 4 =>
				; do not perform double negation.
.L__divdi3arg2pos:
	call	#__udivmoddi3_parts
	
	rrc     r_tmp	; restore sign bits

	bit     #4, r_tmp
	jz      .L__divdi3end
	inv     r_arg1hh
	inv     r_arg1hl
	inv     r_arg1lh
	inv     r_arg1ll
	inc	r_arg1ll
	adc     r_arg1lh
	adc     r_arg1hl
	adc     r_arg1hh
	
.L__divdi3end:
	add	#4, r1
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	ret
	.endfunc
#endif


#if defined (L_moddi3)
	.global	__moddi3
	.func	__moddi3
__moddi3:
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	
	mov	18+0(r1), r_arg2ll	
	mov	18+2(r1), r_arg2lh
	mov	18+4(r1), r_arg2hl
	mov	18+6(r1), r_arg2hh
	
	sub	#4, r1
	
	clr	r_tmp
	bit     #0x8000, r_arg1hh
	jnc     .L__moddi3rempos
	inv	r_arg1hh
	inv     r_arg1hl
	inv     r_arg1lh
	inv     r_arg1ll
	inc	r_arg1ll
	adc     r_arg1lh
	adc     r_arg1hl
	adc     r_arg1hh
	bis     #4, r_tmp
	
.L__moddi3rempos:
	bit     #0x8000, r_arg2hh
	jnc     .L__moddi3arg2pos	
	inv	r_arg2hh
	inv     r_arg2hl
	inv     r_arg2lh
	inv     r_arg2ll
	inc	r_arg2ll
	adc     r_arg2lh
	adc     r_arg2hl
	adc     r_arg2hh

.L__moddi3arg2pos:
	call	#__udivmoddi3_parts
	
	rrc     r_tmp

	bit     #4, r_tmp
	jz      .L__moddi3rem
	
	inv	r_remhh
	inv     r_remhl
	inv     r_remlh
	inv     r_remll
	inc	r_remll
	adc     r_remlh
	adc     r_remhl
	adc     r_remhh

.L__moddi3rem:
	mov	r_remhh, r15
	mov	r_remhl, r14
	mov	r_remlh, r13
	mov	r_remll, r12

	add	#4, r1
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	ret
	.endfunc
#endif


/**************************************************************
 * Multiplication 64 = 64 x 64
 **************************************************************/
#if defined(L_muldi3) && !defined(MSP430_HAS_HWMUL)

#define r_reshh  r11  	/* res = arg1 * arg2 */
#define r_reshl  r10
#define r_reslh	 r9
#define r_resll	 r8

#define r_arg1hh r15	/* arg1 */
#define r_arg1hl r14
#define r_arg1lh r13
#define r_arg1ll r12

#define r_arg2hh r7	/* arg2 */
#define r_arg2hl r6
#define r_arg2lh r5
#define r_arg2ll r4

	.global	__muldi3
	.func	__muldi3
__muldi3:
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	
	mov	18+0(r1), r_arg2ll	; 18 is a stack offset
	mov	18+2(r1), r_arg2lh	; so move arg 2 in.
	mov	18+4(r1), r_arg2hl
	mov	18+6(r1), r_arg2hh

	clr	r_reshh
	clr	r_reshl
	clr	r_reslh
	clr	r_resll
	
.L_muldi3_loop:
	clrc
	rrc	r_arg2hh		; arg2 >>= 1 (shift LSB into carry)
	rrc	r_arg2hl
	rrc	r_arg2lh
	rrc	r_arg2ll
	
	jnc	+8			; check if bit is set
					; yes, it is.
	add	r_arg1ll, r_resll	; res += arg1
	addc	r_arg1lh, r_reslh
	addc	r_arg1hl, r_reshl
	addc	r_arg1hh, r_reshh
	
	rla	r_arg1ll		; arg1 <<= 1
	rlc	r_arg1lh
	rlc	r_arg1hl
	rlc	r_arg1hh
	
	tst	r_arg2ll		; arg2 !=0 ?  loop again , exit otherwise.
	jne	.L_muldi3_loop
	tst	r_arg2lh
	jne	.L_muldi3_loop
	tst	r_arg2hl
	jne	.L_muldi3_loop
	tst	r_arg2hh
	jne	.L_muldi3_loop

	; move result to proper location
	mov	r_resll, r12
	mov	r_reslh, r13
	mov	r_reshl, r14
	mov	r_reshh, r15

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	r7
	pop	r6
	pop	r5
	pop	r4
	ret
	.endfunc
#endif

#if defined(L_muldi3) && defined(MSP430_HAS_HWMUL)

#define r_reshh  r11    /* res = arg1 * arg2 */
#define r_reshl  r10
#define r_reslh  r9
#define r_resll  r8

#define r_arg1hh r15    /* arg1 */
#define r_arg1hl r14
#define r_arg1lh r13
#define r_arg1ll r12

#define r_arg2hh r7     /* arg2 */
#define r_arg2hl r6
#define r_arg2lh r5
#define r_arg2ll r4

          .global __muldi3
          .func   __muldi3
__muldi3:

        push    r4
        push    r5
        push    r6
        push    r7
        push    r8
        push    r9
        push    r10
        push    r11

        mov     18+0(r1), r_arg2ll      ; 18 is a stack offset
        mov     18+2(r1), r_arg2lh      ; so move arg 2 in.
        mov     18+4(r1), r_arg2hl
        mov     18+6(r1), r_arg2hh

;;      r15:r14:r13:r12 * r7:r6:r5:r4 -> r11:r10:r9:r8
;; actual code follows....

        mov     r_arg1ll,&__MPY
        mov     r_arg2ll,&__OP2           ;;                      LL1xLL2
        mov     &__RESLO,r_resll
        mov     &__RESHI,&__RESLO
        mov     &__SUMEXT,&__RESHI

        mov     r_arg1ll,&__MAC
        mov     r_arg2lh,&__OP2           ;;                  LL1xLH2
        mov     r_arg1lh,&__MAC
        mov     r_arg2ll,&__OP2           ;;                  LH1xLL2
        mov     &__RESLO,r_reslh
        mov     &__RESHI,&__RESLO
        mov     &__SUMEXT,&__RESHI

        mov     r_arg2lh,&__OP2           ;;              LH1xLH2
        mov     r_arg1ll,&__MAC
        mov     r_arg2hl,&__OP2           ;;              LL1xHL2
        mov     r_arg1hl,&__MAC
        mov     r_arg2ll,&__OP2           ;;              HL1xLL2
        mov     &__RESLO,r_reshl
        mov     &__RESHI,&__RESLO

        mov     r_arg2lh,&__OP2           ;;          HL1xLH2
        mov     r_arg1ll,&__MAC
        mov     r_arg2hh,&__OP2           ;;          LL1xHH2
        mov     r_arg1lh,&__MAC
        mov     r_arg2hl,&__OP2           ;;          LH1xHL2
        mov     r_arg1hh,&__MAC
        mov     r_arg2ll,&__OP2           ;;          HH1xLL2
        mov     &__RESLO,r_reshh

;; reload result
        mov     r_resll, r12
        mov     r_reslh, r13
        mov     r_reshl, r14
        mov     r_reshh, r15

        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     r7
        pop     r6
        pop     r5
        pop     r4
        ret
.endfunc
#endif
