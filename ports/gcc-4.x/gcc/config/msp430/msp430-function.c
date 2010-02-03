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

#if GCC_VERSION_INT < 0x430
static inline int df_regs_ever_live_p(int reg)
{
	return regs_ever_live[reg];
}
#endif

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

void msp430_function_end_prologue (FILE * file);
void msp430_function_begin_epilogue (FILE * file);

int msp430_epilogue_uses (int regno ATTRIBUTE_UNUSED)
{
	if (reload_completed 
		&& cfun->machine
		&& (cfun->machine->is_interrupt || cfun->machine->is_signal))
		return 1;
	return 0;
}


void msp430_function_end_prologue (FILE * file)
{
	HOST_WIDE_INT frameSize = get_frame_size();
	rtx functionExp = DECL_RTL (current_function_decl);
	const char *functionName = XSTR (XEXP (functionExp, 0), 0);

	if (cfun->machine->is_naked)
	{
		fprintf (file, "\t/* prologue: naked */\n");
		fprintf (file, ".L__FrameSize_%s=0x%x\n", functionName, (unsigned)frameSize);
	}
	else
	{
		int offset = initial_elimination_offset (ARG_POINTER_REGNUM, STACK_POINTER_REGNUM) - 2;
		fprintf (file, "\t/* prologue ends here (frame size = %d) */\n", (unsigned)frameSize);
		fprintf (file, ".L__FrameSize_%s=0x%x\n", functionName, (unsigned)frameSize);
		fprintf (file, ".L__FrameOffset_%s=0x%x\n", functionName, (unsigned)offset);
	}
}

void msp430_function_begin_epilogue (FILE * file)
{
	if (cfun->machine->is_OS_task)
		fprintf (file, "\n\t/* epilogue: empty, task functions never return */\n");
	else if (cfun->machine->is_naked)
		fprintf (file, "\n\t/* epilogue: naked */\n");
	else if (msp430_empty_epilogue ())
		fprintf (file, "\n\t/* epilogue: not required */\n");
	else
		fprintf (file, "\n\t/* epilogue: frame size = %d */\n", (unsigned)get_frame_size());
}

static int msp430_get_stack_reserve(void);

static int msp430_get_stack_reserve(void)
{
	int stack_reserve = 0;
	tree ss = lookup_attribute ("reserve", DECL_ATTRIBUTES (current_function_decl));
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
	return stack_reserve;
}

#include "framehelpers.inl"

