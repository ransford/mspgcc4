/* This work is partially financed by the European Commission under the
* Framework 6 Information Society Technologies Project
* "Wirelessly Accessible Sensor Populations (WASP)".
*/

/*
GCC 4.x port by Ivan Shcherbakov <mspgcc@sysprogs.org>
*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-attr.h"
#include "flags.h"
#include "reload.h"
#include "tree.h"
#include "output.h"
#include "expr.h"
#include "toplev.h"
#include "obstack.h"
#include "function.h"
#include "recog.h"
#include "tm_p.h"
#include "target.h"
#include "target-def.h"
#include "insn-codes.h"
#include "ggc.h"
#include "langhooks.h"

#if GCC_VERSION_INT < 0x430
#define add_builtin_function lang_hooks.builtin_function
#endif


/* The following functions are defined in this file and used by msp430.c */
void msp430_init_builtins (void);
rtx msp430_expand_builtin (tree, rtx, rtx, enum machine_mode, int);


static const struct {
	const char *name;
	int md_code;
} msp430builtins[4] = {
	{"__bic_sr_irq", CODE_FOR_bic_sr_irq},
	{"__bis_sr_irq", CODE_FOR_bis_sr_irq},
	{"__get_frame_address", CODE_FOR_get_frame_address},
	{0, 0}
};

static int msp430_codegen_test_entry_idx = 3;	/* Still redefined later */

