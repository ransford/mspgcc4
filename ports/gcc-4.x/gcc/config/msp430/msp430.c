/* This work is partially financed by the European Commission under the
* Framework 6 Information Society Technologies Project
* "Wirelessly Accessible Sensor Populations (WASP)".
*/

/*
	GCC 4.x port by Ivan Shcherbakov <mspgcc@sysprogs.org>
*/

/* Subroutines for insn-output.c for Texas Instruments MSP430 MCU
Copyright (C) 2001, 2002 Free Software Foundation, Inc.
Contributed by Dmitry Diky <diwil@mail.ru>

This file is part of GNU CC. 
GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

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

#define TARGET_REORDER 0

#if GCC_VERSION_INT < 0x430
static inline int df_regs_ever_live_p(int reg)
{
	return regs_ever_live[reg];
}

#endif


/* Commands count in the compiled file */
int msp430_commands_in_file;

/* Commands in the functions prologues in the compiled file */
int msp430_commands_in_prologues;

/* Commands in the functions epilogues in the compiled file */
int msp430_commands_in_epilogues;

/* the size of the stack space freed during mdr pass */

/* push helper */
int self_push(rtx);

int msp430_case_values_threshold = 30000;
int msp430_has_hwmul = 0;

enum msp430_arch
{
	MSP430_ISA_1 = 1,
	MSP430_ISA_2 = 2,
	MSP430_ISA_110 = 110,
	MSP430_ISA_11 = 11,
	MSP430_ISA_12 = 12,
	MSP430_ISA_13 = 13,
	MSP430_ISA_14 = 14,
	MSP430_ISA_15 = 15,
	MSP430_ISA_16 = 16,
	MSP430_ISA_20 = 20,
	MSP430_ISA_21 = 21,
	MSP430_ISA_22 = 22,
	MSP430_ISA_23 = 23,
	MSP430_ISA_24 = 24,
	MSP430_ISA_241 = 241,
	MSP430_ISA_26 = 26,
	MSP430_ISA_31 = 31,
	MSP430_ISA_32 = 32,
	MSP430_ISA_33 = 33,
	MSP430_ISA_41 = 41,
	MSP430_ISA_42 = 42,
	MSP430_ISA_43 = 43,
	MSP430_ISA_44 = 44,
	MSP430_ISA_46 = 46,
	MSP430_ISA_47 = 47,
	MSP430_ISA_471 = 471,
	MSP430_ISA_54 = 54,
};

struct mcu_type_s
{
	const char *name;
	enum msp430_arch arch;
	int has_hwmul;
};



static struct mcu_type_s msp430_mcu_types[] = {

	/* Match binutils generic types */
	{"msp1",         MSP430_ISA_11, 0}, /* anything without hardware multiply */
	{"msp2",         MSP430_ISA_14, 1}, /* older chips with hardware multiply */
	{"msp3",         MSP430_ISA_46, 1},
	{"msp4",         MSP430_ISA_47, 1},
	{"msp5",         MSP430_ISA_471, 1},
	{"msp6",         MSP430_ISA_54, 1},

	/* F1xx family */
	{"msp430x110",   MSP430_ISA_11, 0},
	{"msp430x112",   MSP430_ISA_11, 0},

	{"msp430x1101",  MSP430_ISA_110, 0},
	{"msp430x1111",  MSP430_ISA_110, 0},
	{"msp430x1121",  MSP430_ISA_110, 0},
	{"msp430x1122",  MSP430_ISA_110, 0},
	{"msp430x1132",  MSP430_ISA_110, 0},

	{"msp430x122",   MSP430_ISA_12, 0},
	{"msp430x123",   MSP430_ISA_12, 0},
	{"msp430x1222",  MSP430_ISA_12, 0},
	{"msp430x1232",  MSP430_ISA_12, 0},

	{"msp430x133",   MSP430_ISA_13, 0},
	{"msp430x135",   MSP430_ISA_13, 0},
	{"msp430x1331",  MSP430_ISA_13, 0},
	{"msp430x1351",  MSP430_ISA_13, 0},

	{"msp430x147",   MSP430_ISA_14, 1},
	{"msp430x148",   MSP430_ISA_14, 1},
	{"msp430x149",   MSP430_ISA_14, 1},
	{"msp430x1471",  MSP430_ISA_14, 1},
	{"msp430x1481",  MSP430_ISA_14, 1},
	{"msp430x1491",  MSP430_ISA_14, 1},

	{"msp430x155",   MSP430_ISA_15, 0},
	{"msp430x156",   MSP430_ISA_15, 0},
	{"msp430x157",   MSP430_ISA_15, 0},

	{"msp430x167",   MSP430_ISA_16, 1},
	{"msp430x168",   MSP430_ISA_16, 1},
	{"msp430x169",   MSP430_ISA_16, 1},
	{"msp430x1610",  MSP430_ISA_16, 1},
	{"msp430x1611",  MSP430_ISA_16, 1},
	{"msp430x1612",  MSP430_ISA_16, 1},

	/* F2xx family */
	{"msp430x2001",  MSP430_ISA_20, 0},
	{"msp430x2011",  MSP430_ISA_20, 0},

	{"msp430x2002",  MSP430_ISA_20, 0},
	{"msp430x2012",  MSP430_ISA_20, 0},

	{"msp430x2003",  MSP430_ISA_20, 0},
	{"msp430x2013",  MSP430_ISA_20, 0},

	{"msp430x2101",  MSP430_ISA_21, 0},
	{"msp430x2111",  MSP430_ISA_21, 0},
	{"msp430x2121",  MSP430_ISA_21, 0},
	{"msp430x2131",  MSP430_ISA_21, 0},

	{"msp430x2112",  MSP430_ISA_22, 0},
	{"msp430x2122",  MSP430_ISA_22, 0},
	{"msp430x2132",  MSP430_ISA_22, 0},

	{"msp430x2201",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2211",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2221",  MSP430_ISA_22, 0}, /* Value-Line */
	{"msp430x2231",  MSP430_ISA_22, 0}, /* Value-Line */

	{"msp430x2232",  MSP430_ISA_22, 0},
	{"msp430x2252",  MSP430_ISA_22, 0},
	{"msp430x2272",  MSP430_ISA_22, 0},

	{"msp430x2234",  MSP430_ISA_22, 0},
	{"msp430x2254",  MSP430_ISA_22, 0},
	{"msp430x2274",  MSP430_ISA_22, 0},

	{"msp430x233",   MSP430_ISA_23, 1},
	{"msp430x235",   MSP430_ISA_23, 1},

	{"msp430x2330",  MSP430_ISA_23, 1},
	{"msp430x2350",  MSP430_ISA_23, 1},
	{"msp430x2370",  MSP430_ISA_23, 1},

	{"msp430x247",   MSP430_ISA_24, 1},
	{"msp430x248",   MSP430_ISA_24, 1},
	{"msp430x249",   MSP430_ISA_24, 1},
	{"msp430x2410",  MSP430_ISA_24, 1},
	{"msp430x2471",  MSP430_ISA_24, 1},
	{"msp430x2481",  MSP430_ISA_24, 1},
	{"msp430x2491",  MSP430_ISA_24, 1},

	{"msp430x2416",  MSP430_ISA_241, 1},
	{"msp430x2417",  MSP430_ISA_241, 1},
	{"msp430x2418",  MSP430_ISA_241, 1},
	{"msp430x2419",  MSP430_ISA_241, 1},

	{"msp430x2616",  MSP430_ISA_26, 1},
	{"msp430x2617",  MSP430_ISA_26, 1},
	{"msp430x2618",  MSP430_ISA_26, 1},
	{"msp430x2619",  MSP430_ISA_26, 1},

	/* 3xx family (ROM) */
	{"msp430x311",   MSP430_ISA_31, 0},
	{"msp430x312",   MSP430_ISA_31, 0},
	{"msp430x313",   MSP430_ISA_31, 0},
	{"msp430x314",   MSP430_ISA_31, 0},
	{"msp430x315",   MSP430_ISA_31, 0},

	{"msp430x323",   MSP430_ISA_32, 0},
	{"msp430x325",   MSP430_ISA_32, 0},

	{"msp430x336",   MSP430_ISA_33, 1},
	{"msp430x337",   MSP430_ISA_33, 1},

	/* F4xx family */
	{"msp430x412",   MSP430_ISA_41, 0},
	{"msp430x413",   MSP430_ISA_41, 0},
	{"msp430x415",   MSP430_ISA_41, 0},
	{"msp430x417",   MSP430_ISA_41, 0},

	{"msp430x423",   MSP430_ISA_42, 1},
	{"msp430x425",   MSP430_ISA_42, 1},
	{"msp430x427",   MSP430_ISA_42, 1},

	{"msp430x4250",  MSP430_ISA_42, 0},
	{"msp430x4260",  MSP430_ISA_42, 0},
	{"msp430x4270",  MSP430_ISA_42, 0},

	{"msp430xG4250", MSP430_ISA_42, 0},
	{"msp430xG4260", MSP430_ISA_42, 0},
	{"msp430xG4270", MSP430_ISA_42, 0},

	{"msp430xE423",  MSP430_ISA_42, 1},
	{"msp430xE425",  MSP430_ISA_42, 1},
	{"msp430xE427",  MSP430_ISA_42, 1},

	{"msp430xE4232", MSP430_ISA_42, 1},
	{"msp430xE4242", MSP430_ISA_42, 1},
	{"msp430xE4252", MSP430_ISA_42, 1},
	{"msp430xE4272", MSP430_ISA_42, 1},

	{"msp430xW423",  MSP430_ISA_42, 0},
	{"msp430xW425",  MSP430_ISA_42, 0},
	{"msp430xW427",  MSP430_ISA_42, 0},

	{"msp430xG437",  MSP430_ISA_43, 0},
	{"msp430xG438",  MSP430_ISA_43, 0},
	{"msp430xG439",  MSP430_ISA_43, 0},

	{"msp430x435",   MSP430_ISA_43, 0},
	{"msp430x436",   MSP430_ISA_43, 0},
	{"msp430x437",   MSP430_ISA_43, 0},

	{"msp430x4351",  MSP430_ISA_43, 0},
	{"msp430x4361",  MSP430_ISA_43, 0},
	{"msp430x4371",  MSP430_ISA_43, 0},

	{"msp430x447",   MSP430_ISA_44, 1},
	{"msp430x448",   MSP430_ISA_44, 1},
	{"msp430x449",   MSP430_ISA_44, 1},

	{"msp430xG4616", MSP430_ISA_46, 1},
	{"msp430xG4617", MSP430_ISA_46, 1},
	{"msp430xG4618", MSP430_ISA_46, 1},
	{"msp430xG4619", MSP430_ISA_46, 1},

	{"msp430x4783",  MSP430_ISA_47, 1},
	{"msp430x4784",  MSP430_ISA_47, 1},
	{"msp430x4793",  MSP430_ISA_47, 1},
	{"msp430x4794",  MSP430_ISA_47, 1},
        
	{"msp430x47166", MSP430_ISA_471, 1},
	{"msp430x47176", MSP430_ISA_471, 1},
	{"msp430x47186", MSP430_ISA_471, 1},
	{"msp430x47196", MSP430_ISA_471, 1},

	{"msp430x47167", MSP430_ISA_471, 1},
	{"msp430x47177", MSP430_ISA_471, 1},
	{"msp430x47187", MSP430_ISA_471, 1},
	{"msp430x47197", MSP430_ISA_471, 1},

	/* F5xxx family */
	{"msp430x5418",  MSP430_ISA_54, 1},
	{"msp430x5419",  MSP430_ISA_54, 1},
	{"msp430x5435",  MSP430_ISA_54, 1},
	{"msp430x5436",  MSP430_ISA_54, 1},
	{"msp430x5437",  MSP430_ISA_54, 1},
	{"msp430x5438",  MSP430_ISA_54, 1},

        {"msp430x5510",  MSP430_ISA_54, 1},
        {"msp430x5513",  MSP430_ISA_54, 1},
        {"msp430x5514",  MSP430_ISA_54, 1},
        {"msp430x5515",  MSP430_ISA_54, 1},
        {"msp430x5517",  MSP430_ISA_54, 1},
        {"msp430x5519",  MSP430_ISA_54, 1},
        {"msp430x5521",  MSP430_ISA_54, 1},
        {"msp430x5522",  MSP430_ISA_54, 1},
        {"msp430x5524",  MSP430_ISA_54, 1},
        {"msp430x5525",  MSP430_ISA_54, 1},
        {"msp430x5526",  MSP430_ISA_54, 1},
        {"msp430x5527",  MSP430_ISA_54, 1},
        {"msp430x5528",  MSP430_ISA_54, 1},
        {"msp430x5529",  MSP430_ISA_54, 1},
        {"msp430x6638",  MSP430_ISA_54, 1},

	/* CC430 family */
	{"cc430x5133",   MSP430_ISA_54, 1},
	{"cc430x5125",   MSP430_ISA_54, 1},
	{"cc430x6125",   MSP430_ISA_54, 1},
	{"cc430x6135",   MSP430_ISA_54, 1},
	{"cc430x6126",   MSP430_ISA_54, 1},
	{"cc430x5137",   MSP430_ISA_54, 1},
	{"cc430x6127",   MSP430_ISA_54, 1},
	{"cc430x6137",   MSP430_ISA_54, 1},

	/* Add new chips to MULTILIB_MATCHES in t-msp430 as well */
	{NULL, 0, 0}
};

static void msp430_globalize_label (FILE *, const char *);
static void msp430_file_start (void);
static void msp430_file_end (void);
static bool msp430_function_ok_for_sibcall PARAMS((tree, tree));
static bool msp430_rtx_costs (rtx, int, int, int *);
int msp430_address_costs (rtx);

/* Defined in msp430-builtins.c */
void msp430_init_builtins (void);
rtx msp430_expand_builtin (tree, rtx, rtx, enum machine_mode, int);

/* Defined in msp430-function.c */
void msp430_function_end_prologue (FILE * file);
void msp430_function_begin_epilogue (FILE * file);

static struct machine_function *msp430_init_machine_status (void)
{
	return ((struct machine_function *) ggc_alloc_cleared (sizeof (struct machine_function)));
}


const struct attribute_spec msp430_attribute_table[];
static tree msp430_handle_fndecl_attribute
PARAMS ((tree *, tree, tree, int, bool *));

/* Initialize the GCC target structure.  */
#undef TARGET_ASM_ALIGNED_HI_OP
#define TARGET_ASM_ALIGNED_HI_OP "\t.word\t"

#undef TARGET_ASM_FILE_START
#define TARGET_ASM_FILE_START msp430_file_start
#undef TARGET_ASM_FILE_START_FILE_DIRECTIVE
#define TARGET_ASM_FILE_START_FILE_DIRECTIVE true
#undef TARGET_ASM_FILE_END
#define TARGET_ASM_FILE_END msp430_file_end

/* Hardcoded prologue/epilogue was replaced by flexible expand_prologue()/expand_epilogue() */
/*#undef 	TARGET_ASM_FUNCTION_PROLOGUE
#define TARGET_ASM_FUNCTION_PROLOGUE msp430_function_prologue
#undef 	TARGET_ASM_FUNCTION_EPILOGUE
#define TARGET_ASM_FUNCTION_EPILOGUE msp430_function_epilogue*/

#undef TARGET_ASM_FUNCTION_END_PROLOGUE
#define TARGET_ASM_FUNCTION_END_PROLOGUE msp430_function_end_prologue
#undef TARGET_ASM_FUNCTION_BEGIN_EPILOGUE
#define TARGET_ASM_FUNCTION_BEGIN_EPILOGUE msp430_function_begin_epilogue

#undef 	TARGET_ATTRIBUTE_TABLE
#define TARGET_ATTRIBUTE_TABLE msp430_attribute_table
#undef 	TARGET_SECTION_TYPE_FLAGS
#define TARGET_SECTION_TYPE_FLAGS msp430_section_type_flags
#undef 	TARGET_RTX_COSTS
#define TARGET_RTX_COSTS msp430_rtx_costs
#undef	TARGET_ADDRESS_COST
#define	TARGET_ADDRESS_COST msp430_address_costs
#undef  TARGET_FUNCTION_OK_FOR_SIBCALL
#define TARGET_FUNCTION_OK_FOR_SIBCALL msp430_function_ok_for_sibcall
#undef  TARGET_ASM_GLOBALIZE_LABEL
#define TARGET_ASM_GLOBALIZE_LABEL msp430_globalize_label
#undef  TARGET_INIT_BUILTINS
#define TARGET_INIT_BUILTINS msp430_init_builtins
#undef  TARGET_EXPAND_BUILTIN
#define TARGET_EXPAND_BUILTIN msp430_expand_builtin
#undef TARGET_INIT_LIBFUNCS
#define TARGET_INIT_LIBFUNCS msp430_init_once

struct gcc_target targetm = TARGET_INITIALIZER;

