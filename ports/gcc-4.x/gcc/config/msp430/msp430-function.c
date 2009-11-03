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
#include "msp430-predicates.inl"

void msp430_function_prologue (FILE * file, HOST_WIDE_INT);
void msp430_function_epilogue (FILE * file, HOST_WIDE_INT);

extern int msp430_commands_in_file;
extern int msp430_commands_in_prologues;
extern int msp430_commands_in_epilogues;

/* ret/reti issue indicator for _current_ function */
static int return_issued = 0;

/* Prologue/Epilogue size in words */
static int prologue_size;
static int epilogue_size;

/* Size of all jump tables in the current function, in words.  */
static int jump_tables_size;

/* This holds the last insn address.  */
static int last_insn_address = 0;

static int msp430_func_num_saved_regs (void);

/* actual frame offset */
static int msp430_current_frame_offset = 0;

/* registers used for incoming funct arguments */
static char arg_register_used[16];

#define FIRST_CUM_REG 16
static CUMULATIVE_ARGS *cum_incoming = 0;

static int msp430_num_arg_regs (enum machine_mode mode, tree type);
static int msp430_saved_regs_frame (void);


/* Output function prologue */
void msp430_function_prologue (FILE *file, HOST_WIDE_INT size)
{
	int i;
	int interrupt_func_p = interrupt_function_p (current_function_decl);
	int signal_func_p = signal_function_p (current_function_decl);
	int leaf_func_p = leaf_function_p ();
	int main_p = MAIN_NAME_P (DECL_NAME (current_function_decl));
	int stack_reserve = 0;
	tree ss = 0;
	rtx x = DECL_RTL (current_function_decl);
	const char *fnname = XSTR (XEXP (x, 0), 0);
	int offset;
	int cfp = msp430_critical_function_p (current_function_decl);
	int tfp = msp430_task_function_p (current_function_decl);
	int ree = msp430_reentrant_function_p (current_function_decl);
	int save_prologue_p =
		msp430_save_prologue_function_p (current_function_decl);
	int num_saved_regs;

	return_issued = 0;
	last_insn_address = 0;
	jump_tables_size = 0;
	prologue_size = 0;

	cfun->machine->is_naked = msp430_naked_function_p (current_function_decl);
	cfun->machine->is_interrupt = interrupt_function_p (current_function_decl);
	cfun->machine->is_OS_task = msp430_task_function_p (current_function_decl);
	
	cfun->machine->is_noint_hwmul = noint_hwmul_function_p (current_function_decl);
	cfun->machine->is_critical = msp430_critical_function_p(current_function_decl);
	cfun->machine->is_reenterant = msp430_reentrant_function_p(current_function_decl);
	cfun->machine->is_wakeup = wakeup_function_p (current_function_decl);


	/* check attributes compatibility */

	if ((cfp && ree) || (ree && interrupt_func_p))
	{
		warning (OPT_Wattributes, "attribute 'reentrant' ignored");
		ree = 0;
	}

	if (cfp && interrupt_func_p)
	{
		warning (OPT_Wattributes, "attribute 'critical' ignored");
		cfp = 0;
	}

	if (signal_func_p && !interrupt_func_p)
	{
		warning (OPT_Wattributes, "attribute 'signal' has no meaning on MSP430. Use 'interrupt' instead.");
		signal_func_p = 0;
	}

	/* naked function discards everything */
	if (msp430_naked_function_p (current_function_decl))
	{
		fprintf (file, "\t/* prologue: naked */\n");
		fprintf (file, ".L__FrameSize_%s=0x%x\n", fnname, (unsigned)size);
		return;
	}
	ss = lookup_attribute ("reserve", DECL_ATTRIBUTES (current_function_decl));
	if (ss)
	{
		ss = TREE_VALUE (ss);
		if (ss)
		{
			ss = TREE_VALUE (ss);
			if (ss)
				stack_reserve = TREE_INT_CST_LOW (ss);
			stack_reserve++;
			stack_reserve &= ~1;
		}
	}

	fprintf (file, "\t/* prologue: frame size = %d */\n", (unsigned)size);
	fprintf (file, ".L__FrameSize_%s=0x%x\n", fnname, (unsigned)size);


	offset = initial_elimination_offset (0, 0) - 2;

	msp430_current_frame_offset = offset;

	fprintf (file, ".L__FrameOffset_%s=0x%x\n", fnname, (unsigned)offset);

	if (signal_func_p && interrupt_func_p)
	{
		prologue_size += 1;
		fprintf (file, "\teint\t; enable nested interrupt\n");
	}

	if (main_p)
	{
		if (TARGET_NO_STACK_INIT)
		{
			if (size || stack_reserve)
				fprintf (file, "\tsub\t#%d, r1\t", size + stack_reserve);
			if (frame_pointer_needed)
			{
				fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM);
				prologue_size += 1;
			}

			if (size)
				prologue_size += 2;
			if (size == 1 || size == 2 || size == 4 || size == 8)
				prologue_size--;
		}
		else
		{
			fprintf (file, "\tmov\t#(%s-%d), r1\n", msp430_init_stack,
				size + stack_reserve);

			if (frame_pointer_needed)
			{
				fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM);
				prologue_size += 1;
			}
			prologue_size += 2;
		}
	}
	else	/* not a main() function */
	{
		/* Here, we've got a chance to jump to prologue saver */
		num_saved_regs = msp430_func_num_saved_regs ();

		if ((TARGET_SAVE_PROLOGUE || save_prologue_p)
			&& !interrupt_func_p && !arg_register_used[12] && num_saved_regs > 4)
		{
			fprintf (file, "\tsub\t#16, r1\n");
			fprintf (file, "\tmov\tr0, r12\n");
			fprintf (file, "\tadd\t#8, r12\n");
			fprintf (file, "\tbr\t#__prologue_saver+%d\n",
				(8 - num_saved_regs) * 4);

			if (cfp && 8 - num_saved_regs)
			{
				int n = 16 - num_saved_regs * 2;
				fprintf (file, "\tadd\t#%d, r1\n", n);
				if (n != 0 && n != 1 && n != 2 && n != 4 && n != 8)
					prologue_size += 1;
			}
			else
				size -= 16 - num_saved_regs * 2;

			prologue_size += 7;
		}
		else if(!tfp)
		{
			for (i = 15; i >= 4; i--)
			{
				if ((df_regs_ever_live_p(i) && (!call_used_regs[i] || interrupt_func_p)) || 
					(!leaf_func_p && (call_used_regs[i] && (interrupt_func_p))))
				{
					fprintf (file, "\tpush\tr%d\n", i);
					prologue_size += 1;
				}
			}
		}

		if (!interrupt_func_p && cfp)
		{
			prologue_size += 3;
			fprintf (file, "\tpush\tr2\n");
			fprintf (file, "\tdint\n");
			if (!size)
				fprintf (file, "\tnop\n");
		}

		if (size)
		{
			/* The next is a hack... I do not undestand why, but if there
			ARG_POINTER_REGNUM and FRAME/STACK are different, 
			the compiler fails to compute corresponding
			displacement */
			if (!optimize && !optimize_size
				&& df_regs_ever_live_p(ARG_POINTER_REGNUM))
			{
				int o = initial_elimination_offset (0, 0) - size;
				fprintf (file, "\tmov\tr1, r%d\n", ARG_POINTER_REGNUM);
				fprintf (file, "\tadd\t#%d, r%d\n", o, ARG_POINTER_REGNUM);
				prologue_size += 2;
				if (o != 0 && o != 1 && o != 2 && o != 4 && o != 8)
					prologue_size += 1;
			}

			/* adjust frame ptr... */
			if (size > 0)
				fprintf (file, "\tsub\t#%d, r1\t;	%d, fpn %d\n", (size + 1) & ~1,
				size, frame_pointer_needed);
			else
			{
				size = -size;
				fprintf (file, "\tadd\t#%d, r1\t;    %d, fpn %d\n",
					(size + 1) & ~1, size, frame_pointer_needed);
			}

			if (frame_pointer_needed)
			{
				fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM);
				prologue_size += 1;
			}

			if (size == 1 || size == 2 || size == 4 || size == 8)
				prologue_size += 1;
			else
				prologue_size += 2;
		}

		/* disable interrupt for reentrant function */
		if (!interrupt_func_p && ree)
		{
			prologue_size += 1;
			fprintf (file, "\tdint\n");
		}
	}

	fprintf (file, "\t/* prologue end (size=%d) */\n\n", prologue_size);
}