void msp430_init_builtins(void)
{
	tree args;
	int builtin_idx = 0;

	args = tree_cons (NULL_TREE, integer_type_node, void_list_node);

	add_builtin_function (msp430builtins[builtin_idx].name, 
		build_function_type (void_type_node, args),
		builtin_idx, BUILT_IN_MD, NULL, NULL_TREE);

	builtin_idx++;

	add_builtin_function (msp430builtins[builtin_idx].name, 
		build_function_type (void_type_node, args),
		builtin_idx, BUILT_IN_MD, NULL, NULL_TREE);

	builtin_idx++;

	args = tree_cons (NULL_TREE, void_type_node, void_list_node);
	add_builtin_function (msp430builtins[builtin_idx].name, 
		build_function_type (ptr_type_node, args),
		builtin_idx, BUILT_IN_MD, NULL, NULL_TREE);

	builtin_idx++;

	args = tree_cons (NULL_TREE, void_type_node, void_list_node);
	add_builtin_function ("__msp430_codegen_test_entry", 
		build_function_type (void_type_node, args),
		builtin_idx, BUILT_IN_MD, NULL, NULL_TREE);

	msp430_codegen_test_entry_idx = builtin_idx;

	builtin_idx++;
}

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
	rtx insn = emit_insn(gen_rtx_SET(VOIDmode, pushword, gen_rtx_REG (HImode, reg_num)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_emit_pop_reg(int reg_num)
{
	rtx popword  = gen_rtx_MEM (HImode, gen_rtx_POST_INC (HImode, stack_pointer_rtx));
	rtx insn = emit_insn(gen_rtx_SET(VOIDmode, gen_rtx_REG (HImode, reg_num), popword));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_sub_sp_const(int num_bytes)
{
	rtx insn = emit_move_insn (stack_pointer_rtx,
		gen_rtx_MINUS (HImode, stack_pointer_rtx, gen_int_mode (num_bytes, HImode)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_add_sp_const(int num_bytes)
{
	rtx insn = emit_move_insn (stack_pointer_rtx,
		gen_rtx_PLUS (HImode, stack_pointer_rtx, gen_int_mode (num_bytes, HImode)));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline void msp430_fh_gen_mov_r2r(int dest_reg, int src_reg)
{
	rtx insn = emit_move_insn (gen_rtx_REG (HImode, dest_reg), gen_rtx_REG (HImode, src_reg));
	RTX_FRAME_RELATED_P(insn) = 1;
}

static inline char *msp430_format_sym_plus_off(const char *sym_name, int offset)
{
	char *pBuf;
	if (!offset)
		return sym_name;
	pBuf = ggc_alloc(strlen(sym_name) + 20);
	sprintf(pBuf, "(%s%s%d)", sym_name, (offset >= 0) ? "+" : "", offset);
	return pBuf;
}

static inline void msp430_fh_load_sp_with_sym_plus_off(const char *sym_name, int offset)
{
	rtx insn;

	insn = emit_move_insn (stack_pointer_rtx, gen_rtx_SYMBOL_REF(HImode, msp430_format_sym_plus_off(sym_name, offset)));
	RTX_FRAME_RELATED_P(insn) = 1;
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
	RTX_FRAME_RELATED_P(insn) = 1;
}

static void msp430_codegen_test(void)
{
	emit_insn(gen_nop());

	/*
	msp430_fh_emit_push_reg(7);
	msp430_fh_sub_sp_const(6);
	msp430_fh_gen_mov_r2r(7, 6);
	*/
	//msp430_fh_load_sp_with_sym_plus_off("__stack", -10);
	//msp430_fh_br_to_symbol_plus_offset("some_jump_target", -10);
	msp430_fh_bic_deref_sp(0x0f);
	/*
	msp430_fh_add_sp_const(6);
	msp430_fh_emit_pop_reg(7);
	*/

	emit_insn(gen_nop());
}

rtx msp430_expand_builtin(tree exp, rtx target ATTRIBUTE_UNUSED, 
					  rtx subtarget ATTRIBUTE_UNUSED, 
					  enum machine_mode mode ATTRIBUTE_UNUSED, 
					  int ignore ATTRIBUTE_UNUSED)
{
	rtx arg=0, retval = 0;
	rtx frame_offset_n;
	rtx insn=0;
	rtx symb, plus, con;
	char *pos;
	tree fndecl, argtree;
	int i,  code;
	rtx x = DECL_RTL (current_function_decl);
	const char *fnname = XSTR (XEXP (x, 0), 0);

#if GCC_VERSION_INT < 0x430
	fndecl = TREE_OPERAND (TREE_OPERAND (exp, 0), 0);
#else
	fndecl = TREE_OPERAND (CALL_EXPR_FN (exp), 0);
#endif

	argtree = TREE_OPERAND (exp, 1);
	i = DECL_FUNCTION_CODE (fndecl);

	if (i == msp430_codegen_test_entry_idx)
	{
		msp430_codegen_test();
		return NULL;
	}

	code = msp430builtins[i].md_code;

	pos = (char*)ggc_alloc(16+strlen(fnname)+1);
	snprintf(pos,16+strlen(fnname)+1,".L__FrameOffset_%s",fnname);

	symb = gen_rtx_REG(HImode,1);
	con = gen_rtx_SYMBOL_REF(HImode, pos);
	plus = gen_rtx_PLUS(HImode, symb, con);  
	frame_offset_n = gen_rtx_MEM(HImode, plus);

	if(code == CODE_FOR_bic_sr_irq || code == CODE_FOR_bis_sr_irq)
	{
#if GCC_VERSION_INT < 0x430
		arg = expand_expr (TREE_VALUE (argtree), NULL_RTX, VOIDmode, 0);
#else
		arg = expand_expr (CALL_EXPR_ARG(exp, 0), NULL_RTX, VOIDmode, 0);
#endif
	}

	if(code == CODE_FOR_bic_sr_irq)
		insn = gen_rtx_SET(HImode, frame_offset_n,
		gen_rtx_UNSPEC_VOLATILE(HImode,
		gen_rtvec(2,arg, GEN_INT(4100001)),41));
	else if(code == CODE_FOR_bis_sr_irq)
		insn = gen_rtx_SET(HImode, frame_offset_n,
		gen_rtx_UNSPEC_VOLATILE(HImode,
		gen_rtvec(2,arg, GEN_INT(4200002)),42));
	else if(code == CODE_FOR_get_frame_address)
	{
		retval = gen_reg_rtx(HImode);
		insn = gen_rtx_SET(HImode, retval, 
			gen_rtx_UNSPEC_VOLATILE(HImode,
			gen_rtvec(2,frame_offset_n, GEN_INT(4300003)),43));
	}
	else
		error("Unknown built-in function");

	if(insn)
		emit_insn(insn);
	else
		error("Unknown built-in function");

	return retval;
}