void expand_prologue (void)
{
	int i;
	int main_p = MAIN_NAME_P (DECL_NAME (current_function_decl));
	int stack_reserve = 0;
	int offset;
	int save_prologue_p = msp430_save_prologue_function_p (current_function_decl);
	int num_saved_regs;
	HOST_WIDE_INT size = get_frame_size();

	rtx insn;	/* Last generated instruction */

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
	cfun->machine->is_signal = signal_function_p (current_function_decl);


	/* check attributes compatibility */

	if ((cfun->machine->is_critical && cfun->machine->is_reenterant) || (cfun->machine->is_reenterant && cfun->machine->is_interrupt))
	{
		warning (OPT_Wattributes, "attribute 'reentrant' ignored");
		cfun->machine->is_reenterant = 0;
	}

	if (cfun->machine->is_critical && cfun->machine->is_interrupt)
	{
		warning (OPT_Wattributes, "attribute 'critical' ignored");
		cfun->machine->is_critical = 0;
	}

	if (cfun->machine->is_signal && !cfun->machine->is_interrupt)
	{
		warning (OPT_Wattributes, "attribute 'signal' has no meaning on MSP430 without 'interrupt' attribute.");
		cfun->machine->is_signal = 0;
	}

	/* naked function discards everything */
	if (cfun->machine->is_naked)
		return;

	stack_reserve = msp430_get_stack_reserve();
	offset = initial_elimination_offset (ARG_POINTER_REGNUM, STACK_POINTER_REGNUM) - 2;

	msp430_current_frame_offset = offset;

	if (cfun->machine->is_signal && cfun->machine->is_interrupt)
	{
		prologue_size += 1;

		insn = emit_insn (gen_enable_interrupt());
		/* fprintf (file, "\teint\t; enable nested interrupt\n"); */
	}

	if (main_p)
	{
		if (TARGET_NO_STACK_INIT)
		{
			if (size || stack_reserve)
			{
				/* fprintf (file, "\tsub\t#%d, r1\t", size + stack_reserve); */

				msp430_fh_sub_sp_const(size + stack_reserve);
			}
			
			if (frame_pointer_needed)
			{
				/* fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM); */
				insn = emit_move_insn (frame_pointer_rtx, stack_pointer_rtx);
				RTX_FRAME_RELATED_P (insn) = 1;

				prologue_size += 1;
			}
			
			if (size)
				prologue_size += 2;
			if (size == 1 || size == 2 || size == 4 || size == 8)
				prologue_size--;
		}
		else
		{
			/*fprintf (file, "\tmov\t#(%s-%d), r1\n", msp430_init_stack, size + stack_reserve);*/
			msp430_fh_load_sp_with_sym_plus_off(msp430_init_stack, -(size + stack_reserve));

			if (frame_pointer_needed)
			{
				/* fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM); */

				insn = emit_move_insn (frame_pointer_rtx, stack_pointer_rtx);
				RTX_FRAME_RELATED_P (insn) = 1;

				prologue_size += 1;
			}
			prologue_size += 2;
		}
		if ((ACCUMULATE_OUTGOING_ARGS) && (!cfun->machine->is_leaf || cfun->calls_alloca) && crtl->outgoing_args_size)
			msp430_fh_sub_sp_const(crtl->outgoing_args_size);
	}
	else	/* not a main() function */
	{
		/* Here, we've got a chance to jump to prologue saver */
		num_saved_regs = msp430_func_num_saved_regs ();

		if (!cfun->machine->is_interrupt && cfun->machine->is_critical)
		{
			prologue_size += 3;
			/*fprintf (file, "\tpush\tr2\n");
			fprintf (file, "\tdint\n");
			if (!size)
				fprintf (file, "\tnop\n");*/

			insn = emit_insn (gen_push_sreg()); /* Pushing R2 using normal push creates a faulty INSN */
			RTX_FRAME_RELATED_P (insn) = 1;
			
			insn = emit_insn (gen_disable_interrupt());
			if (!size)
				insn = emit_insn (gen_nop());
		}

		if ((TARGET_SAVE_PROLOGUE || save_prologue_p)
			&& !cfun->machine->is_interrupt && !arg_register_used[12] && num_saved_regs > 4)
		{
			/* TODO: Expand this as a separate INSN called "call prologue saver", having a meaning of pushing the registers and decreasing SP, 
				so that the debug info generation code will handle this correctly */
			
			/*fprintf (file, "\tsub\t#16, r1\n");
			fprintf (file, "\tmov\tr0, r12\n");
			fprintf (file, "\tadd\t#8, r12\n");
			fprintf (file, "\tbr\t#__prologue_saver+%d\n", (8 - num_saved_regs) * 4);*/

			msp430_fh_sub_sp_const(16);
			msp430_fh_gen_mov_pc_to_reg(12);
			msp430_fh_add_reg_const(12, 8);
			msp430_fh_br_to_symbol_plus_offset("__prologue_saver", (8 - num_saved_regs) * 4);

			if (cfun->machine->is_critical && 8 - num_saved_regs)
			{
				int n = 16 - num_saved_regs * 2;
				/*fprintf (file, "\tadd\t#%d, r1\n", n);*/
				msp430_fh_add_sp_const(n);
				if (n != 0 && n != 1 && n != 2 && n != 4 && n != 8)
					prologue_size += 1;
			}
			else
				size -= 16 - num_saved_regs * 2;

			prologue_size += 7;
		}
		else if(!cfun->machine->is_OS_task)
		{
			for (i = 15; i >= 4; i--)
			{
				if ((df_regs_ever_live_p(i) && (!call_used_regs[i] || cfun->machine->is_interrupt)) || 
					(!cfun->machine->is_leaf && (call_used_regs[i] && (cfun->machine->is_interrupt))))
				{
					/*fprintf (file, "\tpush\tr%d\n", i);*/
					msp430_fh_emit_push_reg(i);
					prologue_size += 1;
				}
			}
		}

		if (size)
		{
			/* The next is a hack... I do not understand why, but if there
			ARG_POINTER_REGNUM and FRAME/STACK are different, 
			the compiler fails to compute corresponding
			displacement */
			if (!optimize && !optimize_size
				&& df_regs_ever_live_p(ARG_POINTER_REGNUM))
			{
				int o = initial_elimination_offset (ARG_POINTER_REGNUM, STACK_POINTER_REGNUM) - size;
				
				/* fprintf (file, "\tmov\tr1, r%d\n", ARG_POINTER_REGNUM);
				fprintf (file, "\tadd\t#%d, r%d\n", o, ARG_POINTER_REGNUM); */

				insn = emit_move_insn (arg_pointer_rtx, stack_pointer_rtx);
				RTX_FRAME_RELATED_P (insn) = 1;
				msp430_fh_add_reg_const(ARG_POINTER_REGNUM, o);

				prologue_size += 2;
				if (o != 0 && o != 1 && o != 2 && o != 4 && o != 8)
					prologue_size += 1;
			}

			/* adjust frame ptr... */
			if (size < 0)
			{
				int subtracted = (size + 1) & ~1;
				/*fprintf (file, "\tsub\t#%d, r1\t;	%d, fpn %d\n", subtracted, size, frame_pointer_needed);*/
				msp430_fh_sub_sp_const(subtracted);
				
			}
			else
			{
				int added;
				size = -size;
				added = (size + 1) & ~1;
				/*fprintf (file, "\tadd\t#%d, r1\t;    %d, fpn %d\n", (size + 1) & ~1, size, frame_pointer_needed);*/
				msp430_fh_add_sp_const(added);
			}

			if (size == 1 || size == 2 || size == 4 || size == 8)
				prologue_size += 1;
			else
				prologue_size += 2;
		}

		if (frame_pointer_needed)
		{
			/*fprintf (file, "\tmov\tr1,r%d\n", FRAME_POINTER_REGNUM);*/

			insn = emit_move_insn (frame_pointer_rtx, stack_pointer_rtx);
			RTX_FRAME_RELATED_P (insn) = 1;

			prologue_size += 1;
		}

		if ((ACCUMULATE_OUTGOING_ARGS) && (!cfun->machine->is_leaf || cfun->calls_alloca) && crtl->outgoing_args_size)
			msp430_fh_sub_sp_const(crtl->outgoing_args_size);

			/* disable interrupt for reentrant function */
		if (!cfun->machine->is_interrupt && cfun->machine->is_reenterant)
		{
			prologue_size += 1;
			/*fprintf (file, "\tdint\n");*/
			insn = emit_insn (gen_disable_interrupt());
		}
	}

	/*fprintf (file, "\t/ * prologue end (size=%d) * /\n\n", prologue_size);*/
}