/****** ATTRIBUTES TO FUNCTION *************************************/
const struct attribute_spec msp430_attribute_table[] = {
	/* { name, min_len, max_len, decl_req, type_req, fn_type_req, handler } */
	{"reserve", 1, 1, false, false, false, msp430_handle_fndecl_attribute},
	{"signal", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"interrupt", 1, 1, true, false, false, msp430_handle_fndecl_attribute},
	{"naked", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"task", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"wakeup", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"critical", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"reentrant", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"saveprologue", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{"noint_hwmul", 0, 0, true, false, false, msp430_handle_fndecl_attribute},
	{NULL, 0, 0, false, false, false, NULL}
};

int msp430_current_function_noint_hwmul_function_p (void)
{
	int rval;
	if (!current_function_decl)
		return (TARGET_NOINT_HWMUL);
	rval = noint_hwmul_function_p (current_function_decl);

	return (TARGET_NOINT_HWMUL || rval);
}

unsigned int
msp430_section_type_flags (tree decl, const char *name, int reloc)

{
	unsigned int flags = 0;

	if (!strcmp (name, ".infomemnobits") || !strcmp (name, ".noinit"))
		flags = SECTION_BSS;

	flags |= default_section_type_flags (decl, name, reloc);
	return flags;
}

/* Handle an attribute requiring a FUNCTION_DECL; arguments as in
struct attribute_spec.handler.  */
static tree
msp430_handle_fndecl_attribute (tree *node, tree name, tree args ATTRIBUTE_UNUSED, int flags ATTRIBUTE_UNUSED, bool *no_add_attrs)
{
	if (TREE_CODE (*node) != FUNCTION_DECL)
	{
		warning (OPT_Wattributes, "%s' attribute only applies to functions.",
			IDENTIFIER_POINTER (name));
		*no_add_attrs = true;
	}
	return NULL_TREE;
}

enum msp430_arch msp430_get_arch (void);

enum msp430_arch
msp430_get_arch (void)
{
	const struct mcu_type_s *t;

	for (t = msp430_mcu_types; t->name; t++)
	{
		if (strcmp (t->name, msp430_mcu_name) == 0)
			break;
	}

	if (!t->name)
	{
		error ("MCU %s not supported", msp430_mcu_name);
		fprintf (stderr, "Known MCU names:\n");
		for (t = msp430_mcu_types; t->name; t++)
			fprintf (stderr, "   %s\n", t->name);
		abort ();
		return -1;
	}
	return t->arch;
}

void
msp430_override_options (void)
{
	const struct mcu_type_s *t;

        if (flag_pic)
          error ("PIC not supported on msp430");

	for (t = msp430_mcu_types; t->name; t++)
	{
		if (strcmp (t->name, msp430_mcu_name) == 0)
			break;
	}

	if (!t->name)
	{
		error ("MCU %s not supported", msp430_mcu_name);
		fprintf (stderr, "Known MCU names:\n");
		for (t = msp430_mcu_types; t->name; t++)
			fprintf (stderr, "   %s\n", t->name);
		abort ();
		return;
	}

	msp430_has_hwmul = t->has_hwmul || TARGET_FORCE_HWMUL;

	if (TARGET_NO_HWMUL)
		msp430_has_hwmul = 0;

	msp430_case_values_threshold = 8;	/* ? or there is a better value ? */
	init_machine_status = msp430_init_machine_status;
}

rtx mpy_rtx, mpys_rtx, mac_rtx, macs_rtx, op2_rtx, reslo_rtx, reshi_rtx,
sumext_rtx, ressi_rtx;


static char __dummy[1024];
rtx sym_ref(enum machine_mode mode,  char *arg);

rtx sym_ref(enum machine_mode mode,  char *arg)
{
	rtx rt;
	static int i = 0;
	rt = (rtx) &__dummy[i];
	i += sizeof(*rt);
	memset(rt,0,4);
	PUT_CODE(rt,SYMBOL_REF);
	PUT_MODE(rt,mode);
	XSTR(rt,0) = arg;

	return rt;
}


rtx gen_rtx_HWREG(const char *name);

rtx gen_rtx_HWREG(const char *name)
{
	rtx ret = gen_rtx_MEM (HImode, sym_ref (HImode, name));
	//MEM_VOLATILE_P(ret) = 1;
	return ret;
}

void msp430_init_once (void)
{
	/* Module register addresses chip-specific, added by msp430_file_start */
	mpy_rtx 	= gen_rtx_HWREG ("__MPY");
	mpys_rtx 	= gen_rtx_HWREG ("__MPYS");
	mac_rtx 	= gen_rtx_HWREG ("__MAC");
	macs_rtx 	= gen_rtx_HWREG ("__MACS");
	op2_rtx 	= gen_rtx_HWREG ("__OP2");
	reslo_rtx = gen_rtx_HWREG ("__RESLO");
	reshi_rtx = gen_rtx_HWREG ("__RESHI");
	sumext_rtx = gen_rtx_HWREG ("__SUMEXT");
	ressi_rtx = gen_rtx_HWREG ("__RESLO");
	return;
}

static char error_here_if_register_count_invalid[(FIRST_VIRTUAL_REGISTER == 17) ? 1 : -1];
static int reg_class_tab[FIRST_VIRTUAL_REGISTER] = {
	PC_REG, STACK_REGS, CG_REGS, CG_REGS,
	GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, 
	GENERAL_REGS, GENERAL_REGS, GENERAL_REGS, GENERAL_REGS,
	GENERAL_REGS, GENERAL_REGS,	GENERAL_REGS, GENERAL_REGS,	/* r0 - r15 */
	GENERAL_REGS,	/* regp */
};


int msp430_regno_ok_for_base_p (int r)
{

	if (r == 2)
		return 0;
	if (r == 3)
		return 0;
	if (r < FIRST_PSEUDO_REGISTER && r > 0)
		return 1;
	if (reg_renumber
		&& reg_renumber[r] < FIRST_PSEUDO_REGISTER
		&& reg_renumber[r] > 0 && reg_renumber[r] != 2 && reg_renumber[r] != 3)
		return 1;

	return 0;

}

enum reg_class
msp430_regno_reg_class (int r)
{
	if (r < (sizeof(reg_class_tab) / sizeof(reg_class_tab[0])))
		return reg_class_tab[r];

	return NO_REGS;
}


enum reg_class
msp430_reg_class_from_letter (
							  int c)
{
	switch (c)
	{
	case 'd':
		return SP_REG;
	default:
		break;
	}

	return NO_REGS;
}



#define NOVECTOR	0xff

void
asm_declare_function_name (
						   FILE *file,
						   const char *name,
						   tree decl ATTRIBUTE_UNUSED)
{
	int interrupt_func_p;
	tree ss = 0;
	int vector = -1;
	int vectors_start;
	int cfp = msp430_critical_function_p (current_function_decl);
	int ree = msp430_reentrant_function_p (current_function_decl);

	interrupt_func_p = interrupt_function_p (current_function_decl);

	if (interrupt_func_p)
	{
		/*
		* .global This_func1
		* .set vector11, This_func1
		* .type   This_func1,@function
		*
		*/
		switch (msp430_get_arch())
		{
		case MSP430_ISA_241:
		case MSP430_ISA_26:
		case MSP430_ISA_46:
			vectors_start = 0xffc0;
			break;
		case MSP430_ISA_54:
			vectors_start = 0xff80;
			break;
		default:
			vectors_start = 0xffe0;
		}

		ss = lookup_attribute ("interrupt",
			DECL_ATTRIBUTES (current_function_decl));
		ss = TREE_VALUE (ss);
		if (ss)
		{
			ss = TREE_VALUE (ss);
			if (ss)
				vector = TREE_INT_CST_LOW (ss);

			if (vector != NOVECTOR)
				vector += vectors_start;
		}

		if (vector == -1)
		{
			warning (OPT_Wattributes, "No valid interrupt vector assigned to ISR `%s'.", name);
		}

		if ((vector < vectors_start || vector > 0xfffe || (vector & 1))
			&& (vector != NOVECTOR && vector != -1))
		{
			warning
				(0, "Interrupt vector 0x%x assigned to ISR `%s' is invalid.",
				vector, name);
		}

		if (vector != NOVECTOR)
		{
			fprintf (file, ".global	vector_%04x\n", vector);
		}
		fprintf (file, "%s", TYPE_ASM_OP);
		assemble_name (file, name);
		putc (',', file);
		fprintf (file, TYPE_OPERAND_FMT, "function");
		putc ('\n', file);
		fprintf (file, "/***********************\n");
		fprintf (file, " * Interrupt %sRoutine `",
			(vector != NOVECTOR) ? "Service " : "Sub-");
		assemble_name (file, name);
		fprintf (file, "' at 0x%04x\n", vector);
		fprintf (file, " ***********************/\n");

		if (vector != NOVECTOR)
		{
			fprintf (file, "vector_%04x:\n", vector);
		}

		ASM_OUTPUT_LABEL (file, name);
	}
	else
	{
		fprintf (file, "%s", TYPE_ASM_OP);
		assemble_name (file, name);
		putc (',', file);
		fprintf (file, TYPE_OPERAND_FMT, "function");
		putc ('\n', file);
		fprintf (file, "/***********************\n");
		fprintf (file, " * Function `");
		assemble_name (file, name);
		fprintf (file, "' %s\n ***********************/\n",
			cfp ? "(OS critical)" : ree ? "(reentrant)" : "");
		ASM_OUTPUT_LABEL (file, name);
	}
}


/* Attempts to replace X with a valid
memory address for an operand of mode MODE  */
/* FIXME: broken call */
rtx
legitimize_address (x, oldx, mode)
rtx x;
rtx oldx ATTRIBUTE_UNUSED;
enum machine_mode mode ATTRIBUTE_UNUSED;
{
	/*    if (GET_CODE (oldx) == MEM
	&& GET_CODE (XEXP(oldx,0)) == PLUS
	&& GET_CODE (XEXP(XEXP(oldx,0),0)) == MEM)
	{
	x = force_operand (oldx,0);
	return x;
	}

	return oldx;
	*/
	return x;
}

int
legitimate_address_p (mode, operand, strict)
enum machine_mode mode ATTRIBUTE_UNUSED;
rtx operand;
int strict;
{
	rtx xfoob, x = operand;

	xfoob = XEXP (operand, 0);

	/* accept @Rn (Rn points to operand address ) */
	if (GET_CODE (operand) == REG
		&& (strict ? REG_OK_FOR_BASE_STRICT_P (x)
		: REG_OK_FOR_BASE_NOSTRICT_P (x)))
		goto granted;

	/* accept address */
	if (CONSTANT_P (operand))
		goto granted;

	/* accept X(Rn) Rn + X points to operand address */
	if (GET_CODE (operand) == PLUS
		&& GET_CODE (XEXP (operand, 0)) == REG
		&& CONSTANT_P (XEXP (operand, 1))
		&& (strict ? (REG_OK_FOR_BASE_STRICT_P (xfoob))
		: (REG_OK_FOR_BASE_NOSTRICT_P (xfoob))))
		goto granted;

	if (TARGET_ALL_DEBUG)
		fprintf (stderr, "Address Failed\n");
	return 0;

granted:
	if (TARGET_ALL_DEBUG)
		fprintf (stderr, "Address granted\n");
	return 1;
}


void
print_operand_address (file, addr)
FILE *file;
rtx addr;
{
	/* hopefully will be never entered. */
	switch (GET_CODE (addr))
	{
	case REG:
		fprintf (file, "r%d", REGNO (addr));
		return;
	case POST_INC:
		fprintf (file, "@r%d+", REGNO (XEXP(addr,0)));
		return;
	case SYMBOL_REF:
	case LABEL_REF:
	case CONST:
		fprintf (file, "#");
		break;
	case CODE_LABEL:
		break;
	default:
		abort ();
		fprintf (file, "&");
	}
	output_addr_const (file, addr);
}

void print_sub_operand PARAMS ((FILE *, rtx, int));

const char *trim_array[] = { "llo", "lhi", "hlo", "hhi" };



void
print_sub_operand (file, x, code)
FILE *file;
rtx x;
int code;
{

	if (GET_CODE (x) == SYMBOL_REF || GET_CODE (x) == LABEL_REF)
	{
		output_addr_const (file, x);
		return;
	}
	else if (GET_CODE (x) == CONST)
	{
		print_sub_operand (file, XEXP (x, 0), code);
		return;
	}
	else if (GET_CODE (x) == PLUS)
	{
		print_sub_operand (file, XEXP (x, 0), code);
		fprintf (file, "+");
		print_sub_operand (file, XEXP (x, 1), code);
		return;
	}
	else if (GET_CODE (x) == CONST_INT)
	{
		fprintf (file, "%d", INTVAL (x));
		return;
	}
	else
		abort ();
}

void print_operand (FILE *file, rtx x, int code)
{
	int shift = 0;
	int ml = GET_MODE_SIZE (x->mode);
	int source_reg = 0;


	if (ml > 1)
		ml = 2;

	if (code >= 'A' && code <= 'D')
		shift = code - 'A';

	if (code >= 'E' && code <= 'H')
	{
		shift = code - 'E';
		source_reg = 1;
	}

	if (code >= 'I' && code <= 'L')
	{
		ml = 1;
		shift = code - 'I';
	}

	if (GET_CODE (x) == PLUS)
	{
		fprintf (file, "add%s", shift ? "c" : "");
	}
	else if (GET_CODE (x) == MINUS)
	{
		fprintf (file, "sub%s", shift ? "c" : "");
	}
	else if (GET_CODE (x) == AND)
	{
		fprintf (file, "and");
	}
	else if (GET_CODE (x) == IOR)
	{
		fprintf (file, "bis");
	}
	else if (GET_CODE (x) == XOR)
	{
		fprintf (file, "xor");
	}
	else if (REG_P (x))
	{
		fprintf (file, reg_names[true_regnum (x) + shift]);
	}
	else if (GET_CODE (x) == CONST_INT)
	{
		if (code != 'F')
		{
			int intval = INTVAL (x);
			if (!shift && !(intval & ~0xFFFF)) /* For improved ASM readability, omit #llo(const) for small constants*/
				fprintf (file, "#%d", intval);	
			else
				fprintf (file, "#%s(%d)", trim_array[shift], intval);
		}
		else
			fprintf (file, "%d", INTVAL (x));
	}
	else if (GET_CODE (x) == MEM)
	{
		rtx addr = XEXP (x, 0);

		if (GET_CODE (addr) == POST_INC)
		{
			fprintf (file, "@r%d+", REGNO (XEXP(addr,0)));
		}
		else if (GET_CODE (addr) == REG)
		{			/* for X(Rn) */
			if (shift || !source_reg)
			{
				if (shift)
					fprintf (file, "%d(r%d)", shift * ml, REGNO (addr));
				else
					fprintf (file, "@r%d", REGNO (addr));
			}
			else if (source_reg)
			{
				fprintf (file, "r%d", REGNO (addr) + shift);
			}
			else
			{
				fprintf (file, "@r%d", REGNO (addr));
			}
		}
		else if (GET_CODE (addr) == SYMBOL_REF)
		{
			fprintf (file, "&");
			output_addr_const (file, addr);
			if (shift)
				fprintf (file, "+%d", shift * ml);
		}
		else if (GET_CODE (addr) == CONST || GET_CODE (addr) == CONST_INT)
		{
			fputc ('&', file);
			output_addr_const (file, addr);
			if (shift)
				fprintf (file, "+%d", shift * ml);
		}
		else if (GET_CODE (addr) == PLUS)
		{

			print_sub_operand (file, XEXP (addr, 1), code);

			/* shift if the indirect pointer register is the stack pointer */
			if ((code >= 'M' && code <= 'N') && (REGNO (XEXP (addr, 0)) == 1))
				shift = code - 'M';

			if (shift)
				fprintf (file, "+%d", shift * ml);

			if (REG_P (XEXP (addr, 0)))
				fprintf (file, "(r%d)", REGNO (XEXP (addr, 0)));
			else
				abort ();
		}
		else if (GET_CODE (addr) == MEM)
		{
			fprintf (file, "@(Invalid addressing mode)");
			print_operand (file, addr, code);
		}
		else
		{
			fprintf (file, "Unknown operand. Please check.");
		}
	}
	else if (GET_CODE (x) == SYMBOL_REF)
	{
		fprintf (file, "#");
		output_addr_const (file, x);
		if (shift)
			fprintf (file, "+%d", shift * ml);
	}
	else if (GET_CODE (x) == CONST_DOUBLE)
	{
		if (GET_MODE (x) == VOIDmode)	/* FIXME: may be long long?? */
		{
			if (shift < 2)
				fprintf (file, "#%s(%d)", trim_array[shift], CONST_DOUBLE_LOW (x));
			else
				fprintf (file, "#%s(%d)", trim_array[shift - 2],
				CONST_DOUBLE_HIGH (x));
		}
		else if (GET_MODE (x) == SFmode || GET_MODE (x) == SImode)
		{
			long val;
			REAL_VALUE_TYPE rv;
			REAL_VALUE_FROM_CONST_DOUBLE (rv, x);
			REAL_VALUE_TO_TARGET_SINGLE (rv, val);
			asm_fprintf (file, "#%s(0x%lx)", trim_array[shift], val);
		}
		else
		{
			fatal_insn ("Internal compiler bug. Unknown mode:", x);
		}
	}
	else
		print_operand_address (file, x);
}

/* mode for branch instruction */
int
msp430_jump_dist (rtx x, rtx insn)
{
	if(insn_addresses_)
	{
		int dest_addr = INSN_ADDRESSES (INSN_UID (GET_CODE (x) == LABEL_REF
			? XEXP (x, 0) : x));
		int cur_addr = INSN_ADDRESSES (INSN_UID (insn));
		int jump_distance = dest_addr - cur_addr;

		return jump_distance;
	}
	else
		return 1024;
}


rtx
msp430_libcall_value (mode)
enum machine_mode mode;
{
	int offs = GET_MODE_SIZE (mode);
	offs >>= 1;
	if (offs < 1)
		offs = 1;
	return gen_rtx_REG (mode, (RET_REGISTER + 1 - offs));
}

rtx
msp430_function_value (type, func)
tree type;
tree func ATTRIBUTE_UNUSED;
{
	int offs;
	if (TYPE_MODE (type) != BLKmode)
		return msp430_libcall_value (TYPE_MODE (type));

	offs = int_size_in_bytes (type);
	offs >>= 1;
	if (offs < 1)
		offs = 1;
	if (offs > 1 && offs < (GET_MODE_SIZE (SImode) >> 1))
		offs = GET_MODE_SIZE (SImode) >> 1;
	else if (offs > (GET_MODE_SIZE (SImode) >> 1)
		&& offs < (GET_MODE_SIZE (DImode) >> 1))
		offs = GET_MODE_SIZE (DImode) >> 1;

	return gen_rtx_REG (BLKmode, (RET_REGISTER + 1 - offs));
}

int
halfnibble_integer (op, mode)
rtx op;
enum machine_mode mode;
{
	int hi, lo;
	int val;

	if (!const_int_operand (op, mode))
		return 0;

	/* this integer is the one of form:
	0xXXXX0000 or 0x0000XXXX,
	where XXXX not one of -1,1,2,4,8 
	*/
	val = INTVAL (op);
	hi = ((val & 0xffff0000ul) >> 16) & 0xffff;
	lo = (val & 0xffff);

	if (hi && lo)
		return 0;

	if (hi && hi != 0xffff && hi != 1 && hi != 2 && hi != 4 && hi != 8)
		return 1;
	if (lo && lo != 0xffff && lo != 1 && lo != 2 && lo != 4 && lo != 8)
		return 1;

	return 0;
}


int
halfnibble_constant (op, mode)
rtx op;
enum machine_mode mode;
{
	int hi, lo;
	int val;

	if (!const_int_operand (op, mode))
		return 0;

	/* this integer is the one of form:
	0xXXXX0000 or 0x0000XXXX,
	where XXXX one of -1,1,2,4,8 
	*/
	val = INTVAL (op);
	hi = ((val & 0xffff0000ul) >> 16) & 0x0000ffff;
	lo = (val & 0x0000ffff);

	if ((hi && lo) || (!hi && !lo))
		return 0;

	if (hi == 0xffff || hi == 1 || hi == 2 || hi == 4 || hi == 8)
		return 1;
	if (lo == 0xffff || lo == 1 || lo == 2 || lo == 4 || lo == 8)
		return 1;

	if (!(hi && lo))
		return 1;

	return 0;
}


int
halfnibble_integer_shift (op, mode)
rtx op;
enum machine_mode mode;
{
	int hi, lo;
	int val;

	if (!immediate_operand (op, mode))
		return 0;

	/* this integer is the one of form:
	0xXXXX0000 or 0x0000XXXX,
	where XXXX not one of -1,1,2,4,8 
	*/
	val = 1 << INTVAL (op);
	hi = ((val & 0xffff0000ul) >> 16) & 0x0000ffff;
	lo = (val & 0x0000ffff);

	if (hi && lo)
		return 0;

	if (hi && hi != 0xffff && hi != 1 && hi != 2 && hi != 4 && hi != 8)
		return 1;
	if (lo && lo != 0xffff && lo != 1 && lo != 2 && lo != 4 && lo != 8)
		return 1;

	return 0;
}


int
halfnibble_constant_shift (op, mode)
rtx op;
enum machine_mode mode;
{
	int hi, lo;
	int val;

	if (!immediate_operand (op, mode))
		return 0;

	/* this integer is the one of form:
	0xXXXX0000 or 0x0000XXXX,
	where XXXX one of -1,1,2,4,8 
	*/
	val = 1 << INTVAL (op);
	hi = ((val & 0xffff0000ul) >> 16) & 0x0000ffff;
	lo = (val & 0x0000ffff);

	if (hi && lo)
		return 0;

	if (hi && hi == 0xffff && hi == 1 && hi == 2 && hi == 4 && hi == 8)
		return 1;
	if (lo && lo == 0xffff && lo == 1 && lo == 2 && lo == 4 && lo == 8)
		return 1;

	return 0;
}


int
which_nibble (val)
int val;
{
	if (val & 0xffff0000ul)
		return 1;
	return 0;
}


int
which_nibble_shift (val)
int val;
{
	if (val & 0xffff0000ul)
		return 1;
	return 0;
}


int
extra_constraint (x, c)
rtx x;
int c;
{

	if (c == 'R')
	{
		if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == REG)
		{
			rtx xx = XEXP (x, 0);
			int regno = REGNO (xx);
			if (regno >= 4 || regno == 1)
				return 1;
		}
	}
	else if (c == 'Q')
	{
		if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == REG)
		{
			rtx xx = XEXP (x, 0);
			int regno = REGNO (xx);
			if (regno >= 4 || regno == 1)
				return 1;
		}

		if (GET_CODE (x) == MEM
			&& GET_CODE (XEXP (x, 0)) == PLUS
			&& GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT)
		{
			rtx xx = XEXP (XEXP (x, 0), 0);
			int regno = REGNO (xx);
			if (regno >= 4 || regno == 1)
				return 1;
		}

		if (GET_CODE (x) == MEM
			&& GET_CODE (XEXP (x, 0)) == PLUS && REG_P (XEXP (XEXP (x, 0), 0)))
		{
			return 1;
		}

	}
	else if (c == 'S')
	{
		if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == SYMBOL_REF)
		{
			return 1;
		}
	}

	return 0;
}

int
indexed_location (x)
rtx x;
{
	int r = 0;

	if (GET_CODE (x) == MEM && GET_CODE (XEXP (x, 0)) == REG)
	{
		r = 1;
	}

	if (TARGET_ALL_DEBUG)
	{
		fprintf (stderr, "indexed_location %s: %s  \n",
			r ? "granted" : "failed",
			reload_completed ? "reload completed" : "reload in progress");
		debug_rtx (x);
	}

	return r;
}


int
zero_shifted (x)
rtx x;
{
	int r = 0;

	if (GET_CODE (x) == MEM &&
		GET_CODE (XEXP (x, 0)) == REG
		&& REGNO (XEXP (x, 0)) != STACK_POINTER_REGNUM
		&& REGNO (XEXP (x, 0)) != FRAME_POINTER_REGNUM
		/* the following is Ok, cause we do not corrupt r4 within ISR */
		/*&& REGNO(XEXP (x,0)) != ARG_POINTER_REGNUM */ )
	{
		r = 1;
	}
	return r;
}



void
order_regs_for_local_alloc ()
{
	unsigned int i;

	if (TARGET_REORDER)
	{
		reg_alloc_order[0] = 12;
		reg_alloc_order[1] = 13;
		reg_alloc_order[2] = 14;
		reg_alloc_order[3] = 15;
		for (i = 4; i < 16; i++)
			reg_alloc_order[i] = 15 - i;
	}
	else
	{
		for (i = 0; i < 16; i++)
			reg_alloc_order[i] = 15 - i;
	}

	return;
}

/* Output rtx VALUE as .byte to file FILE */
void
asm_output_char (file, value)
FILE *file;
rtx value;
{
	fprintf (file, "\t.byte\t");
	output_addr_const (file, value);
	fprintf (file, "\n");
}

/* Output VALUE as .byte to file FILE */
void
asm_output_byte (file, value)
FILE *file;
int value;
{
	fprintf (file, "\t.byte 0x%x\n", value & 0xff);
}

/* Output rtx VALUE as .word to file FILE */
void
asm_output_short (file, value)
FILE *file;
rtx value;
{
	fprintf (file, "\t.word ");
	output_addr_const (file, (value));
	fprintf (file, "\n");
}

#if 0
/* Output real N to file FILE */
void
asm_output_float (file, n)
FILE *file;
REAL_VALUE_TYPE n;
{
	long val;
	char dstr[100];

	REAL_VALUE_TO_TARGET_SINGLE (n, val);
	REAL_VALUE_TO_DECIMAL (n, "%g", dstr);
	fprintf (file, "\t.long 0x%08lx\t/* %s */\n", val, dstr);
}
#endif

/* Output section name to file FILE
We make the section read-only and executable for a function decl,
read-only for a const data decl, and writable for a non-const data decl.  */

void
asm_output_section_name (file, decl, name, reloc)
FILE *file;
tree decl;
const char *name;
int reloc ATTRIBUTE_UNUSED;
{
	fprintf (file, ".section %s, \"%s\", @progbits\n", name,
		decl && TREE_CODE (decl) == FUNCTION_DECL ? "ax" :
		decl && TREE_READONLY (decl) ? "a" : "aw");
}


/* The routine used to output NUL terminated strings.  We use a special
version of this for most svr4 targets because doing so makes the
generated assembly code more compact (and thus faster to assemble)
as well as more readable, especially for targets like the i386
(where the only alternative is to output character sequences as
comma separated lists of numbers).   */

void
gas_output_limited_string (file, str)
FILE *file;
const char *str;
{
	const unsigned char *_limited_str = (unsigned char *) str;
	unsigned ch;
	fprintf (file, "%s\"", STRING_ASM_OP);
	for (; (ch = *_limited_str); _limited_str++)
	{
		int escape;
		switch (escape = ESCAPES[ch])
		{
		case 0:
			putc (ch, file);
			break;
		case 1:
			fprintf (file, "\\%03o", ch);
			break;
		default:
			putc ('\\', file);
			putc (escape, file);
			break;
		}
	}
	fprintf (file, "\"\n");
}

/* The routine used to output sequences of byte values.  We use a special
version of this for most svr4 targets because doing so makes the
generated assembly code more compact (and thus faster to assemble)
as well as more readable.  Note that if we find subparts of the
character sequence which end with NUL (and which are shorter than
STRING_LIMIT) we output those using ASM_OUTPUT_LIMITED_STRING.  */

void
gas_output_ascii (file, str, length)
FILE *file;
const char *str;
size_t length;
{
	const unsigned char *_ascii_bytes = (const unsigned char *) str;
	const unsigned char *limit = _ascii_bytes + length;
	unsigned bytes_in_chunk = 0;
	for (; _ascii_bytes < limit; _ascii_bytes++)
	{
		const unsigned char *p;
		if (bytes_in_chunk >= 60)
		{
			fprintf (file, "\"\n");
			bytes_in_chunk = 0;
		}
		for (p = _ascii_bytes; p < limit && *p != '\0'; p++)
			continue;
		if (p < limit && (p - _ascii_bytes) <= (signed) STRING_LIMIT)
		{
			if (bytes_in_chunk > 0)
			{
				fprintf (file, "\"\n");
				bytes_in_chunk = 0;
			}
			gas_output_limited_string (file, (char *) _ascii_bytes);
			_ascii_bytes = p;
		}
		else
		{
			int escape;
			unsigned ch;
			if (bytes_in_chunk == 0)
				fprintf (file, "\t.ascii\t\"");
			switch (escape = ESCAPES[ch = *_ascii_bytes])
			{
			case 0:
				putc (ch, file);
				bytes_in_chunk++;
				break;
			case 1:
				fprintf (file, "\\%03o", ch);
				bytes_in_chunk += 4;
				break;
			default:
				putc ('\\', file);
				putc (escape, file);
				bytes_in_chunk += 2;
				break;
			}
		}
	}
	if (bytes_in_chunk > 0)
		fprintf (file, "\"\n");
}



/* Outputs to the stdio stream FILE some
appropriate text to go at the start of an assembler file.  */

