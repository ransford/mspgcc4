/* This work is partially financed by the European Commission under the
* Framework 6 Information Society Technologies Project
* "Wirelessly Accessible Sensor Populations (WASP)".
*/

/*
	GDB 6.x/7.x port by Ivan Shcherbakov <mspgcc@sysprogs.org>
*/

/* Target-dependent code for the Texas Instruments MSP430 MCUs.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003 Free Software
   Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA. */

/*  Contributed by Steve Underwood <steveu@coppice.org> */

#include "defs.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "symtab.h"
#include "gdbtypes.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdb_string.h"
#include "value.h"
#include "inferior.h"
#include "dis-asm.h"
#include "symfile.h"
#include "objfiles.h"
#include "language.h"
#include "arch-utils.h"
#include "regcache.h"
#include "remote.h"
#include "floatformat.h"
#include "sim-regno.h"
#include "disasm.h"
#include "trad-frame.h"
#include "dwarf2-frame.h"

#include "gdb_assert.h"

#include "_gdbver.h"

#if GDB_VERSION_INT < 0x700
#define extract_unsigned_integer(a,b,c) extract_unsigned_integer(a,b)
#endif

#define NUM_REGS 16

struct gdbarch_tdep
{
  int dummy;
};

/* MSP430 register names. */

enum regnums
{
  /* The MSP430 has 16 registers (R0-R15). Registers R5-R15 are general
     purpose. 
     R0 is the program counter
     R1 is the stack pointer
     R2 is the status register
     R3 is hardwired to 0
     R4 is used as the frame pointer (but is a general purpose register).

     Functions will return their values in registers R15-R12, as they fit.
  */
  E_R0_REGNUM, E_PC_REGNUM = E_R0_REGNUM,
  E_R1_REGNUM, E_SP_REGNUM = E_R1_REGNUM,
  E_R2_REGNUM, E_PSW_REGNUM = E_R2_REGNUM,
  E_R3_REGNUM,
  E_R4_REGNUM, E_FP_REGNUM = E_R4_REGNUM,
  E_R5_REGNUM,
  E_R6_REGNUM,
  E_R7_REGNUM, E_LST_ARG_REGNUM = E_R7_REGNUM,
  E_R8_REGNUM,
  E_R9_REGNUM,
  E_R10_REGNUM,
  E_R11_REGNUM,
  E_R12_REGNUM,
  E_R13_REGNUM,
  E_R14_REGNUM,
  E_R15_REGNUM, E_PTR_RET_REGNUM = E_R15_REGNUM,
  E_NUM_REGS,
  /* msp430 calling convention. */
  E_FIRST_ARG_REG = E_R15_REGNUM,
  E_LAST_ARG_REG = E_R14_REGNUM,
  E_NEXT_ARG_REG_DELTA = -1,	//Register numbers decreases for every next argument
  
  RET1_REGNUM = E_R15_REGNUM,
};

/*
   our 'frame' member of 'fi' is a stack bottom after func. prologue.
   return address is located at 'frame'
        Scanning prologues.
   A typical msp430 function's prologue is one of the following:
   
   1. some call
   
   push rXX                 <-- where XX is r15 down to r4
   ...
   push r4                  <-- If used or fpn
   sub #[SIZE], r1
   mov r1, r4               <-- If frame pointer needed
   
   2. main() w/o -mno-stack-init
   
   mov #[SMTH], r1
   mov  r1, r4              <-- If frame pointer needed

   3. with -mno-stack-init it looks like (1), but no regs are saved.
   
   4. No optimize, usual funct:
   push rXX
   push r5
   push r4
   mov r1, r5
   add #[sizeofpushed+2], r5
   sub #[framesize], r1
   mov r1, r4
   
   5. main does not look any different
   
   6. any prologue may start from eint (0xd2, 0x32);
*/

/* patterns */
static unsigned short eint      = 0xd232;
static unsigned short push_rn   = 0x1200; /* bitmask: push rX */
static unsigned short load_fp   = 0x4104; /* match: mov r1, r4*/
static unsigned short sub_val   = 0x8031; /* sub #VAL, r1 , VAL is not one of the consts. */
static unsigned short sub_2     = 0x8321;
static unsigned short sub_4     = 0x8221;  
static unsigned short sub_8     = 0x8231;
static unsigned short load_ap   = 0x4105; /* match: mov r1, r5 */
static unsigned short add_val   = 0x5035; /* followed by add value */
static unsigned short add_2     = 0x5325;
static unsigned short add_4     = 0x5225;
static unsigned short add_8     = 0x5235;
static unsigned short init_sp   = 0x4031; /* followed by value ... XXX 
                                             assuming no stupid things happen */

/* The base of the current frame is actually in the stack pointer.
   This happens when there is no frame pointer (msp430 ABI does not
   require a frame pointer) or when we're stopped in the prologue or
   epilogue itself.  In these cases, msp430_analyze_prologue will need
   to update fi->frame before returning or analyzing the register
   save instructions. */
#define MY_FRAME_IN_SP 0x1

/* The base of the current frame is in a frame pointer register.
   This register is noted in frame_extra_info->fp_regnum.

   Note that the existence of an FP might also indicate that the
   function has called alloca. */
#define MY_FRAME_IN_FP 0x2

/* This flag is set to indicate that this frame is the top-most
   frame. This tells frame chain not to bother trying to unwind
   beyond this frame. */
#define NO_MORE_FRAMES 0x4

static CORE_ADDR current_sp;
static int msp430_real_fp = E_SP_REGNUM;

/* Local functions */

extern void _initialize_msp430_tdep (void);

static void msp430_prepare_to_trace (void);

static void msp430_get_trace_data (void);

static CORE_ADDR
msp430_frame_align (struct gdbarch *gdbarch, CORE_ADDR sp)
{
  return sp & ~1;
}

/* Should we use EXTRACT_STRUCT_VALUE_ADDRESS instead of
   EXTRACT_RETURN_VALUE?  GCC_P is true if compiled with gcc
   and TYPE is the type (which is known to be struct, union or array).

   The msp430 returns anything less than 8 bytes in size in
   registers. */

static int
msp430_use_struct_convention (struct type *type)
{
  long alignment;
  int i;
  enum type_code code = TYPE_CODE (type);

  /* The msp430 only passes a struct in a register when that structure
     has an alignment that matches the size of a register. */
  /* If the structure doesn't fit in 4 registers, put it on the
     stack. */
  if (TYPE_LENGTH (type) > 8)
    return 1;

  //If type is not a structure, it is passed through a register
  if (code != TYPE_CODE_STRUCT)
	  return 0;

  /* If the struct contains only one field, don't put it on the stack
     - gcc can fit it in one or more registers. */
  if (TYPE_NFIELDS (type) == 1)
    return 0;
  alignment = TYPE_LENGTH (TYPE_FIELD_TYPE (type, 0));
  for (i = 1; i < TYPE_NFIELDS (type); i++)
    {
      /* If the alignment changes, just assume it goes on the
         stack. */
      if (TYPE_LENGTH (TYPE_FIELD_TYPE (type, i)) != alignment)
        return 1;
    }
  /* If the alignment is suitable for the msp430's 16 bit registers,
     don't put it on the stack. */
  if (alignment == 2 || alignment == 4)
    return 0;
  return 1;
}


