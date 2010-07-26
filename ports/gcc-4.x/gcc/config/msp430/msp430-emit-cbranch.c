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

struct rtx_def *msp430_compare_op0;
struct rtx_def *msp430_compare_op1;

void msp430_emit_cbranch (enum rtx_code code, rtx loc)
{
	rtx op0 = msp430_compare_op0;
	rtx op1 = msp430_compare_op1;
	rtx condition_rtx, loc_ref, branch;
	enum machine_mode mode;
	int mem_volatil=0;

	if (!msp430_compare_op0 && !msp430_compare_op1)
	{
		/* this is a branch upon previous insn issued */
		loc_ref = gen_rtx_LABEL_REF (VOIDmode, loc);
		condition_rtx = gen_rtx_fmt_ee (code, VOIDmode, cc0_rtx, const0_rtx);

		branch = gen_rtx_SET (VOIDmode,
			pc_rtx,
			gen_rtx_IF_THEN_ELSE (VOIDmode,
			condition_rtx,
			loc_ref, pc_rtx));
		emit_jump_insn (branch);
		return;
	}

	mode = GET_MODE (op0);
	if (mode != SImode && mode != HImode && mode != QImode)
		abort ();


	/* now convert codes */
	code = msp430_canonicalize_comparison (code, &op0, &op1);

	/* for HI and QI modes everything is simple.
	Also, if code is eq or ne in SI mode, no clobbers required. */

	if (mode == SImode && !(code == EQ || code == NE))
	{
		/* check if only high nibbles required */
		if (GET_CODE (op1) == CONST_INT
			&& INTVAL (op1) == 0 && (code == LT || code == GE))
		{
			mem_volatil = MEM_VOLATILE_P(op0);
			MEM_VOLATILE_P(op0) = 0;
			op0 = gen_highpart (HImode, op0);
			MEM_VOLATILE_P(op0) = mem_volatil;
			mode = HImode;
			PUT_MODE (op1, VOIDmode);	/* paranoia ? */
		}
		else if (GET_CODE (op1) == CONST_INT
			&& ((INTVAL (op1) + 1) & 0xffff) == 0
			&& (code == GT || code == GTU || code == LE || code == LEU))
		{
			/* check if this can be done simple. 
			we will not clobber const operand. */
			int x = INTVAL (op1);
			x++;
			x >>= 16;
			MEM_VOLATILE_P(op0) = 0;
			op0 = gen_highpart (HImode, op0);
			MEM_VOLATILE_P(op0) = mem_volatil;
			mode = HImode;
			op1 = GEN_INT (trunc_int_for_mode (x, HImode));

			if (code == GT)
				code = GE;
			else if (code == GTU)
				code = GEU;
			else if (code == LEU)
				code = LTU;
			else if (code == LE)
				code = LT;
		}
		else
		{
			rtvec vec;
			/* the redudant move will be deleted */
			op0 = copy_to_mode_reg (SImode, op0);
			condition_rtx = gen_rtx_fmt_ee (code, mode, op0, op1);
			loc_ref = gen_rtx_LABEL_REF (VOIDmode, loc);
			branch = gen_rtx_SET (VOIDmode, pc_rtx,
				gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
				loc_ref, pc_rtx));
			vec = gen_rtvec (2, branch, gen_rtx_CLOBBER (SImode, op0));
			emit_jump_insn (gen_rtx_PARALLEL (VOIDmode, vec));
			msp430_compare_op0 = 0;
			msp430_compare_op1 = 0;
			return;
		}
	}
	else if(mode == SImode && code == NE
		&& GET_CODE(op1)!= CONST_INT && op1 != const0_rtx)
	{
		rtx op0lo, op0hi, op1lo, op1hi;

		mem_volatil = MEM_VOLATILE_P(op0);
		op0lo = gen_lowpart(HImode, op0);
		op0hi = gen_highpart(HImode, op0);
		MEM_VOLATILE_P(op0) = mem_volatil;

		mem_volatil = MEM_VOLATILE_P(op1);
		op1lo = gen_lowpart(HImode, op1);
		op1hi = gen_highpart(HImode, op1);
		MEM_VOLATILE_P(op1) = mem_volatil;

		condition_rtx = gen_rtx_fmt_ee (NE,HImode,op0lo,op1lo);
		loc_ref = gen_rtx_LABEL_REF (VOIDmode, loc);
		branch = gen_rtx_SET (VOIDmode, pc_rtx,
			gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
			loc_ref, pc_rtx));
		emit_jump_insn (branch);
		condition_rtx = gen_rtx_fmt_ee (NE,HImode,op0hi,op1hi);
		branch = gen_rtx_SET (VOIDmode, pc_rtx,
			gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
			loc_ref, pc_rtx));
		emit_jump_insn (branch);
		msp430_compare_op0 = 0;
		msp430_compare_op1 = 0;
		return;
	}
	else if(mode == SImode && code == EQ && GET_CODE(op1)!= CONST_INT )
	{
		rtx tlabel = gen_label_rtx();
		rtx tloc_ref;
		rtx op0lo, op0hi, op1lo, op1hi;

		mem_volatil = MEM_VOLATILE_P(op0);
		op0lo = gen_lowpart(HImode, op0);
		op0hi = gen_highpart(HImode, op0);
		MEM_VOLATILE_P(op0) = mem_volatil;

		mem_volatil = MEM_VOLATILE_P(op1);
		op1lo = gen_lowpart(HImode, op1);
		op1hi = gen_highpart(HImode, op1);
		MEM_VOLATILE_P(op1) = mem_volatil;

		condition_rtx = gen_rtx_fmt_ee (NE,HImode,op0lo,op1lo);
		tloc_ref = gen_rtx_LABEL_REF (VOIDmode, tlabel);
		branch = gen_rtx_SET (VOIDmode, pc_rtx,
			gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
			tloc_ref, pc_rtx));
		emit_jump_insn (branch);

		condition_rtx = gen_rtx_fmt_ee (EQ,HImode,op0hi,op1hi);
		loc_ref = gen_rtx_LABEL_REF (VOIDmode, loc);
		branch = gen_rtx_SET (VOIDmode, pc_rtx,
			gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
			loc_ref, pc_rtx));
		emit_jump_insn (branch);
		emit_label(tlabel);
		msp430_compare_op0 = 0;
		msp430_compare_op1 = 0;
		return ;
	}

	condition_rtx = gen_rtx_fmt_ee (code, mode, op0, op1);
	loc_ref = gen_rtx_LABEL_REF (VOIDmode, loc);
	branch = gen_rtx_SET (VOIDmode, pc_rtx,
		gen_rtx_IF_THEN_ELSE (VOIDmode, condition_rtx,
		loc_ref, pc_rtx));

	emit_jump_insn (branch);

	msp430_compare_op0 = 0;
	msp430_compare_op1 = 0;
	return;
}

RTX_CODE msp430_canonicalize_comparison (RTX_CODE code, rtx *op0, rtx *op1)
{
	RTX_CODE rc = code;

	if ( CONSTANT_P(*op1) )
	{
		;				/* nothing to be done */
	}
	else
	{
		switch (code)
		{
		case GT:
		case LE:
		case GTU:
		case LEU:
			{
				rtx x;
				rc = swap_condition (code);
				x = *op0;
				*op0 = *op1;
				*op1 = x;
			}
			break;
		default:
			break;
		}
	}
	return rc;
}