static void msp430_file_start (void)
{
	FILE *file = asm_out_file;
	output_file_directive (file, main_input_filename);
	fprintf (file, "\t.arch %s\n\n", msp430_mcu_name);

	if (msp430_has_hwmul)
	{
		switch (msp430_get_arch())
		{
		case MSP430_ISA_471:
		case MSP430_ISA_47:
			fprintf (file, "/* Hardware multiplier registers: */\n"
				"__MPY=0x130\n"
				"__MPYS=0x132\n"
				"__MAC=0x134\n"
				"__MACS=0x136\n"
				"__OP2=0x138\n"
				"__RESLO=0x13a\n"
				"__RESHI=0x13c\n"
				"__SUMEXT=0x13e\n"
				"__MPY32L=0x140\n"
				"__MPY32H=0x142\n"
				"__MPYS32L=0x144\n"
				"__MPYS32H=0x146\n"
				"__MAC32L=0x148\n"
				"__MAC32H=0x14a\n"
				"__MACS32L=0x14c\n"
				"__MACS32H=0x14e\n"
				"__OP2L=0x150\n"
				"__OP2H=0x152\n"
				"__RES0=0x154\n"
				"__RES1=0x156\n"
				"__RES2=0x158\n"
				"__RES3=0x15a\n"
				"__MPY32CTL0=0x15c\n"
				"\n");
			break;
		case MSP430_ISA_54:
			fprintf (file, "/* Hardware multiplier registers: */\n"
				"__MPY=0x4c0\n"
				"__MPYS=0x4c2\n"
				"__MAC=0x4c4\n"
				"__MACS=0x4c6\n"
				"__OP2=0x4c8\n"
				"__RESLO=0x4ca\n"
				"__RESHI=0x4cc\n"
				"__SUMEXT=0x4ce\n"
				"__MPY32L=0x4d0\n"
				"__MPY32H=0x4d2\n"
				"__MPYS32L=0x4d4\n"
				"__MPYS32H=0x4d6\n"
				"__MAC32L=0x4d8\n"
				"__MAC32H=0x4da\n"
				"__MACS32L=0x4dc\n"
				"__MACS32H=0x4de\n"
				"__OP2L=0x4e0\n"
				"__OP2H=0x4e2\n"
				"__RES0=0x4e4\n"
				"__RES1=0x4e6\n"
				"__RES2=0x4e8\n"
				"__RES3=0x4ea\n"
				"__MPY32CTL0=0x4ec\n"
				"\n");
			break;
		default:
			fprintf (file, "/* Hardware multiplier registers: */\n"
				"__MPY=0x130\n"
				"__MPYS=0x132\n"
				"__MAC=0x134\n"
				"__MACS=0x136\n"
				"__OP2=0x138\n"
				"__RESLO=0x13a\n" 
				"__RESHI=0x13c\n" 
				"__SUMEXT=0x13e\n" 
				"\n");
		}

	}

	msp430_commands_in_file = 0;
	msp430_commands_in_prologues = 0;
	msp430_commands_in_epilogues = 0;
}

/* Outputs to the stdio stream FILE some
appropriate text to go at the end of an assembler file.  */

static void msp430_file_end (void)
{
	fprintf (asm_out_file,
		"\n"
		"/*********************************************************************\n"
		" * File %s: code size: %d words (0x%x)\n * incl. words in prologues: %d, epilogues: %d\n"
		" *********************************************************************/\n",
		main_input_filename,
		msp430_commands_in_file,
		msp430_commands_in_file, msp430_commands_in_prologues, msp430_commands_in_epilogues);
}

int
msp430_hard_regno_mode_ok (regno, mode)
int regno ATTRIBUTE_UNUSED;
enum machine_mode mode ATTRIBUTE_UNUSED;
{
	return 1;
}

#if GCC_VERSION_INT >= 0x440
#define current_function_calls_alloca cfun->calls_alloca
#endif

int
frame_pointer_required_p ()
{
	return (current_function_calls_alloca
		/*            || current_function_args_info.nregs == 0 */
		/*|| current_function_varargs*/);

	/* || get_frame_size () > 0); */
}

enum reg_class
preferred_reload_class (x, class)
rtx x ATTRIBUTE_UNUSED;
enum reg_class class;
{
	return class;
}

int
adjust_insn_length (insn, len)
rtx insn;
int len;
{

	rtx patt = PATTERN (insn);
	rtx set;

	set = single_set (insn);

	if (GET_CODE (patt) == SET)
	{
		rtx op[10];
		op[1] = SET_SRC (patt);
		op[0] = SET_DEST (patt);

		if (general_operand (op[1], VOIDmode)
			&& general_operand (op[0], VOIDmode))
		{
			op[2] = SET_SRC (patt);
			switch (GET_MODE (op[0]))
			{
			case QImode:
			case HImode:
				if (indexed_location (op[1]))
					len--;
				break;

			case SImode:
			case SFmode:
				/* get length first */
				msp430_movesi_code (insn, op, &len);

				if (zero_shifted (op[1]) && regsi_ok_safe (op))
				{
					rtx reg = XEXP (op[1], 0);
					if (dead_or_set_p (insn, reg))
						len -= 1;
				}
				else if (!zero_shifted (op[1]) && indexed_location (op[1]))
				{
					len -= 1;
				}
				break;
			case DImode:
				msp430_movedi_code (insn, op, &len);
				if (zero_shifted (op[1]) && regdi_ok_safe (op))
				{
					rtx reg = XEXP (op[1], 0);
					if (dead_or_set_p (insn, reg))
						len -= 1;
				}
				else if (!zero_shifted (op[1]) && indexed_location (op[1]))
				{
					len -= 1;
				}
				break;

			default:
				break;
			}

			if (GET_CODE (op[2]) == CONST_INT)
			{
				if (GET_MODE (op[0]) == DImode)
				{
					int x = INTVAL (op[2]);
					int y = (x & 0xffff0000ul) >> 16;
					x = x & 0xffff;

					len -= 2;

					if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
						|| x == 0xffff)
						len--;
					if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
						|| y == 0xffff)
						len--;
				}
				else if (GET_MODE (op[0]) == SImode)
				{
					int x = INTVAL (op[2]);
					int y = (x & 0xffff0000ul) >> 16;
					x = x & 0xffff;

					if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
						|| x == 0xffff)
						len--;
					if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
						|| y == 0xffff)
						len--;
				}
				else
				{
					/* mighr be hi or qi modes */
					int x = INTVAL (op[2]);
					x = x & 0xffff;
					if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
						|| x == 0xffff)
						len--;
				}
			}

			if (GET_CODE (op[2]) == CONST_DOUBLE)
			{
				if (GET_MODE (op[0]) == SFmode)
				{
					long val;
					int y, x;
					REAL_VALUE_TYPE rv;
					REAL_VALUE_FROM_CONST_DOUBLE (rv, op[2]);
					REAL_VALUE_TO_TARGET_SINGLE (rv, val);

					y = (val & 0xffff0000ul) >> 16;
					x = val & 0xffff;
					if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
						|| x == 0xffff)
						len--;
					if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
						|| y == 0xffff)
						len--;
				}
				else
				{
					int hi = CONST_DOUBLE_HIGH (op[2]);
					int lo = CONST_DOUBLE_LOW (op[2]);
					int x, y, z;

					x = (hi & 0xffff0000ul) >> 16;
					y = hi & 0xffff;
					z = (lo & 0xffff0000ul) >> 16;
					if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
						|| x == 0xffff)
						len--;
					if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
						|| y == 0xffff)
						len--;
					if (z == 0 || z == 1 || z == 2 || z == 4 || z == 8
						|| z == 0xffff)
						len--;
					z = lo & 0xffff;
					if (z == 0 || z == 1 || z == 2 || z == 4 || z == 8
						|| z == 0xffff)
						len--;
				}
			}

			return len;
		}
		else if (GET_CODE (op[1]) == MULT)
		{
			rtx ops[10];
			ops[0] = op[0];
			ops[1] = XEXP (op[1], 0);
			ops[2] = XEXP (op[1], 1);

			if (GET_MODE (ops[0]) != SImode
				&& GET_MODE (ops[0]) != SFmode && GET_MODE (ops[0]) != DImode)
			{
				if (indexed_location (ops[1]))
					len--;
				if (indexed_location (ops[2]))
					len--;
			}
		}
		else if (GET_CODE (op[1]) == ASHIFT
			|| GET_CODE (op[1]) == ASHIFTRT || GET_CODE (op[1]) == LSHIFTRT)
		{
			rtx ops[10];
			ops[0] = op[0];
			ops[1] = XEXP (op[1], 0);
			ops[2] = XEXP (op[1], 1);

			switch (GET_CODE (op[1]))
			{
			case ASHIFT:
				switch (GET_MODE (op[0]))
				{
				case QImode:
					msp430_emit_ashlqi3 (insn, ops, &len);
					break;
				case HImode:
					msp430_emit_ashlhi3 (insn, ops, &len);
					break;
				case SImode:
					msp430_emit_ashlsi3 (insn, ops, &len);
					break;
				case DImode:
					msp430_emit_ashldi3 (insn, ops, &len);
					break;
				default:
					break;
				}
				break;

			case ASHIFTRT:
				switch (GET_MODE (op[0]))
				{
				case QImode:
					msp430_emit_ashrqi3 (insn, ops, &len);
					break;
				case HImode:
					msp430_emit_ashrhi3 (insn, ops, &len);
					break;
				case SImode:
					msp430_emit_ashrsi3 (insn, ops, &len);
					break;
				case DImode:
					msp430_emit_ashrdi3 (insn, ops, &len);
					break;
				default:
					break;
				}
				break;

			case LSHIFTRT:
				switch (GET_MODE (op[0]))
				{
				case QImode:
					msp430_emit_lshrqi3 (insn, ops, &len);
					break;
				case HImode:
					msp430_emit_lshrhi3 (insn, ops, &len);
					break;
				case SImode:
					msp430_emit_lshrsi3 (insn, ops, &len);
					break;
				case DImode:
					msp430_emit_lshrdi3 (insn, ops, &len);
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
		}
		else if (GET_CODE (op[1]) == PLUS
			|| GET_CODE (op[1]) == MINUS
			|| GET_CODE (op[1]) == AND
			|| GET_CODE (op[1]) == IOR || GET_CODE (op[1]) == XOR)
		{
			rtx ops[10];
			ops[0] = op[0];
			ops[1] = XEXP (op[1], 0);
			ops[2] = XEXP (op[1], 1);

			if (GET_CODE (op[1]) == AND && !general_operand (ops[1], VOIDmode))
				return len;

			switch (GET_MODE (ops[0]))
			{
			case QImode:
			case HImode:
				if (indexed_location (ops[2]))
					len--;
				break;
			case SImode:
			case SFmode:

				if (GET_CODE (op[1]) == PLUS)
					msp430_addsi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == MINUS)
					msp430_subsi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == AND)
					msp430_andsi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == IOR)
					msp430_iorsi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == XOR)
					msp430_xorsi_code (insn, ops, &len);

				if (zero_shifted (ops[2]) && regsi_ok_safe (ops))
				{
					rtx reg = XEXP (ops[2], 0);
					if (dead_or_set_p (insn, reg))
						len -= 1;
				}
				else if (!zero_shifted (ops[2]) && indexed_location (ops[2]))
				{
					len -= 1;
				}
				break;
			case DImode:

				if (GET_CODE (op[1]) == PLUS)
					msp430_adddi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == MINUS)
					msp430_subdi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == AND)
					msp430_anddi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == IOR)
					msp430_iordi_code (insn, ops, &len);
				if (GET_CODE (op[1]) == XOR)
					msp430_xordi_code (insn, ops, &len);

				if (zero_shifted (ops[2]) && regdi_ok_safe (ops))
				{
					rtx reg = XEXP (ops[2], 0);
					if (dead_or_set_p (insn, reg))
						len -= 1;
				}
				else if (!zero_shifted (ops[2]) && indexed_location (ops[2]))
				{
					len -= 1;
				}
				break;

			default:
				break;
			}

			if (GET_MODE (ops[0]) == SImode)
			{
				if (GET_CODE (ops[2]) == CONST_INT)
				{
					if (GET_CODE (op[1]) == AND)
					{
						msp430_emit_immediate_and2 (insn, ops, &len);
					}
					else if (GET_CODE (op[1]) == IOR)
					{
						msp430_emit_immediate_ior2 (insn, ops, &len);
					}
					else
					{
						if (GET_MODE (ops[0]) == SImode)
						{
							int x = INTVAL (ops[2]);
							int y = (x & 0xffff0000ul) >> 16;
							x = x & 0xffff;

							if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
								|| x == 0xffff)
								len--;
							if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
								|| y == 0xffff)
								len--;
						}
					}
				}
			}

			if (GET_MODE (ops[0]) == SFmode || GET_MODE (ops[0]) == DImode)
			{
				if (GET_CODE (ops[2]) == CONST_DOUBLE)
				{

					if (GET_CODE (op[1]) == AND)
					{
						msp430_emit_immediate_and4 (insn, ops, &len);
					}
					else if (GET_CODE (op[1]) == IOR)
					{
						msp430_emit_immediate_ior4 (insn, ops, &len);
					}
					else if (GET_MODE (ops[0]) == SFmode)
					{
						long val;
						int y, x;
						REAL_VALUE_TYPE rv;
						REAL_VALUE_FROM_CONST_DOUBLE (rv, ops[2]);
						REAL_VALUE_TO_TARGET_SINGLE (rv, val);

						y = (val & 0xffff0000ul) >> 16;
						x = val & 0xffff;
						if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
							|| x == 0xffff)
							len--;
						if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
							|| y == 0xffff)
							len--;
					}
					else
					{
						int hi = CONST_DOUBLE_HIGH (ops[2]);
						int lo = CONST_DOUBLE_LOW (ops[2]);
						int x, y, z;

						x = (hi & 0xffff0000ul) >> 16;
						y = hi & 0xffff;
						z = (lo & 0xffff0000ul) >> 16;
						if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8
							|| x == 0xffff)
							len--;
						if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8
							|| y == 0xffff)
							len--;
						if (z == 0 || z == 1 || z == 2 || z == 4 || z == 8
							|| z == 0xffff)
							len--;
					}
				}
			}

			return len;
		}
		else if (GET_CODE (op[1]) == NOT
			|| GET_CODE (op[1]) == ABS || GET_CODE (op[1]) == NEG)
		{
			if (GET_MODE (op[0]) == HImode || GET_MODE (op[0]) == QImode)
				if (indexed_location (XEXP (op[1], 0)))
					len--;
			/* consts handled by cpp */
			/* nothing... */
		}
		else if (GET_CODE (op[1]) == ZERO_EXTEND)
		{
			rtx ops[10];
			ops[0] = op[0];
			ops[1] = XEXP (op[1], 0);

			if (GET_MODE (ops[1]) == QImode)
			{
				if (GET_MODE (ops[0]) == HImode)
					zeroextendqihi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == SImode)
					zeroextendqisi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == DImode)
					zeroextendqidi (insn, ops, &len);
			}
			else if (GET_MODE (ops[1]) == HImode)
			{
				if (GET_MODE (ops[0]) == SImode)
					zeroextendhisi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == DImode)
					zeroextendhidi (insn, ops, &len);
			}
			else if (GET_MODE (ops[1]) == SImode)
			{
				if (GET_MODE (ops[1]) == DImode)
					zeroextendsidi (insn, ops, &len);
			}
		}
		else if (GET_CODE (op[1]) == SIGN_EXTEND)
		{
			rtx ops[10];
			ops[0] = op[0];	/* dest */
			ops[1] = XEXP (op[1], 0);	/* src */

			if (GET_MODE (ops[1]) == QImode)
			{
				if (GET_MODE (ops[0]) == HImode)
					signextendqihi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == SImode)
					signextendqisi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == DImode)
					signextendqidi (insn, ops, &len);
			}
			else if (GET_MODE (ops[1]) == HImode)
			{
				if (GET_MODE (ops[0]) == SImode)
					signextendhisi (insn, ops, &len);
				else if (GET_MODE (ops[0]) == DImode)
					signextendhidi (insn, ops, &len);
			}
			else if (GET_MODE (ops[1]) == SImode)
			{
				if (GET_MODE (ops[0]) == DImode)
					signextendsidi (insn, ops, &len);
			}
		}
		else if (GET_CODE (op[1]) == IF_THEN_ELSE)
		{
			if (GET_CODE (op[0]) == PC)
			{
				rtx ops[5];
				ops[0] = XEXP (op[1], 1);
				ops[1] = XEXP (op[1], 0);
				ops[2] = XEXP (ops[1], 0);
				ops[3] = XEXP (ops[1], 1);
				msp430_cbranch (insn, ops, &len, 0);
			}
		}
		else if (GET_CODE (op[0]) == MEM
			&& GET_CODE (XEXP (op[0], 0)) == POST_DEC)
		{
			rtx ops[4];
			ops[0] = op[1];
			if (GET_MODE (op[0]) == QImode)
				msp430_pushqi (insn, ops, &len);
			if (GET_MODE (op[0]) == HImode)
				msp430_pushhi (insn, ops, &len);
			if (GET_MODE (op[0]) == SImode)
				msp430_pushsisf (insn, ops, &len);
			if (GET_MODE (op[0]) == DImode)
				msp430_pushdi (insn, ops, &len);
		}
	}

	if (set)
	{
		rtx op[10];
		op[1] = SET_SRC (set);
		op[0] = SET_DEST (set);

		if (GET_CODE (patt) == PARALLEL)
		{
			if (GET_CODE (op[0]) == PC && GET_CODE (op[1]) == IF_THEN_ELSE)
			{
				rtx ops[5];
				ops[0] = XEXP (op[1], 1);
				ops[1] = XEXP (op[1], 0);
				ops[2] = XEXP (ops[1], 0);
				ops[3] = XEXP (ops[1], 1);
				msp430_cbranch (insn, ops, &len, 0);
			}

			if (GET_CODE (op[1]) == ASHIFT
				|| GET_CODE (op[1]) == ASHIFTRT || GET_CODE (op[1]) == LSHIFTRT)
			{
				rtx ops[10];
				ops[0] = op[0];
				ops[1] = XEXP (op[1], 0);
				ops[2] = XEXP (op[1], 1);

				switch (GET_CODE (op[1]))
				{
				case ASHIFT:
					switch (GET_MODE (op[0]))
					{
					case QImode:
						msp430_emit_ashlqi3 (insn, ops, &len);
						break;
					case HImode:
						msp430_emit_ashlhi3 (insn, ops, &len);
						break;
					case SImode:
						msp430_emit_ashlsi3 (insn, ops, &len);
						break;
					case DImode:
						msp430_emit_ashldi3 (insn, ops, &len);
						break;
					default:
						break;
					}
					break;

				case ASHIFTRT:
					switch (GET_MODE (op[0]))
					{
					case QImode:
						msp430_emit_ashrqi3 (insn, ops, &len);
						break;
					case HImode:
						msp430_emit_ashrhi3 (insn, ops, &len);
						break;
					case SImode:
						msp430_emit_ashrsi3 (insn, ops, &len);
						break;
					case DImode:
						msp430_emit_ashrdi3 (insn, ops, &len);
						break;
					default:
						break;
					}
					break;

				case LSHIFTRT:
					switch (GET_MODE (op[0]))
					{
					case QImode:
						msp430_emit_lshrqi3 (insn, ops, &len);
						break;
					case HImode:
						msp430_emit_lshrhi3 (insn, ops, &len);
						break;
					case SImode:
						msp430_emit_lshrsi3 (insn, ops, &len);
						break;
					case DImode:
						msp430_emit_lshrdi3 (insn, ops, &len);
						break;
					default:
						break;
					}
					break;

				default:
					break;
				}
			}
		}
	}

	return len;
}

void
machine_dependent_reorg (first_insn)
rtx first_insn ATTRIBUTE_UNUSED;
{
	/* nothing to be done here this time */
	return;
}


int
test_hard_reg_class (class, x)
enum reg_class class;
rtx x;
{
	int regno = true_regnum (x);
	if (regno < 0)
		return 0;
	return TEST_HARD_REG_CLASS (class, regno);
}


/* Returns 1 if SCRATCH are safe to be allocated as a scratch
registers (for a define_peephole2) in the current function.  */
/* UNUSED ... yet... */
int
msp430_peep2_scratch_safe (scratch)
rtx scratch;
{
	if ((interrupt_function_p (current_function_decl)
		|| signal_function_p (current_function_decl)) && cfun->machine->is_leaf)
	{
		int first_reg = true_regnum (scratch);
		int last_reg;
		int size = GET_MODE_SIZE (GET_MODE (scratch));
		int reg;

		size >>= 1;
		if (!size)
			size = 1;

		last_reg = first_reg + size - 1;

		for (reg = first_reg; reg <= last_reg; reg++)
		{
			if (!df_regs_ever_live_p(reg))
				return 0;
		}
	}

	return 1;
}


/* Update the condition code in the INSN.
now mostly unused */

void
notice_update_cc (body, insn)
rtx body ATTRIBUTE_UNUSED;
rtx insn ATTRIBUTE_UNUSED;
{
	CC_STATUS_INIT;
}



/*********************************************************************/

/*
Next two return non zero for rtx as
(set (reg:xx)
(mem:xx (reg:xx))

*/

int
regsi_ok_safe (operands)
rtx operands[];
{
	rtx dest = operands[0];
	rtx areg;
	int src_reg;
	int dst_reg;

	if (operands[2])
		areg = XEXP (operands[2], 0);
	else
		areg = XEXP (operands[1], 0);

	if (GET_CODE (dest) == MEM)
	{
		dest = XEXP (operands[0], 0);
		if (GET_CODE (dest) == PLUS && GET_CODE (XEXP (dest, 0)) == REG)
		{
			dest = XEXP (dest, 0);
		}
		else if (GET_CODE (dest) == REG)
		{
			;			/* register */
		}
		else
			return 1;
	}

	if (REGNO (dest) >= FIRST_PSEUDO_REGISTER
		|| REGNO (areg) >= FIRST_PSEUDO_REGISTER)
		return 1;

	dst_reg = true_regnum (dest);
	src_reg = true_regnum (areg);
	if (dst_reg > src_reg || dst_reg + 1 < src_reg)
	{
		return 1;
	}
	return 0;
}

int
regsi_ok_clobber (operands)
rtx operands[];
{
	rtx dest = operands[0];
	rtx areg = XEXP (operands[2], 0);
	int src_reg;
	int dst_reg;
	int regno = REGNO (dest);


	if (GET_CODE (dest) == MEM)
	{
		dest = XEXP (operands[0], 0);
		if (GET_CODE (dest) == PLUS && GET_CODE (XEXP (dest, 0)) == REG)
		{
			dest = XEXP (dest, 0);
		}
		else if (GET_CODE (dest) == REG)
		{
			;			/* register */
		}
		else
			return 1;
	}

	if (regno >= FIRST_PSEUDO_REGISTER || REGNO (areg) >= FIRST_PSEUDO_REGISTER)
		return 1;

	dst_reg = true_regnum (dest);
	src_reg = true_regnum (areg);
	if (dst_reg + 1 == src_reg)
		return 1;
	return 0;
}

int
regdi_ok_safe (operands)
rtx operands[];
{
	rtx dest = operands[0];
	rtx areg = XEXP (operands[2], 0);
	int src_reg;
	int dst_reg;


	if (GET_CODE (dest) == MEM)
	{
		dest = XEXP (operands[0], 0);
		if (GET_CODE (dest) == PLUS && GET_CODE (XEXP (dest, 0)) == REG)
		{
			dest = XEXP (dest, 0);
		}
		else if (GET_CODE (dest) == REG)
		{
			;			/* register */
		}
		else
			return 1;
	}

	if (REGNO (dest) >= FIRST_PSEUDO_REGISTER
		|| REGNO (areg) >= FIRST_PSEUDO_REGISTER)
		return 1;

	dst_reg = true_regnum (dest);
	src_reg = true_regnum (areg);
	if (dst_reg > src_reg || dst_reg + 3 < src_reg)
	{
		return 1;
	}

	return 0;
}

int
regdi_ok_clobber (operands)
rtx operands[];
{
	rtx dest = operands[0];
	rtx areg = XEXP (operands[2], 0);
	int src_reg;
	int dst_reg;
	int regno = REGNO (dest);

	if (GET_CODE (dest) == MEM)
	{
		dest = XEXP (operands[0], 0);
		if (GET_CODE (dest) == PLUS && GET_CODE (XEXP (dest, 0)) == REG)
		{
			dest = XEXP (dest, 0);
		}
		else if (GET_CODE (dest) == REG)
		{
			;			/* register */
		}
		else
			return 1;
	}

	if (regno >= FIRST_PSEUDO_REGISTER || REGNO (areg) >= FIRST_PSEUDO_REGISTER)
		return 1;

	dst_reg = true_regnum (dest);
	src_reg = true_regnum (areg);
	if (dst_reg + 3 == src_reg)
		return 1;
	return 0;
}