static const unsigned char *
msp430_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pcptr, int *lenptr)
{
  static char breakpoint[] = {0x00, 0x00};

  *lenptr = 2;
  return breakpoint;
}

/* Map the REG_NR onto an ascii name.  Return NULL or an empty string
   when the reg_nr isn't valid. */

static const char *
msp430_register_name (struct gdbarch *gdbarch, int reg_nr)
{
  static char *register_names[] = {
    "r0", "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
  };

  if (reg_nr < 0)
    return NULL;
  if (reg_nr >= (sizeof (register_names) / sizeof (*register_names)))
    return NULL;
  return register_names[reg_nr];
}

/* Return the GDB type object for the "standard" data type
   of data in register N. */

#if GDB_VERSION_INT >= 0x700
static struct type * msp430_register_type (struct gdbarch *gdbarch, int reg_nr)
{
  if (reg_nr == E_PC_REGNUM)
    return builtin_type (gdbarch)->builtin_func_ptr;
  if (reg_nr == E_SP_REGNUM || reg_nr == E_FP_REGNUM)
    return builtin_type (gdbarch)->builtin_data_ptr;
  return builtin_type (gdbarch)->builtin_int16;
}
#else
static struct type * msp430_register_type (struct gdbarch *gdbarch, int reg_nr)
{
  if (reg_nr == E_PC_REGNUM)
    return builtin_type_void_func_ptr;
  if (reg_nr == E_SP_REGNUM || reg_nr == E_FP_REGNUM)
    return builtin_type_void_data_ptr;
  return builtin_type_int16;
}
#endif

#if GDB_VERSION_INT >= 0x700
static void msp430_address_to_pointer (struct gdbarch *gdbarch, struct type *type, gdb_byte *buf, CORE_ADDR addr)
#else
static void msp430_address_to_pointer (struct type *type, gdb_byte *buf, CORE_ADDR addr)
#endif
{
#if GDB_VERSION_INT >= 0x700
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  store_unsigned_integer (buf, TYPE_LENGTH (type), byte_order, addr);
#else
  store_unsigned_integer (buf, TYPE_LENGTH (type), addr);
#endif
}

#if GDB_VERSION_INT >= 0x700
static CORE_ADDR msp430_pointer_to_address (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf)
#else
static CORE_ADDR msp430_pointer_to_address (struct type *type, const gdb_byte *buf)
#endif
{
#if GDB_VERSION_INT >= 0x700
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
#endif
  return extract_unsigned_integer (buf, TYPE_LENGTH (type), byte_order);
}

/* Don't do anything if we have an integer. This way users can type 'x
   <addr>' w/o having gdb outsmarting them.  The internal gdb conversions
   to the correct space are taken care of in the pointer_to_address
   function.  If we don't do this, 'x $fp' will not work. */
static CORE_ADDR
msp430_integer_to_address (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf)
{
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  return extract_unsigned_integer (buf, TYPE_LENGTH (type), byte_order);
}

/* Write into appropriate registers a function return value
   of type TYPE, given in virtual format.  

   Things always get returned in RET1_REGNUM, RET2_REGNUM, ... */

static void
msp430_store_return_value (struct type *type,
                           struct regcache *regcache,
                           const gdb_byte *valbuf)
{
  /* Only char return values need to be shifted right within the first
     regnum. */
  if (TYPE_LENGTH (type) == 1
      && TYPE_CODE (type) == TYPE_CODE_INT)
    {
      bfd_byte tmp[2];
      tmp[1] = *(bfd_byte *)valbuf;
      regcache_cooked_write (regcache, RET1_REGNUM, tmp);
    }
  else
    {
      int reg;
      /* A structure is never more than 8 bytes long. See
         use_struct_convention(). */
      gdb_assert (TYPE_LENGTH (type) <= 8);
      /* Write out most registers, stop loop before trying to write
         out any dangling byte at the end of the buffer. */
      for (reg = 0; (reg*2) + 1 < TYPE_LENGTH (type); reg++)
        {
          regcache_cooked_write (regcache,
                                 RET1_REGNUM - reg,
                                 (bfd_byte *) valbuf + reg*2);
        }
      /* Write out any dangling byte at the end of the buffer. */
      if ((reg*2) + 1 == TYPE_LENGTH (type))
        regcache_cooked_write_part (regcache, reg, 0, 1,
                                    (bfd_byte *) valbuf + reg*2);
    }
}


static int
get_insn (CORE_ADDR pc, struct gdbarch *gdbarch)
{
  char buf[4];
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

#if GDB_VERSION_INT >= 0x700
  read_memory(0xfffful & pc, buf, 2);
#else
  int status = read_memory_nobpt (0xfffful & pc, buf, 2);
  if (status != 0)
    return 0;
#endif
	
  return extract_unsigned_integer (buf, 2, byte_order);
}

typedef struct _parsed_prologue
{
	int valid;
	int saved_register_block_size;
	int locals_block_size;
	int register_offsets[NUM_REGS];
	int flags;
	int sp_already_decremented_by;	//Value between 0 (first instruction of prologue is about to execute) and frame_size (prologue has completed)
} parsed_prologue;