/* Output function epilogue */

void expand_epilogue (void)
{
	int i;
	int interrupt_func_p = cfun->machine->is_interrupt;
	int main_p = MAIN_NAME_P (DECL_NAME (current_function_decl));
	int wakeup_func_p = cfun->machine->is_wakeup;
	int cfp = cfun->machine->is_critical;
	int ree = cfun->machine->is_reenterant;
	int save_prologue_p = msp430_save_prologue_function_p (current_function_decl);
	/*int function_size;*/
	HOST_WIDE_INT size = get_frame_size();

	rtx insn;


	last_insn_address = 0;
	jump_tables_size = 0;
	epilogue_size = 0;
	/*function_size = (INSN_ADDRESSES (INSN_UID (get_last_insn ())) - INSN_ADDRESSES (INSN_UID (get_insns ())));*/

	if (cfun->machine->is_OS_task || cfun->machine->is_naked)
	{
		emit_jump_insn (gen_return ());	/* Otherwise, epilogue with 0 instruction causes a segmentation fault */
		return;
	}

	if (msp430_empty_epilogue ())
	{
		if (!return_issued)
		{
			/*fprintf (file, "\t%s\n", msp430_emit_return (NULL, NULL, NULL));*/
			emit_jump_insn (gen_return ());
			epilogue_size++;
		}
		/*fprintf (file, "\n\t/ * epilogue: not required * /\n");*/
		goto done_epilogue;
	}

	if ((cfp || interrupt_func_p) && ree)
		ree = 0;
	if (cfp && interrupt_func_p)
		cfp = 0;

	/*fprintf (file, "\n\t/ * epilogue : frame size = %d * /\n", size);*/

	if (main_p)
	{
		int totalsize = (size + 1) & ~1;
		if ((ACCUMULATE_OUTGOING_ARGS) && (!cfun->machine->is_leaf || cfun->calls_alloca) && crtl->outgoing_args_size)
			totalsize += crtl->outgoing_args_size;
		if (totalsize)
		{
			msp430_fh_add_sp_const(totalsize);
			/*fprintf (file, "\tadd\t#%d, r1\n", (size + 1) & ~1);*/
		}
		/*fprintf (file, "\tbr\t#%s\n", msp430_endup);*/
		msp430_fh_br_to_symbol_plus_offset(msp430_endup, 0);
		epilogue_size += 4;
		if (size == 1 || size == 2 || size == 4 || size == 8)
			epilogue_size--;
	}
	else
	{
		int totalsize = (size + 1) & ~1;
		if ((ACCUMULATE_OUTGOING_ARGS) && (!cfun->machine->is_leaf || cfun->calls_alloca) && crtl->outgoing_args_size)
			totalsize += crtl->outgoing_args_size;

			if (ree)
		{
			/*fprintf (file, "\teint\n");*/

			insn = emit_insn (gen_enable_interrupt());

			epilogue_size += 1;
		}

		if (totalsize)
		{
			/*fprintf (file, "\tadd\t#%d, r1\n", (size + 1) & ~1);*/
			msp430_fh_add_sp_const(totalsize);

			if (size == 1 || size == 2 || size == 4 || size == 8)
				epilogue_size += 1;
			else
				epilogue_size += 2;
		}

		if ((TARGET_SAVE_PROLOGUE || save_prologue_p)
			&& !interrupt_func_p && msp430_func_num_saved_regs () > 2)
		{
			/*fprintf (file, "\tbr\t#__epilogue_restorer+%d\n",(8 - msp430_func_num_saved_regs ()) * 2);*/

			msp430_fh_br_to_symbol_plus_offset("__epilogue_restorer", (8 - msp430_func_num_saved_regs ()) * 2);

			epilogue_size += 2;
		}
		else if ((TARGET_SAVE_PROLOGUE || save_prologue_p) && interrupt_func_p)
		{
			/*fprintf (file, "\tbr\t#__epilogue_restorer_intr+%d\n", (12 - msp430_func_num_saved_regs ()) * 2);*/
			msp430_fh_br_to_symbol_plus_offset("__epilogue_restorer_intr", (12 - msp430_func_num_saved_regs ()) * 2);
		}
		else
		{
			for (i = 4; i < 16; i++)
			{
				if ((df_regs_ever_live_p(i)
					&& (!call_used_regs[i]
				|| interrupt_func_p))
					|| (!cfun->machine->is_leaf && (call_used_regs[i] && interrupt_func_p)))
				{
					/*fprintf (file, "\tpop\tr%d\n", i);*/
					msp430_fh_emit_pop_reg(i);
					epilogue_size += 1;
				}
			}

			if (interrupt_func_p && wakeup_func_p)
			{
				/*fprintf (file, "\tbic\t#0xf0,0(r1)\n");*/
				msp430_fh_bic_deref_sp(0xF0);
				epilogue_size += 3;
			}
			emit_jump_insn (gen_return ());
			/*fprintf (file, "\tret\n");*/
			epilogue_size += 1;
		}
	}

	/*fprintf (file, "\t/ * epilogue end (size=%d) * /\n", epilogue_size);*/
done_epilogue:
	/*fprintf (file, "\t/ * function %s size %d (%d) * /\n", current_function_name, prologue_size + function_size + epilogue_size, function_size);*/

	msp430_commands_in_file += prologue_size + /*function_size +*/ epilogue_size;
	msp430_commands_in_prologues += prologue_size;
	msp430_commands_in_epilogues += epilogue_size;
}