/***************** ARITHMETIC *******************/

int
emit_indexed_arith (insn, operands, m, cmd, iscarry)
rtx insn;
rtx operands[];
int m;
const char *cmd;
int iscarry;
{
	char template[256];
	register int i = 0;
	char *p;
	rtx reg = NULL;
	int len = m * 2;
	rtx x = operands[0];
	int havestop = 0;
	rtx pattern;
	rtx next = next_real_insn (insn);


	pattern = PATTERN (next);

	if (pattern && GET_CODE (pattern) == PARALLEL)
	{
		pattern = XVECEXP (pattern, 0, 0);
	}

	if (followed_compare_condition (insn) != UNKNOWN
		|| GET_CODE(insn) == JUMP_INSN
		|| (pattern
		&& GET_CODE (pattern) == SET
		&& SET_DEST (pattern) == cc0_rtx)
		|| (pattern && GET_CODE (pattern) == SET
		&& SET_DEST (pattern) == pc_rtx))
	{
		/* very exotic case */

		snprintf (template, 255, "%s\t" "%%A%d, %%A0", cmd, operands[2] ? 2 : 1);
		output_asm_insn (template, operands);
		snprintf (template, 255, "%s%s\t" "%%B%d, %%B0", cmd, iscarry ? "c" : "",
			operands[2] ? 2 : 1);
		output_asm_insn (template, operands);

		if (m == 2)
			return len;

		snprintf (template, 255, "%s%s\t" "%%C%d, %%C0", cmd, iscarry ? "c" : "",
			operands[2] ? 2 : 1);
		output_asm_insn (template, operands);
		snprintf (template, 255, "%s%s\t" "%%D%d, %%D0", cmd, iscarry ? "c" : "",
			operands[2] ? 2 : 1);
		output_asm_insn (template, operands);

		return len;
	}

	if (operands[2])
		reg = XEXP (operands[2], 0);
	else
		reg = XEXP (operands[1], 0);

	if (GET_CODE (x) == REG)
	{
		int src;
		int dst = REGNO (x);

		if (!reg)
		{
			reg = XEXP (operands[1], 0);
		}

		src = REGNO (reg);

		/* check if registers overlap */
		if (dst > src || (dst + m - 1) < src)
		{
			;			/* fine ! */
		}
		else if ((dst + m - 1) == src)
		{
			havestop = 1;		/* worse */
		}
		else
		{
			/* cannot do reverse assigment */
			while (i < m)
			{
				p = (char *) (template + strlen (cmd));
				p += (i && iscarry) ? 3 : 2;
				strcpy (template, cmd);
				strcat (template, (i && iscarry) ? "c\t%" : "\t%");
				*p = 'A' + i;
				p++;
				*p = 0;
				strcat (template, "0, %");
				p += 2;
				*p = 'A' + i;
				p++;
				*p = 0;
				strcat (template, operands[2] ? "2" : "1");
				output_asm_insn (template, operands);
				i++;
			}
			return m * 3;
		}
	}

	while (i < (m - havestop))
	{
		p = template + strlen (cmd);

		strcpy (template, cmd);

		if (i && iscarry)
		{
			strcat (template, "c\t");
			p += 2;
		}
		else
		{
			strcat (template, "\t");
			p += 1;
		}
		strcat (template, operands[2] ? "@%E2+, %" : "@%E1+, %");
		p += 8;
		*p = 'A' + i;
		p++;
		*p = 0;
		strcat (template, "0");
		p++;
		output_asm_insn (template, operands);
		i++;
	}

	if (havestop)
	{
		len++;
		p = template + strlen (cmd);
		strcpy (template, cmd);
		if (i && iscarry)
		{
			strcat (template, "c\t");
			p += 2;
		}
		else
		{
			strcat (template, "\t");
			p += 1;
		}
		strcat (template, operands[2] ? "@%E2, %" : "@%E1, %");
		p += 8;
		*p = 'A' + i;
		p++;
		*p = 0;
		strcat (template, "0	;	register won't die");
		p += 1;
		output_asm_insn (template, operands);
	}

	if (!dead_or_set_p (insn, reg) && !havestop)
	{
		len++;
		p = template + 3;
		strcpy (template, "sub");
		strcat (template, "\t#");
		p += 2;
		*p = '0' + m * 2;
		p++;
		*p = 0;

		if (operands[2])
			strcat (template, ",    %E2	;	restore %E2");
		else
			strcat (template, ",    %E1	;	restore %E1");
		output_asm_insn (template, operands);
	}

	return len;
}

static int sameoperand_p PARAMS ((rtx, rtx));

int
sameoperand (operands, i)
rtx operands[];
int i;
{
	rtx dst = operands[0];
	rtx src = operands[i];

	return sameoperand_p (src, dst);
}

static int
sameoperand_p (src, dst)
rtx src;
rtx dst;
{
	enum rtx_code scode = GET_CODE (src);
	enum rtx_code dcode = GET_CODE (dst);
	/* cannot use standard functions here 
	cause operands have different modes:
	*/

	if (scode != dcode)
		return 0;

	switch (scode)
	{
	case REG:
		return REGNO (src) == REGNO (dst);
		break;
	case MEM:
		return sameoperand_p (XEXP (src, 0), XEXP (dst, 0));
		break;
	case PLUS:
		return sameoperand_p (XEXP (src, 0), XEXP (dst, 0))
			&& sameoperand_p (XEXP (src, 1), XEXP (dst, 1));
		break;
	case CONST_INT:
		return INTVAL (src) == INTVAL (dst);
		break;
	case SYMBOL_REF:
		return XSTR (src, 0) == XSTR (dst, 0);
		break;
	default:
		break;
	}
	return 0;

}

#define OUT_INSN(x,p,o) \
	do {                            \
	if(!x) output_asm_insn (p,o);   \
	} while(0)



/************** MOV CODE *********************************/

const char *
movstrsi_insn (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{

	/* operands 0 and 1 are registers !!! */
	/* operand 2 is a cnt and not zero */
	output_asm_insn ("\n.Lmsn%=:", operands);
	output_asm_insn ("mov.b\t@%1+,0(%0)", operands);
	output_asm_insn ("inc\t%0", operands);
	output_asm_insn ("dec\t%2", operands);
	output_asm_insn ("jnz\t.Lmsn%=", operands);

	return "";
}


const char *
clrstrsi_insn (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{

	/* operand 0 is a register !!! */
	/* operand 1 is a cnt and not zero */
	output_asm_insn ("\n.Lcsn%=:", operands);
	output_asm_insn ("clr.b\t0(%0)	;	clr does not support @rn+",
		operands);
	output_asm_insn ("inc\t%0", operands);
	output_asm_insn ("dec\t%1", operands);
	output_asm_insn ("jnz\t.Lcsn%=", operands);
	return "";
}

const char *
movstrhi_insn (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{

	/* operands 0 and 1 are registers !!! */
	/* operand 2 is a cnt and not zero */
	output_asm_insn ("\n.Lmsn%=:", operands);
	output_asm_insn ("mov.b\t@%1+,0(%0)", operands);
	output_asm_insn ("inc\t%0", operands);
	output_asm_insn ("dec\t%2", operands);
	output_asm_insn ("jnz\t.Lmsn%=", operands);
	return "";
}

const char *
clrstrhi_insn (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{

	/* operand 0 is a register !!! */
	/* operand 1 is a cnt and not zero */
	output_asm_insn ("\n.Lcsn%=:", operands);
	output_asm_insn ("clr.b\t0(%0)", operands);
	output_asm_insn ("inc\t%0", operands);
	output_asm_insn ("dec\t%1", operands);
	output_asm_insn ("jnz\t.Lcsn%=", operands);
	return "";
}

int
msp430_emit_indexed_mov (insn, operands, m, cmd)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int m;
const char *cmd;
{
	char template[256];
	register int i = 0;
	char *p;
	rtx reg = XEXP (operands[1], 0);
	int len = m * 2;
	rtx dst = 0;
	int sreg,dreg = 0;

	if(memory_operand(operands[0], VOIDmode))
	{
		if(  REG_P(XEXP(operands[0],0)))
			dreg = REGNO(XEXP(operands[0],0));
		else if(GET_CODE(XEXP(operands[0],0)) == PLUS
			&& REG_P(XEXP(XEXP(operands[0],0),0)) )
			dreg = REGNO(XEXP(XEXP(operands[0],0),0));
	}


	sreg = REGNO(XEXP(operands[1],0));

	while (i < m)
	{
		p = template + strlen (cmd);

		strcpy (template, cmd);
		strcat (template, "\t");
		p += 1;
		strcat (template, "@%E1+, ");
		p += 7;

		if(dreg==sreg)
		{
			*p = '-'; p++;
			*p = '2'; p++;
			*p = '+'; p++;
		}

		*p = '%'; p++;
		*p = 'A' + ((dreg==sreg)?0:i);

		p++;
		*p = 0;
		strcat (template, "0");
		p += 1;
		output_asm_insn (template, operands);
		i++;
	}

	if (!dead_or_set_p (insn, reg))
	{
		len++;
		p = template + 3;
		strcpy (template, "sub");
		strcat (template, "\t#");
		p += 2;
		*p = '0' + m * 2;
		p++;
		*p = 0;
		strcat (template, ",    %E1	;	restore %E1");
		output_asm_insn (template, operands);
	}

	return len;
}

const char *
msp430_emit_indexed_mov2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	msp430_emit_indexed_mov (insn, operands, 2, "mov");
	return "";
}

const char *
msp430_emit_indexed_mov4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	msp430_emit_indexed_mov (insn, operands, 4, "mov");
	return "";
}

const char *
movsisf_regmode (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	rtx dest = operands[0];
	rtx src = operands[1];
	rtx areg = XEXP (src, 0);
	int src_reg = true_regnum (areg);
	int dst_reg = true_regnum (dest);


	if (dst_reg > src_reg || dst_reg + 1 < src_reg)
	{
		output_asm_insn ("mov\t@%E1+, %A0", operands);
		output_asm_insn ("mov\t@%E1+, %B0", operands);
		if (!dead_or_set_p (insn, areg))
		{
			output_asm_insn ("sub\t#4, %E1\t;\trestore %E1", operands);
		}
		return "";
	}
	else if (dst_reg + 1 == src_reg)
	{
		output_asm_insn ("mov\t@%E1+, %A0", operands);
		output_asm_insn ("mov\t@%E1, %B0", operands);
		return "";
	}
	else
	{
		/* destination overlaps with source.
		so, update destination in reverse way */
		output_asm_insn ("mov\t%B1, %B0", operands);
		output_asm_insn ("mov\t@%E1, %A0", operands);
	}

	return "";			/* make compiler happy */
}


/* From Max Behensky <maxb@twinlanes.com>  
This function tells you what the index register in an operand is.  It
returns the register number, or -1 if it is not an indexed operand */
static int get_indexed_reg PARAMS ((rtx));
static int
get_indexed_reg (x)
rtx x;
{
	int code;

	code = GET_CODE (x);

	if (code != MEM)
		return (-1);

	x = XEXP (x, 0);
	code = GET_CODE (x);
	if (code == REG)
		return (REGNO (x));

	if (code != PLUS)
		return (-1);

	x = XEXP (x, 0);
	code = GET_CODE (x);
	if (code != REG)
		return (-1);

	return (REGNO (x));
}


const char *
msp430_movesi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op1 = operands[1];


	if (memory_operand (op0, VOIDmode)
		&& memory_operand (op1, VOIDmode) && zero_shifted (op1))
	{
		if (!len)
			msp430_emit_indexed_mov2 (insn, operands, NULL);
		else
			*len = 5;
		return "";
	}	
	else if (register_operand (op0, VOIDmode)
		&& memory_operand (op1, VOIDmode) && zero_shifted (op1))
	{
		if (!len)
			movsisf_regmode (insn, operands, NULL);
		else
			*len = 3;
		return "";
	}

	if (!len)
	{
		if ((register_operand (op0, VOIDmode)
			&& register_operand (op1, VOIDmode)
			&& REGNO (op1) + 1 == REGNO (op0))
			|| (register_operand (op0, VOIDmode)
			&& memory_operand (op1, VOIDmode)
			&& get_indexed_reg (op1) == true_regnum (op0)))
		{
			output_asm_insn ("mov\t%B1, %B0", operands);
			output_asm_insn ("mov\t%A1, %A0", operands);
		}
		else
		{
			output_asm_insn ("mov\t%A1, %A0", operands);
			output_asm_insn ("mov\t%B1, %B0", operands);
		}
	}
	else
	{
		*len = 2;			/* base length */

		if (register_operand (op0, VOIDmode))
			*len += 0;
		else if (memory_operand (op0, VOIDmode))
			*len += 2;

		if (register_operand (op1, VOIDmode))
			*len += 0;
		else if (memory_operand (op1, VOIDmode))
			*len += 2;
		else if (immediate_operand (op1, VOIDmode))
			*len += 2;
	}

	return "";
}

const char *
movdidf_regmode (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	rtx dest = operands[0];
	rtx src = operands[1];
	rtx areg = XEXP (src, 0);

	int src_reg = true_regnum (areg);
	int dst_reg = true_regnum (dest);


	if (dst_reg > src_reg || dst_reg + 3 < src_reg)
	{
		output_asm_insn ("mov\t@%E1+, %A0", operands);
		output_asm_insn ("mov\t@%E1+, %B0", operands);
		output_asm_insn ("mov\t@%E1+, %C0", operands);
		output_asm_insn ("mov\t@%E1+, %D0", operands);
		if (!dead_or_set_p (insn, areg))
		{
			output_asm_insn ("sub\t#8, %E1\t;\trestore %E1", operands);
		}
	}
	else if (dst_reg + 3 == src_reg)
	{
		output_asm_insn ("mov\t@%E1+, %A0", operands);
		output_asm_insn ("mov\t@%E1+, %B0", operands);
		output_asm_insn ("mov\t@%E1+, %C0", operands);
		output_asm_insn ("mov\t@%E1,  %D0	;	%E1 == %D0", operands);
	}
	else
	{
		/* destination overlaps source.
		so, update destination in reverse way */
		output_asm_insn ("mov\t%D1, %D0	; %E1 overlaps wit one of %A0 - %D0",
			operands);
		output_asm_insn ("mov\t%C1, %C0", operands);
		output_asm_insn ("mov\t%B1, %B0", operands);
		output_asm_insn ("mov\t@%E1, %A0", operands);
	}

	return "";
}

const char *
msp430_movedi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op1 = operands[1];

	if (memory_operand (op0, DImode)
		&& memory_operand (op1, DImode) && zero_shifted (op1))
	{
		if (!len)
			msp430_emit_indexed_mov4 (insn, operands, NULL);
		else
			*len = 9;
		return "";
	}
	else if (register_operand (op0, DImode)
		&& memory_operand (op1, DImode) && zero_shifted (op1))
	{
		if (!len)
			movdidf_regmode (insn, operands, NULL);
		else
			*len = 5;
		return "";
	}

	if (!len)
	{
		if (register_operand (op0, SImode)
			&& register_operand (op1, SImode) && REGNO (op1) + 3 == REGNO (op0))
		{
			output_asm_insn ("mov\t%D1, %D0", operands);
			output_asm_insn ("mov\t%C1, %C0", operands);
			output_asm_insn ("mov\t%B1, %B0", operands);
			output_asm_insn ("mov\t%A1, %A0", operands);
		}
		else
		{
			output_asm_insn ("mov\t%A1, %A0", operands);
			output_asm_insn ("mov\t%B1, %B0", operands);
			output_asm_insn ("mov\t%C1, %C0", operands);
			output_asm_insn ("mov\t%D1, %D0", operands);
		}
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op1, DImode))
			*len += 0;
		else if (memory_operand (op1, DImode))
			*len += 4;
		else if (immediate_operand (op1, DImode))
			*len += 4;
	}

	return "";
}




/**************	ADD CODE *********************************/


const char *
msp430_emit_indexed_add2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 2, "add", 1);
	return "";
}

const char *
msp430_emit_indexed_add4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 4, "add", 1);
	return "";
}

const char *
msp430_addsi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];
	rtx ops[4];

	if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_add2 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, SImode))
				*len = 5;
			else if (register_operand (op0, SImode))
				*len = 3;
		}
		return "";
	}
	else if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("add\t@%E2+, %A0", operands);
			output_asm_insn ("addc\t@%E2+, %B0", operands);
		}
		else
		{
			if (register_operand (op0, SImode))
				*len = 2;
			else if (memory_operand (op0, SImode))
				*len = 4;
			else
				abort ();
		}
		return "";
	}

	ops[0] = operands[0];
	ops[2] = operands[2];

	if (!len)
	{
		output_asm_insn ("add\t%A2, %A0", ops);
		output_asm_insn ("addc\t%B2, %B0", ops);
	}

	if (len)
	{
		*len = 2;			/* base length */

		if (register_operand (ops[0], SImode))
			*len += 0;
		else if (memory_operand (ops[0], SImode))
			*len += 2;

		if (register_operand (ops[2], SImode))
			*len += 0;
		else if (memory_operand (ops[2], SImode))
			*len += 2;
		else if (immediate_operand (ops[2], SImode))
		{
			int x = INTVAL (ops[2]);
			if (x == -2 || x == -4 || x == -8)
			{
				*len += 1;
			}
			else
				*len += 2;
		}
	}
	return "";
}

const char *
msp430_adddi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_add4 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, DImode))
				*len = 9;
			else if (register_operand (op0, DImode))
				*len = 5;
		}

		return "";
	}
	else if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("add\t@%E2+, %A0", operands);
			output_asm_insn ("addc\t@%E2+, %B0", operands);
			output_asm_insn ("addc\t@%E2+, %C0", operands);
			output_asm_insn ("addc\t@%E2+, %D0", operands);
		}
		else
		{
			if (register_operand (op0, DImode))
				*len = 4;
			else if (memory_operand (op0, DImode))
				*len = 8;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("add\t%A2, %A0", operands);
		output_asm_insn ("addc\t%B2, %B0", operands);
		output_asm_insn ("addc\t%C2, %C0", operands);
		output_asm_insn ("addc\t%D2, %D0", operands);
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op2, DImode))
			*len += 0;
		else if (memory_operand (op2, DImode))
			*len += 4;
		else if (immediate_operand (op2, DImode))
		{
			int x = INTVAL (op2);

			if (x == -2 || x == -4 || x == -8)
				*len += 0;
			else
				*len += 4;
		}
		else
			abort ();
	}

	return "";
}


/**************	SUB CODE *********************************/

const char *
msp430_emit_indexed_sub2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 2, "sub", 1);
	return "";
}

const char *
msp430_emit_indexed_sub4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 4, "sub", 1);
	return "";
}

const char *
msp430_subsi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_sub2 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, SImode))
				*len = 5;
			else if (register_operand (op0, SImode))
				*len = 3;
		}

		return "";
	}
	else if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("sub\t@%E2+, %A0", operands);
			output_asm_insn ("subc\t@%E2+, %B0", operands);
		}
		else
		{
			if (register_operand (op0, SImode))
				*len = 2;
			else if (memory_operand (op0, SImode))
				*len = 4;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("sub\t%A2, %A0", operands);
		output_asm_insn ("subc\t%B2, %B0", operands);
	}
	else
	{
		*len = 2;			/* base length */

		if (register_operand (op0, SImode))
			*len += 0;
		else if (memory_operand (op0, SImode))
			*len += 2;

		if (register_operand (op2, SImode))
			*len += 0;
		else if (memory_operand (op2, SImode))
			*len += 2;
		else if (immediate_operand (op2, SImode))
			*len += 2;
	}

	return "";
}


const char *
msp430_subdi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_sub4 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, DImode))
				*len = 9;
			else if (register_operand (op0, DImode))
				*len = 5;
		}

		return "";
	}
	else if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("sub\t@%E2+, %A0", operands);
			output_asm_insn ("subc\t@%E2+, %B0", operands);
			output_asm_insn ("subc\t@%E2+, %C0", operands);
			output_asm_insn ("subc\t@%E2+, %D0", operands);
		}
		else
		{
			if (register_operand (op0, DImode))
				*len = 4;
			else if (memory_operand (op0, DImode))
				*len = 8;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("sub\t%A2, %A0", operands);
		output_asm_insn ("subc\t%B2, %B0", operands);
		output_asm_insn ("subc\t%C2, %C0", operands);
		output_asm_insn ("subc\t%D2, %D0", operands);
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op2, DImode))
			*len += 0;
		else if (memory_operand (op2, DImode))
			*len += 4;
		else if (immediate_operand (op2, DImode))
			*len += 4;
		else
			abort ();
	}

	return "";
}


/**************	AND CODE *********************************/

const char *
msp430_emit_indexed_and2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 2, "and", 0);
	return "";
}

const char *
msp430_emit_indexed_and4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 4, "and", 0);
	return "";
}

const char *
msp430_emit_immediate_and2 (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int v;
	int l = INTVAL (operands[2]);
	int r = REG_P (operands[0]);
	int list1 = ((~1) & 0xffff);
	int list2 = ((~2) & 0xffff);
	int list4 = ((~4) & 0xffff);
	int list8 = ((~8) & 0xffff);

	rtx op[4];

	op[0] = operands[0];
	op[1] = operands[1];
	op[2] = operands[2];

	/* check nibbles */

	v = (l) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %A0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %A0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	v = (l >> 16) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %B0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %B0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	if (len)
		*len = dummy;
	return "";
}

const char *
msp430_emit_immediate_and4 (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int v;
	int l = CONST_DOUBLE_LOW (operands[2]);
	int h = CONST_DOUBLE_HIGH (operands[2]);
	int r = REG_P (operands[0]);
	int list1 = ((~1) & 0xffff);
	int list2 = ((~2) & 0xffff);
	int list4 = ((~4) & 0xffff);
	int list8 = ((~8) & 0xffff);
	rtx op[4];

	op[0] = operands[0];
	op[1] = operands[1];
	op[2] = operands[2];

	/* check if operand 2 is really const_double */
	if (GET_CODE (operands[2]) == CONST_INT)
	{
		l = INTVAL (operands[2]);
		h = 0;
	}

	/* check nibbles */
	v = (l) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %A0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %A0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	v = (l >> 16) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %B0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %B0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	v = (h) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %C0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %C0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	v = (h >> 16) & 0xffff;
	if (v != 0xffff)
	{
		if (v == list1 || v == list2 || v == list4 || v == list8)
		{
			op[2] = gen_rtx_CONST_INT (SImode, ~v);
			OUT_INSN (len, "bic\t%A2, %D0", op);
			dummy++;
			if (!r)
				dummy++;
		}
		else
		{
			op[2] = gen_rtx_CONST_INT (SImode, v);
			OUT_INSN (len, "and\t%A2, %D0", op);
			dummy++;
			dummy++;
			if (!r)
				dummy++;
			if (v == 0 || v == 1 || v == 2 || v == 4 || v == 8)
				dummy--;
		}
	}

	if (len)
		*len = dummy;
	return "";
}