static CORE_ADDR msp430_scan_prologue (struct gdbarch *gdbarch, CORE_ADDR pc, parsed_prologue *parsed)
{
  CORE_ADDR func_addr, func_end, addr, stop;
  CORE_ADDR stack_size;
  //CORE_ADDR stack_top;
  int insn, rn;
  int status;
  int fp_regnum = 0; /* dummy, valid when (flags & MY_FRAME_IN_FP) */
  int flags;
  int framesize;
  int register_offsets[NUM_REGS];
  int pushed_reg_count = 0;
  int sp_already_decremented_by = 0;
  char *name;
  int vpc = 0;
  CORE_ADDR start_addr;
  int i;
  int reti = 0;
  CORE_ADDR original_pc = pc;

  msp430_real_fp = E_SP_REGNUM;        /* Wild guess */
    
  /* Find the start of this function. */
  status = find_pc_partial_function (pc, &name, &func_addr, &func_end);

  /* If the start of this function could not be found or if the debbuger
     is stopped at the first instruction of the prologue, do nothing. */
  if (status == 0)
    return 0xfffful & pc;

  /* If the debugger is entry function, give up. */
  if (func_addr == entry_point_address ())
    return 0xfffful & pc;


  /* At the start of a function, our frame is in the stack pointer. */
  flags = MY_FRAME_IN_SP;
  /* Get the first insn from memory (all msp430 instructions are 16 bits) */
  insn = get_insn (pc, gdbarch);

  if (func_addr)
    pc = func_addr;

  /* Figure out where to stop scanning */
  stop = func_end;

  /* Don't walk off the end of the function */
  stop = (stop > func_end ? func_end : stop);

  /* REGISTER_OFFSETS will contain offsets, from the top of the frame
     (NOT the frame pointer), for the various saved registers or -1
     if the register is not saved. */
  for (rn = 0; rn < NUM_REGS; rn++)
    register_offsets[rn] = -1;

  /* stack_top = read_sp(); */
  /* stack_top = read_register (E_SP_REGNUM); */
  /* regcache_cooked_read_unsigned(get_current_regcache(), E_SP_REGNUM, &stack_top); */

  /* Analyze the prologue. Things we determine from analyzing the
     prologue include:
     * the size of the frame
     * where saved registers are located (and which are saved)
     * FP used? */
  framesize = 0;

  if (name && strcmp ("main", name) == 0)
  {
    start_addr = func_addr;
    insn = get_insn(pc + vpc, gdbarch);

    if (insn == init_sp)
    {
      /* ok... will be a frame pointer adjustment? */
      vpc += 2;
      insn = get_insn(pc + vpc, gdbarch);
      vpc += 2;        /* skip this and value.*/
      start_addr = func_addr + vpc;
    }
    else if (insn == sub_val)        /* -mno-stack-init */
    {
      vpc += 2;
      insn = get_insn(pc + vpc, gdbarch);
      vpc += 2;
      start_addr = func_addr + vpc;
    }
    else if (insn == sub_2)
    {
      vpc += 2;
      start_addr = func_addr + vpc;
    }
    else if (insn == sub_4)
    {
      vpc += 2;
      start_addr = func_addr + vpc;
    }
    else if (insn == sub_8)
    {
      vpc += 2;
      start_addr = func_addr + vpc;
    }
    else
    {
      /* we're here cause no frame adjustment required 
        and main main frame size is over any reasonable value...
        should never happen cause memory not less than 0x200*/
      vpc += 2;
      start_addr = func_addr + vpc;
    }
    insn = get_insn(pc + vpc, gdbarch);        /* check if fp being initialized */
    if (insn == load_fp)
    {
      vpc += 2;
      start_addr = func_addr + vpc;
      msp430_real_fp = E_SP_REGNUM;
    }
    return start_addr;
  }

  //Function is not "main()". Start analyzing prologue
    
  /* check eint 
     Actually, there is only way to check if this is an interrupt
     is to check eint - enable nested.
     Ordinary interrupts cannot be caught */
  insn = get_insn(pc + vpc, gdbarch);
  if (insn == eint)
    vpc += 2;
    
  /* check pushes */
  for (i = 15;  i >= 2;  i--)
  {
    int rn;
    insn = get_insn(pc + vpc, gdbarch);
    if ((insn & 0xfff0) != push_rn)
      break;

	if (original_pc > (pc + vpc))
		sp_already_decremented_by += 2;	//This 'push' instruction has already been executed

    rn  = insn & 15;
    vpc += 2;
    pushed_reg_count++;
    register_offsets[rn] = pushed_reg_count*2;	//Offset from beginning of the frame [0 = return value]
  }
    
  /* now check if there is an arg pointer */
  insn = get_insn(pc + vpc, gdbarch);
  if (insn == load_ap)
  {
    vpc += 2;
    insn = get_insn(pc + vpc, gdbarch);
    if (insn == add_val)
    {
      vpc += 2; /* insn */
      vpc += 2; /* actual value */
      insn = get_insn(pc + vpc, gdbarch);
    }
    else if (insn == add_2 || insn == add_4 || insn == add_8)
    {
      vpc += 2; 
      insn = get_insn(pc + vpc, gdbarch);
    }
  }
    
  /* check if stack pointer has been adjusted */
    
  if (insn == sub_val)
  {
    vpc += 2;
    framesize = get_insn(pc + vpc, gdbarch);
    vpc += 2;
    insn = get_insn(pc + vpc, gdbarch);
  }
  else if (insn == sub_2)
  {
    framesize = 2;
    vpc += 2;
    insn = get_insn(pc + vpc, gdbarch);
  }
  else if (insn == sub_4)
  {
    framesize = 4;
    vpc += 2;
    insn = get_insn(pc + vpc, gdbarch);
  }
  else if (insn == sub_8)
  {
    framesize = 8;
    vpc += 2;
    insn = get_insn(pc + vpc, gdbarch);
  }

  if (original_pc > (pc + vpc - 2))
	  sp_already_decremented_by += framesize;	//The sp decreasing instruction has already been executed

  /* check if fp loaded */
  if (insn == load_fp)
  {
    msp430_real_fp = E_SP_REGNUM;

	if (original_pc > (pc + vpc))
		flags = MY_FRAME_IN_FP;	//Frame is already in FP only if the FP loading instruction is already executed
    vpc += 2;
  }

  if (parsed)
  {
	  parsed->valid = 1;
	  memcpy(parsed->register_offsets, register_offsets, sizeof(parsed->register_offsets));
	  parsed->flags = flags;
	  parsed->locals_block_size = framesize;
	  parsed->saved_register_block_size = pushed_reg_count * 2;
	  
	  if (sp_already_decremented_by > (parsed->saved_register_block_size + parsed->locals_block_size))
		  sp_already_decremented_by = parsed->saved_register_block_size + parsed->locals_block_size;

	  parsed->sp_already_decremented_by = sp_already_decremented_by;
  }

  /* Return addr of first non-prologue insn. */
  return func_addr + vpc;
}

static CORE_ADDR msp430_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
	return msp430_scan_prologue(gdbarch, pc, NULL);
}

typedef int bool;
static const int true = 1;
static const int false = 0;

typedef struct _msp430_global_cache	//Various global stuff. Used by IsValidCodePtr(), IsValidDataPtr()
{
	bool cache_valid;
	ULONGEST min_code_ptr;
	ULONGEST max_code_ptr;
	ULONGEST min_data_ptr;
	ULONGEST max_data_ptr;
} msp430_global_cache;

static bool ProvideValidGlobalCache(msp430_global_cache *pCache);
static bool IsValidCodePtr(msp430_global_cache *pCache, ULONGEST ptr);
static bool IsValidDataPtr(msp430_global_cache *pCache, ULONGEST ptr);

