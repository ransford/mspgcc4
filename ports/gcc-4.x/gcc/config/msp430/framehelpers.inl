#pragma once

extern void *ggc_alloc(size_t);

/*
This function is used to test, how certain generated INSNs are actually matched and written to the
assembly file.
The typical test scenario for msp430_codegen_test() is:
1. Create a C file containing the following
int test()
{
__msp430_codegen_test_entry();
}
2. Examine the output with "msp430-gcc -S 0.c -o 0.S && less 0.S"
3. Compare the output in 0.S with the expected result of msp430_codegen_test() run
*/

/*
Functions beginning with msp430_fh_ are prologue/epilogue (frame) helpers.
*/
static inline void msp430_fh_emit_push_reg(int reg_num)
{
	rtx pushword = gen_rtx_MEM (HImode, gen_rtx_PRE_DEC (HImode, stack_pointer_rtx));
	//rtx insn = emit_insn(gen_rtx_SET(VOIDmode, pushword, gen_rtx_REG (HImode, reg_num)));
	rtx insn = emit_insn(gen_pushhi_prologue(gen_rtx_REG (HImode, reg_num), pushword));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_emit_pop_reg(int reg_num)
{
	/*rtx popword  = gen_rtx_MEM (HImode, gen_rtx_POST_INC (HImode, stack_pointer_rtx));
	rtx insn = emit_insn(gen_rtx_SET(VOIDmode, gen_rtx_REG (HImode, reg_num), popword));
	RTX_FRAME_RELATED_P(insn) = 1;*/

	rtx insn = emit_insn(gen_pophi_reg(gen_rtx_REG (HImode, reg_num)));
}

static inline void msp430_fh_sub_sp_const(int num_bytes)
{
	rtx insn = emit_move_insn (stack_pointer_rtx,
//		gen_rtx_MINUS (HImode, stack_pointer_rtx, gen_int_mode (num_bytes, HImode)));
		gen_rtx_PLUS (HImode, stack_pointer_rtx, gen_int_mode (-num_bytes, HImode)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_add_sp_const(int num_bytes)
{
	rtx insn = emit_move_insn (stack_pointer_rtx,
		gen_rtx_PLUS (HImode, stack_pointer_rtx, gen_int_mode (num_bytes, HImode)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_add_reg_const(int reg_num, int num_bytes)
{
	rtx reg_rtx = gen_rtx_REG (HImode, reg_num);
	rtx insn = emit_move_insn (reg_rtx,
		gen_rtx_PLUS (HImode, reg_rtx, gen_int_mode (num_bytes, HImode)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_gen_mov_r2r(int dest_reg, int src_reg)
{
	rtx insn = emit_move_insn (gen_rtx_REG (HImode, dest_reg), gen_rtx_REG (HImode, src_reg));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_gen_mov_pc_to_reg(int dest_reg)
{
	rtx insn = emit_insn(gen_save_pc_to_reg(gen_rtx_REG (HImode, dest_reg)));
}

static inline const char *msp430_format_sym_plus_off(const char *sym_name, int offset)
{
	char *pBuf;
	if (!offset)
		return sym_name;
	pBuf = (char *)ggc_alloc(strlen(sym_name) + 20);
	sprintf(pBuf, "(%s%s%d)", sym_name, (offset >= 0) ? "+" : "", offset);
	return pBuf;
}

static inline void msp430_fh_load_sp_with_sym_plus_off(const char *sym_name, int offset)
{
	rtx insn;

	insn = emit_move_insn (stack_pointer_rtx, gen_rtx_SYMBOL_REF(HImode, msp430_format_sym_plus_off(sym_name, offset)));
}

/*
	WARNING! This function is called from 2 places:
		* Prologue saver
		* Exit from main()
	As the "explicit_br" INSN does not report to be modifying PC, this may screw up DWARF2 frame info generation.
	Optimally, the "explicit_br" INSN should be replaced by something similar to "call_prologue_saves" INSN from
	the AVR implementation. As for return from main, the question is still open.
*/
static inline void msp430_fh_br_to_symbol_plus_offset(const char *sym_name, int offset)
{
	rtx insn;
	insn = gen_explicit_br(gen_rtx_SYMBOL_REF(HImode, msp430_format_sym_plus_off(sym_name, offset)));
	/*insn = gen_rtx_SET (VOIDmode,
						pc_rtx,
						gen_rtx_LABEL_REF(VOIDmode, gen_rtx_SYMBOL_REF(HImode, msp430_format_sym_plus_off(sym_name, offset))));*/
	emit_insn (insn);
}

static inline void msp430_fh_bic_deref_sp(int mask)
{
	rtx insn, sp_deref = gen_rtx_MEM (HImode, stack_pointer_rtx);
	insn = emit_insn(gen_nandhi(sp_deref, gen_int_mode(mask, HImode), sp_deref));
}