const char *
msp430_andsi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (nonimmediate_operand (op0, SImode) && immediate_operand (op2, SImode))
	{
		if (!len)
			msp430_emit_immediate_and2 (insn, operands, NULL);
		return "";
	}

	if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_and2 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, SImode))
				*len = 5;
			else if (register_operand (op0, SImode))
				*len = 3;
		}

		return "";
	}
	else if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("and\t@%E2+, %A0", operands);
			output_asm_insn ("and\t@%E2+, %B0", operands);
		}
		else
		{
			if (register_operand (op0, SImode))
				*len = 2;
			else if (memory_operand (op0, SImode))
				*len = 4;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("and\t%A2, %A0", operands);
		output_asm_insn ("and\t%B2, %B0", operands);
	}
	else
	{
		*len = 2;			/* base length */

		if (register_operand (op0, SImode))
			*len += 0;
		else if (memory_operand (op0, SImode))
			*len += 2;

		if (register_operand (op2, SImode))
			*len += 0;
		else if (memory_operand (op2, SImode))
			*len += 2;
		else if (immediate_operand (op2, SImode))
			*len += 2;
	}

	return "";
}


const char *
msp430_anddi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (nonimmediate_operand (op0, DImode) && immediate_operand (op2, DImode))
	{
		if (!len)
			msp430_emit_immediate_and4 (insn, operands, NULL);
		return "";
	}

	if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_and4 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, DImode))
				*len = 9;
			else if (register_operand (op0, DImode))
				*len = 5;
		}

		return "";
	}
	else if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("and\t@%E2+, %A0", operands);
			output_asm_insn ("and\t@%E2+, %B0", operands);
			output_asm_insn ("and\t@%E2+, %C0", operands);
			output_asm_insn ("and\t@%E2+, %D0", operands);
		}
		else
		{
			if (register_operand (op0, DImode))
				*len = 4;
			else if (memory_operand (op0, DImode))
				*len = 8;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("and\t%A2, %A0", operands);
		output_asm_insn ("and\t%B2, %B0", operands);
		output_asm_insn ("and\t%C2, %C0", operands);
		output_asm_insn ("and\t%D2, %D0", operands);
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op2, DImode))
			*len += 0;
		else if (memory_operand (op2, DImode))
			*len += 4;
		else if (immediate_operand (op2, DImode))
			*len += 4;
		else
			abort ();
	}

	return "";
}

/**************	IOR CODE *********************************/

const char *
msp430_emit_indexed_ior2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 2, "bis", 0);
	return "";
}

const char *
msp430_emit_indexed_ior4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	emit_indexed_arith (insn, operands, 4, "bis", 0);
	return "";
}

const char *
msp430_emit_immediate_ior2 (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int l = INTVAL (operands[2]);
	int r = REG_P (operands[0]);
	int v;


	v = l & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%A2,%A0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	v = (l >> 16) & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%B2,%B0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	if (len)
		*len = dummy;
	return "";
}

const char *
msp430_emit_immediate_ior4 (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int l = CONST_DOUBLE_LOW (operands[2]);
	int h = CONST_DOUBLE_HIGH (operands[2]);
	int r = REG_P (operands[0]);
	int v;

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		l = INTVAL (operands[2]);
		h = 0;
	}

	v = l & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%A2,%A0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	v = (l >> 16) & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%B2,%B0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	l = h;
	v = l & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%C2,%C0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	v = (l >> 16) & 0xffff;

	if (v)
	{
		OUT_INSN (len, "bis\t%D2,%D0", operands);
		dummy++;
		dummy++;
		if (v == 0xffff || v == 1 || v == 2 || v == 4 || v == 8)
			dummy--;
		if (!r)
			dummy++;
	}

	if (len)
		*len = dummy;
	return "";
}

const char *
msp430_iorsi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (nonimmediate_operand (op0, SImode) && immediate_operand (op2, SImode))
	{
		if (!len)
			msp430_emit_immediate_ior2 (insn, operands, NULL);
		return "";
	}

	if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_ior2 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, SImode))
				*len = 5;
			else if (register_operand (op0, SImode))
				*len = 3;
		}

		return "";
	}
	else if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("bis\t@%E2+, %A0", operands);
			output_asm_insn ("bis\t@%E2+, %B0", operands);
		}
		else
		{
			if (register_operand (op0, SImode))
				*len = 2;
			else if (memory_operand (op0, SImode))
				*len = 4;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("bis\t%A2, %A0", operands);
		output_asm_insn ("bis\t%B2, %B0", operands);
	}
	else
	{
		*len = 2;			/* base length */

		if (register_operand (op0, SImode))
			*len += 0;
		else if (memory_operand (op0, SImode))
			*len += 2;

		if (register_operand (op2, SImode))
			*len += 0;
		else if (memory_operand (op2, SImode))
			*len += 2;
		else if (immediate_operand (op2, SImode))
			*len += 2;
	}

	return "";
}

const char *
msp430_iordi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (nonimmediate_operand (op0, DImode) && immediate_operand (op2, DImode))
	{
		if (!len)
			msp430_emit_immediate_ior4 (insn, operands, NULL);
		return "";
	}

	if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_ior4 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, DImode))
				*len = 9;
			else if (register_operand (op0, DImode))
				*len = 5;
		}

		return "";
	}
	else if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("bis\t@%E2+, %A0", operands);
			output_asm_insn ("bis\t@%E2+, %B0", operands);
			output_asm_insn ("bis\t@%E2+, %C0", operands);
			output_asm_insn ("bis\t@%E2+, %D0", operands);
		}
		else
		{
			if (register_operand (op0, DImode))
				*len = 4;
			else if (memory_operand (op0, DImode))
				*len = 8;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("bis\t%A2, %A0", operands);
		output_asm_insn ("bis\t%B2, %B0", operands);
		output_asm_insn ("bis\t%C2, %C0", operands);
		output_asm_insn ("bis\t%D2, %D0", operands);
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op2, DImode))
			*len += 0;
		else if (memory_operand (op2, DImode))
			*len += 4;
		else if (immediate_operand (op2, DImode))
			*len += 4;
		else
			abort ();
	}

	return "";
}


/************************* XOR CODE *****************/

const char *
msp430_emit_indexed_xor2 (insn, operands, l)
rtx insn;
rtx operands[];
int *l;
{
	int dummy = emit_indexed_arith (insn, operands, 2, "xor", 0);
	if (!l)
		l = &dummy;
	*l = dummy;
	return "";
}

const char *
msp430_emit_indexed_xor4 (insn, operands, l)
rtx insn;
rtx operands[];
int *l;
{
	int dummy = emit_indexed_arith (insn, operands, 4, "xor", 0);
	if (!l)
		l = &dummy;
	*l = dummy;
	return "";
}


const char *
msp430_emit_indexed_xor2_3 (insn, operands, l)
rtx insn;
rtx operands[];
int *l;
{
	int dummy;
	rtx x = operands[2];
	if (zero_shifted (x))
	{
		dummy = emit_indexed_arith (insn, operands, 2, "xor", 0);
	}
	else
	{
		dummy = 6;
		output_asm_insn ("xor\t%A2, %A0", operands);
		output_asm_insn ("xor\t%B2, %B0", operands);
	}

	if (!l)
		l = &dummy;
	*l = dummy;
	return "";
}

const char *
msp430_emit_indexed_xor4_3 (insn, operands, l)
rtx insn;
rtx operands[];
int *l;
{

	int dummy;
	rtx x = operands[2];
	if (zero_shifted (x))
	{
		dummy = emit_indexed_arith (insn, operands, 4, "xor", 0);
	}
	else
	{
		dummy = 8;
		output_asm_insn ("xor\t%A2, %A0", operands);
		output_asm_insn ("xor\t%B2, %B0", operands);
		output_asm_insn ("xor\t%C2, %C0", operands);
		output_asm_insn ("xor\t%D2, %D0", operands);
	}

	if (!l)
		l = &dummy;
	*l = dummy;
	return "";
}

const char *
msp430_xorsi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_xor2 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, SImode))
				*len = 5;
			else if (register_operand (op0, SImode))
				*len = 3;
		}

		return "";
	}
	else if (memory_operand (op2, SImode)
		&& zero_shifted (operands[2]) && regsi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("xor\t@%E2+, %A0", operands);
			output_asm_insn ("xor\t@%E2+, %B0", operands);
		}
		else
		{
			if (register_operand (op0, SImode))
				*len = 2;
			else if (memory_operand (op0, SImode))
				*len = 4;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{

		if (immediate_operand (op2, SImode))
		{
			if (INTVAL (op2) & 0xfffful)
				output_asm_insn ("xor\t%A2, %A0", operands);

			if (INTVAL (op2) & 0xffff0000ul)
				output_asm_insn ("xor\t%B2, %B0", operands);
		}
		else
		{
			output_asm_insn ("xor\t%A2, %A0", operands);
			output_asm_insn ("xor\t%B2, %B0", operands);
		}

	}
	else
	{
		*len = 2;			/* base length */

		if (register_operand (op0, SImode))
			*len += 0;
		else if (memory_operand (op0, SImode))
			*len += 2;

		if (register_operand (op2, SImode))
			*len += 0;
		else if (memory_operand (op2, SImode))
			*len += 2;
		else if (immediate_operand (op2, SImode))
		{
			if (INTVAL (op2) & 0xfffful)
				*len += 1;
			if (INTVAL (op2) & 0xffff0000ul)
				*len += 1;
		}
	}

	return "";
}

const char *
msp430_xordi_code (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	rtx op0 = operands[0];
	rtx op2 = operands[2];

	if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_safe (operands))
	{
		if (!len)
			msp430_emit_indexed_xor4 (insn, operands, NULL);
		else
		{
			if (memory_operand (op0, DImode))
				*len = 9;
			else if (register_operand (op0, DImode))
				*len = 5;
		}

		return "";
	}
	else if (memory_operand (op2, DImode)
		&& zero_shifted (operands[2]) && regdi_ok_clobber (operands))
	{
		if (!len)
		{
			output_asm_insn ("xor\t@%E2+, %A0", operands);
			output_asm_insn ("xor\t@%E2+, %B0", operands);
			output_asm_insn ("xor\t@%E2+, %C0", operands);
			output_asm_insn ("xor\t@%E2+, %D0", operands);
		}
		else
		{
			if (register_operand (op0, DImode))
				*len = 4;
			else if (memory_operand (op0, DImode))
				*len = 8;
			else
				abort ();
		}
		return "";
	}

	if (!len)
	{
		output_asm_insn ("xor\t%A2, %A0", operands);
		output_asm_insn ("xor\t%B2, %B0", operands);
		output_asm_insn ("xor\t%C2, %C0", operands);
		output_asm_insn ("xor\t%D2, %D0", operands);
	}
	else
	{
		*len = 4;			/* base length */

		if (register_operand (op0, DImode))
			*len += 0;
		else if (memory_operand (op0, DImode))
			*len += 4;

		if (register_operand (op2, DImode))
			*len += 0;
		else if (memory_operand (op2, DImode))
			*len += 4;
		else if (immediate_operand (op2, DImode))
			*len += 4;
		else
			abort ();
	}

	return "";
}


/********* ABS CODE ***************************************/
const char *
msp430_emit_abssi (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	output_asm_insn ("tst\t%B0", operands);
	output_asm_insn ("jge\t.Lae%=", operands);
	output_asm_insn ("inv\t%A0", operands);
	output_asm_insn ("inv\t%B0", operands);
	output_asm_insn ("inc\t%A0", operands);
	output_asm_insn ("adc\t%B0", operands);
	output_asm_insn (".Lae%=:", operands);
	return "";
}

const char *
msp430_emit_absdi (insn, operands, l)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *l ATTRIBUTE_UNUSED;
{
	output_asm_insn ("tst\t%D0", operands);
	output_asm_insn ("jge\t.Lae%=", operands);
	output_asm_insn ("inv\t%A0", operands);
	output_asm_insn ("inv\t%B0", operands);
	output_asm_insn ("inv\t%C0", operands);
	output_asm_insn ("inv\t%D0", operands);
	output_asm_insn ("inc\t%A0", operands);
	output_asm_insn ("adc\t%B0", operands);
	output_asm_insn ("adc\t%C0", operands);
	output_asm_insn ("adc\t%D0", operands);
	output_asm_insn (".Lae%=:", operands);
	return "";
}


/***** SIGN EXTEND *********/
const char *
signextendqihi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (indexed_location (operands[1]))
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "sxt\t%A0", operands);
	dummy += 2;

	if (zs || GET_CODE (operands[0]) == REG)
		dummy -= 1;

	if (len)
		*len = dummy;

	return "";
}

const char *
signextendqisi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (indexed_location (operands[1]))
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "sxt\t%A0", operands);
	OUT_INSN (len, "mov\t%A0, %B0", operands);
	OUT_INSN (len, "rla\t%B0", operands);
	OUT_INSN (len, "subc\t%B0, %B0", operands);
	OUT_INSN (len, "inv\t%B0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy += 5;
	else if (zs)
		dummy += 10;
	else
		dummy += 12;

	if (len)
		*len = dummy;

	return "";
}

const char *
signextendqidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (indexed_location (operands[1]))
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "sxt\t%A0", operands);
	OUT_INSN (len, "mov\t%A0, %B0", operands);
	OUT_INSN (len, "rla\t%B0", operands);
	OUT_INSN (len, "subc\t%B0, %B0", operands);
	OUT_INSN (len, "inv\t%B0", operands);
	OUT_INSN (len, "mov\t%B0, %C0", operands);
	OUT_INSN (len, "mov\t%C0, %D0", operands);


	if (GET_CODE (operands[0]) == REG)
		dummy += 7;
	else if (zs)
		dummy += 16;
	else
		dummy += 18;

	if (len)
		*len = dummy;

	return "";
}

const char *
signextendhisi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov\t%A1, %A0", operands);
		dummy = 3;
		if (indexed_location (operands[1]))
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "mov\t%A0, %B0", operands);
	OUT_INSN (len, "rla\t%B0", operands);
	OUT_INSN (len, "subc\t%B0, %B0", operands);
	OUT_INSN (len, "inv\t%B0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy += 4;
	else if (zs)
		dummy += 9;
	else
		dummy += 11;

	if (len)
		*len = dummy;

	return "";
}

const char *
signextendhidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov\t%A1, %A0", operands);
		dummy = 3;
		if (indexed_location (operands[1]))
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "mov\t%A0, %B0", operands);
	OUT_INSN (len, "rla\t%B0", operands);
	OUT_INSN (len, "subc\t%B0, %B0", operands);
	OUT_INSN (len, "inv\t%B0", operands);
	OUT_INSN (len, "mov\t%B0, %C0", operands);
	OUT_INSN (len, "mov\t%C0, %D0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy += 6;
	else if (zs)
		dummy += 13;
	else
		dummy += 14;

	if (len)
		*len = dummy;

	return "";
}

const char *
signextendsidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov\t%A1, %A0", operands);
		OUT_INSN (len, "mov\t%B1, %B0", operands);
		dummy = 6;
		if (indexed_location (operands[1]))
			dummy = 4;
		if (GET_CODE (operands[0]) == REG)
			dummy -= 2;
		if (GET_CODE (operands[1]) == REG)
			dummy -= 2;
	}

	OUT_INSN (len, "mov\t%B0, %C0", operands);
	OUT_INSN (len, "rla\t%C0", operands);
	OUT_INSN (len, "subc\t%C0, %C0", operands);
	OUT_INSN (len, "inv\t%C0", operands);
	OUT_INSN (len, "mov\t%C0, %D0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy += 5;
	else
		dummy += 13;

	if (len)
		*len = dummy;

	return "";
}


/**** ZERO EXTEND *****/

const char *
zeroextendqihi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[1]) || indexed_location (operands[1]);

	if(operands[0] == op2_rtx)
	{
		OUT_INSN (len, "and	#0xff00, %0",operands);
		dummy = 3;
		return "";
	}
	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (zs)
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	if (!REG_P (operands[0]))
	{
		OUT_INSN (len, "clr.b\t%J0", operands);
		dummy += 2;
	}
	else if (sameoperand (operands, 1))
	{
		OUT_INSN (len, "and.b\t#-1,%0", operands);
		dummy++;
	}

	if (len)
		*len = dummy;

	return "";
}

const char *
zeroextendqisi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[1]) || indexed_location (operands[1]);

	if (!sameoperand (operands, 1) || REG_P (operands[0]))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (zs)
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}


	if (!REG_P (operands[0]))
	{
		OUT_INSN (len, "clr.b\t%J0", operands);
	}
	else if (sameoperand (operands, 1))
	{
		OUT_INSN (len, "and.b\t#-1,%0", operands);
		dummy++;
	}
	OUT_INSN (len, "clr\t%B0", operands);
	dummy += 2;
	if (GET_CODE (operands[0]) == REG)
		dummy--;

	if (len)
		*len = dummy;

	return "";
}


const char *
zeroextendqidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[1]) || indexed_location (operands[1]);

	if (!sameoperand (operands, 1) || REG_P (operands[0]))
	{
		OUT_INSN (len, "mov.b\t%A1, %A0", operands);
		dummy = 3;
		if (zs)
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	if (!REG_P (operands[0]))
	{
		OUT_INSN (len, "clr.b\t%J0", operands);
		dummy += 2;
	}
	else if (sameoperand (operands, 1))
	{
		OUT_INSN (len, "and.b\t#-1,%0", operands);
		dummy++;
	}
	dummy += 6;
	OUT_INSN (len, "clr\t%B0", operands);
	OUT_INSN (len, "clr\t%C0", operands);
	OUT_INSN (len, "clr\t%D0", operands);

	if (GET_CODE (operands[0]) == REG && len)
		*len -= 3;

	if (len)
		*len = dummy;

	return "";
}

const char *
zeroextendhisi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[1]) || indexed_location (operands[1]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov\t%A1, %A0", operands);
		dummy = 3;
		if (zs)
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	OUT_INSN (len, "clr\t%B0", operands);
	dummy += 2;
	if (GET_CODE (operands[0]) == REG)
		dummy--;

	if (len)
		*len = dummy;

	return "";

}

const char *
zeroextendhidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[1]) || indexed_location (operands[1]);

	if (!sameoperand (operands, 1))
	{
		OUT_INSN (len, "mov\t%A1, %A0", operands);
		dummy = 3;
		if (zs)
			dummy = 2;
		if (GET_CODE (operands[0]) == REG)
			dummy--;
		if (GET_CODE (operands[1]) == REG)
			dummy--;
	}

	dummy += 6;
	OUT_INSN (len, "clr\t%B0", operands);
	OUT_INSN (len, "clr\t%C0", operands);
	OUT_INSN (len, "clr\t%D0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy -= 3;

	if (len)
		*len = dummy;

	return "";
}

const char *
zeroextendsidi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int dummy = 0;

	if (!sameoperand (operands, 1))
	{
		if (zero_shifted (operands[1]))
		{
			rtx reg = XEXP (operands[1], 0);

			OUT_INSN (len, "mov\t@%E1+, %A0", operands);
			OUT_INSN (len, "mov\t@%E1+, %B0", operands);
			dummy = 4;
			if (GET_CODE (operands[0]) == REG)
				dummy -= 2;

			if (!dead_or_set_p (insn, reg))
			{
				OUT_INSN (len, "sub\t#4, %E1", operands);
				dummy += 1;
			}
		}
		else
		{
			OUT_INSN (len, "mov\t%A1, %A0", operands);
			OUT_INSN (len, "mov\t%B1, %B0", operands);
			dummy = 6;
			if (GET_CODE (operands[0]) == REG)
				dummy -= 2;
			if (GET_CODE (operands[1]) == REG)
				dummy -= 2;
			if (indexed_location (operands[1]))
				dummy--;
		}
	}

	dummy += 4;
	OUT_INSN (len, "clr\t%C0", operands);
	OUT_INSN (len, "clr\t%D0", operands);

	if (GET_CODE (operands[0]) == REG)
		dummy -= 2;

	if (len)
		*len = dummy;

	return "";
}

/******************* TESTS AND JUMPS *********************/


/*****************  AUXES FOR TESTS *********************/

RTX_CODE
followed_compare_condition (insn)
rtx insn;
{
	rtx next = next_real_insn (insn);
	RTX_CODE cond = UNKNOWN;

	if (next && GET_CODE (next) == JUMP_INSN)
	{
		rtx pat = PATTERN (next);
		rtx src, t;

		if (pat && GET_CODE (pat) == PARALLEL)
			pat = XVECEXP (pat, 0, 0);

		if (!pat || GET_CODE (pat) != SET)
			return UNKNOWN;

		src = SET_SRC (pat);
		t = XEXP (src, 0);
		cond = GET_CODE (t);
	}
	else if (next && GET_CODE (next) == INSN)
	{
		/* here, two possible : sgeu ans sltu */

		rtx pat = PATTERN (next);
		rtx src;

		if (!pat || GET_CODE (pat) != SET)
			return UNKNOWN;

		src = SET_SRC (pat);
		cond = GET_CODE (src);	/* this must be IF_THEN_ELSE */
		if (cond != IF_THEN_ELSE)
			return UNKNOWN;
	}
	return cond;
}


/* SHIFT GUARDS */
int
msp430_ashlhi3 (operands)
rtx operands[];
{
	int x;
	rtx set, shift;
	rtx dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (HImode, operands[0]);
		op1 = force_reg (HImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_ashlhi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);

	if (x > 15 || x < 0)
	{
		emit_move_insn (operands[0], const0_rtx);
		return 1;
	}

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (x < 3)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_ASHIFT (HImode, dst, const1_rtx);
		set = gen_rtx_SET (HImode, dst, shift);
		while (x--)
			emit_insn (set);
		return 1;
	}

	if (x == 15)
	{
		shift = gen_rtx_ASHIFT (HImode, operands[1], GEN_INT (15));
		set = gen_rtx_SET (HImode, operands[0], shift);
		emit_insn (set);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (HImode, operands[1]);
	else
		dst = operands[1];
	if (x > 7)
	{
		emit_insn (gen_andhi3 (dst, dst, GEN_INT (0xff)));
		emit_insn (gen_swpb (dst, dst));
		x -= 8;
	}

	shift = gen_rtx_ASHIFT (HImode, dst, const1_rtx);
	set = gen_rtx_SET (HImode, dst, shift);

	while (x--)
		emit_insn (set);
	if (dst != operands[0])
		emit_move_insn (operands[0], dst);
	return 1;
}

int
msp430_ashlsi3 (operands)
rtx operands[];
{
	int x;
	rtx shift, set, dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (SImode, operands[0]);
		op1 = force_reg (SImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_ashlsi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);

	if (x >= 32 || x < 0)
	{
		emit_move_insn (operands[0], const0_rtx);
		return 1;
	}

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (x == 1)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_ASHIFT (SImode, dst, operands[2]);
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (SImode, operands[1]);
	else
		dst = operands[1];

	if (x == 31)
	{
		shift = gen_rtx_ASHIFT (SImode, operands[1], GEN_INT (31));
		set = gen_rtx_SET (SImode, operands[0], shift);
		emit_insn (set);
		return 1;
	}

	if (x >= 16)
	{
		rtx dhi = gen_highpart (HImode, operands[0]);
		rtx dlo = gen_lowpart (HImode, operands[0]);
		rtx shi = gen_highpart (HImode, operands[1]);
		rtx slo = gen_lowpart (HImode, operands[1]);

		emit_move_insn (dhi, slo);
		emit_move_insn (dlo, const0_rtx); 
		x -= 16;
		if (x)
		{
			rtx ops[3];
			ops[0] = dhi;
			ops[1] = dhi;
			ops[2] = GEN_INT (x);
			msp430_ashlhi3 (ops);
		}
		return 1;
	}

	if (x >= 8)
	{
		shift = gen_rtx_ASHIFT (SImode, dst, GEN_INT (8));
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		x -= 8;
	}

	shift = gen_rtx_ASHIFT (SImode, dst, GEN_INT (1));
	set = gen_rtx_SET (SImode, dst, shift);

	while (x--)
		emit_insn (set);
	if (dst != operands[0])
		emit_move_insn (operands[0], dst);
	return 1;
}