struct msp430_unwind_cache
{
  /* The previous frame's inner most stack address.  Used as this
     frame ID's stack_addr. */
  CORE_ADDR prev_sp;
  /* The frame's base, optionally used by the high-level debug info. */
  CORE_ADDR base;
  //int size;
  /* How far the SP and r4 (FP) have been offset from the start of
     the stack frame (as defined by the previous frame's stack
     pointer). */
//  LONGEST sp_offset;
//  LONGEST fp_offset;
  int uses_frame;
  /* Table indicating the location of each and every register. */
  struct trad_frame_saved_reg *saved_regs;
  msp430_global_cache global_cache;
};


static int DetectPushedArgumentsSize(CORE_ADDR FirstInstructionAfterCall)
{
	return 0;
}

static struct msp430_unwind_cache *msp430_build_unwind_cache_from_epilogue(struct frame_info *associated_frame, void **this_prologue_cache)
{
	//Note that on GDB v7 associated_frame is the curent frame, while on gdb v6, it is the next frame
#if GDB_VERSION_INT >= 0x700
	struct frame_info *this_frame = associated_frame;
#else
	struct frame_info *next_frame = associated_frame;
#endif

	const int TargetPointerSize = 2;

	struct gdbarch *gdbarch = get_frame_arch (associated_frame);

	ULONGEST this_pc, this_sp;
	ULONGEST frame_bottom;	//Either SP or FP, depending on whether FP is used

	int insn, pc_off = 0, i, prev_sp = 0;

	int register_offsets[E_NUM_REGS];

	struct msp430_unwind_cache *info;


#if GDB_VERSION_INT >= 0x700
	if (frame_relative_level (this_frame) != 0)
#else
	if (frame_relative_level (next_frame) != -1)
#endif
		return NULL;

	if ((*this_prologue_cache))
		return (*this_prologue_cache);

	//Extract PC and SP (start of frame) for THIS frame.
#if GDB_VERSION_INT >= 0x700
	this_pc = get_frame_pc (this_frame);
	this_sp = get_frame_sp (this_frame);
#else
	this_pc = frame_unwind_register_unsigned (next_frame, E_PC_REGNUM);
	this_sp = frame_unwind_register_unsigned (next_frame, E_SP_REGNUM);
#endif

	if (!this_sp || !this_pc)
		return info;	//Cannot extract any useful frame information

	memset(register_offsets, -1, sizeof(register_offsets));

	for (pc_off = 0; pc_off <= 16; pc_off += 2)
	{
		insn = get_insn (this_pc + pc_off, gdbarch);
		if ((insn & 0xFFF0) == 0x4130)
		{	/* This is a POP instruction */
			int poppedRegNum = insn & 0x000F;
			register_offsets[poppedRegNum] = pc_off;
			if (poppedRegNum == E_PC_REGNUM)
			{
				prev_sp = this_sp + pc_off + 2;
				break;
			}
		}
		else if (insn == 0x1300)
		{	/* This is a "reti" instruction */
			register_offsets[E_PSW_REGNUM] = pc_off;
			register_offsets[E_PC_REGNUM] = pc_off + 2;
			prev_sp = this_sp + pc_off + 4;
			break;
		}
		else
			return NULL;
	}

	info = FRAME_OBSTACK_ZALLOC (struct msp430_unwind_cache);
	(*this_prologue_cache) = info;
	info->saved_regs = trad_frame_alloc_saved_regs (associated_frame);

	for (i = 0; i < E_NUM_REGS; i++)
	{
		if (register_offsets[i] != -1)
		{
			ULONGEST reg_val = get_frame_memory_unsigned(associated_frame, this_sp +register_offsets[i], 2);
			trad_frame_set_value (info->saved_regs, i, reg_val);
		}
	}

	if (prev_sp)
		trad_frame_set_value (info->saved_regs, E_SP_REGNUM, prev_sp);

	info->prev_sp = prev_sp;
	info->base = prev_sp;

	return info;
}

static struct msp430_unwind_cache *msp430_build_unwind_cache_from_epilogue(struct frame_info *associated_frame, void **this_prologue_cache);

#if GDB_VERSION_INT >= 0x700
static void msp430_epilogue_frame_this_id (struct frame_info *this_frame,
										   void **this_prologue_cache,
struct frame_id *this_id)
{
	struct msp430_unwind_cache *info = msp430_build_unwind_cache_from_epilogue (this_frame, this_prologue_cache);
	CORE_ADDR base;
	CORE_ADDR func;
	struct frame_id id;

	if (!info)
		return;

	/* The FUNC is easy. */
	func = get_frame_func (this_frame);

	/* Hopefully the prologue analysis either correctly determined the
	frame's base (which is the SP from the previous frame), or set
	that base to "NULL".  */
	base = info->prev_sp;
	if (base == 0)
		return;

	id = frame_id_build (base, func);
	(*this_id) = id;
}
#else
static void msp430_epilogue_frame_this_id (struct frame_info *next_frame,
										   void **this_prologue_cache,
struct frame_id *this_id)
{
	struct msp430_unwind_cache *info
		= msp430_build_unwind_cache_from_epilogue (next_frame, this_prologue_cache);
	CORE_ADDR base;
	CORE_ADDR func;
	struct frame_id id;

	/* The FUNC is easy. */
	func = frame_func_unwind (next_frame, NORMAL_FRAME);

	/* Hopefully the prologue analysis either correctly determined the
	frame's base (which is the SP from the previous frame), or set
	that base to "NULL".  */
	base = info->prev_sp;
	if (base == 0)
		return;

	id = frame_id_build (base, func);

	/* Check that we're not going round in circles with the same frame
	ID (but avoid applying the test to sentinel frames which do go
	round in circles).  Can't use frame_id_eq() as that doesn't yet
	compare the frame's PC value.  */
	if (frame_relative_level (next_frame) >= 0
		&& get_frame_type (next_frame) != DUMMY_FRAME
		&& frame_id_eq (get_frame_id (next_frame), id))
		return;

	(*this_id) = id;
}
#endif

/* -------------------------------------------------------------------------------------------------- */

/* Put here the code to store, into fi->saved_regs, the addresses of
   the saved registers of frame described by FRAME_INFO.  This
   includes special registers such as pc and fp saved in special ways
   in the stack frame.  sp is even more special: the address we return
   for it IS the sp for the next frame. */

static struct msp430_unwind_cache * msp430_frame_unwind_cache (struct frame_info *associated_frame, void **this_prologue_cache)
{
	//Note that on GDB v7 associated_frame is the curent frame, while on gdb v6, it is the next frame
#if GDB_VERSION_INT >= 0x700
	struct frame_info *this_frame = associated_frame;
#else
	struct frame_info *next_frame = associated_frame;
#endif

	const int TargetPointerSize = 2;