/* Output function epilogue */

void msp430_function_epilogue (FILE *file, HOST_WIDE_INT size)
{
	int i;
	int interrupt_func_p = cfun->machine->is_interrupt;
	int leaf_func_p = leaf_function_p ();
	int main_p = MAIN_NAME_P (DECL_NAME (current_function_decl));
	int wakeup_func_p = cfun->machine->is_wakeup;
	int cfp = cfun->machine->is_critical;
	int ree = cfun->machine->is_reenterant;
	int save_prologue_p = msp430_save_prologue_function_p (current_function_decl);
	int still_return = 1;
	int function_size;


	last_insn_address = 0;
	jump_tables_size = 0;
	epilogue_size = 0;
	function_size = (INSN_ADDRESSES (INSN_UID (get_last_insn ()))
		- INSN_ADDRESSES (INSN_UID (get_insns ())));

	if (cfun->machine->is_OS_task)
	{
		fprintf (file, "\n\t/* epilogue: empty, task functions never return */\n");
		return;
	}

	if (cfun->machine->is_naked)
	{
		fprintf (file, "\n\t/* epilogue: naked */\n");
		return;
	}

	if (msp430_empty_epilogue ())
	{
		if (!return_issued)
		{
			fprintf (file, "\t%s\n", msp430_emit_return (NULL, NULL, NULL));
			epilogue_size++;
		}
		fprintf (file, "\n\t/* epilogue: not required */\n");
		goto done_epilogue;
	}

	if ((cfp || interrupt_func_p) && ree)
		ree = 0;
	if (cfp && interrupt_func_p)
		cfp = 0;

	fprintf (file, "\n\t/* epilogue: frame size=%d */\n", size);

	if (main_p)
	{
		if (size)
			fprintf (file, "\tadd\t#%d, r1\n", (size + 1) & ~1);
		fprintf (file, "\tbr\t#%s\n", msp430_endup);
		epilogue_size += 4;
		if (size == 1 || size == 2 || size == 4 || size == 8)
			epilogue_size--;
	}
	else
	{
		if (ree)
		{
			fprintf (file, "\teint\n");
			epilogue_size += 1;
		}

		if (size)
		{
			fprintf (file, "\tadd\t#%d, r1\n", (size + 1) & ~1);
			if (size == 1 || size == 2 || size == 4 || size == 8)
				epilogue_size += 1;
			else
				epilogue_size += 2;
		}

		if (!interrupt_func_p && cfp)
		{
			epilogue_size += 1;
			if (msp430_saved_regs_frame () == 2)
			{
				fprintf (file, "\treti\n");
				still_return = 0;
			}
			else
				fprintf (file, "\tpop\tr2\n");
		}

		if ((TARGET_SAVE_PROLOGUE || save_prologue_p)
			&& !interrupt_func_p && msp430_func_num_saved_regs () > 2)
		{
			fprintf (file, "\tbr\t#__epilogue_restorer+%d\n",
				(8 - msp430_func_num_saved_regs ()) * 2);
			epilogue_size += 2;
		}
		else if ((TARGET_SAVE_PROLOGUE || save_prologue_p) && interrupt_func_p)
		{
			fprintf (file, "\tbr\t#__epilogue_restorer_intr+%d\n",
				(12 - msp430_func_num_saved_regs ()) * 2);
		}
		else
		{
			for (i = 4; i < 16; i++)
			{
				if ((df_regs_ever_live_p(i)
					&& (!call_used_regs[i]
				|| interrupt_func_p))
					|| (!leaf_func_p && (call_used_regs[i] && interrupt_func_p)))
				{
					fprintf (file, "\tpop\tr%d\n", i);
					epilogue_size += 1;
				}
			}

			if (interrupt_func_p)
			{
				if (wakeup_func_p)
				{
					fprintf (file, "\tbic\t#0xf0,0(r1)\n");
					epilogue_size += 3;
				}

				fprintf (file, "\treti\n");
				epilogue_size += 1;
			}
			else
			{
				if (still_return)
					fprintf (file, "\tret\n");
				epilogue_size += 1;
			}
		}
	}

	fprintf (file, "\t/* epilogue end (size=%d) */\n", epilogue_size);
done_epilogue:
	fprintf (file, "\t/* function %s size %d (%d) */\n", current_function_name,
		prologue_size + function_size + epilogue_size, function_size);

	msp430_commands_in_file += prologue_size + function_size + epilogue_size;
	msp430_commands_in_prologues += prologue_size;
	msp430_commands_in_epilogues += epilogue_size;
}

