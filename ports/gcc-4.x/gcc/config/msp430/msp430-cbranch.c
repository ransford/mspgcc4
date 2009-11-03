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

static const char *msp430_emit_blt0si (rtx operands[], int len);
static const char *msp430_emit_beq (rtx operands[], int len);
static const char *msp430_emit_bne (rtx operands[], int len);
static const char *msp430_emit_bgt (rtx operands[], int len);
static const char *msp430_emit_bgtu (rtx operands[], int len);
static const char *msp430_emit_blt (rtx operands[], int len);
static const char *msp430_emit_bltnoovfl (rtx operands[], int len);
static const char *msp430_emit_bltu (rtx operands[], int len);
static const char *msp430_emit_bge (rtx operands[], int len);
static const char *msp430_emit_bgeu (rtx operands[], int len);
static const char *msp430_emit_ble (rtx operands[], int len);
static const char *msp430_emit_bleu (rtx operands[], int len);

static int msp430_cc_source (rtx, enum rtx_code, rtx, rtx);

#define OUT_INSN(x,p,o) \
	do {                            \
	if(!x) output_asm_insn (p,o);   \
	} while(0)


const char *msp430_cbranch (rtx insn, rtx operands[], int *len, int is_cc0_branch)
{
	rtx ops[3];
	enum rtx_code code;
	rtx locs[3];
	int dummy = 0;
	enum machine_mode mode;
	int quater = 0;
	rtx loc = operands[0];
	int distance = msp430_jump_dist (loc, insn);
	int predist = 0;
	int nooverflow = 0;

#define ECOND(f,x) do{if(!len)msp430_emit_b##f(locs,predist + x);dummy+=(predist + x);}while(0)
	locs[0] = operands[0];
	ops[0] = operands[2];
	ops[1] = operands[3];

	if (ops[1] && ops[0])
	{
		mode = GET_MODE (operands[2]);
		code = GET_CODE (operands[1]);
		quater = (mode == QImode);
	}
	else
	{
		mode = HImode;
		code = GET_CODE (operands[1]);
	}

	/* here check wiered conditions */
	if (ops[1] && GET_CODE (ops[1]) == CONST_INT
		&& (code == GT || code == LE || code == GTU || code == LEU))
	{
		int x = INTVAL (ops[1]);
		switch (code)
		{
		case GT:
			ops[1] = GEN_INT (x + 1);
			code = GE;
			break;
		case LE:
			ops[1] = GEN_INT (x + 1);
			code = LT;
			break;
		case GTU:
			ops[1] = GEN_INT (x + 1);
			code = GEU;
			break;
		case LEU:
			ops[1] = GEN_INT (x + 1);
			code = LTU;
			break;
		default:
			break;
		}
	}
	else if (ops[1] && CONSTANT_P (ops[1]) && GET_MODE(ops[1]) == HImode
		&& (code == GT || code == LE || code == GTU || code == LEU))
	{
		/* Handle pointers here */
		ops[1] = gen_rtx_CONST(HImode,gen_rtx_PLUS(HImode,ops[1],GEN_INT(1)));

		switch (code)
		{
		case GT:
			code = GE;
			break;
		case LE:
			code = LT;
			break;
		case GTU:
			code = GEU;
			break;
		case LEU:
			code = LTU;
			break;
		default:
			break;
		}
	}

	if (!is_cc0_branch && ops[0] != cc0_rtx && ops[1] && ops[0])
	{
		if (code == NE || code == EQ)
		{
			/* check if op0 is zero shited - win 1 byte */
			if (indexed_location (ops[0]) && !CONSTANT_P (ops[1]))
			{
				rtx x = ops[0];
				ops[0] = ops[1];
				ops[1] = x;
			}
		}

		/* check if compares were not issued */
		if ((mode == QImode || mode == HImode)
			&& msp430_cc_source (insn, code, ops[0], ops[1]))
		{
			/* check if overflow can be usefull here. */
			if( ops[1] == const0_rtx 
				|| (GET_CODE(ops[1]) == CONST_INT
				&& INTVAL(ops[1]) == 0 ))
			{
				if(code == LT || code == GE)
					nooverflow = 1;
			}
		}
		else if (mode == QImode || mode == HImode)
		{
			/* check if previous insns did not set CC correctly */
			if (quater)
				OUT_INSN (len, "cmp.b\t%1, %0", ops);
			else
				OUT_INSN (len, "cmp\t%1, %0", ops);
			dummy += 3;
			if (REG_P (ops[0]))
				dummy--;
			if (REG_P (ops[1]))
				dummy--;
			if (indexed_location (ops[1]))
				dummy--;
			if (GET_CODE (ops[1]) == CONST_INT)
			{
				int x = INTVAL (ops[1]) & 0xffff;
				if (x == 0 || x == -1 || x == 1 || x == 2 || x == 4 || x == 8)
					dummy--;
			}
		}

		/* adjust distance */
		distance -= dummy;

		if (mode == SImode && (code == EQ || code == NE))
		{
			/* compare against zero and can we clobber source register ? */
			if (((GET_CODE (ops[1]) == CONST_INT
				&& INTVAL (ops[1]) == 0)
				|| ops[1] == const0_rtx)
				&& REG_P (ops[0]) && dead_or_set_p (insn, ops[0]))
			{
				OUT_INSN (len, "bis\t%A0, %B0", ops);
				OUT_INSN (len, "tst\t%B0", ops);
				dummy += 2;
			}
			else
			{
				/* cannot clobber or something... */
				OUT_INSN (len, "cmp\t%A1, %A0", ops);
				dummy += 3;
				if (REG_P (ops[0]))
					dummy--;
				if (REG_P (ops[1]))
					dummy--;
				if (indexed_location (ops[1]))
					dummy--;
				if (GET_CODE (ops[1]) == CONST_INT)
				{
					int x = INTVAL (ops[1]) & 0xffff;
					if (x == 0 || x == 1 || x == -1 || x == 2 || x == 4
						|| x == 8)
						dummy--;
				}
				distance -= dummy;
				if (distance > 500 || distance < -500)
					predist = 3;
				else
					predist = 1;

				if (code == EQ)
				{
					OUT_INSN (len, "jne\t.LcmpSIe%=", ops);
					OUT_INSN (len, "cmp\t%B1, %B0", ops);
					dummy++;
				}
				else
				{
					ECOND (ne, 0);
					OUT_INSN (len, "cmp\t%B1, %B0", ops);
				}

				dummy += 3;
				if (REG_P (ops[0]))
					dummy--;
				if (REG_P (ops[1]))
					dummy--;
				if (GET_CODE (ops[1]) == CONST_INT)
				{
					int x = (INTVAL (ops[1]) >> 16) & 0xffff;
					if (x == 0 || x == 0xffff || x == 1 || x == 2 || x == 4
						|| x == 8)
						dummy--;
				}
			}
		}
		else if (mode == SImode)
		{
			int dl = 0;
			rtx oops[3];
			oops[0] = ops[0];
			oops[1] = ops[0];
			oops[2] = ops[1];

			if (len)
				msp430_subsi_code (insn, oops, &dl);
			else
				msp430_subsi_code (insn, oops, NULL);

			if (len)
			{
				/* not handeled by adjust_insn_len() */
				dummy += dl;
				if (GET_CODE (ops[1]) == CONST_INT)
				{
					int x = (INTVAL (ops[1]) >> 16) & 0xffff;
					if (x == 0 || x == 1 || x == -1 || x == 2 || x == 4
						|| x == 8)
						dummy--;
					x = (INTVAL (ops[1]) >> 0) & 0xffff;
					if (x == 0 || x == 1 || x == -1 || x == 2 || x == 4
						|| x == 8)
						dummy--;
				}
			}
		}
	}

	distance -= dummy;

	if (distance > 500 || distance < -500)
		predist = 3;
	else
		predist = 1;

	/* out assembler commands if required */
	switch (code)
	{
	case EQ:
		ECOND (eq, 0);
		if (mode == SImode)
		{
			OUT_INSN (len, ".LcmpSIe%=:", operands);
		}
		break;
	case NE:
		ECOND (ne, 0);
		break;
	case LT:
		if(nooverflow)
			ECOND (ltnoovfl,0);
		else
			ECOND (lt, 0);
		break;
	case GE:
		if(nooverflow)
		{
			if(len) *len += 2;
			if(mode == QImode)
				OUT_INSN (len, "bit.b\t#0x80, %0",ops);
			else
				OUT_INSN (len, "bit\t#0x8000, %0",ops);
		}
		ECOND (ge, 0);
		break;
	case LTU:
		ECOND (ltu, 0);
		break;
	case GEU:
		ECOND (geu, 0);
		break;
		/* hopfully the following will not occure */
	case LEU:
		ECOND (leu, 1);
		break;
	case GT:
		ECOND (gt, 1);
		break;
	case GTU:
		ECOND (gtu, 1);
		break;
	case LE:
		ECOND (le, 1);
		break;

	default:
		break;
	}

	if (len)
		*len = dummy;
	return "";
}