	struct gdbarch *gdbarch = get_frame_arch (associated_frame);
	parsed_prologue parsed = {0, };
	/*CORE_ADDR pc;
	ULONGEST prev_sp;
	ULONGEST this_base;
	int i;*/

	ULONGEST this_pc, this_sp, this_fp, sp_iter;
	ULONGEST frame_bottom;	//Either SP or FP, depending on whether FP is used

	struct msp430_unwind_cache *info;

	if ((*this_prologue_cache))
		return (*this_prologue_cache);

	info = FRAME_OBSTACK_ZALLOC (struct msp430_unwind_cache);
	(*this_prologue_cache) = info;
	info->saved_regs = trad_frame_alloc_saved_regs (associated_frame);

	//info->size = 0;
	//info->sp_offset = 0;

	//Extract PC and SP (start of frame) for THIS frame.

#if GDB_VERSION_INT >= 0x700
	this_pc = get_frame_pc (this_frame);
	this_sp = get_frame_sp (this_frame);
	this_fp = get_frame_register_unsigned(this_frame, E_FP_REGNUM);
#else
	this_pc = frame_unwind_register_unsigned (next_frame, E_PC_REGNUM);
	this_sp = frame_unwind_register_unsigned (next_frame, E_SP_REGNUM);
	this_fp = frame_unwind_register_unsigned (next_frame, E_FP_REGNUM);
#endif

	if (!this_sp || !this_pc)
		return info;	//Cannot extract any useful frame information

	if (!ProvideValidGlobalCache(&info->global_cache))
		return info;

	msp430_scan_prologue(gdbarch, this_pc, &parsed);
	frame_bottom = this_sp;

	if (parsed.valid)
	{
		ULONGEST saved_return_ptr, retptr;
		info->uses_frame = ((parsed.flags & MY_FRAME_IN_FP) != 0);
		if (info->uses_frame && this_fp)
			frame_bottom = this_fp;

		saved_return_ptr = frame_bottom + parsed.sp_already_decremented_by;//  parsed.locals_block_size + parsed.saved_register_block_size;
		retptr = get_frame_memory_unsigned (associated_frame, saved_return_ptr, TargetPointerSize);
		if (IsValidCodePtr(&info->global_cache, retptr))
		{
			int i;
			info->prev_sp = saved_return_ptr + TargetPointerSize;
			
			if (!info->uses_frame)
				info->prev_sp += DetectPushedArgumentsSize(retptr);
			
			info->base = saved_return_ptr;
			
			//Read and save pushed register values, discovered by msp430_scan_prologue()
			for (i = 0; i < (sizeof(parsed.register_offsets)/sizeof(parsed.register_offsets[0])); i++)
			{
				ULONGEST saved_reg_addr, reg_val;
				if ((i == E_PC_REGNUM) || (i == E_SP_REGNUM))
					continue;
				if (parsed.register_offsets[i] == -1)
					continue;

				if (parsed.register_offsets[i] > parsed.sp_already_decremented_by)
					continue;	//We are now in the middle of the prologue and this register has not yet been saved to stack.

				saved_reg_addr = info->base - parsed.register_offsets[i];
				reg_val = get_frame_memory_unsigned(associated_frame, saved_reg_addr, TargetPointerSize);

				trad_frame_set_value (info->saved_regs, i, reg_val);
			}

			trad_frame_set_value (info->saved_regs, E_PC_REGNUM, retptr);
			trad_frame_set_value (info->saved_regs, E_SP_REGNUM, info->prev_sp);
			return info;
		}
		//If the saved return address is not valid, the frame was detected incorrectly. Use generic frame scanning then.
	}

	//We failed to rebuild frame structure based on function prologue. Just scan the stack for the next valid code address and assume it is a return address.

	for (sp_iter = frame_bottom; sp_iter <= info->global_cache.max_data_ptr; sp_iter += TargetPointerSize)	//Traverse stack towards the top
	{
		ULONGEST ptr = get_frame_memory_unsigned (associated_frame, sp_iter, TargetPointerSize);
		if (IsValidCodePtr(&info->global_cache, ptr))	//If a valid pointer was found on stack, assume it is the return pointer
		{
			trad_frame_set_value (info->saved_regs, E_PC_REGNUM, ptr);
			info->prev_sp = sp_iter + TargetPointerSize;
			trad_frame_set_value (info->saved_regs, E_SP_REGNUM, info->prev_sp);
			info->base = sp_iter;
			return info;
		}		
	}

	return info;
}

static void
msp430_print_registers_info (struct gdbarch *gdbarch,
                             struct ui_file *file,
                             struct frame_info *frame,
                             int regnum,
                             int all)
{
  if ((regnum >= 0) && gdbarch)
    {
      default_print_registers_info (gdbarch, file, frame, regnum, all);
      return;
    }
  {
    int r;
    for (r = 0; r < E_NUM_REGS; r++)
      {
        ULONGEST tmp = get_frame_register_unsigned(frame, r);
        //regcache_cooked_read_unsigned(get_current_regcache(), r, &tmp);
        switch (r)
        {
        case E_PC_REGNUM:
          fprintf_filtered (file, "pc/");
          break;
        case E_SP_REGNUM:
          fprintf_filtered (file, "sp/");
          break;
        case E_PSW_REGNUM:
          fprintf_filtered (file, "sr/");
          break;
        case E_FP_REGNUM:
          fprintf_filtered (file, "fp/");
          break;
        default:
          if (r < 10)
            fprintf_filtered (file, "   ");
          else
            fprintf_filtered (file, "  ");
          break;
        }
        fprintf_filtered (file, "r%d: %04lx  ", r, (long) tmp);
        if ((r%4) == 3)
          fprintf_filtered (file, "\n");
      }
  }
}

static void
show_regs (char *args, int from_tty)
{
  msp430_print_registers_info (get_frame_arch(get_current_frame()),
                               gdb_stdout,
                               get_current_frame (),
                               -1,
                               1);
}

static CORE_ADDR
msp430_read_pc (struct regcache *regcache)
{
  ULONGEST pc;
  regcache_cooked_read_unsigned(regcache, E_PC_REGNUM, &pc);

  return pc;
}

static void
msp430_write_pc (struct regcache *regcache, CORE_ADDR pc)
{
  regcache_cooked_write_unsigned(regcache, E_PC_REGNUM, pc);
}

static CORE_ADDR
msp430_unwind_sp (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  ULONGEST sp;
  sp=frame_unwind_register_unsigned (next_frame, E_SP_REGNUM);
  return sp;
}

/* When arguments must be pushed onto the stack, they go on in reverse
   order.  The below implements a FILO (stack) to do this. */

struct stack_item
{
  int len;
  struct stack_item *prev;
  void *data;
};

static struct stack_item *push_stack_item (struct stack_item *prev,
                                           const void *contents, int len);