/* Returns a number of pushed registers */
static int msp430_func_num_saved_regs (void)
{
	int i;
	int saves = 0;
	int interrupt_func_p = interrupt_function_p (current_function_decl);
	int leaf_func_p = leaf_function_p ();

	for (i = 4; i < 16; i++)
	{
		if ((df_regs_ever_live_p(i)
			&& (!call_used_regs[i]
		|| interrupt_func_p))
			|| (!leaf_func_p && (call_used_regs[i] && interrupt_func_p)))
		{
			saves += 1;
		}
	}

	return saves;
}

const char *msp430_emit_return (rtx insn ATTRIBUTE_UNUSED, rtx operands[] ATTRIBUTE_UNUSED, int *len ATTRIBUTE_UNUSED)
{
	return_issued = 1;
	if (msp430_critical_function_p (current_function_decl) || interrupt_function_p(current_function_decl))
		return "ret";

	return "reti";
}

void msp430_output_addr_vec_elt (FILE *stream, int value)
{
	fprintf (stream, "\t.word	.L%d\n", value);
	jump_tables_size++;
}

/* Output all insn addresses and their sizes into the assembly language
output file.  This is helpful for debugging whether the length attributes
in the md file are correct.
Output insn cost for next insn.  */

void final_prescan_insn (rtx insn, rtx *operand ATTRIBUTE_UNUSED, int num_operands ATTRIBUTE_UNUSED)
{
	int uid = INSN_UID (insn);

	if (TARGET_ALL_DEBUG)
	{
		fprintf (asm_out_file, "/*DEBUG: 0x%x\t\t%d\t%d */\n",
			INSN_ADDRESSES (uid),
			INSN_ADDRESSES (uid) - last_insn_address,
			rtx_cost (PATTERN (insn), INSN, !optimize_size));
	}
	last_insn_address = INSN_ADDRESSES (uid);
}