/* arithmetic right */
int
msp430_ashrhi3 (operands)
rtx operands[];
{
	int x;
	rtx shift, set, dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (HImode, operands[0]);
		op1 = force_reg (HImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_ashrhi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);
	if (x >= 15 || x < 0)
	{
		dst = gen_lowpart (QImode, operands[0]);
		emit_move_insn (operands[0], operands[1]);
		emit_insn (gen_swpb (operands[0], operands[0]));
		emit_insn (gen_extendqihi2 (operands[0], dst));
		emit_insn (gen_swpb (operands[0], operands[0]));
		emit_insn (gen_extendqihi2 (operands[0], dst));
		return 1;
	}

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (x < 3)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_ASHIFTRT (HImode, dst, const1_rtx);
		set = gen_rtx_SET (HImode, dst, shift);

		while (x--)
			emit_insn (set);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (HImode, operands[1]);
	else
		dst = operands[1];

	if (x >= 8)
	{
		rtx dlo = gen_lowpart (QImode, dst);
		emit_insn (gen_swpb (dst, dst));
		emit_insn (gen_extendqihi2 (dst, dlo));
		x -= 8;
	}

	shift = gen_rtx_ASHIFTRT (HImode, dst, const1_rtx);
	set = gen_rtx_SET (HImode, dst, shift);

	while (x--)
		emit_insn (set);

	if (dst != operands[0])
		emit_move_insn (operands[0], dst);

	return 1;
}

int
msp430_ashrsi3 (operands)
rtx operands[];
{
	int x;
	rtx shift, set, dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (SImode, operands[0]);
		op1 = force_reg (SImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_ashrsi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (SImode, operands[1]);
	else
		dst = operands[1];

	if (x >= 31 || x < 0)
	{

		shift = gen_rtx_ASHIFTRT (SImode, dst, GEN_INT (31));
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);

		if (dst != operands[0])
			emit_move_insn (operands[0], dst);
		return 1;
	}

	if (x == 1)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_ASHIFTRT (SImode, dst, operands[2]);
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		return 1;
	}

	if (x >= 16)
	{
		rtx dlo = gen_lowpart (HImode, operands[0]);
		rtx shi = gen_highpart (HImode, dst);

		emit_move_insn (gen_highpart (HImode, operands[0]), const0_rtx);
		emit_insn (gen_extendhisi2 (operands[0], shi));
		x -= 16;
		if (x)
		{
			rtx ops[3];
			ops[0] = dlo;
			ops[1] = dlo;
			ops[2] = GEN_INT (x);
			msp430_ashrhi3 (ops);
		}
		return 1;
	}

	if (x >= 8)
	{
		shift = gen_rtx_ASHIFTRT (SImode, dst, GEN_INT (8));
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		x -= 8;
	}

	shift = gen_rtx_ASHIFTRT (SImode, dst, GEN_INT (1));
	set = gen_rtx_SET (SImode, dst, shift);

	while (x--)
		emit_insn (set);
	if (dst != operands[0])
		emit_move_insn (operands[0], dst);
	return 1;
}

/* logical right */
int
msp430_lshrhi3 (operands)
rtx operands[];
{
	int x;
	rtx shift, set, dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (HImode, operands[0]);
		op1 = force_reg (HImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_lshrhi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);
	if (x > 15 || x < 0)
	{
		emit_move_insn (operands[0], const0_rtx);
		return 1;
	}

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (x < 3)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_LSHIFTRT (HImode, dst, const1_rtx);
		set = gen_rtx_SET (HImode, dst, shift);
		emit_insn (set);
		x--;

		if (x)
		{
			shift = gen_rtx_ASHIFTRT (HImode, dst, const1_rtx);
			set = gen_rtx_SET (HImode, dst, shift);
			emit_insn (set);
		}
		return 1;
	}

	if (x == 15)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_LSHIFTRT (HImode, dst, GEN_INT (15));
		set = gen_rtx_SET (HImode, dst, shift);
		emit_insn (set);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (HImode, operands[1]);
	else
		dst = operands[1];

	if (x >= 8)
	{
		rtx dlo = gen_lowpart (QImode, dst);
		emit_insn (gen_swpb (dst, dst));
		emit_insn (gen_zero_extendqihi2 (dst, dlo));
		x -= 8;
	}

	if (x)
	{
		shift = gen_rtx_LSHIFTRT (HImode, dst, const1_rtx);
		set = gen_rtx_SET (HImode, dst, shift);
		x--;
		emit_insn (set);
	}
	shift = gen_rtx_ASHIFTRT (HImode, dst, const1_rtx);
	set = gen_rtx_SET (HImode, dst, shift);

	while (x--)
		emit_insn (set);

	if (dst != operands[0])
		emit_move_insn (operands[0], dst);

	return 1;
}

int
msp430_lshrsi3 (operands)
rtx operands[];
{
	int x;
	rtx shift, set, dst;

	if (!const_int_operand (operands[2], VOIDmode))
	{
		rtx op0, op1;

		op0 = force_reg (SImode, operands[0]);
		op1 = force_reg (SImode, operands[1]);
		operands[2] = copy_to_mode_reg (HImode, operands[2]);
		emit_insn (gen_lshrsi3_cnt (op0, op1, operands[2]));
		emit_move_insn (operands[0], op0);
		return 1;
	}

	x = INTVAL (operands[2]);

	if (x == 0)
	{
		emit_move_insn (operands[0], operands[1]);
		return 1;
	}

	if (x == 1)
	{
		emit_move_insn (operands[0], operands[1]);
		dst = operands[0];
		shift = gen_rtx_LSHIFTRT (SImode, dst, operands[2]);
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		return 1;
	}

	if (x > 31 || x < 0)
	{
		emit_move_insn (operands[0], const0_rtx);
		return 1;
	}

	if (operands[0] != operands[1])
		dst = copy_to_mode_reg (SImode, operands[1]);
	else
		dst = operands[1];

	if (x >= 16)
	{
		rtx dlo = gen_lowpart (HImode, operands[0]);
		rtx shi = gen_highpart (HImode, dst);

		emit_move_insn (gen_highpart (HImode, operands[0]), const0_rtx);
		emit_insn (gen_zero_extendhisi2 (operands[0], shi));
		x -= 16;
		if (x)
		{
			rtx ops[3];
			ops[0] = dlo;
			ops[1] = dlo;
			ops[2] = GEN_INT (x);
			msp430_lshrhi3 (ops);
		}
		return 1;
	}

	if (x >= 8)
	{
		shift = gen_rtx_LSHIFTRT (SImode, dst, GEN_INT (8));
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		x -= 8;
	}

	if (x)
	{
		shift = gen_rtx_LSHIFTRT (SImode, dst, const1_rtx);
		set = gen_rtx_SET (SImode, dst, shift);
		emit_insn (set);
		x--;
	}

	shift = gen_rtx_ASHIFTRT (SImode, dst, GEN_INT (1));
	set = gen_rtx_SET (SImode, dst, shift);

	while (x--)
		emit_insn (set);
	if (dst != operands[0])
		emit_move_insn (operands[0], dst);
	return 1;
}

/******* COMMON SHIFT CODE ***************/
int
is_shift_better_in_reg (operands)
rtx operands[];
{
	rtx x = operands[0];
	rtx cnt = operands[2];
	int size = GET_MODE_SIZE (x->mode);
	int icnt = -1;
	int r = 0;

	if (!optimize)
		return 0;

	if (GET_CODE (cnt) == CONST_INT)
		icnt = INTVAL (cnt);
	else
		return 1;

	switch (size)
	{
	case 1:
		if (icnt != 1 && icnt != 2 && icnt != 7)
			r = 1;
		break;
	case 2:
		if (icnt != 1 && icnt != 2 && icnt != 8 && icnt != 15)
			r = 2;
		break;
	case 4:
		if (icnt != 1
			&& icnt != 2 && icnt != 8 && icnt != 16 && icnt != 24 && icnt != 31)
			r = 4;
		break;
	case 8:
		if (icnt != 1
			&& icnt != 2 && icnt != 16 && icnt != 32 && icnt != 48 && icnt != 63)
			r = 8;
		break;
	}

	return r;
}


static int set_len PARAMS ((rtx, int, int));
/* for const operand2 and for SI, DI modes.*/
static int
set_len (x, bl, sc)
rtx x;			/* operand0 */
int bl;			/* base length in assumption of memory operand */
int sc;			/* shift count */
{
	int dummy;
	int zs = zero_shifted (x);
	int size = GET_MODE_SIZE (x->mode);
	int sshi = 0;

	if (size == 4)
		sshi = 1;
	else if (size == 8)
		sshi = 2;

	if (size == 1)
		size++;

	if (GET_CODE (x) == REG)
		dummy = (bl >> 1) - sshi;	/* bl / 2 is not fully correct */
	else if (zs)
		dummy = bl - (size >> 1) + 1;
	else if (indexed_location (x))
		dummy = bl - 1;
	else
		dummy = bl;

	return dummy * sc;
}

static int set_ren PARAMS ((rtx, int, int));
/* for const operand2 and for SI, DI modes.*/
static int
set_ren (x, bl, sc)
rtx x;			/* operand0 */
int bl;			/* base length in assumption of memory operand */
int sc;			/* shift count */
{
	int dummy;

	bl *= sc;
	if (GET_CODE (x) == REG)
		dummy = bl / 2;
	else if (indexed_location (x))
		dummy = bl - sc;
	else
		dummy = bl;
	return dummy;
}

static int set_rel PARAMS ((rtx, int, int));
/* for const operand2 and for SI, DI modes.*/
static int
set_rel (x, bl, sc)
rtx x;			/* operand0 */
int bl;			/* base length in assumption of memory operand */
int sc;			/* shift count */
{
	int dummy;

	bl *= sc;
	if (GET_CODE (x) == REG)
		dummy = bl / 2;
	else if (indexed_location (x))
		dummy = bl - sc;
	else
		dummy = bl;
	dummy += sc;
	return dummy;
}



#define INST_THRESHOLD  16

int
msp430_emit_shift_cnt (set_len_fun, pattern, insn, operands, len, lsc)
int (*set_len_fun) (rtx, int, int);
const char *pattern;
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
int lsc;
{
	rtx op[10];
	int dummy = 0;

	op[0] = operands[0];
	op[1] = operands[1];
	op[2] = operands[2];
	op[3] = operands[3];


	OUT_INSN (len, "tst\t%2", op);
	OUT_INSN (len, "jz\t.Lsend%=\n.Lsst%=:", op);
	OUT_INSN (len, pattern, op);
	OUT_INSN (len, "dec\t%2", op);
	OUT_INSN (len, "jnz\t.Lsst%=\n.Lsend%=:", op);
	dummy = (set_len_fun) (op[0], lsc, 1) + 4;
	if (!REG_P (op[2]) && !indexed_location (op[2]))
		dummy += 2;


	if (len)
		*len = dummy;
	return 0;
}


/* <<<<<<<<<<<<< SHIFT LEFT CODE <<<<<<<<<<<<<<<<<     */

const char *
msp430_emit_ashlqi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *pattern;
	int shiftpos;

	if (zs)
		pattern = "rla.b\t@%E0";
	else
		pattern = "rla.b\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		default:
			if (zs)
				OUT_INSN (len, "clr.b\t@%E0", operands);
			else
				OUT_INSN (len, "clr.b\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy >>= 1;
			break;

		case 0:		/* paranoia setting */
			dummy = 0;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 3, 1);
			}
			break;

		case 7:
			if (zs)
			{
				OUT_INSN (len, "rra.b\t%0", operands);
				OUT_INSN (len, "clr.b\t%0", operands);
				OUT_INSN (len, "rrc.b\t%0", operands);
				dummy = 5;
			}
			else
			{
				OUT_INSN (len, "rra.b\t%0", operands);
				OUT_INSN (len, "clr.b\t%0", operands);
				OUT_INSN (len, "rrc.b\t%0", operands);
				dummy = 6;
				if (REG_P (operands[0]))
					dummy = 3;
			}

			break;
		}

		if (len)
			*len = dummy;
		return "";

	}
	else
	{
		msp430_emit_shift_cnt (set_len, pattern, insn, operands, len, 3);
	}

	return "";
}


const char *
msp430_emit_ashlhi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs;
	const char *pattern;
	int shiftpos;

	zs = zero_shifted (operands[0]) || indexed_location (operands[0]);

	if (zs)
		pattern = "rla\t@%E0";
	else
		pattern = "rla\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:		/* paranoia setting */
			dummy = 0;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 3, 1);
			}
			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			if (zs)
			{
				dummy = 3;
				OUT_INSN (len, "and.b\t#0xffff, %A0", operands);
				OUT_INSN (len, "swpb\t@%E0", operands);
			}
			else
			{
				dummy = 4;
				OUT_INSN (len, "and.b\t#0xffff, %A0", operands);
				OUT_INSN (len, "swpb\t%A0", operands);
				if (REG_P (operands[0]))
					dummy = 2;
			}


			shiftpos -= 8;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 3, 1);
			}
			break;

		case 15:
			if (zs)
			{
				OUT_INSN (len, "rra\t%0", operands);
				OUT_INSN (len, "clr\t%0", operands);
				OUT_INSN (len, "rrc\t%0", operands);
				dummy = 5;
			}
			else
			{
				OUT_INSN (len, "rra\t%0", operands);
				OUT_INSN (len, "clr\t%0", operands);
				OUT_INSN (len, "rrc\t%0", operands);
				dummy = 6;
				if (REG_P (operands[0]))
					dummy = 3;
			}

			break;


		default:

			OUT_INSN (len, "clr\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy = 1;
			break;
		}

		if (len)
			*len = dummy;
		return "";
	}
	else
	{
		msp430_emit_shift_cnt (set_len, pattern, insn, operands, len, 3);
	}

	return "";
}


const char *
msp430_emit_ashlsi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{

	int dummy = 0;
	int zs;
	const char *pattern;

	zs = zero_shifted (operands[0]);

	if (zs)
		pattern = "add\t@%E0+, -2(%E0)\n\taddc\t@%E0+, -2(%E0)\n\tsub\t#4, %E0";
	else
		pattern = "rla\t%A0\n\trlc\t%B0";


	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{

		case 0:
			dummy = 0;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 6, 1);
			}
			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:

			if (zs || indexed_location (operands[0]))
			{
				OUT_INSN (len, "xor.b\t@%E0, %B0", operands);
				OUT_INSN (len, "xor\t@%E0, %B0", operands);
				OUT_INSN (len, "swpb\t%B0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				OUT_INSN (len, "swpb\t@%E0", operands);
				dummy = 9;
			}
			else
			{
				OUT_INSN (len, "xor.b\t%A0, %B0", operands);
				OUT_INSN (len, "xor\t%A0, %B0", operands);
				OUT_INSN (len, "swpb\t%B0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				OUT_INSN (len, "swpb\t%A0", operands);
				dummy = 12;
				if (REG_P (operands[0]))
					dummy = 5;
			}

			shiftpos -= 8;

			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 6, 1);
			}

			if (len)
				*len = dummy;
			return "";

			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:

			if (zs || indexed_location (operands[0]))
			{
				OUT_INSN (len, "mov\t@%E0, %B0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				dummy = 4;
			}
			else
			{
				OUT_INSN (len, "mov\t%A0, %B0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				dummy = 5;
				if (REG_P (operands[0]))
					dummy = 3;
			}

			shiftpos -= 16;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 6, 1);
			}

			if (len)
				*len = dummy;
			return "";
			break;

		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			if (zs || indexed_location (operands[0]))
			{
				OUT_INSN (len, "mov.b\t@%E0,%B0", operands);
				OUT_INSN (len, "swpb\t%B0", operands);
				OUT_INSN (len, "clr\t@%E0", operands);
				dummy = 6;
			}
			else
			{
				OUT_INSN (len, "mov.b\t%A0,%B0", operands);
				OUT_INSN (len, "swpb\t%B0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				dummy = 8;
				if (GET_CODE (operands[0]) == REG)
					dummy = 3;
			}

			shiftpos -= 24;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 6, 1);
			}

			if (len)
				*len = dummy;
			return "";

			break;

		case 31:
			if (zs || indexed_location (operands[0]))
			{
				OUT_INSN (len, "rra\t@%E0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "rrc\t%B0", operands);
				dummy = 9;

			}
			else
			{
				OUT_INSN (len, "rra\t%A0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "rrc\t%B0", operands);
				dummy = 10;
				if (REG_P (operands[0]))
					dummy = 4;
			}

			if (len)
				*len = dummy;
			return "";
			break;

		default:
			OUT_INSN (len, "clr\t%A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			if (len)
				*len = set_len (operands[0], 6, 1);
			return "";
			break;

		}			/* switch */

		if (len)
			*len = dummy;
		return "";
	}
	else
		msp430_emit_shift_cnt (set_len, pattern, insn, operands, len, 6);

	return "";

}

const char *
msp430_emit_ashldi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{

	int dummy = 0;
	int zs;
	const char *pattern;

	zs = zero_shifted (operands[0]);

	if (zs)
		pattern =
		"add\t@%E0+,-2(%E0)\n\taddc\t@%E0+,-2(%E0)\n\taddc\t@%E0+,-2(%E0)\n\taddc\t@%E0+,-2(%E0)\n\tsub\t#8,%E0";
	else
		pattern = "rla\t%A0\n\trlc\t%B0\n\trlc\t%C0\n\trlc\t%D0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:
			dummy = 0;
			if (len)
				*len = dummy;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 12, 1);
			}
			if (len)
				*len = dummy;
			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			if (zs || indexed_location (operands[0]))
			{
				dummy = 10;
				OUT_INSN (len, "mov\t%C0, %D0", operands);
				OUT_INSN (len, "mov\t%B0, %C0", operands);
				OUT_INSN (len, "mov\t@%E0, %B0", operands);
				OUT_INSN (len, "clr\t@%E0", operands);
			}
			else
			{
				dummy = 11;
				OUT_INSN (len, "mov\t%C0, %D0", operands);
				OUT_INSN (len, "mov\t%B0, %C0", operands);
				OUT_INSN (len, "mov\t%A0, %B0", operands);
				OUT_INSN (len, "clr\t%A0", operands);

			}
			if (GET_CODE (operands[0]) == REG)
				dummy = 4;
			shiftpos -= 16;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 12, 1);
			}
			if (len)
				*len = dummy;
			break;

		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			if (zs)
			{
				dummy = 8;
				OUT_INSN (len, "mov\t@%E0, %D0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);

			}
			else
			{
				dummy = 9;
				OUT_INSN (len, "mov\t%A0, %D0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);
			}
			if (GET_CODE (operands[0]) == REG)
				dummy = 4;

			shiftpos -= 16;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 12, 1);
			}

			if (len)
				*len = dummy;
			break;

		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:

			if (zs)
			{
				OUT_INSN (len, "mov\t@%E0+, %C0", operands);
				OUT_INSN (len, "mov\t@%E0+, %D0", operands);
				OUT_INSN (len, "sub\t#4, %E0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				dummy = 9;
			}
			else
			{
				dummy = 10;
				OUT_INSN (len, "mov\t%A0, %C0", operands);
				OUT_INSN (len, "mov\t%B0, %D0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
			}
			if (GET_CODE (operands[0]) == REG)
				dummy = 4;

			shiftpos -= 32;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 12, 1);
			}

			if (len)
				*len = dummy;
			break;

		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
			if (zs)
			{
				dummy = 8;
				OUT_INSN (len, "mov\t@%E0, %D0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);
			}
			else
			{
				dummy = 9;
				OUT_INSN (len, "mov\t%A0, %D0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);
			}

			shiftpos -= 48;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_len (operands[0], 12, 1);
			}

			if (GET_CODE (operands[0]) == REG)
				dummy = 4;
			if (len)
				*len = dummy;

			break;

		case 63:
			if (zs || indexed_location (operands[0]))
			{
				OUT_INSN (len, "rra\t@%E0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);
				OUT_INSN (len, "clr\t%D0", operands);
				OUT_INSN (len, "rrc\t%D0", operands);
				dummy = 11;
			}
			else
			{
				OUT_INSN (len, "rra\t%A0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "clr\t%B0", operands);
				OUT_INSN (len, "clr\t%C0", operands);
				OUT_INSN (len, "clr\t%D0", operands);
				OUT_INSN (len, "rrc\t%D0", operands);
				dummy = 12;
				if (REG_P (operands[0]))
					dummy = 6;
			}

			if (len)
				*len = dummy;

			break;		/* make compiler happy */

		default:
			OUT_INSN (len, "clr\t%A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "clr\t%C0", operands);
			OUT_INSN (len, "clr\t%D0", operands);
			dummy = 8;
			if (zs)
				dummy--;
			if (REG_P (operands[0]))
				dummy = 4;

			if (len)
				*len = dummy;

		}			/* switch */

		return "";
	}
	else
		msp430_emit_shift_cnt (set_len, pattern, insn, operands, len, 12);

	return "";			/* make compiler happy */
}

/********* SHIFT RIGHT CODE ***************************************/
const char *
msp430_emit_ashrqi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *pattern;
	int shiftpos;

	if (zs)
		pattern = "rra.b\t@%E0";
	else
		pattern = "rra.b\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{

		shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:		/* paranoia setting */
			dummy = 0;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += 2;
			}
			break;

		case 7:
			if (zs)
			{
				OUT_INSN (len, "sxt\t@%E0", operands);
				OUT_INSN (len, "swpb\t@%E0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				dummy = 4;
			}
			else
			{
				OUT_INSN (len, "sxt\t%A0", operands);
				OUT_INSN (len, "swpb\t%A0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				dummy = 6;
			}
			if (REG_P (operands[0]))
				dummy = 3;
			if (len)
				*len = dummy;
			return "";

			break;

		default:
			OUT_INSN (len, "clr.b\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy = 1;
		}

		if (len)
			*len = dummy;
		return "";
	}
	else
	{
		msp430_emit_shift_cnt (set_ren, pattern, insn, operands, len, 2);
	}

	return "";
}

const char *
msp430_emit_ashrhi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *pattern;
	int shiftpos;

	if (zs)
		pattern = "rra\t@%E0";
	else
		pattern = "rra\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:		/* paranoia setting */
			dummy = 0;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += 2;
			}
			if (zs || REG_P (operands[0]))
				dummy >>= 1;
			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			if (zs)
			{
				OUT_INSN (len, "swpb\t@%E0", operands);
				OUT_INSN (len, "sxt\t@%E0", operands);
				dummy = 2;
			}
			else
			{
				OUT_INSN (len, "swpb\t%A0", operands);
				OUT_INSN (len, "sxt\t%A0", operands);
				dummy = 4;
				if (REG_P (operands[0]))
					dummy = 2;
			}
			shiftpos -= 8;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += (zs || REG_P (operands[0])) ? 1 : 2;
			}
			break;

		case 15:
			if (zs)
			{
				OUT_INSN (len, "swpb\t@%E0", operands);
				OUT_INSN (len, "sxt\t@%E0", operands);
				OUT_INSN (len, "swpb\t@%E0", operands);
				OUT_INSN (len, "swpb\t@%E0", operands);
				dummy = 4;
			}
			else
			{
				OUT_INSN (len, "swpb\t%A0", operands);
				OUT_INSN (len, "sxt\t%A0", operands);
				OUT_INSN (len, "swpb\t%A0", operands);
				OUT_INSN (len, "sxt\t%A0", operands);
				dummy = 8;
			}
			if (REG_P (operands[0]))
				dummy = 4;
			break;

		default:
			OUT_INSN (len, "clr\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy = 1;
		}

		if (len)
			*len = dummy;
		return "";
	}
	else
	{
		msp430_emit_shift_cnt (set_ren, pattern, insn, operands, len, 2);
	}

	return "";
}