static struct stack_item *
push_stack_item (struct stack_item *prev, const void *contents, int len)
{
  struct stack_item *si;
  si = xmalloc (sizeof (struct stack_item));
  si->data = xmalloc (len);
  si->len = len;
  si->prev = prev;
  memcpy (si->data, contents, len);
  return si;
}

static struct stack_item *pop_stack_item (struct stack_item *si);
static struct stack_item *
pop_stack_item (struct stack_item *si)
{
  struct stack_item *dead = si;
  si = si->prev;
  xfree (dead->data);
  xfree (dead);
  return si;
}

static CORE_ADDR
msp430_push_dummy_call (struct gdbarch *gdbarch,
						struct value *func_addr,
						struct regcache *regcache,
						CORE_ADDR bp_addr,
						int nargs,
						struct value **args,
						CORE_ADDR sp,
						int struct_return,
						CORE_ADDR struct_addr)
{
	int i;
	int regnum = E_FIRST_ARG_REG;
	struct stack_item *si = NULL;
	long val;
	enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

	/* Set the return address.  For the msp430, the return breakpoint is
	always at BP_ADDR. */
	//regcache_cooked_write_unsigned (regcache, LR_REGNUM, bp_addr);

	/* If STRUCT_RETURN is true, then the struct return address (in
	STRUCT_ADDR) will consume the first argument-passing register.
	Both adjust the register count and store that value. */
	if (struct_return)
	{
		regcache_cooked_write_unsigned (regcache, regnum, struct_addr);
		regnum++;
	}

	/* Fill in registers and arg lists */
	for (i = 0; i < nargs; i++)
	{
		struct value *arg = args[i];
		struct type *type = check_typedef (value_type (arg));
		const gdb_byte *contents = value_contents (arg);
		int len = TYPE_LENGTH (type);

		/* printf ("push: type=%d len=%d\n", TYPE_CODE (type), len); */
		if (len <= 2  &&  (regnum != E_LAST_ARG_REG))
			/* fits in a single register, do not align */
		{
			val = extract_unsigned_integer (contents, len, byte_order);
			regcache_cooked_write_unsigned (regcache, regnum, val);
			regnum += E_NEXT_ARG_REG_DELTA;
		}
		else 
		{
			int regs_remaining;
			static char invalid_e_last_reg[(E_LAST_ARG_REG & 1) == 0];	//Last register should be aligned by 2. Otherwise, the following code will fail horribly.
			
			if (regnum & 0x01)
				regnum += E_NEXT_ARG_REG_DELTA;

			if (E_NEXT_ARG_REG_DELTA > 0)
				regs_remaining = (E_LAST_ARG_REG - regnum) + 1;
			else
				regs_remaining = (regnum - E_LAST_ARG_REG) + 1;

			if (len <= regs_remaining)
				/* value fits in remaining registers, store keeping left
				aligned */
			{
				int b;
				for (b = 0; b < (len & ~1); b += 2)
				{
					val = extract_unsigned_integer (&contents[b], 2, byte_order);
					regcache_cooked_write_unsigned (regcache, regnum++, val);
				}
				if (b < len)
				{
					val = extract_unsigned_integer (&contents[b], 1, byte_order);
					regcache_cooked_write_unsigned (regcache, regnum++, (val << 8));
				}
			}
			else
			{
				/* arg will go onto stack */
				regnum = E_LAST_ARG_REG;
				si = push_stack_item (si, contents, len);
			}
		}
	}

	while (si)
	{
		sp = (sp - si->len) & ~1;
		write_memory (sp, si->data, si->len);
		si = pop_stack_item (si);
	}

	//Push return address
	sp -= 2;
	write_memory (sp, (gdb_byte *)&bp_addr, 2);

	/* Finally, update the SP register. */
	regcache_cooked_write_unsigned (regcache, E_SP_REGNUM, sp);

	//The SP value returned here is used to verify the dummy frame after the function has exited.
	//When the function returns, it pops the return address from the stack. Thus, the SP value will
	//be greater by sizeof(void *).
	return sp + 2;
}


/* Given a return value in `regbuf' with a type `valtype', 
   extract and copy its value into `valbuf'. */
static void msp430_extract_return_value (struct type *type, struct regcache *regcache, gdb_byte *valbuf)
{
#if GDB_VERSION_INT >= 0x700
  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
#endif
  int len;
  if (TYPE_LENGTH (type) == 1)
    {
      ULONGEST c;
      regcache_cooked_read_unsigned (regcache, RET1_REGNUM, &c);
#if GDB_VERSION_INT >= 0x700
      store_unsigned_integer (valbuf, 1, byte_order, c);
#else
      store_unsigned_integer (valbuf, 1, c);
#endif
    }
  else
    {
      /* For return values of odd size, the first byte is in the
         least significant part of the first register.  The
         remaining bytes in remaining registers. Interestingly, when
         such values are passed in, the last byte is in the most
         significant byte of that same register - wierd. */
      int reg = RET1_REGNUM;
      int off = 0;
      if (TYPE_LENGTH (type) & 1)
        {
          regcache_cooked_read_part (regcache, reg, 1, 1,
                                     (bfd_byte *)valbuf + off);
          off++;
          reg++;
        }
      /* Transfer the remaining registers. */
      for (; off < TYPE_LENGTH (type); reg++, off += 2)
        {
          regcache_cooked_read (regcache, reg,
                                (bfd_byte *) valbuf + off);
        }
    }
}

static enum return_value_convention
#if GDB_VERSION_INT < 0x700
msp430_return_value (struct gdbarch *gdbarch, struct type *valtype,
                        struct regcache *regcache,
                        gdb_byte *readbuf, const gdb_byte *writebuf)
#else
msp430_return_value (struct gdbarch *gdbarch, struct type *func_type,
		  struct type *valtype, struct regcache *regcache,
		  gdb_byte *readbuf, const gdb_byte *writebuf)
#endif
{
  if (msp430_use_struct_convention (valtype))
    return RETURN_VALUE_STRUCT_CONVENTION;
  if (writebuf)
    msp430_store_return_value (valtype, regcache, writebuf);
  else if (readbuf)
    msp430_extract_return_value (valtype, regcache, readbuf);
  return RETURN_VALUE_REGISTER_CONVENTION;
}


/* Translate a GDB virtual ADDR/LEN into a format the remote target
   understands.  Returns number of bytes that can be transfered
   starting at TARG_ADDR.  Return ZERO if no bytes can be transfered
   (segmentation fault).  Since the simulator knows all about how the
   VM system works, we just call that to do the translation. */

static void eraseflash_command (char *, int);

static void mcu_info (char *, int);

/* MSP430 specific releasejtag command sets this variable */
static int release_jtag_on_go;