/* Returns a number of pushed registers */
static int msp430_func_num_saved_regs (void)
{
	int i;
	int saves = 0;
	int interrupt_func_p = interrupt_function_p (current_function_decl);

	for (i = 4; i < 16; i++)
	{
		if ((df_regs_ever_live_p(i)
			&& (!call_used_regs[i]
		|| interrupt_func_p))
			|| (!cfun->machine->is_leaf && (call_used_regs[i] && interrupt_func_p)))
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
		return "reti";

	return "ret";
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
	int offset = interrupt_func_p ? 0 : (cfp ? 2 : 0);
	int reg;

	for (reg = 4; reg < 16; ++reg)
	{
		if ((!cfun->machine->is_leaf && call_used_regs[reg] && (interrupt_func_p))
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

/* cfp minds the fact that the function may save r2 */
int initial_elimination_offset (int from, int to)
{
	int outgoingArgsSize = 0;
	if((ACCUMULATE_OUTGOING_ARGS) && (!cfun->machine->is_leaf || cfun->calls_alloca) && crtl->outgoing_args_size)
		outgoingArgsSize = crtl->outgoing_args_size;

	/*
		'Reloading' is mapping pseudo-registers into hardware registers and stack slots.
		More information here: http://gcc.gnu.org/onlinedocs/gccint/RTL-passes.html
		
		Apparently the leaf_function_p() can erroneously return 1 if called after the reload has 
		completed. To handle this, we use the AVR port behavior, caching the is_leaf flag before
		reload and using it from cache afterwards.
	*/

	if (!reload_completed)
		cfun->machine->is_leaf = leaf_function_p();

	int reg;
	if (from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
		return outgoingArgsSize;
	else if (from == ARG_POINTER_REGNUM)
	{
		int interrupt_func_p = interrupt_function_p (current_function_decl);
		int cfp = msp430_critical_function_p (current_function_decl);
		int offset = interrupt_func_p ? 0 : (cfp ? 2 : 0);
		
		gcc_assert((to == FRAME_POINTER_REGNUM) || (to == STACK_POINTER_REGNUM));

		for (reg = 4; reg < 16; ++reg)
		{
			if ((!cfun->machine->is_leaf && call_used_regs[reg] && (interrupt_func_p))
				|| (df_regs_ever_live_p(reg)
				&& (!call_used_regs[reg] || interrupt_func_p)))
			{
				offset += 2;
			}
		}
		if (to == FRAME_POINTER_REGNUM)
			return get_frame_size () + offset + 2;
		else
			return get_frame_size () + offset + 2 + outgoingArgsSize;
	}
	gcc_unreachable();
}