/* Controls whether a function argument is passed
in a register, and which register. */
rtx function_arg (CUMULATIVE_ARGS *cum, enum machine_mode mode, tree type, int named ATTRIBUTE_UNUSED)
{
	int regs = msp430_num_arg_regs (mode, type);

	if (cum->nregs && regs <= cum->nregs)
	{
		int regnum = cum->regno - regs;

		if (cum == cum_incoming)
		{
			arg_register_used[regnum] = 1;
			if (regs >= 2)
				arg_register_used[regnum + 1] = 1;
			if (regs >= 3)
				arg_register_used[regnum + 2] = 1;
			if (regs >= 4)
				arg_register_used[regnum + 3] = 1;
		}

		return gen_rtx_REG (mode, regnum);
	}
	return NULL_RTX;
}

/* the same in scope of the cum.args., buf usefull for a
function call */
void init_cumulative_incoming_args (CUMULATIVE_ARGS *cum, tree fntype, rtx libname)
{
	int i;
	cum->nregs = 4;
	cum->regno = FIRST_CUM_REG;
	if (!libname)
	{
		int stdarg = (TYPE_ARG_TYPES (fntype) != 0
			&& (TREE_VALUE (tree_last (TYPE_ARG_TYPES (fntype)))
			!= void_type_node));
		if (stdarg)
			cum->nregs = 0;
	}

	for (i = 0; i < 16; i++)
		arg_register_used[i] = 0;

	cum_incoming = cum;
}