/* MSP430 specific "erase" command */
static void
eraseflash_command (char *args, int from_tty)
{
  /* Erase a section of flash memory. */

  if (args == NULL  ||  (strcmp(args, "all") == 0))
  {
    /* No args or "all" */
    printf_filtered ("Erasing all flash memory\n");
    current_target.to_rcmd ("erase all", gdb_stdtarg);
  }
  else if (strcmp(args, "info") == 0)
  {
    /* 1st arg "info" */
    printf_filtered ("Erasing info flash memory\n");
    current_target.to_rcmd ("erase info", gdb_stdtarg);
  }
  else if (strcmp(args, "main") == 0)
  {
    /* 1st arg "main" */
    printf_filtered ("Erasing main flash memory\n");
    current_target.to_rcmd ("erase main", gdb_stdtarg);
  }
  else
  {
    /* Address as arg */
    printf_filtered ("Erasing flash memory from %d for %d bytes\n", 0, 0);
    current_target.to_rcmd ("erase ???", gdb_stdtarg);
    //snprintf(buf2, 1000, "segment at 0x%04llx, size %ld bytes...", address, size);
  }
}

/* MSP430 specific "info mcu" command, which displays information about the attached device
   (part number, memory limits, etc.). */
static void
mcu_info (char *args, int from_tty)
{
  current_target.to_rcmd ("identify", gdb_stdtarg);
}

/* Collect trace data from the target board and format it into a form
   more useful for display. */

static CORE_ADDR
msp430_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  ULONGEST pc;
  pc=frame_unwind_register_unsigned (next_frame, E_PC_REGNUM);
  return pc;
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct. */

#if GDB_VERSION_INT >= 0x700
static void msp430_frame_this_id (struct frame_info *this_frame,
                      void **this_prologue_cache,
                      struct frame_id *this_id)
{
  struct msp430_unwind_cache *info
    = msp430_frame_unwind_cache (this_frame, this_prologue_cache);
  CORE_ADDR base;
  CORE_ADDR func;
  struct frame_id id;

  /* The FUNC is easy. */
  func = get_frame_func (this_frame);

  /* Hopefully the prologue analysis either correctly determined the
     frame's base (which is the SP from the previous frame), or set
     that base to "NULL".  */
  base = info->prev_sp;
  if (base == 0)
    return;

  id = frame_id_build (base, func);
  (*this_id) = id;
}
#else
static void msp430_frame_this_id (struct frame_info *next_frame,
                      void **this_prologue_cache,
                      struct frame_id *this_id)
{
  struct msp430_unwind_cache *info
    = msp430_frame_unwind_cache (next_frame, this_prologue_cache);
  CORE_ADDR base;
  CORE_ADDR func;
  struct frame_id id;

  /* The FUNC is easy. */
  func = frame_func_unwind (next_frame, NORMAL_FRAME);

  /* Hopefully the prologue analysis either correctly determined the
     frame's base (which is the SP from the previous frame), or set
     that base to "NULL".  */
  base = info->prev_sp;
  if (base == 0)
    return;

  id = frame_id_build (base, func);

  /* Check that we're not going round in circles with the same frame
     ID (but avoid applying the test to sentinel frames which do go
     round in circles).  Can't use frame_id_eq() as that doesn't yet
     compare the frame's PC value.  */
  if (frame_relative_level (next_frame) >= 0
      && get_frame_type (next_frame) != DUMMY_FRAME
      && frame_id_eq (get_frame_id (next_frame), id))
    return;

  (*this_id) = id;
}
#endif

#if GDB_VERSION_INT < 0x700
static void
#else
static struct value *
#endif
msp430_frame_prev_register (struct frame_info *associated_frame,
                            void **this_prologue_cache,
                            int regnum
							
#if GDB_VERSION_INT < 0x700
							,
                            int *optimizedp,
                            enum lval_type *lvalp,
                            CORE_ADDR *addrp,
                            int *realnump,
                            gdb_byte *bufferp
#endif
							
							)
{
  struct msp430_unwind_cache *info
    = msp430_frame_unwind_cache (associated_frame, this_prologue_cache);
  return trad_frame_get_prev_register (associated_frame, info->saved_regs, regnum
#if GDB_VERSION_INT < 0x700
							, optimizedp, lvalp, addrp, realnump, bufferp
#endif
							);
}

static int msp430_epilogue_frame_sniffer (const struct frame_unwind *self, struct frame_info *this_frame, void **this_cache)
{
	struct msp430_unwind_cache *info = msp430_build_unwind_cache_from_epilogue (this_frame, this_cache);
	return (info != NULL);
}


static const struct frame_unwind msp430_frame_unwind = {
  NORMAL_FRAME,
  msp430_frame_this_id,
  msp430_frame_prev_register,
#if GDB_VERSION_INT >= 0x700
  NULL,
  default_frame_sniffer,
#endif
};

static const struct frame_unwind msp430_epilogue_frame_unwind = {
	NORMAL_FRAME,
	msp430_epilogue_frame_this_id,
	msp430_frame_prev_register,
	NULL,
	msp430_epilogue_frame_sniffer,
};

static const struct frame_unwind *
msp430_frame_sniffer (struct frame_info *next_frame)
{
  return &msp430_frame_unwind;
}

static CORE_ADDR
msp430_frame_base_address (struct frame_info *associated_frame, void **this_cache)
{
  struct msp430_unwind_cache *info = msp430_frame_unwind_cache (associated_frame, this_cache);
  return info->base;
}

static const struct frame_base msp430_frame_base = {
  &msp430_frame_unwind,
  msp430_frame_base_address,
  msp430_frame_base_address,
  msp430_frame_base_address
};

/* Assuming NEXT_FRAME->prev is a dummy, return the frame ID of that
   dummy frame.  The frame ID's base needs to match the TOS value
   saved by save_dummy_frame_tos(), and the PC match the dummy frame's
   breakpoint. */

  
#if GDB_VERSION_INT < 0x700
static struct frame_id
msp430_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_id_build (msp430_unwind_sp (gdbarch, next_frame),
                         frame_pc_unwind (next_frame));
}
#else

static struct frame_id
msp430_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  ULONGEST base;
  base = get_frame_register_unsigned (this_frame, E_SP_REGNUM);
  return frame_id_build (base, get_frame_pc (this_frame));
}

#endif

static gdbarch_init_ftype msp430_gdbarch_init;