const char *
msp430_emit_ashrsi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{

	int dummy = 0;
	const char *pattern;
	int zs = zero_shifted (operands[0]);

	pattern = "rra\t%B0\n\trrc\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:
			dummy = 0;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_ren (operands[0], 4, 1);
			}
			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			OUT_INSN (len, "swpb\t%A0", operands);
			OUT_INSN (len, "swpb\t%B0", operands);
			OUT_INSN (len, "xor.b\t%B0, %A0", operands);
			OUT_INSN (len, "xor\t%B0, %A0", operands);
			OUT_INSN (len, "sxt\t%B0", operands);
			dummy = 12;

			if (REG_P (operands[0]))
				dummy = 5;
			shiftpos -= 8;
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_ren (operands[0], 4, 1);
			}
			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "bit\t#0x8000, %B0", operands);
			OUT_INSN (len, "jz\t.Lsrc%=", operands);
			OUT_INSN (len, "bis\t#0xffff, %B0", operands);
			OUT_INSN (len, "jmp\t.Lsre%=\n.Lsrc%=:", operands);
			OUT_INSN (len, "clr\t%B0\n.Lsre%=:", operands);
			dummy = 12;

			if (GET_CODE (operands[0]) == REG)
				dummy = 7;

			shiftpos -= 16;
			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				dummy += 2;
				if (GET_CODE (operands[0]) == REG || zs)
					dummy--;
			}

			break;

		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			OUT_INSN (len, "swpb\t%B0", operands);
			OUT_INSN (len, "sxt\t%B0", operands);
			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "swpb\t%B0", operands);
			OUT_INSN (len, "sxt\t%B0", operands);
			dummy = 11;

			if (GET_CODE (operands[0]) == REG)
				dummy = 5;

			shiftpos -= 24;
			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				dummy += 2;
				if (GET_CODE (operands[0]) == REG || zs)
					dummy--;
			}
			break;

		case 31:
			OUT_INSN (len, "tst\t%B0", operands);
			OUT_INSN (len, "mov\t#-1,%B0", operands);
			OUT_INSN (len, "mov\t#-1,%A0", operands);
			if (GET_CODE (operands[0]) == REG)
				OUT_INSN (len, "jn\t+4", operands);
			else
				OUT_INSN (len, "jn\t+8", operands);
			OUT_INSN (len, "clr\t%A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			dummy = 11;
			if (GET_CODE (operands[0]) == REG)
				dummy = 6;
			break;

		default:
			dummy = 0;		/* leave it alone!!! */
			break;

		}			/* switch */

		if (len)
			*len = dummy;
		return "";
	}
	else
		msp430_emit_shift_cnt (set_ren, pattern, insn, operands, len, 4);

	return "";

}

const char *
msp430_emit_ashrdi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{

	int dummy = 0;
	const char *pattern;

	pattern = "rra\t%D0\n\trrc\t%C0\n\trrc\t%B0\n\trrc\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		switch (shiftpos)
		{
		case 0:
			dummy = 0;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			while (shiftpos--)
			{
				OUT_INSN (len, pattern, operands);
				dummy += set_ren (operands[0], 8, 1);
			}
			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:

			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "mov\t%C0, %B0", operands);
			OUT_INSN (len, "mov\t%D0, %C0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);

			dummy = 17;
			if (GET_CODE (operands[0]) == REG)
				dummy = 7;
			shiftpos -= 16;
			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%C0\n\trrc\t%B0\n\trrc\t%A0", operands);
				dummy += set_ren (operands[0], 6, 1);
			}

			break;

		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			OUT_INSN (len, "mov\t%C0, %A0", operands);
			OUT_INSN (len, "mov\t%D0, %B0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "mov\t%D0, %C0", operands);
			dummy = 17;
			if (GET_CODE (operands[0]) == REG)
				dummy = 8;
			shiftpos -= 32;
			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%B0\n\trrc\t%A0", operands);
				dummy += set_ren (operands[0], 4, 1);
			}
			break;

		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
			OUT_INSN (len, "mov\t%D0, %A0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "mov\t%D0, %C0", operands);
			OUT_INSN (len, "mov\t%D0, %B0", operands);
			dummy = 17;
			if (GET_CODE (operands[0]) == REG)
				dummy = 7;
			shiftpos -= 48;
			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				dummy += set_ren (operands[0], 2, 1);
			}
			break;

		case 63:
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "swpb\t%D0", operands);
			OUT_INSN (len, "sxt\t%D0", operands);
			OUT_INSN (len, "mov\t%D0, %C0", operands);
			OUT_INSN (len, "mov\t%D0, %B0", operands);
			OUT_INSN (len, "mov\t%D0, %A0", operands);
			dummy = 17;
			if (GET_CODE (operands[0]) == REG)
				dummy = 7;
			break;

		default:
			dummy = 0;

		}			/* case */

		if (len)
			*len = dummy;
		return "";
	}
	else
		msp430_emit_shift_cnt (set_ren, pattern, insn, operands, len, 8);
	return "";
}

/********* LOGICAL SHIFT RIGHT CODE ***************************************/
const char *
msp430_emit_lshrqi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *pattern;
	const char *second_pat;
	int shiftpos;

	if (zs)
	{
		pattern = "clrc\n\trrc.b\t@%E0";
		second_pat = "rra.b\t@%E0";
	}
	else
	{
		pattern = "clrc\n\trrc.b\t%A0";
		second_pat = "rra.b\t%A0";
	}

	if (GET_CODE (operands[2]) == CONST_INT)
	{

		shiftpos = INTVAL (operands[2]);

		if (shiftpos != 7 && shiftpos)
		{
			OUT_INSN (len, pattern, operands);
			dummy += set_rel (operands[0], 2, 1);
			shiftpos--;
		}

		switch (shiftpos)
		{
		case 0:
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:

			while (shiftpos--)
			{
				OUT_INSN (len, second_pat, operands);
				dummy += set_rel (operands[0], 2, 1) - 1;
			}

			break;

		case 7:
			if (zs)
			{
				OUT_INSN (len, "rla.b\t@%E0", operands);
				OUT_INSN (len, "clr.b\t%A0", operands);
				OUT_INSN (len, "rlc.b\t@%E0", operands);
				dummy = 4;
			}
			else
			{
				OUT_INSN (len, "rla.b\t%A0", operands);
				OUT_INSN (len, "clr.b\t%A0", operands);
				OUT_INSN (len, "rlc.b\t%A0", operands);
				dummy = 6;
			}
			if (REG_P (operands[0]))
				dummy = 3;
			break;

		default:
			OUT_INSN (len, "clr.b\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy = 1;
			break;
		}

		if (len)
			*len = dummy;
	}
	else
	{
		msp430_emit_shift_cnt (set_rel, pattern, insn, operands, len, 2);
	}

	return "";
}

const char *
msp430_emit_lshrhi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *pattern;
	const char *second_pat;
	int shiftpos;

	if (zs)
	{
		pattern = "clrc\n\trrc\t@%E0";
		second_pat = "rra\t@%E0";
	}
	else
	{
		pattern = "clrc\n\trrc\t%A0";
		second_pat = "rra\t%A0";
	}

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		shiftpos = INTVAL (operands[2]);

		if (shiftpos < 8 && shiftpos)
		{
			OUT_INSN (len, pattern, operands);
			dummy += set_rel (operands[0], 2, 1);
			shiftpos--;
		}

		switch (shiftpos)
		{
		case 0:
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:

			while (shiftpos--)
			{
				OUT_INSN (len, second_pat, operands);
				dummy += set_rel (operands[0], 2, 1) - 1;
			}

			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:

			if (zs)
			{
				OUT_INSN (len, "swpb\t@%E0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				dummy = 3;
			}
			else
			{
				OUT_INSN (len, "swpb\t%A0", operands);
				OUT_INSN (len, "and.b\t#-1, %A0", operands);
				dummy = 4;
			}
			if (REG_P (operands[0]))
				dummy = 2;
			shiftpos -= 8;
			while (shiftpos--)
			{
				OUT_INSN (len, second_pat, operands);
				dummy += set_rel (operands[0], 2, 1) - 1;
			}
			break;

		case 15:

			if (zs)
			{
				OUT_INSN (len, "rla\t@%E0", operands);
				OUT_INSN (len, "clr\t@%E0", operands);
				OUT_INSN (len, "rlc\t@%E0", operands);
				dummy = 3;
			}
			else
			{
				OUT_INSN (len, "rla\t%A0", operands);
				OUT_INSN (len, "clr\t%A0", operands);
				OUT_INSN (len, "rlc\t%A0", operands);
				dummy = 6;
			}
			if (REG_P (operands[0]))
				dummy = 3;
			break;

		default:
			OUT_INSN (len, "clr\t%A0", operands);
			dummy = 2;
			if (REG_P (operands[0]))
				dummy = 1;
			break;
		}

		if (len)
			*len = dummy;
		return "";
	}
	else
	{
		msp430_emit_shift_cnt (set_rel, pattern, insn, operands, len, 2);
	}

	return "";

}

const char *
msp430_emit_lshrsi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	const char *pattern;
	int dummy = 0;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *second_pat = "rra\t%B0\n\trrc\t%A0";

	pattern = "clrc\n\trrc\t%B0\n\trrc\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		if (shiftpos < 8 && shiftpos)
		{
			OUT_INSN (len, pattern, operands);
			/* This function was underestimating the length by 1 for shifts from
			1 to 7.  I added one here - Max */
			dummy += set_rel (operands[0], 2, 1) + 1;
			shiftpos--;
		}

		switch (shiftpos)
		{
		case 0:
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			while (shiftpos--)
			{
				OUT_INSN (len, second_pat, operands);
				dummy += set_rel (operands[0], 4, 1) - 1;
			}

			break;

		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			OUT_INSN (len, "swpb\t%A0", operands);
			OUT_INSN (len, "swpb\t%B0", operands);
			OUT_INSN (len, "xor.b\t%B0, %A0", operands);
			OUT_INSN (len, "xor\t%B0, %A0", operands);
			OUT_INSN (len, "and.b\t#-1, %B0", operands);
			dummy = 12;

			if (REG_P (operands[0]))
				dummy = 5;
			shiftpos -= 8;
			while (shiftpos--)
			{
				OUT_INSN (len, second_pat, operands);
				dummy += set_rel (operands[0], 4, 1) - 1;
			}
			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			dummy = 5;
			if (REG_P (operands[0]))
				dummy = 2;

			shiftpos -= 16;
			if (shiftpos)
			{
				OUT_INSN (len, "clrc\n\trrc\t%A0", operands);
				dummy += 2;
				if (!zs && !REG_P (operands[0]))
					dummy++;
				shiftpos--;
			}

			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				dummy += 1;
				if (!zs && !REG_P (operands[0]))
					dummy++;
			}
			break;

		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "swpb\t%A0", operands);
			OUT_INSN (len, "and.b\t#-1, %A0", operands);
			dummy = 9;
			if (REG_P (operands[0]))
				dummy = 4;
			if (indexed_location (operands[0]))
				dummy -= 1;
			shiftpos -= 24;

			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				dummy += 1;
				if (!zs && !REG_P (operands[0]))
					dummy++;
			}
			break;

		case 31:
			OUT_INSN (len, "rla\r%B0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "clr\t%A0", operands);
			OUT_INSN (len, "rlc\t%A0", operands);
			dummy = 8;
			if (REG_P (operands[0]))
				dummy = 4;
			if (indexed_location (operands[0]))
				dummy -= 1;
			break;

		default:
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "clr\t%A0", operands);
			dummy = 4;
			if (REG_P (operands[0]))
				dummy = 2;
			break;

		}			/* switch */

		if (len)
			*len = dummy;
		return "";
	}
	else
		msp430_emit_shift_cnt (set_rel, pattern, insn, operands, len, 4);

	return "";
}

const char *
msp430_emit_lshrdi3 (insn, operands, len)
rtx insn;
rtx operands[];
int *len;
{
	int dummy = 0;
	const char *pattern;
	int zs = zero_shifted (operands[0]) || indexed_location (operands[0]);
	const char *secondary_pat = "rra\t%D0\n\trrc\t%C0\n\trrc\t%B0\n\trrc\t%A0";

	pattern = "clrc\n\trrc\t%D0\n\trrc\t%C0\n\trrc\t%B0\n\trrc\t%A0";

	if (GET_CODE (operands[2]) == CONST_INT)
	{
		int shiftpos = INTVAL (operands[2]);

		if (shiftpos < 16 && shiftpos)
		{
			OUT_INSN (len, pattern, operands);
			dummy += set_rel (operands[0], 2, 1);
			shiftpos--;
		}

		switch (shiftpos)
		{
		case 0:
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			while (shiftpos--)
			{
				OUT_INSN (len, secondary_pat, operands);
				dummy += set_rel (operands[0], 8, 1) - 1;
			}

			break;

		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			OUT_INSN (len, "mov\t%B0, %A0", operands);
			OUT_INSN (len, "mov\t%C0, %B0", operands);
			OUT_INSN (len, "mov\t%D0, %C0", operands);
			OUT_INSN (len, "clr\t%D0", operands);
			dummy = 11;
			if (REG_P (operands[0]))
				dummy = 4;
			shiftpos -= 16;

			if (shiftpos)
			{
				OUT_INSN (len, secondary_pat, operands);
				dummy += set_rel (operands[0], 8, 1) - 1;
				shiftpos--;
			}

			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%C0\n\trrc\t%B0\n\trrc\t%A0", operands);
				if (REG_P (operands[0]))
					dummy = 3;
				else
					dummy += 6;
				if (zs)
					dummy--;
			}

			break;

		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			OUT_INSN (len, "mov\t%C0, %A0", operands);
			OUT_INSN (len, "mov\t%D0, %B0", operands);
			OUT_INSN (len, "clr\t%C0", operands);
			OUT_INSN (len, "clr\t%D0", operands);

			dummy = 10;
			if (GET_CODE (operands[0]) == REG)
				dummy = 4;

			shiftpos -= 32;

			if (shiftpos)
			{
				OUT_INSN (len, "clrc\n\trrc\t%B0,rrc\t%A0", operands);
				if (REG_P (operands[0]))
					dummy += 3;
				else
					dummy += 5;
				if (zs)
					dummy--;
				shiftpos--;
			}

			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%B0,rrc\t%A0", operands);
				if (REG_P (operands[0]))
					dummy += 2;
				else
					dummy += 4;
				if (zs)
					dummy--;
			}
			break;

		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
			OUT_INSN (len, "mov\t%D0, %A0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "clr\t%C0", operands);
			OUT_INSN (len, "clr\t%D0", operands);
			dummy = 9;
			if (GET_CODE (operands[0]) == REG)
				dummy = 4;
			shiftpos -= 48;

			if (shiftpos)
			{
				OUT_INSN (len, "clrc\n\trrc\t%A0", operands);
				if (REG_P (operands[0]) || zs)
					dummy += 2;
				else
					dummy += 3;

				shiftpos--;
			}

			while (shiftpos--)
			{
				OUT_INSN (len, "rra\t%A0", operands);
				if (REG_P (operands[0]) || zs)
					dummy++;
				else
					dummy += 2;
			}
			break;

		case 63:

			OUT_INSN (len, "rla\t%D0", operands);
			OUT_INSN (len, "clr\t%D0", operands);
			OUT_INSN (len, "clr\t%C0", operands);
			OUT_INSN (len, "clr\t%B0", operands);
			OUT_INSN (len, "clr\t%A0", operands);
			OUT_INSN (len, "rlc\t%A0", operands);
			if (REG_P (operands[0]))
				dummy += 6;
			else
				dummy += 13;

			if (zs)
				dummy--;
			break;

		default:
			break;
		}			/* case */

		if (len)
			*len = dummy;
	}
	else
		msp430_emit_shift_cnt (set_rel, pattern, insn, operands, len, 8);

	return "";
}

/*
*	Multiplication helpers
*	1. As shifts, 2. the rest 
*/

#define SOME_SHIFT_THRESHOLD_VAL 	10

int
msp430_easy_mul (operands, sext)
rtx operands[];
int sext;
{
	enum machine_mode op0mode = GET_MODE (operands[0]);
	enum machine_mode op1mode = GET_MODE (operands[1]);
	rtx op0 = operands[0];
	rtx op1 = operands[1];
	rtx insn;
	int m = INTVAL (operands[2]);
	int sign = (m < 0);
	int val = (m > 0 ? m : -m);
	int shift1 = 0, shift0 = 0;
	int add1 = 0, sub1 = 0;
	int t0, t1;
	int ops = 0;

	m = val;
	/* 
	we can do: 
	const == single bit const +- N that (shift0 + add1/sub1 < 8 instructions) 
	*/
	shift0 = 1;
	shift1 = 0;

	for (t0 = 2;
		t0 <= val * 2 && shift0 < GET_MODE_SIZE (op0mode) * BITS_PER_UNIT;
		t0 <<= 1)
	{
		if (t0 == val)
			goto done;

		for (t1 = 1; t1 < t0 && shift1 < GET_MODE_SIZE (op1mode) * BITS_PER_UNIT;
			t1 <<= 1)
		{
			add1 = 0;
			sub1 = 0;
			if (t0 + t1 == m)
			{
				add1 = 1;
				goto done;
			}
			if (t0 - t1 == m)
			{
				sub1 = 1;
				goto done;
			}

			if (t0 + t1 * 3 == m)
			{
				add1 = 3;
				goto done;
			}

			if (t0 - t1 * 3 == m)
			{
				sub1 = 3;
				goto done;
			}

			add1 = 0;
			sub1 = 0;
			shift1++;

		}
		shift1 = 0;
		shift0++;
	}

	return 0;
done:

	ops = shift0 * (op0mode == SImode ? 2 : 1);
	ops += shift1 + add1 + sub1;
	if (op0mode != op1mode)
	{
		ops += (op0mode == SImode ? 2 : 1) * ((add1 || sub1) ? 2 : 1);
	}

	if (ops > SOME_SHIFT_THRESHOLD_VAL)
		return 0;

	if (op0mode != op1mode)
	{
		rtx extend;
		if (sext)
			extend = gen_rtx_SIGN_EXTEND (op0mode, op1);
		else
			extend = gen_rtx_ZERO_EXTEND (op0mode, op1);
		insn = gen_rtx_SET (VOIDmode, op0, extend);
		emit_insn (insn);
	}
	else
	{
		emit_move_insn (op0, op1);
	}

	/* shift0 */
	switch (op0mode)
	{
	case QImode:
		emit_insn (gen_ashlqi3 (op0, op0, GEN_INT (shift0)));
		break;
	case HImode:
		emit_insn (gen_ashlhi3 (op0, op0, GEN_INT (shift0)));
		break;
	case SImode:
		emit_insn (gen_ashlsi3 (op0, op0, GEN_INT (shift0)));
		break;
	case DImode:
		emit_insn (gen_ashldi3 (op0, op0, GEN_INT (shift0)));
		break;
	default:
		abort ();
	}

	if (op0mode != op1mode && (add1 || sub1 || shift1))
	{
		/* equalize operands modes */
		rtx extend;
		rtx treg = gen_reg_rtx (op0mode);

		if (sext)
			extend = gen_rtx_SIGN_EXTEND (op0mode, op1);
		else
			extend = gen_rtx_ZERO_EXTEND (op0mode, op1);
		insn = gen_rtx_SET (VOIDmode, treg, extend);
		emit_insn (insn);
		op1 = treg;
		op1mode = GET_MODE (treg);
	}
	else if (add1 || sub1 || shift1)
	{
		rtx treg = gen_reg_rtx (op0mode);
		emit_move_insn (treg, op1);
		op1 = treg;
	}

	if (shift1 && (add1 || sub1))
	{
		switch (op1mode)
		{
		case QImode:
			emit_insn (gen_ashlqi3 (op1, op1, GEN_INT (shift1)));
			break;
		case HImode:
			emit_insn (gen_ashlhi3 (op1, op1, GEN_INT (shift1)));
			break;
		case SImode:
			emit_insn (gen_ashlsi3 (op1, op1, GEN_INT (shift1)));
			break;
		case DImode:
			emit_insn (gen_ashldi3 (op1, op1, GEN_INT (shift1)));
			break;
		default:
			abort ();
		}
	}
	else if (shift1)
		abort ();			/* paranoia */

	while (add1--)
	{
		insn =
			gen_rtx_SET (VOIDmode, op0, gen_rtx_PLUS (GET_MODE (op0), op0, op1));
		emit_insn (insn);
	}

	while (sub1--)
	{
		insn =
			gen_rtx_SET (VOIDmode, op0,
			gen_rtx_MINUS (GET_MODE (op0), op0, op1));
		emit_insn (insn);
	}

	if (sign)
	{
		switch (op0mode)
		{
		case QImode:
			emit_insn (gen_negqi2 (op0, op0));
			break;
		case HImode:
			emit_insn (gen_neghi2 (op0, op0));
			break;
		case SImode:
			emit_insn (gen_negsi2 (op0, op0));
			break;
		case DImode:
			emit_insn (gen_negdi2 (op0, op0));
			break;
		default:
			abort ();
		}
	}

	return 1;
}

/* multiplication guards */
#define LOAD_MPY(x)	\
	do{ \
	if(GET_MODE(x) == QImode)		\
	emit_insn(gen_load_mpyq(x));	\
  else					\
  emit_insn(gen_load_mpy(x));	\
	}while(0)

#define LOAD_MPYS(x)	\
	do{ \
	if(GET_MODE(x) == QImode)		\
	emit_insn(gen_load_mpysq(x));	\
  else					\
  emit_insn(gen_load_mpys(x));	\
	}while(0)

#define LOAD_OP2(x)	\
	do{ \
	if(GET_MODE(x) == QImode)		\
	emit_insn(gen_load_op2q(x));	\
  else					\
  emit_insn(gen_load_op2(x));	\
	}while(0)