/* Initializing the variable cum for the state at the beginning
of the argument list.  */
void init_cumulative_args (CUMULATIVE_ARGS *cum, tree fntype, rtx libname, int indirect ATTRIBUTE_UNUSED)
{
	cum->nregs = 4;
	cum->regno = FIRST_CUM_REG;
	if (!libname)
	{
		int stdarg = (TYPE_ARG_TYPES (fntype) != 0
			&& (TREE_VALUE (tree_last (TYPE_ARG_TYPES (fntype)))
			!= void_type_node));
		if (stdarg)
			cum->nregs = 0;
	}
}


/* Update the summarizer variable CUM to advance past an argument
in the argument list.  */
void function_arg_advance (CUMULATIVE_ARGS *cum, enum machine_mode mode, tree type, int named ATTRIBUTE_UNUSED)
{
	int regs = msp430_num_arg_regs (mode, type);

	cum->nregs -= regs;
	cum->regno -= regs;

	if (cum->nregs <= 0)
	{
		cum->nregs = 0;
		cum->regno = FIRST_CUM_REG;
	}
}

/* Returns the number of registers to allocate for a function argument.  */
static int msp430_num_arg_regs (enum machine_mode mode, tree type)
{
	int size;

	if (mode == BLKmode)
		size = int_size_in_bytes (type);
	else
		size = GET_MODE_SIZE (mode);

	if (size < 2)
		size = 2;

	/* we do not care if argument is passed in odd register
	so, do not align the size ...
	BUT!!! even char argument passed in 16 bit register
	so, align the size */
	return ((size + 1) & ~1) >> 1;
}

static int msp430_saved_regs_frame (void)
{
	int interrupt_func_p = interrupt_function_p (current_function_decl);
	int cfp = msp430_critical_function_p (current_function_decl);
	int leaf_func_p = leaf_function_p ();
	int offset = interrupt_func_p ? 0 : (cfp ? 2 : 0);
	int reg;

	for (reg = 4; reg < 16; ++reg)
	{
		if ((!leaf_func_p && call_used_regs[reg] && (interrupt_func_p))
			|| (df_regs_ever_live_p(reg)
			&& (!call_used_regs[reg] || interrupt_func_p)))
		{
			offset += 2;
		}
	}

	return offset;
}

int msp430_empty_epilogue (void)
{
	int cfp = msp430_critical_function_p (current_function_decl);
	int ree = msp430_reentrant_function_p (current_function_decl);
	int nfp = msp430_naked_function_p (current_function_decl);
	int ifp = interrupt_function_p (current_function_decl);
	int wup = wakeup_function_p (current_function_decl);
	int size = msp430_saved_regs_frame ();
	int fs = get_frame_size ();

	if (cfp && ree)
		ree = 0;

	/* the following combination of attributes forces to issue
	some commands in function epilogue */
	if (ree
		|| nfp || fs || wup || MAIN_NAME_P (DECL_NAME (current_function_decl)))
		return 0;

	size += fs;

	/* <= 2 necessary for first call */
	if (size <= 2 && cfp)
		return 2;
	if (size == 0 && !cfp && !ifp)
		return 1;
	if (size == 0 && ifp)
		return 2;

	return 0;
}