static const char *msp430_emit_blt0si (rtx operands[], int len)
{
	output_asm_insn ("tst\t%B2", operands);
	switch (len)
	{
	case 2:
		output_asm_insn ("jl\t%0", operands);
		break;
	case 4:
		output_asm_insn ("jge\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_beq (rtx operands[], int len)
{

	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jeq\t%0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jne\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bne (rtx operands[], int len)
{

	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jne\t%0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jeq\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bgt (rtx operands[], int len)
{
	switch (len)
	{
	case 2:
		output_asm_insn ("jeq\t+2", operands);
		output_asm_insn ("jge\t%0", operands);

		break;
	case 4:
		output_asm_insn ("jeq\t+6", operands);
		output_asm_insn ("jl\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bgtu (rtx operands[], int len)
{
	switch (len)
	{
	case 2:
		output_asm_insn ("jeq\t+2", operands);
		output_asm_insn ("jhs\t%0", operands);

		break;
	case 4:
		output_asm_insn ("jeq\t+6", operands);
		output_asm_insn ("jlo\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_blt (rtx operands[], int len)
{
	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jl\t%0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jge\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}


static const char *msp430_emit_bltnoovfl (rtx operands[], int len)
{
	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jn\t%0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jn\t+2",operands);
		output_asm_insn ("jmp\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bltu (rtx operands[], int len)
{
	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jlo\t%0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jhs\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bge (rtx operands[], int len)
{
	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jge\t%l0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jl\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bgeu (rtx operands[], int len)
{
	switch (len)
	{
	case 1:
	case 2:
		output_asm_insn ("jhs\t%l0", operands);
		break;
	case 3:
	case 4:
		output_asm_insn ("jlo\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_ble (rtx operands[], int len)
{
	switch (len)
	{
	case 2:
		output_asm_insn ("jeq\t%0", operands);
		output_asm_insn ("jl\t%0", operands);
		break;
	case 4:
		output_asm_insn ("jeq\t+2", operands);
		output_asm_insn ("jge\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

static const char *msp430_emit_bleu (rtx operands[], int len)
{
	switch (len)
	{
	case 2:
		output_asm_insn ("jeq\t%0", operands);
		output_asm_insn ("jlo\t%0", operands);
		break;
	case 4:
		output_asm_insn ("jeq\t+2", operands);
		output_asm_insn ("jhs\t+4", operands);
		output_asm_insn ("br\t#%0", operands);
		break;
	default:
		return "bug!!!";
	}

	return "";
}

/*  x - dst
y - src */
static int msp430_cc_source (rtx insn, enum rtx_code code, rtx x, rtx y)
{
	rtx prev = insn;
	enum attr_cc cc;
	rtx set;
	rtx src, dst;
	rtx x1 = 0;

	if(GET_CODE(x) == MEM)
	{
		x1 = XEXP(x,0);
		if(GET_CODE(x1) == PLUS)
		{
			x1 = XEXP(x1,0);
		}

		if(!REG_P(x1)) x1 = 0;
	}

	while (0 != (prev = PREV_INSN (prev)))
	{
		if (GET_CODE (prev) == CODE_LABEL
			|| GET_CODE (prev) == BARRIER || GET_CODE (prev) == CALL_INSN)
			return 0;

		if (GET_CODE (prev) == INSN)
		{
			set = single_set (prev);

			if(!set)
				return 0;

			cc = get_attr_cc (prev);

			if (cc == CC_NONE)	/* does not change CC */
			{
				/*The one spot by Nick C. */
				dst = SET_DEST (set);
				if((dst && rtx_equal_p (x, dst)) ||
					(x1 && dst && rtx_equal_p (x1, dst)))
					return 0;
				else
					continue;
			}

			if (cc == CC_CLOBBER)	/* clobber */
				return 0;

			if (cc == CC_OPER)	/* post-incremental stuff */
			{
				src = SET_SRC (set);
				if (GET_CODE (set) == IOR)	/* does not change CC */
				{
					dst = SET_DEST (set);
					if(dst && rtx_equal_p (x, dst))
						return 0;
					else
						continue;
				}
			}

			/* all other attributes are bit messy.
			So, we'll record destination and check if 
			this matches 'x' and compare is against zero */
			dst = SET_DEST (set);
			if (rtx_equal_p (x, dst) && rtx_equal_p (y, const0_rtx))
				return 1;
			else
				return 0;
		}
		else if (GET_CODE (prev) == JUMP_INSN)
		{
			/* if 2 consequent jump insns were issued, this means
			that operands (more likely src) are different.
			however, some jumps optimization can equalize these operands
			and everything will be bad. Therefore, assume that
			any jump insn clobbers condition codes.*/
			return 0;
		}
	}
	return 0;
}