int
msp430_mul3_guard (operands, sext)
rtx operands[];
int sext;
{
	rtx m_mpys = mpys_rtx;
	rtx m_op2 = op2_rtx;
	rtx m_reslo = reslo_rtx;
	enum machine_mode op0mode = GET_MODE (operands[0]);
	enum machine_mode op1mode = GET_MODE (operands[1]);
	rtx r12 = gen_rtx_REG (op1mode, 12);
	rtx r10 = gen_rtx_REG (op1mode, 10);
	rtx r14 = gen_rtx_REG (op0mode, 14);

	if (const_int_operand (operands[2], VOIDmode) &&
		msp430_easy_mul (operands, sext))
		return 1;

	if (!msp430_has_hwmul)
	{
		rtx clob1 = gen_rtx_CLOBBER (VOIDmode, r10);
		rtx clob2 = gen_rtx_CLOBBER (VOIDmode, r12);
		rtx set;
		rtx mult, op1, op2;
		rtvec vec;
		/* prepare 'call' pattern */
		if (sext==1)
		{
			op1 = gen_rtx_SIGN_EXTEND (op0mode, r10);
			op2 = gen_rtx_SIGN_EXTEND (op0mode, r12);
		}
		else
		{
			op1 = r10;
			op2 = r12;
		}
		mult = gen_rtx_MULT (op0mode, op1, op2);
		set = gen_rtx_SET (op0mode, r14, mult);
		vec = gen_rtvec (3, set, clob1, clob2);

		emit_move_insn (r10, operands[1]);
		emit_move_insn (r12, operands[2]);
		emit (gen_rtx_PARALLEL (VOIDmode, vec));
		emit_move_insn (operands[0], r14);
		return 1;
	}
	if (op1mode == QImode)
	{
		m_mpys = gen_lowpart (QImode, mpys_rtx);
		m_op2 = gen_lowpart (QImode, op2_rtx);
	}

	if (op0mode == QImode)
		m_reslo = gen_lowpart (QImode, reslo_rtx);

	if (!MSP430_NOINT_HWMUL)
		emit_insn (gen_reent_in ());

	LOAD_MPYS (operands[1]);
	if (sext)
		emit_insn (gen_extendqihi2 (mpys_rtx, m_mpys));
	LOAD_OP2 (operands[2]);
	if (sext)
		emit_insn (gen_extendqihi2 (op2_rtx, m_op2));

	if (MSP430_NOINT_HWMUL)
	{
		if (op0mode == HImode)
			emit_insn (gen_fetch_result_hi_nint (operands[0]));
		else
			emit_insn (gen_fetch_result_qi_nint (operands[0]));
	}
	else
	{
		if (op0mode == HImode)
			emit_insn (gen_fetch_result_hi (operands[0]));
		else
			emit_insn (gen_fetch_result_qi (operands[0]));
	}

	return 1;
}


int
msp430_umul3_guard (operands, sext)
rtx operands[];
int sext ATTRIBUTE_UNUSED;
{
	rtx m_mpy = mpy_rtx;
	rtx m_op2 = op2_rtx;
	enum machine_mode op0mode = GET_MODE (operands[0]);
	enum machine_mode op1mode = GET_MODE (operands[1]);
	rtx r12 = gen_rtx_REG (op1mode, 12);
	rtx r10 = gen_rtx_REG (op1mode, 10);
	rtx r14 = gen_rtx_REG (op0mode, 14);

	if (const_int_operand (operands[2], VOIDmode) &&
		msp430_easy_mul (operands, 0))
		return 1;

	if (!msp430_has_hwmul)
	{
		rtx clob1 = gen_rtx_CLOBBER (VOIDmode, r10);
		rtx clob2 = gen_rtx_CLOBBER (VOIDmode, gen_rtx_REG (op1mode, 12));
		rtx set;
		rtx mult, op1, op2;
		rtvec vec;
		/* prepare 'call' pattern */
		op1 = gen_rtx_ZERO_EXTEND (op0mode, r10);
		op2 = gen_rtx_ZERO_EXTEND (op0mode, r12);

		mult = gen_rtx_MULT (op0mode, op1, op2);
		set = gen_rtx_SET (op0mode, r14, mult);
		vec = gen_rtvec (3, set, clob1, clob2);

		emit_move_insn (r10, operands[1]);
		emit_move_insn (r12, operands[2]);
		emit (gen_rtx_PARALLEL (VOIDmode, vec));
		emit_move_insn (operands[0], r14);
		return 1;
	}

	m_mpy = gen_lowpart (QImode, mpy_rtx);
	m_op2 = gen_lowpart (QImode, op2_rtx);

	if (!MSP430_NOINT_HWMUL)
		emit_insn (gen_reent_in ());

	/*  LOAD_MPY (gen_lowpart (QImode,operands[1]));
	//emit_insn (gen_zero_extendqihi2 (mpy_rtx, m_mpy));
	//LOAD_OP2 (gen_lowpart (QImode,operands[2]));
	//emit_insn (gen_zero_extendqihi2 (op2_rtx, m_op2));
	emit_move_insn(m_op2, gen_lowpart (QImode,operands[2]));*/

	//The code above does not work on GCC v4, as the optimizer removes the move INSN
	LOAD_MPY (operands[1]);
	//emit_insn (gen_zero_extendqihi2 (mpy_rtx, m_mpy)); // No need for extension, as the HWMUL recognizes the operand width
	LOAD_OP2 (operands[2]);
	//emit_insn (gen_zero_extendqihi2 (op2_rtx, m_op2));


	if (MSP430_NOINT_HWMUL)
		emit_insn (gen_fetch_result_hi_nint (operands[0]));
	else
		emit_insn (gen_fetch_result_hi (operands[0]));

	return 1;
}


int
msp430_mulhisi_guard (operands)
rtx operands[];
{
	enum machine_mode op0mode = GET_MODE (operands[0]);
	enum machine_mode op1mode = GET_MODE (operands[1]);
	rtx r12 = gen_rtx_REG (op1mode, 12);
	rtx r10 = gen_rtx_REG (op1mode, 10);
	rtx r14 = gen_rtx_REG (op0mode, 14);
	rtx r11 = gen_rtx_REG (op1mode, 11);
	rtx r13 = gen_rtx_REG (op1mode, 13);

	if (const_int_operand (operands[2], VOIDmode) &&
		msp430_easy_mul (operands, 1))
		return 1;

	if (!msp430_has_hwmul)
	{
		rtx clob1 = gen_rtx_CLOBBER (VOIDmode, r10);
		rtx clob2 = gen_rtx_CLOBBER (VOIDmode, r11);
		rtx clob3 = gen_rtx_CLOBBER (VOIDmode, r12);
		rtx clob4 = gen_rtx_CLOBBER (VOIDmode, r13);

		rtx set;
		rtx mult, op1, op2;
		rtvec vec;
		/* prepare 'call' pattern */
		op1 = gen_rtx_SIGN_EXTEND (op0mode, r10);
		op2 = gen_rtx_SIGN_EXTEND (op0mode, r12);

		mult = gen_rtx_MULT (op0mode, op1, op2);
		set = gen_rtx_SET (op0mode, r14, mult);
		vec = gen_rtvec (5, set, clob1, clob2, clob3, clob4);

		emit_move_insn (r10, operands[1]);
		emit_move_insn (r12, operands[2]);
		emit (gen_rtx_PARALLEL (VOIDmode, vec));
		emit_move_insn (operands[0], r14);
		return 1;
	}
	if (!MSP430_NOINT_HWMUL)
		emit_insn (gen_reent_in ());

	LOAD_MPYS (operands[1]);
	LOAD_OP2 (operands[2]);

	if (MSP430_NOINT_HWMUL)
	{
		emit_insn (gen_fetch_result_si_nint (operands[0]));
	}
	else
		emit_insn (gen_fetch_result_si (operands[0]));

	return 1;
}


int
msp430_umulhisi_guard (operands)
rtx operands[];
{
	enum machine_mode op0mode = GET_MODE (operands[0]);
	enum machine_mode op1mode = GET_MODE (operands[1]);
	rtx r12 = gen_rtx_REG (op1mode, 12);
	rtx r10 = gen_rtx_REG (op1mode, 10);
	rtx r14 = gen_rtx_REG (op0mode, 14);
	rtx r11 = gen_rtx_REG (op1mode, 11);
	rtx r13 = gen_rtx_REG (op1mode, 13);

	if (const_int_operand (operands[2], VOIDmode) &&
		msp430_easy_mul (operands, 0))
		return 1;

	if (!msp430_has_hwmul)
	{
		rtx clob1 = gen_rtx_CLOBBER (VOIDmode, r10);
		rtx clob2 = gen_rtx_CLOBBER (VOIDmode, r11);
		rtx clob3 = gen_rtx_CLOBBER (VOIDmode, r12);
		rtx clob4 = gen_rtx_CLOBBER (VOIDmode, r13);

		rtx set;
		rtx mult, op1, op2;
		rtvec vec;
		/* prepare 'call' pattern */
		op1 = gen_rtx_ZERO_EXTEND (op0mode, r10);
		op2 = gen_rtx_ZERO_EXTEND (op0mode, r12);

		mult = gen_rtx_MULT (op0mode, op1, op2);
		set = gen_rtx_SET (op0mode, r14, mult);
		vec = gen_rtvec (5, set, clob1, clob2, clob3, clob4);

		emit_move_insn (r10, operands[1]);
		emit_move_insn (r12, operands[2]);
		emit (gen_rtx_PARALLEL (VOIDmode, vec));
		emit_move_insn (operands[0], r14);
		return 1;
	}

	if (!MSP430_NOINT_HWMUL)
		emit_insn (gen_reent_in ());

	LOAD_MPY (operands[1]);
	LOAD_OP2 (operands[2]);

	if (MSP430_NOINT_HWMUL)
	{
		emit_insn (gen_fetch_result_si_nint (operands[0]));
	}
	else
		emit_insn (gen_fetch_result_si (operands[0]));

	return 1;
}


/* something like 'push x(r1)' or 'push @r1' */
int self_push (rtx x)
{
	rtx c;
	rtx r;

	if (GET_CODE (x) != MEM)
		return 0;

	c = XEXP (x, 0);

	if (REG_P (c) && REGNO (c) == 1)
		return 1;

	if (GET_CODE (c) == PLUS)
	{
		r = XEXP (c, 0);
		if (REG_P (r) && REGNO (r) == 1)
			return 1;
	}
	return 0;
}

const char * msp430_emit_call (rtx operands[])
{
	rtx x = operands[0];
	rtx c;
	rtx r;

	if (GET_CODE (x) == MEM)
	{
		c = XEXP (x, 0);

		if (REG_P (c) && REGNO (c) == 1)
		{
			OUT_INSN (NULL, "call\t2(%E0)", operands);
			return "";
		}

		if (GET_CODE (c) == PLUS)
		{
			r = XEXP (c, 0);
			if (REG_P (r) && REGNO (r) == 1)
			{
				OUT_INSN (NULL, "call\t2+%A0", operands);
				return "";
			}
		}
	}

	OUT_INSN(NULL, "call\t%0", operands);
	return "";
}

/* difficult pushes.
if planets are not aligned, the combiner does not allocate
r4 as a frame pointer. Instead, it uses stack pointer for frame.
If there is a va_arg call and non-register local var has to be passed 
as a function parameter, the push X(r1) in SI, SF and DI modes will
corrupt passed var. The following minds this fact */

const char *
msp430_pushqi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int sp = self_push (operands[0]);
	int dummy = 0;

	if (sp)
	{
		rtx c;
		c = XEXP (operands[0], 0);
		if (REG_P (c))
			OUT_INSN (len, "push.b\t2(%E0)", operands);
		else
			OUT_INSN (len, "push.b\t2+%A0", operands);
		dummy = 2;
	}
	else
	{
		OUT_INSN (len, "push.b\t%A0", operands);
		dummy = 2;

		if (GET_CODE (operands[0]) == CONST_INT)
		{
			int cval = INTVAL (operands[0]);
			int x = (cval) & 0x0fffful;
			if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8 || x == 0xffff)
				dummy--;

		}
		else if (GET_CODE (operands[0]) == REG)
			dummy--;
		else if (GET_CODE (operands[0]) == MEM && REG_P (XEXP (operands[0], 0)))
			dummy--;
	}

	return "";
}

const char *
msp430_pushhi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int sp = self_push (operands[0]);
	int dummy = 0;

	if (sp)
	{
		rtx c;
		c = XEXP (operands[0], 0);
		if (REG_P (c))
			OUT_INSN (len, "push\t2(%E0)", operands);
		else
			OUT_INSN (len, "push\t2+%A0", operands);
		dummy = 2;
	}
	else
	{
		OUT_INSN (len, "push\t%A0", operands);
		dummy = 2;

		if (GET_CODE (operands[0]) == CONST_INT)
		{
			int cval = INTVAL (operands[0]);
			int x = (cval) & 0x0fffful;

			if (cval == 99999999)
			{
				if (len)
					*len = 3;
				return "";
			}

			if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8 || x == 0xffff)
				dummy--;

		}
		else if (GET_CODE (operands[0]) == REG)
			dummy--;
		else if (GET_CODE (operands[0]) == MEM && REG_P (XEXP (operands[0], 0)))
			dummy--;
	}
	if (len)
		*len = dummy;
	return "";
}

const char *
msp430_pushsisf (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int sp = self_push (operands[0]);
	int dummy = 0;

	if (!sp)
	{
		OUT_INSN (len, "push\t%B0", operands);
		OUT_INSN (len, "push\t%A0", operands);
		dummy = 4;
		if (indexed_location (operands[0]))
			dummy--;
		if (REG_P (operands[0]))
			dummy -= 2;
		if (GET_CODE (operands[0]) == CONST_INT)
		{
			int cval = INTVAL (operands[0]);
			int x = (cval) & 0x0fffful;
			int y = (((unsigned long) (cval)) & 0xffff0000ul >> 16);
			if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8 || x == 0xffff)
				dummy--;
			if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8 || y == 0xffff)
				dummy--;
		}
		else if (GET_CODE (operands[0]) == CONST_DOUBLE
			&& GET_MODE (operands[0]) == SFmode)
		{
			long val;
			int y, x;
			REAL_VALUE_TYPE rv;
			REAL_VALUE_FROM_CONST_DOUBLE (rv, operands[0]);
			REAL_VALUE_TO_TARGET_SINGLE (rv, val);

			y = (val & 0xffff0000ul) >> 16;
			x = val & 0xffff;
			if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8 || x == 0xffff)
				dummy--;
			if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8 || y == 0xffff)
				dummy--;
		}
	}
	else
	{
		OUT_INSN (len, "push\t2+%B0", operands);
		OUT_INSN (len, "push\t2+%B0", operands);
		dummy = 4;
	}

	if (len)
		*len = dummy;

	return "";
}


const char *
msp430_pushdi (insn, operands, len)
rtx insn ATTRIBUTE_UNUSED;
rtx operands[];
int *len;
{
	int sp = self_push (operands[0]);
	int dummy = 0;

	if (!sp)
	{
		OUT_INSN (len, "push\t%D0", operands);
		OUT_INSN (len, "push\t%C0", operands);
		OUT_INSN (len, "push\t%B0", operands);
		OUT_INSN (len, "push\t%A0", operands);

		dummy = 8;
		if (indexed_location (operands[0]))
			dummy--;
		if (REG_P (operands[0]))
			dummy -= 4;
		if (GET_CODE (operands[0]) == CONST_DOUBLE)
		{
			int hi = CONST_DOUBLE_HIGH (operands[0]);
			int lo = CONST_DOUBLE_LOW (operands[0]);
			int x, y, z;

			x = (hi & 0xffff0000ul) >> 16;
			y = hi & 0xffff;
			z = (lo & 0xffff0000ul) >> 16;
			if (x == 0 || x == 1 || x == 2 || x == 4 || x == 8 || x == 0xffff)
				dummy--;
			if (y == 0 || y == 1 || y == 2 || y == 4 || y == 8 || y == 0xffff)
				dummy--;
			if (z == 0 || z == 1 || z == 2 || z == 4 || z == 8 || z == 0xffff)
				dummy--;
			z = lo & 0xffff;
			if (z == 0 || z == 1 || z == 2 || z == 4 || z == 8 || z == 0xffff)
				dummy--;
		}
	}
	else
	{
		OUT_INSN (len, "push\t2+%D0", operands);
		OUT_INSN (len, "push\t2+%D0", operands);
		OUT_INSN (len, "push\t2+%D0", operands);
		OUT_INSN (len, "push\t2+%D0", operands);
		dummy = 8;
	}

	if (len)
		*len = dummy;

	return "";
}

int dead_or_set_in_peep (int which, rtx insn ATTRIBUTE_UNUSED, rtx x)
{
	extern int peep2_current_count;
	rtx r;
	rtx next;

	if (which > peep2_current_count)
		return 0;

	next = peep2_next_insn (which);
	if (!next)
		return 0;
	if (!REG_P (x))
		return 0;
	r = find_regno_note (next, REG_DEAD, REGNO (x));

	if (!r)
		return 0;

	r = XEXP (r, 0);
	return GET_MODE (r) == GET_MODE (x);
}

void
msp430_trampoline_template (FILE * fd)
{
	fprintf (fd, "; TRAMPOLINE HERE\n"
		"; move context (either r1 or r4) to r6\n"
		"; call function (0xf0f0 will be changed)\n");
	fprintf (fd, "\tmov	#0xf0f0, r6\n");
	fprintf (fd, "\tbr	#0xf0f0\n");
	fprintf (fd, "; END OF TRAMPOLINE\n\n");
}

void
msp430_initialize_trampoline (tramp, fn, ctx)
rtx tramp;
rtx fn;
rtx ctx;
{
	emit_move_insn (gen_rtx_MEM (HImode, plus_constant (tramp, 2)), ctx);
	emit_move_insn (gen_rtx_MEM (HImode, plus_constant (tramp, 6)), fn);
}

//----------------------------------------------------------------------------------------------------------------------------------------------

static bool
msp430_rtx_costs (rtx x, int code, int outer_code, int *total)
{
	int cst;
	rtx op0, op1;
	/***
	[(outer:mode1 (inner:mode (op1) (op2))]


	inner		outer	mode1	op1   op2
	--------------------------
	CONST_INT	UNKNOWN	VOID
	PLUS		SET	HI	reg + reg
	ASHIFT	SET	HI	reg << 1
	ASHIFT	SET	HI	reg << 15
	NEG		SET	HI	reg
	DIV		SET	HI	reg / 32
	MOD		SET	HI	reg % 32
	UDIV		SET	QI	reg / reg
	MULT		SET	QI	reg * reg
	MULT		SET	HI	0<-reg * 0<-reg
	TRUNCATE	SET	QI	HI -> QI
	UDIV		SET	HI	reg / reg
	MULT		SET	HI	reg * reg
	MULT		SET	SI	0<-reg * 0<-reg
	TRUNCATE	SET	HI	SI -> HI
	UDIV		SET	SI	reg / reg
	MULT		SET	SI	reg * reg
	MULT		SET	DI	0<-reg * 0<-reg
	TRUNCATE	SET	SI	DI -> SI
	UDIV		SET	DI	reg / reg
	MULT		SET	DI	reg * reg
	MULT		SET	TI	0<-reg * 0<-reg
	TRUNCATE	SET	DI	TI -> DI
	UDIV		SET	TI	reg / reg
	MULT		SET	TI	reg * reg
	MULT		SET	OI	reg * reg
	TRUNCATE	SET	TI	OI -> TI
	UDIV		SET	OI	reg / reg
	MULT		SET	OI	reg * reg
	CONST_INT	COMPARE	VOID	
	...
	CONST_INT	PLUS	HI	reg + const
	PLUS		MEM	HI	X(rn)
	PLUS		CONST_INT	???????? ?????
	MEM		SET	any
	PLUS		MEM	HI	reg + const

	***/
	cst = COSTS_N_INSNS (5);
	if (outer_code == SET)
	{
		op0 = XEXP (x, 0);
		op1 = XEXP (x, 1);
		switch (code)
		{
		case CONST_INT:	/* source only !!! */
			{
				int i = INTVAL (x);
				if (i == -1 || i == 0 || i == 2 || i == 4 || i == 8)
					cst = COSTS_N_INSNS (1);
				else
					cst = COSTS_N_INSNS (2);
			}
			break;
		case PLUS:
		case MINUS:
		case AND:
		case IOR:
		case XOR:
		case UNSPEC:
		case UNSPEC_VOLATILE:
			cst = COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			break;
		case ASHIFT:
		case LSHIFTRT:
		case ASHIFTRT:
			/* cst = COSTS_N_INSNS(10);
			break; */
			if (CONSTANT_P (op1) && INTVAL (op1) == 1)
				cst =
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			else if (CONSTANT_P (op1) && INTVAL (op1) == 15)
				cst =
				3 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			else if (CONSTANT_P (op1) && INTVAL (op1) == 8)
				cst =
				2 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			else if (CONSTANT_P (op1) && INTVAL (op1) == 16)
				cst =
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			else if (CONSTANT_P (op1) && INTVAL (op1) == 24)
				cst =
				4 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			else if (CONSTANT_P (op1) && INTVAL (op1) == 31)
				cst =
				3 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			if (code == ASHIFTRT)
				cst += COSTS_N_INSNS (1);
			break;

		case NEG:
			cst =
				2 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			break;
		case DIV:
		case MOD:
		case MULT:
		case UDIV:
			cst = COSTS_N_INSNS (64);
			break;
		case TRUNCATE:
			cst = COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			break;
		case ZERO_EXTEND:
			cst =
				2 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			break;
		case SIGN_EXTEND:
		case ABS:
			cst =
				2 *
				COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
			cst += COSTS_N_INSNS (2);
			break;
		default:
			cst = 0;
		}
	}
	else if (outer_code == COMPARE)
	{
		cst = COSTS_N_INSNS (((GET_MODE_SIZE (GET_MODE (x)) + 1) & ~1) >> 1);
		cst += COSTS_N_INSNS (2);
	}
	else if (outer_code == JUMP_INSN)
	{
		cst = COSTS_N_INSNS (2);
	}
	else if (outer_code == CALL_INSN)
	{
		cst = COSTS_N_INSNS (4);
	}
	else
		return false;

	*total = cst;
	if (cst)
		return true;
	return false;
}

int
default_rtx_costs (rtx X ATTRIBUTE_UNUSED, enum rtx_code code, enum rtx_code outer_code ATTRIBUTE_UNUSED)
{
	int cost = 4;

	switch (code)
	{
	case SYMBOL_REF:
		cost += 2;
		break;
	case LABEL_REF:
		cost += 2;
		break;
	case MEM:
		cost += 2;
		break;
	case CONST_INT:
		cost += 2;
		break;
	case SIGN_EXTEND:
	case ZERO_EXTEND:
		cost += 2;
		break;
	default:
		break;
	}
	return cost;
}


static void 
msp430_globalize_label(FILE *stream, const char *name)
{
	if(*name == '*' || *name == '@') name++;
	if(*name >='0' && *name <='9') return;
	fputs (GLOBAL_ASM_OP, stream);
	assemble_name (stream, name);
	putc ('\n', stream);
}

static bool msp430_function_ok_for_sibcall(tree decl ATTRIBUTE_UNUSED, tree exp ATTRIBUTE_UNUSED)
{
	int cfp = msp430_critical_function_p (current_function_decl);
	int ree = msp430_reentrant_function_p (current_function_decl);
	int nfp = msp430_naked_function_p (current_function_decl);
	int ifp = interrupt_function_p (current_function_decl);
	int wup = wakeup_function_p (current_function_decl);
	int fee = msp430_empty_epilogue ();

	/*
	function must be:
	- not critical
	- not reentrant
	- not naked
	- not interrupt
	- nor wakeup
	- must have empty epilogue
	*/

	if(nfp || ifp || wup || ree || cfp || !fee)
		return false;
	return true;
}

int
msp430_address_costs (rtx x)
{
	enum rtx_code code = GET_CODE (x);
	rtx op0, op1;

	switch (code)
	{
	case PLUS:			/* X(rn), addr + X */
		op0 = XEXP (x, 0);
		op1 = XEXP (x, 1);
		if (REG_P (op0))
		{
			if (INTVAL (op1) == 0)
				return COSTS_N_INSNS (2);
			else
				return COSTS_N_INSNS (3);
		}
		break;
	case REG:
		return COSTS_N_INSNS (2);
		break;
	default:
		break;
	}
	return COSTS_N_INSNS (3);
}

void msp430_expand_mov_intptr (rtx dest, rtx src)
{
  if (push_operand (dest, HImode) && ! general_no_elim_operand (src, HImode))
	src = copy_to_mode_reg (HImode, src);

  emit_insn (gen_rtx_SET (VOIDmode, dest, src));
}