static struct gdbarch *
msp430_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;

  /* Find a candidate among the list of pre-declared architectures. */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != NULL)
    return arches->gdbarch;

  /* None found, create a new architecture from the information
     provided. */
  tdep = XMALLOC (struct gdbarch_tdep);
  gdbarch = gdbarch_alloc (&info, tdep);

  switch (info.bfd_arch_info->mach)
    {
    default:
      break;
    }

  set_gdbarch_short_bit (gdbarch, 2 * TARGET_CHAR_BIT);
  set_gdbarch_int_bit (gdbarch, 2 * TARGET_CHAR_BIT);
  set_gdbarch_long_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_long_long_bit (gdbarch, 8 * TARGET_CHAR_BIT);
  set_gdbarch_ptr_bit (gdbarch, 2 * TARGET_CHAR_BIT);
  set_gdbarch_addr_bit (gdbarch, 16);

  /* NOTE: The msp430 has 32 bit 'float' and 'double'. 'long double' is 64 bits. */
  set_gdbarch_float_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_double_bit (gdbarch, 4 * TARGET_CHAR_BIT);
  set_gdbarch_long_double_bit (gdbarch, 8 * TARGET_CHAR_BIT);

  set_gdbarch_float_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_double_format (gdbarch, floatformats_ieee_single);
  set_gdbarch_long_double_format (gdbarch, floatformats_ieee_double);

  set_gdbarch_read_pc (gdbarch, msp430_read_pc);
  set_gdbarch_write_pc (gdbarch, msp430_write_pc);
  set_gdbarch_unwind_sp (gdbarch, msp430_unwind_sp);

  set_gdbarch_num_regs (gdbarch, E_NUM_REGS);

  set_gdbarch_sp_regnum (gdbarch, E_SP_REGNUM);
  set_gdbarch_pc_regnum (gdbarch, E_PC_REGNUM);

  set_gdbarch_register_name (gdbarch, msp430_register_name);
  set_gdbarch_register_type (gdbarch, msp430_register_type);

  set_gdbarch_return_value (gdbarch, msp430_return_value);
  set_gdbarch_print_insn (gdbarch, print_insn_msp430);

  set_gdbarch_address_to_pointer (gdbarch, msp430_address_to_pointer);
  set_gdbarch_pointer_to_address (gdbarch, msp430_pointer_to_address);
  set_gdbarch_integer_to_address (gdbarch, msp430_integer_to_address);

  /* set_gdbarch_use_struct_convention (gdbarch, msp430_use_struct_convention); */

  set_gdbarch_skip_prologue (gdbarch, msp430_skip_prologue);
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);

  set_gdbarch_decr_pc_after_break (gdbarch, 4);
  
  /* These values and methods are used when gdb calls a target function. */
  set_gdbarch_push_dummy_call (gdbarch, msp430_push_dummy_call);
  set_gdbarch_breakpoint_from_pc (gdbarch, msp430_breakpoint_from_pc);
  set_gdbarch_return_value (gdbarch, msp430_return_value);

  /* set_gdbarch_function_start_offset (gdbarch, 0); */

  set_gdbarch_frame_args_skip (gdbarch, 0);
  /* set_gdbarch_frameless_function_invocation (gdbarch, frameless_look_for_prologue); */

  /*set_gdbarch_store_return_value (gdbarch, msp430_store_return_value);*/

  set_gdbarch_frame_align (gdbarch, msp430_frame_align);

  set_gdbarch_print_registers_info (gdbarch, msp430_print_registers_info);

#if GDB_VERSION_INT >= 0x700
  frame_unwind_append_unwinder (gdbarch, &msp430_epilogue_frame_unwind);
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &msp430_frame_unwind);
#else
  frame_unwind_prepend_unwinder(gdbarch, &msp430_epilogue_frame_unwind);
  frame_unwind_append_sniffer (gdbarch, dwarf2_frame_sniffer);
  frame_unwind_append_sniffer (gdbarch, msp430_frame_sniffer);
#endif

  
  frame_base_set_default (gdbarch, &msp430_frame_base);

  /* Methods for saving / extracting a dummy frame's ID.  The ID's
     stack address must match the SP value returned by
     PUSH_DUMMY_CALL, and saved by generic_save_dummy_frame_tos. */
#if GDB_VERSION_INT >= 0x700
  set_gdbarch_dummy_id (gdbarch, msp430_dummy_id);
#else
  set_gdbarch_unwind_dummy_id (gdbarch, msp430_unwind_dummy_id);
#endif

  set_gdbarch_unwind_pc (gdbarch, msp430_unwind_pc);

  return gdbarch;
}

void
_initialize_msp430_tdep (void)
{
  register_gdbarch_init (bfd_arch_msp430, msp430_gdbarch_init);

  //target_resume_hook = msp430_prepare_to_trace;
  //target_wait_loop_hook = msp430_get_trace_data;

  /* Add backwards compatible deprecated MSP430 specific commands */
  deprecate_cmd (add_com ("regs", class_vars, show_regs, "Print all registers"),
                 "info registers");

  /* Add new MSP430 specific commands */
  add_com ("eraseflash",
           class_support,
           eraseflash_command,
           "Erase flash memory.");

  add_info ("mcu",
            mcu_info,
            "Display info about the attached MCU.");

  add_setshow_boolean_cmd ("releasejtag",
                           no_class,
                           &release_jtag_on_go,
                           "Set release JTAG pins on go.\n",
                           "Show release JTAG pins on go.\n",
                           "Use on to set pins and off to release them... ahh I dont know really.\n",
                           NULL,
                           NULL,
                           &setlist,
                           &showlist);
  remote_timeout = 999999999;
}

//----------------------------------------------------------------------------------------------------------------------

bool ProvideValidGlobalCache(msp430_global_cache *pCache)
{
	struct minimal_symbol *pSym = NULL;

	if (pCache->cache_valid)
		return true;
	pSym = lookup_minimal_symbol ("__data_start", NULL, NULL);
	if (!pSym)
		return false;
	pCache->min_data_ptr = pSym->ginfo.value.address;

	pSym = lookup_minimal_symbol ("__stack", NULL, NULL);
	if (!pSym)
		return false;
	pCache->max_data_ptr = pSym->ginfo.value.address - 1;

	pSym = lookup_minimal_symbol ("__dtors_end", NULL, NULL);
	if (!pSym)
		return false;
	pCache->min_code_ptr = pSym->ginfo.value.address;

	pSym = lookup_minimal_symbol ("_etext", NULL, NULL);
	if (!pSym)
		return false;
	pCache->max_code_ptr= pSym->ginfo.value.address - 1;

	pCache->cache_valid = true;
	return true;
}

bool IsValidCodePtr(msp430_global_cache *pCache, ULONGEST ptr)
{
	if (!ProvideValidGlobalCache(pCache))
		return false;
	return (ptr >= pCache->min_code_ptr) && (ptr <= pCache->max_code_ptr);
}

bool IsValidDataPtr(msp430_global_cache *pCache, ULONGEST ptr)
{
	if (!ProvideValidGlobalCache(pCache))
		return false;
	return (ptr >= pCache->min_data_ptr) && (ptr <= pCache->max_data_ptr);
}
