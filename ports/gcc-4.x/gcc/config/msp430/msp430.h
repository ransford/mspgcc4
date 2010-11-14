/* This work is partially financed by the European Commission under the
 * Framework 6 Information Society Technologies Project
 * "Wirelessly Accessible Sensor Populations (WASP)".
 */

/* Definitions of target machine for GNU compiler,
   for Texas Instruments MSP430 microcontrollers.
   Copyright (C) 2001-2009 Free Software Foundation, Inc.
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


/* This declaration should be present. */
/*
#include <stdio.h>
*/

#include "_gccver.h"

#define TARGET_CPU_CPP_BUILTINS()                       \
do{							\
  builtin_define_std ("MSP430");			\
  builtin_define_std ("__MSP430__");			\
  if(msp430_has_hwmul)					\
    builtin_define ("MSP430_HAS_HWMUL");		\
  else							\
    builtin_define ("MSP430_NO_HWMUL");		        \
}while(0)


extern int target_flags;

extern const char *msp430_endup;
extern const char *msp430_init_stack;
extern const char *msp430_mcu_name;
extern int msp430_has_hwmul;


#define MSP430_HAS_HWMUL_INTERNAL (msp430_has_hwmul)

int msp430_current_function_noint_hwmul_function_p(void);
#define MSP430_NOINT_HWMUL (msp430_current_function_noint_hwmul_function_p())


#define TARGET_VERSION fprintf (stderr, " (GNU assembler syntax)");

#define OVERRIDE_OPTIONS msp430_override_options()

/*#define FRAME_POINTER_CFA_OFFSET(FNDECL) 0*/
#define CAN_DEBUG_WITHOUT_FP
/* Define this macro if debugging can be performed even without a
   frame pointer.  If this macro is defined, GNU CC will turn on the
   `-fomit-frame-pointer' option whenever `-O' is specified.  */

#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN 0
#define WORDS_BIG_ENDIAN 0
#define BITS_PER_UNIT 8
#define BITS_PER_WORD 16

#ifdef IN_LIBGCC2
/* This is to get correct SI and DI modes in libgcc2.c (32 and 64 bits).  */
#define UNITS_PER_WORD 4
#else
/* Width of a word, in units (bytes). */
#define UNITS_PER_WORD 2
#endif

/* Width in bits of a pointer.
   See also the macro `Pmode' defined below.  */
#define POINTER_SIZE 16


/* Maximum sized of reasonable data type
   DImode or Dfmode ...  */
#define MAX_FIXED_MODE_SIZE 32

/* Allocation boundary (in *bits*) for storing arguments in argument list. */
#define PARM_BOUNDARY 16

/* Allocation boundary (in *bits*) for the code of a function. */
#define FUNCTION_BOUNDARY 16

/* Alignment of field after `int : 0' in a structure. */
#define EMPTY_FIELD_BOUNDARY 16

/* No data type wants to be aligned rounder than this. */
#define BIGGEST_ALIGNMENT 16

/* Every structure's size must be a multiple of this.  */
#define STRUCTURE_SIZE_BOUNDARY 16

/* Define this if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT 1

/* A C expression for the size in bits of the type `int' on the
     target machine.  If you don't define this, the default is one word.  */
#define INT_TYPE_SIZE ( 16)


/* A C expression for the size in bits of the type `short' on the
   target machine.  If you don't define this, the default is half a
   word.  (If this would be less than one storage unit, it is rounded
   up to one unit.)  */
#define SHORT_TYPE_SIZE (INT_TYPE_SIZE == 8 ? INT_TYPE_SIZE : 16)

/* A C expression for the size in bits of the type `long' on the
   target machine.  If you don't define this, the default is one word.  */
#define LONG_TYPE_SIZE (INT_TYPE_SIZE == 8 ? 16 : 32)

#define LONG_LONG_TYPE_SIZE 64
/* A C expression for the size in bits of the type `long long' on the
   target machine.  If you don't define this, the default is two
   words.  If you want to support GNU Ada on your machine, the value
   of macro must be at least 64.  */


#define  CHAR_TYPE_SIZE 8
/* A C expression for the size in bits of the type `char' on the
   target machine.  If you don't define this, the default is one
   quarter of a word.  (If this would be less than one storage unit,
   it is rounded up to one unit.)  */

#define FLOAT_TYPE_SIZE 32
/* A C expression for the size in bits of the type `float' on the
   target machine.  If you don't define this, the default is one word.  */

#define DOUBLE_TYPE_SIZE 32
/* A C expression for the size in bits of the type `double' on the
   target machine.  If you don't define this, the default is two
   words. */


#define LONG_DOUBLE_TYPE_SIZE 32
/* A C expression for the size in bits of the type `long double' on
   the target machine.  If you don't define this, the default is two
   words.  */

#define DEFAULT_SIGNED_CHAR 1
/* An expression whose value is 1 or 0, according to whether the type
   `char' should be signed or unsigned by default.  The user can
   always override this default with the options `-fsigned-char' and
   `-funsigned-char'.  */

/* `DEFAULT_SHORT_ENUMS'
   A C expression to determine whether to give an `enum' type only as
   many bytes as it takes to represent the range of possible values
   of that type.  A nonzero value means to do that; a zero value
   means all `enum' types should be allocated like `int'.

   If you don't define the macro, the default is 0.  */

#define SIZE_TYPE (INT_TYPE_SIZE == 8 ? "long unsigned int" : "unsigned int")
/* A C expression for a string describing the name of the data type
   to use for size values.  The typedef name `size_t' is defined
   using the contents of the string.
   
   The string can contain more than one keyword.  If so, separate
   them with spaces, and write first any length keyword, then
   `unsigned' if appropriate, and finally `int'.  The string must
   exactly match one of the data type names defined in the function
   `init_decl_processing' in the file `c-decl.c'.  You may not omit
   `int' or change the order--that would cause the compiler to crash
   on startup.
   
   If you don't define this macro, the default is `"long unsigned
   int"'.  */

#define PTRDIFF_TYPE (INT_TYPE_SIZE == 8 ? "long int" :"int")
/* A C expression for a string describing the name of the data type
   to use for the result of subtracting two pointers.  The typedef
   name `ptrdiff_t' is defined using the contents of the string.  See
   `SIZE_TYPE' above for more information.
   
   If you don't define this macro, the default is `"long int"'.  */


#define WCHAR_TYPE_SIZE 16
/* A C expression for the size in bits of the data type for wide
   characters.  This is used in `cpp', which cannot make use of
   `WCHAR_TYPE'.  */

#define FIRST_PSEUDO_REGISTER 17
/* Number of hardware registers known to the compiler.  They receive
   numbers 0 through `FIRST_PSEUDO_REGISTER-1'; thus, the first
   pseudo register's number really is assigned the number
   `FIRST_PSEUDO_REGISTER'.  */

/*#define DWARF_FRAME_REGISTERS 16*/

#define FIXED_REGISTERS {\
  1,1,/* r0 r1 == PC  SP */\
  1,1,/* r2 r3 == CG1(SR) CG2*/\
  0,0,/* r4 r5 */\
  0,0,/* r6 r7 */\
  0,0,/* r8 r9 */\
  0,0,/* r10 r11 */\
  0,0,/* r12 r13 */\
  0,0,/* r14 r15 */\
  1, /* argp register */\
}
/* An initializer that says which registers are used for fixed
   purposes all throughout the compiled code and are therefore not
   available for general allocation.  These would include the stack
   pointer, the frame pointer (except on machines where that can be
   used as a general register when no frame pointer is needed), the
   program counter on machines where that is considered one of the
   addressable registers, and any other numbered register with a
   standard use.

   This information is expressed as a sequence of numbers, separated
   by commas and surrounded by braces.  The Nth number is 1 if
   register N is fixed, 0 otherwise.

   The table initialized from this macro, and the table initialized by
   the following one, may be overridden at run time either
   automatically, by the actions of the macro
   `CONDITIONAL_REGISTER_USAGE', or by the user with the command
   options `-ffixed-REG', `-fcall-used-REG' and `-fcall-saved-REG'.  */

#define CALL_USED_REGISTERS {			\
    1,1,/* r0 r1 */				\
    1,1,/* r2 r3 */				\
    0,0,/* r4 r5 */				\
    0,0,/* r6 r7 */				\
    0,0,/* r8 r9 */				\
    0,0,/* r10 r11 */				\
    1,1,/* r12 r13 */				\
    1,1,/* r14 r15 */				\
	1, \
}
/* Like `FIXED_REGISTERS' but has 1 for each register that is
   clobbered (in general) by function calls as well as for fixed
   registers.  This macro therefore identifies the registers that are
   not available for general allocation of values that must live
   across function calls.

   If a register has 0 in `CALL_USED_REGISTERS', the compiler
   automatically saves it on function entry and restores it on
   function exit, if the register is used within the function.  */

#define REG_ALLOC_ORDER { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 }

/* If defined, an initializer for a vector of integers, containing the
   numbers of hard registers in the order in which GNU CC should
   prefer to use them (from most preferred to least).
   
   If this macro is not defined, registers are used lowest numbered
   first (all else being equal).
   
   One use of this macro is on machines where the highest numbered
   registers must always be saved and the save-multiple-registers
   instruction supports only sequences of consetionve registers.  On
   such machines, define `REG_ALLOC_ORDER' to be an initializer that
   lists the highest numbered allocatable register first. */

#define ORDER_REGS_FOR_LOCAL_ALLOC order_regs_for_local_alloc ()
/* ORDER_REGS_FOR_LOCAL_ALLOC'
   A C statement (sans semicolon) to choose the order in which to
   allocate hard registers for pseudo-registers local to a basic
   block.

   Store the desired register order in the array `reg_alloc_order'.
   Element 0 should be the register to allocate first; element 1, the
   next register; and so on.

   The macro body should not assume anything about the contents of
   `reg_alloc_order' before execution of the macro.

   On most machines, it is not necessary to define this macro.  */


#define HARD_REGNO_NREGS(REGNO, MODE) \
((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

/* A C expression for the number of consecutive hard registers,
   starting at register number REGNO, required to hold a value of mode
   MODE.

   On a machine where all registers are exactly one word, a suitable
   definition of this macro is

   #define HARD_REGNO_NREGS(REGNO, MODE)            \
   ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1)  \
   / UNITS_PER_WORD))  */

#define HARD_REGNO_MODE_OK(REGNO, MODE) 1
/*
msp430_hard_regno_mode_ok(REGNO, MODE)
*/
/* A C expression that is nonzero if it is permissible to store a
   value of mode MODE in hard register number REGNO (or in several
   registers starting with that one).  For a machine where all
   registers are equivalent, a suitable definition is

   #define HARD_REGNO_MODE_OK(REGNO, MODE) 1

   It is not necessary for this macro to check for the numbers of
   fixed registers, because the allocation mechanism considers them
   to be always occupied.

   On some machines, double-precision values must be kept in even/odd
   register pairs.  The way to implement that is to define this macro
   to reject odd register numbers for such modes.

   The minimum requirement for a mode to be OK in a register is that
   the `movMODE' instruction pattern support moves between the
   register and any other hard register for which the mode is OK; and
   that moving a value into the register and back out not alter it.

   Since the same instruction used to move `SImode' will work for all
   narrower integer modes, it is not necessary on any machine for
   `HARD_REGNO_MODE_OK' to distinguish between these modes, provided
   you define patterns `movhi', etc., to take advantage of this.  This
   is useful because of the interaction between `HARD_REGNO_MODE_OK'
   and `MODES_TIEABLE_P'; it is very desirable for all integer modes
   to be tieable.

   Many machines have special registers for floating point arithmetic.
   Often people assume that floating point machine modes are allowed
   only in floating point registers.  This is not true.  Any
   registers that can hold integers can safely *hold* a floating
   point machine mode, whether or not floating arithmetic can be done
   on it in those registers.  Integer move instructions can be used
   to move the values.

   On some machines, though, the converse is true: fixed-point machine
   modes may not go in floating registers.  This is true if the
   floating registers normalize any value stored in them, because
   storing a non-floating value there would garble it.  In this case,
   `HARD_REGNO_MODE_OK' should reject fixed-point machine modes in
   floating registers.  But if the floating registers do not
   automatically normalize, if you can store any bit pattern in one
   and retrieve it unchanged without a trap, then any machine mode
   may go in a floating register, so you can define this macro to say
   so.

   The primary significance of special floating registers is rather
   that they are the registers acceptable in floating point arithmetic
   instructions.  However, this is of no concern to
   `HARD_REGNO_MODE_OK'.  You handle it by writing the proper
   constraints for those instructions.

   On some machines, the floating registers are especially slow to
   access, so that it is better to store a value in a stack frame
   than in such a register if floating point arithmetic is not being
   done.  As long as the floating registers are not in class
   `GENERAL_REGS', they will not be used unless some pattern's
   constraint asks for one.  */

#define MODES_TIEABLE_P(MODE1, MODE2)           	\
0
/*
  ((MODE1) == (MODE2)                           	\
     || ((MODE1) == QImode && (MODE2) == HImode)	\
     || ((MODE1) == HImode && (MODE2) == QImode))
*/       
/* A C expression that is nonzero if it is desirable to choose
   register allocation so as to avoid move instructions between a
   value of mode MODE1 and a value of mode MODE2.

   If `HARD_REGNO_MODE_OK (R, MODE1)' and `HARD_REGNO_MODE_OK (R,
   MODE2)' are ever different for any R, then `MODES_TIEABLE_P (MODE1,
   MODE2)' must be zero.  */


enum reg_class {
  NO_REGS,
  PC_REG,			/* r0  - PC	*/
  SP_REG,			/* r1  - SP	*/
  STACK_REGS,			/* r2  - SR	*/
  CG1_REG,			/* r2  - CG1	*/
  CG2_REG,			/* r3  - CG2	*/
  CG_REGS,			/* r2, r3	*/
  GENERAL_REGS,			/* r4 - r15 	*/
  POINTER_REGS,
  FFOUR_REG,
  ALL_REGS, LIM_REG_CLASSES
};
  
/* An enumeral type that must be defined with all the register class
   names as enumeral values.  `NO_REGS' must be first.  `ALL_REGS'
   must be the last register class, followed by one more enumeral
   value, `LIM_REG_CLASSES', which is not a register class but rather
   tells how many classes there are.

   Each register class has a number, which is the value of casting
   the class name to type `int'.  The number serves as an index in
   many of the tables described below.  */


#define N_REG_CLASSES (int) LIM_REG_CLASSES
/* The number of distinct register classes, defined as follows:
   #define N_REG_CLASSES (int) LIM_REG_CLASSES  */

#define REG_CLASS_NAMES {		\
		"NO_REGS",		\
		"PC_REG",		\
		"SP_REG",		\
		"STACK_REGS",		\
		"CG1_REG",		\
		"CG2_REG",		\
		"CG_REGS",		\
		"GENERAL_REGS",		\
		"POINTER_REGS",		\
		"FFOUR_REG",		\
		"ALL_REGS" 		\
}

/* An initializer containing the names of the register classes as C
   string constants.  These names are used in writing some of the
   debugging dumps.  */

#define REG_CLASS_CONTENTS {				\
  {0x00000000ul},	/* NO_REGS	*/	\
  {0x00000001ul},	/* PC_REG 	*/	\
  {0x00000002ul},	/* SP_REG 	*/	\
  {0x00000004ul},	/* r2		*/	\
  {0x00000004ul},	/* r2		*/	\
  {0x00000008ul},	/* r3		*/	\
  {0x0000000cul},	/* r2,r3	*/	\
  {0x0001fff2ul}, /* r4 - r15,r1  */      \
  {0x0001fff2ul}, /* r4 - r15,r1  */      \
  {0x0001fff0ul},	/* r4 - r15	*/	\
  {0x0001fffful}	/* ALL_REGS */		\
}
/* An initializer containing the contents of the register classes, as
   integers which are bit masks.  The Nth integer specifies the
   contents of class N.  The way the integer MASK is interpreted is
   that register R is in the class if `MASK & (1 << R)' is 1.

   When the machine has more than 32 registers, an integer does not
   suffice.  Then the integers are replaced by sub-initializers,
   braced groupings containing several integers.  Each
   sub-initializer must be suitable as an initializer for the type
   `HARD_REG_SET' which is defined in `hard-reg-set.h'.  */


enum reg_class msp430_regno_reg_class PARAMS ((int));
#define REGNO_REG_CLASS(R) msp430_regno_reg_class(R)
/* A C expression whose value is a register class containing hard
   register REGNO.  In general there is more than one such class;
   choose a class which is "minimal", meaning that no smaller class
   also contains the register.  */

#define BASE_REG_CLASS POINTER_REGS
/* A macro whose definition is the name of the class to which a valid
   base register must belong.  A base register is one used in an
   address which is the register value plus a displacement.  */

#define INDEX_REG_CLASS NO_REGS
/* A macro whose definition is the name of the class to which a valid
   index register must belong.  An index register is one used in an
   address where its value is either multiplied by a scale factor or
   added to another register (as well as added to a displacement).  */

#define REG_CLASS_FROM_LETTER(C) msp430_reg_class_from_letter(C)
/* A C expression which defines the machine-dependent operand
   constraint letters for register classes.  If CHAR is such a
   letter, the value should be the register class corresponding to
   it.  Otherwise, the value should be `NO_REGS'.  The register
   letter `r', corresponding to class `GENERAL_REGS', will not be
   passed to this macro; you do not need to handle it.  */

#define REGNO_OK_FOR_BASE_P(r) msp430_regno_ok_for_base_p(r) 
					
/* A C expression which is nonzero if register number NUM is suitable
   for use as a base register in operand addresses.  It may be either
   a suitable hard register or a pseudo register that has been
   allocated such a hard register.  */

/* #define REGNO_MODE_OK_FOR_BASE_P(r, m) 
   A C expression that is just like `REGNO_OK_FOR_BASE_P', except that
   that expression may examine the mode of the memory reference in
   MODE.  You should define this macro if the mode of the memory
   reference affects whether a register may be used as a base
   register.  If you define this macro, the compiler will use it
   instead of `REGNO_OK_FOR_BASE_P'.  */

#define REGNO_OK_FOR_INDEX_P(NUM) 0
/* A C expression which is nonzero if register number NUM is suitable
   for use as an index register in operand addresses.  It may be
   either a suitable hard register or a pseudo register that has been
   allocated such a hard register.

   The difference between an index register and a base register is
   that the index register may be scaled.  If an address involves the
   sum of two registers, neither one of them scaled, then either one
   may be labeled the "base" and the other the "index"; but whichever
   labeling is used must fit the machine's constraints of which
   registers may serve in each capacity.  The compiler will try both
   labelings, looking for one that is valid, and will reload one or
   both registers only if neither labeling works.  */

#define PREFERRED_RELOAD_CLASS(X, CLASS) FFOUR_REG

/*
referred_reload_class(X,CLASS)
 A C expression that places additional restrictions on the register
   class to use when it is necessary to copy value X into a register
   in class CLASS.  The value is a register class; perhaps CLASS, or
   perhaps another, smaller class.  On many machines, the following
   definition is safe:

   #define PREFERRED_RELOAD_CLASS(X,CLASS) CLASS

   Sometimes returning a more restrictive class makes better code.
   For example, on the 68000, when X is an integer constant that is
   in range for a `moveq' instruction, the value of this macro is
   always `DATA_REGS' as long as CLASS includes the data registers.
   Requiring a data register guarantees that a `moveq' will be used.

   If X is a `const_double', by returning `NO_REGS' you can force X
   into a memory constant.  This is useful on certain machines where
   immediate floating values cannot be loaded into certain kinds of
   registers.  */
/* `PREFERRED_OUTPUT_RELOAD_CLASS (X, CLASS)'
   Like `PREFERRED_RELOAD_CLASS', but for output reloads instead of
   input reloads.  If you don't define this macro, the default is to
   use CLASS, unchanged.  */

/* `LIMIT_RELOAD_CLASS (MODE, CLASS)'
   A C expression that places additional restrictions on the register
   class to use when it is necessary to be able to hold a value of
   mode MODE in a reload register for which class CLASS would
   ordinarily be used.

   Unlike `PREFERRED_RELOAD_CLASS', this macro should be used when
   there are certain modes that simply can't go in certain reload
   classes.

   The value is a register class; perhaps CLASS, or perhaps another,
   smaller class.

   Don't define this macro unless the target machine has limitations
   which require the macro to do something nontrivial.  */

/*
#define SECONDARY_RELOAD_CLASS(CLASS, MODE, X) GENERAL_REGS


#define  SECONDARY_INPUT_RELOAD_CLASS(CLASS, MODE, X) GENERAL_REGS


   `SECONDARY_RELOAD_CLASS (CLASS, MODE, X)'
   `SECONDARY_OUTPUT_RELOAD_CLASS (CLASS, MODE, X)'
   Many machines have some registers that cannot be copied directly
   to or from memory or even from other types of registers.  An
   example is the `MQ' register, which on most machines, can only be
   copied to or from general registers, but not memory.  Some
   machines allow copying all registers to and from memory, but
   require a scratch register for stores to some memory locations
   (e.g., those with symbolic address on the RT, and those with
   certain symbolic address on the Sparc when compiling PIC).  In
   some cases, both an intermediate and a scratch register are
   required.

   You should define these macros to indicate to the reload phase
   that it may need to allocate at least one register for a reload in
   addition to the register to contain the data.  Specifically, if
   copying X to a register CLASS in MODE requires an intermediate
   register, you should define `SECONDARY_INPUT_RELOAD_CLASS' to
   return the largest register class all of whose registers can be
   used as intermediate registers or scratch registers.

   If copying a register CLASS in MODE to X requires an intermediate
   or scratch register, `SECONDARY_OUTPUT_RELOAD_CLASS' should be
   defined to return the largest register class required.  If the
   requirements for input and output reloads are the same, the macro
   `SECONDARY_RELOAD_CLASS' should be used instead of defining both
   macros identically.

   The values returned by these macros are often `GENERAL_REGS'.
   Return `NO_REGS' if no spare register is needed; i.e., if X can be
   directly copied to or from a register of CLASS in MODE without
   requiring a scratch register.  Do not define this macro if it
   would always return `NO_REGS'.

   If a scratch register is required (either with or without an
   intermediate register), you should define patterns for
   `reload_inM' or `reload_outM', as required (*note Standard
   Names::..  These patterns, which will normally be implemented with
   a `define_expand', should be similar to the `movM' patterns,
   except that operand 2 is the scratch register.

   Define constraints for the reload register and scratch register
   that contain a single register class.  If the original reload
   register (whose class is CLASS) can meet the constraint given in
   the pattern, the value returned by these macros is used for the
   class of the scratch register.  Otherwise, two additional reload
   registers are required.  Their classes are obtained from the
   constraints in the insn pattern.

   X might be a pseudo-register or a `subreg' of a pseudo-register,
   which could either be in a hard register or in memory.  Use
   `true_regnum' to find out; it will return -1 if the pseudo is in
   memory and the hard register number if it is in a register.

   These macros should not be used in the case where a particular
   class of registers can only be copied to memory and not to another
   class of registers.  In that case, secondary reload registers are
   not needed and would not be helpful.  Instead, a stack location
   must be used to perform the copy and the `movM' pattern should use
   memory as a intermediate storage.  This case often occurs between
   floating-point and general registers.  */

/* `SECONDARY_MEMORY_NEEDED (CLASS1, CLASS2, M)'
   Certain machines have the property that some registers cannot be
   copied to some other registers without using memory.  Define this
   macro on those machines to be a C expression that is non-zero if
   objects of mode M in registers of CLASS1 can only be copied to
   registers of class CLASS2 by storing a register of CLASS1 into
   memory and loading that memory location into a register of CLASS2.

   Do not define this macro if its value would always be zero.

   `SECONDARY_MEMORY_NEEDED_RTX (MODE)'
   Normally when `SECONDARY_MEMORY_NEEDED' is defined, the compiler
   allocates a stack slot for a memory location needed for register
   copies.  If this macro is defined, the compiler instead uses the
   memory location defined by this macro.

   Do not define this macro if you do not define
   `SECONDARY_MEMORY_NEEDED'.  */

#define SMALL_REGISTER_CLASSES 1
/* Normally the compiler avoids choosing registers that have been
   explicitly mentioned in the rtl as spill registers (these
   registers are normally those used to pass parameters and return
   values).  However, some machines have so few registers of certain
   classes that there would not be enough registers to use as spill
   registers if this were done.

   Define `SMALL_REGISTER_CLASSES' to be an expression with a non-zero
   value on these machines.  When this macro has a non-zero value, the
   compiler allows registers explicitly used in the rtl to be used as
   spill registers but avoids extending the lifetime of these
   registers.

   It is always safe to define this macro with a non-zero value, but
   if you unnecessarily define it, you will reduce the amount of
   optimizations that can be performed in some cases.  If you do not
   define this macro with a non-zero value when it is required, the
   compiler will run out of spill registers and print a fatal error
   message.  For most machines, you should not define this macro at
   all.  */

/*
#define CLASS_LIKELY_SPILLED_P(CLASS) \
	( (CLASS) != ALL_REGS && (CLASS) != GENERAL_REGS)
 A C expression whose value is nonzero if pseudos that have been
   assigned to registers of class CLASS would likely be spilled
   because registers of CLASS are needed for spill registers.

   The default value of this macro returns 1 if CLASS has exactly one
   register and zero otherwise.  On most machines, this default
   should be used.  Only define this macro to some other expression
   if pseudo allocated by `local-alloc.c' end up in memory because
   their hard registers were needed for spill registers.  If this
   macro returns nonzero for those classes, those pseudos will only
   be allocated by `global.c', which knows how to reallocate the
   pseudo to another register.  If there would not be another
   register available for reallocation, you should not change the
   definition of this macro since the only effect of such a
   definition would be to slow down register allocation.  */

#define CLASS_MAX_NREGS(CLASS, MODE)	\
	((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)
/* A C expression for the maximum number of consecutive registers of
   class CLASS needed to hold a value of mode MODE.

   This is closely related to the macro `HARD_REGNO_NREGS'.  In fact,
   the value of the macro `CLASS_MAX_NREGS (CLASS, MODE)' should be
   the maximum value of `HARD_REGNO_NREGS (REGNO, MODE)' for all
   REGNO values in the class CLASS.

   This macro helps control the handling of multiple-word values in
   the reload pass.  */

#define CONST_OK_FOR_LETTER_P(VALUE, C)				\
  ((C) == 'I' ? (VALUE) >= -32767 && (VALUE) <= 32767 :		\
   (C) == 'J' ? (VALUE) <= 0 && (VALUE) >= -32767:		\
   (C) == 'K' ? (VALUE) >= 0 && (VALUE) <= 32767 :		\
   (C) == 'L' ? (VALUE) >= 0 && (VALUE) <= 0xff	 :		\
   (C) == 'M' ? (VALUE) >= 0x10 && (VALUE) <= 0xff :		\
   (C) == 'N' ? (VALUE) >= 0x100 && (VALUE) <= 0x1ff :		\
   (C) == 'O' ? (VALUE)&1:					\
   (C) == 'P' ? ((VALUE)==-1||(VALUE)==1||(VALUE)==2||(VALUE)==4||(VALUE)==8 ||(VALUE)==0) : 0)
/* A C expression that defines the machine-dependent operand
   constraint letters (`I', `J', `K', ... `P') that specify
   particular ranges of integer values.  If C is one of those
   letters, the expression should check that VALUE, an integer, is in
   the appropriate range and return 1 if so, 0 otherwise.  If C is
   not one of those letters, the value should be 0 regardless of
   VALUE.  */


#define CONST_DOUBLE_OK_FOR_LETTER_P(VALUE, C) \
  ((C) == 'G' ? (VALUE) == CONST0_RTX (SFmode)	\
   : 0)
/* `CONST_DOUBLE_OK_FOR_LETTER_P (VALUE, C)'
   A C expression that defines the machine-dependent operand
   constraint letters that specify particular ranges of
   `const_double' values (`G' or `H').

   If C is one of those letters, the expression should check that
   VALUE, an RTX of code `const_double', is in the appropriate range
   and return 1 if so, 0 otherwise.  If C is not one of those
   letters, the value should be 0 regardless of VALUE.

   `const_double' is used for all floating-point constants and for
   `DImode' fixed-point constants.  A given letter can accept either
   or both kinds of values.  It can use `GET_MODE' to distinguish
   between these kinds.  */

#define EXTRA_CONSTRAINT(x, c) extra_constraint(x, c)
/* A C expression that defines the optional machine-dependent
   constraint letters (``Q', `R', `S', `T', `U') that can'
   be used to segregate specific types of operands, usually memory
   references, for the target machine.  Normally this macro will not
   be defined.  If it is required for a particular target machine, it
   should return 1 if VALUE corresponds to the operand type
   represented by the constraint letter C.  If C is not defined as an
   extra constraint, the value returned should be 0 regardless of
   VALUE.

   For example, on the ROMP, load instructions cannot have their
   output in r0 if the memory reference contains a symbolic address.
   Constraint letter `Q' is defined as representing a memory address
   that does *not* contain a symbolic address.  An alternative is
   specified with a `Q' constraint on the input and `r' on the
   output.  The next alternative specifies `m' on the input and a
   register class that does not include r0 on the output.  */

/*  This is an undocumented variable which describes
    how GCC will push a data */
#define STACK_PUSH_CODE PRE_DEC

#define PUSH_ARGS !TARGET_ACCUMULATE_OUTGOING_ARGS
#define ACCUMULATE_OUTGOING_ARGS TARGET_ACCUMULATE_OUTGOING_ARGS

#define STACK_GROWS_DOWNWARD
/* Define this macro if pushing a word onto the stack moves the stack
   pointer to a smaller address.

   When we say, "define this macro if ...," it means that the
   compiler checks this macro only with `#ifdef' so the precise
   definition used does not matter.  */

#define STARTING_FRAME_OFFSET 0
/* Offset from the frame pointer to the first local variable slot to
   be allocated.

   If `FRAME_GROWS_DOWNWARD', find the next slot's offset by
   subtracting the first slot's length from `STARTING_FRAME_OFFSET'.
   Otherwise, it is found by adding the length of the first slot to
   the value `STARTING_FRAME_OFFSET'.  */

#define STACK_POINTER_OFFSET 0
/* Offset from the stack pointer register to the first location at
   which outgoing arguments are placed.  If not specified, the
   default value of zero is used.  This is the proper value for most
   machines.

   If `ARGS_GROW_DOWNWARD', this is the offset to the location above
   the first location at which outgoing arguments are placed.  */

#define FIRST_PARM_OFFSET(FUNDECL) 0
/* Offset from the argument pointer register to the first argument's
   address.  On some machines it may depend on the data type of the
   function.

   If `ARGS_GROW_DOWNWARD', this is the offset to the location above
   the first argument's address.  */

/* `STACK_DYNAMIC_OFFSET (FUNDECL)'
   Offset from the stack pointer register to an item dynamically
   allocated on the stack, e.g., by `alloca'.

   The default value for this macro is `STACK_POINTER_OFFSET' plus the
   length of the outgoing arguments.  The default is correct for most
   machines.  See `function.c' for details.  */

#define STACK_BOUNDARY 16
/* Define this macro if there is a guaranteed alignment for the stack
   pointer on this machine.  The definition is a C expression for the
   desired alignment (measured in bits).  This value is used as a
   default if PREFERRED_STACK_BOUNDARY is not defined.  */

#define STACK_POINTER_REGNUM 1
/* The register number of the stack pointer register, which must also
   be a fixed register according to `FIXED_REGISTERS'.  On most
   machines, the hardware determines which register this is.  */

#define FRAME_POINTER_REGNUM 4
/* The register number of the frame pointer register, which is used to
   access automatic variables in the stack frame.  On some machines,
   the hardware determines which register this is.  On other
   machines, you can choose any register you wish for this purpose.  */

/* We use a pseudo-register called "argp" to represent argument pointer.
	As the register is marked as fixed, it will always be eliminated
	(replaced by sp + offset).*/
#define ARG_POINTER_REGNUM 16
/* The register number of the arg pointer register, which is used to
   access the function's argument list.  On some machines, this is
   the same as the frame pointer register.  On some machines, the
   hardware determines which register this is.  On other machines,
   you can choose any register you wish for this purpose.  If this is
   not the same register as the frame pointer register, then you must
   mark it as a fixed register according to `FIXED_REGISTERS', or
   arrange to be able to eliminate it (*note Elimination::.).  */

#define STATIC_CHAIN_REGNUM 6
/* Register numbers used for passing a function's static chain
   pointer.  If register windows are used, the register number as
   seen by the called function is `STATIC_CHAIN_INCOMING_REGNUM',
   while the register number as seen by the calling function is
   `STATIC_CHAIN_REGNUM'.  If these registers are the same,
   `STATIC_CHAIN_INCOMING_REGNUM' need not be defined.

   The static chain register need not be a fixed register.

   If the static chain is passed in memory, these macros should not be
   defined; instead, the next two macros should be defined.  */

#define FRAME_POINTER_REQUIRED   frame_pointer_required_p()

/*
 A C expression which is nonzero if a function must have and use a
   frame pointer.  This expression is evaluated  in the reload pass.
   If its value is nonzero the function will have a frame pointer.

   The expression can in principle examine the current function and
   decide according to the facts, but on most machines the constant 0
   or the constant 1 suffices.  Use 0 when the machine allows code to
   be generated with no frame pointer, and doing so saves some time
   or space.  Use 1 when there is no possible advantage to avoiding a
   frame pointer.

   In certain cases, the compiler does not know how to produce valid
   code without a frame pointer.  The compiler recognizes those cases
   and automatically gives the function a frame pointer regardless of
   what `FRAME_POINTER_REQUIRED' says.  You don't need to worry about
   them.

   In a function that does not require a frame pointer, the frame
   pointer register can be allocated for ordinary usage, unless you
   mark it as a fixed register.  See `FIXED_REGISTERS' for more
   information.  */

#define ELIMINABLE_REGS {			\
   {ARG_POINTER_REGNUM, STACK_POINTER_REGNUM}, \
   {ARG_POINTER_REGNUM, FRAME_POINTER_REGNUM}, \
   {FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM}, \
}
/* If defined, this macro specifies a table of register pairs used to
   eliminate unneeded registers that point into the stack frame.  If
   it is not defined, the only elimination attempted by the compiler
   is to replace references to the frame pointer with references to
   the stack pointer.

   The definition of this macro is a list of structure
   initializations, each of which specifies an original and
   replacement register.

   On some machines, the position of the argument pointer is not
   known until the compilation is completed.  In such a case, a
   separate hard register must be used for the argument pointer.
   This register can be eliminated by replacing it with either the
   frame pointer or the argument pointer, depending on whether or not
   the frame pointer has been eliminated.

   In this case, you might specify:
   #define ELIMINABLE_REGS  \
   {{ARG_POINTER_REGNUM, STACK_POINTER_REGNUM}, \
   {ARG_POINTER_REGNUM, FRAME_POINTER_REGNUM}, \
   {FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM}}

   Note that the elimination of the argument pointer with the stack
   pointer is specified first since that is the preferred elimination.  */

#define CAN_ELIMINATE(FROM, TO) 1
/*#define CAN_ELIMINATE(FROM, TO) (((FROM) == ARG_POINTER_REGNUM		   \
				  && (TO) == FRAME_POINTER_REGNUM)	   \
				 || (((FROM) == FRAME_POINTER_REGNUM)	   \
				     && ! FRAME_POINTER_REQUIRED	   \
				     ))*/
/* A C expression that returns non-zero if the compiler is allowed to
   try to replace register number FROM-REG with register number
   TO-REG.  This macro need only be defined if `ELIMINABLE_REGS' is
   defined, and will usually be the constant 1, since most of the
   cases preventing register elimination are things that the compiler
   already knows about.  */

#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET)			\
     OFFSET = initial_elimination_offset (FROM, TO)
/* This macro is similar to `INITIAL_FRAME_POINTER_OFFSET'.  It
   specifies the initial difference between the specified pair of
   registers.  This macro must be defined if `ELIMINABLE_REGS' is
   defined.  */
/*
#define RETURN_ADDR_RTX(count, x) \
  gen_rtx_MEM (Pmode, memory_address (Pmode, plus_constant (tem, 1)))
*/
#define PUSH_ROUNDING(NPUSHED) ((NPUSHED+1) & ~1)
/* A C expression that is the number of bytes actually pushed onto the
   stack when an instruction attempts to push NPUSHED bytes.

   If the target machine does not have a push instruction, do not
   define this macro.  That directs GNU CC to use an alternate
   strategy: to allocate the entire argument block and then store the
   arguments into it.

   On some machines, the definition

   #define PUSH_ROUNDING(BYTES) (BYTES)

   will suffice.  But on other machines, instructions that appear to
   push one byte actually push two bytes in an attempt to maintain
   alignment.  Then the definition should be

   #define PUSH_ROUNDING(BYTES) (((BYTES) + 1) & ~1)  */

#define RETURN_POPS_ARGS(FUNDECL, FUNTYPE, STACK_SIZE) 0
/* A C expression that should indicate the number of bytes of its own
   arguments that a function pops on returning, or 0 if the function
   pops no arguments and the caller must therefore pop them all after
   the function returns.

   FUNDECL is a C variable whose value is a tree node that describes
   the function in question.  Normally it is a node of type
   `FUNCTION_DECL' that describes the declaration of the function.
   From this you can obtain the DECL_MACHINE_ATTRIBUTES of the
   function.

   FUNTYPE is a C variable whose value is a tree node that describes
   the function in question.  Normally it is a node of type
   `FUNCTION_TYPE' that describes the data type of the function.
   From this it is possible to obtain the data types of the value and
   arguments (if known).

   When a call to a library function is being considered, FUNDECL
   will contain an identifier node for the library function.  Thus, if
   you need to distinguish among various library functions, you can
   do so by their names.  Note that "library function" in this
   context means a function used to perform arithmetic, whose name is
   known specially in the compiler and was not mentioned in the C
   code being compiled.

   STACK-SIZE is the number of bytes of arguments passed on the
   stack.  If a variable number of bytes is passed, it is zero, and
   argument popping will always be the responsibility of the calling
   function.

   On the Vax, all functions always pop their arguments, so the
   definition of this macro is STACK-SIZE.  On the 68000, using the
   standard calling convention, no functions pop their arguments, so
   the value of the macro is always 0 in this case.  But an
   alternative calling convention is available in which functions
   that take a fixed number of arguments pop them but other functions
   (such as `printf') pop nothing (the caller pops all).  When this
   convention is in use, FUNTYPE is examined to determine whether a
   function takes a fixed number of arguments.  */

#define FUNCTION_ARG(CUM, MODE, TYPE, NAMED) \
(function_arg (&(CUM), MODE, TYPE, NAMED))

/*
#define FUNCTION_INCOMING_ARG(CUM, MODE, TYPE, NAMED) \
(function_incoming_arg (&(CUM), MODE, TYPE, NAMED))
*/
/* A C expression that controls whether a function argument is passed
   in a register, and which register.

   The arguments are CUM, which summarizes all the previous
   arguments; MODE, the machine mode of the argument; TYPE, the data
   type of the argument as a tree node or 0 if that is not known
   (which happens for C support library functions); and NAMED, which
   is 1 for an ordinary argument and 0 for nameless arguments that
   correspond to `...' in the called function's prototype.

   The value of the expression is usually either a `reg' RTX for the
   hard register in which to pass the argument, or zero to pass the
   argument on the stack.

   For machines like the Vax and 68000, where normally all arguments
   are pushed, zero suffices as a definition.

   The value of the expression can also be a `parallel' RTX.  This is
   used when an argument is passed in multiple locations.  The mode
   of the of the `parallel' should be the mode of the entire
   argument.  The `parallel' holds any number of `expr_list' pairs;
   each one describes where part of the argument is passed.  In each
   `expr_list', the first operand can be either a `reg' RTX for the
   hard register in which to pass this part of the argument, or zero
   to pass the argument on the stack.  If this operand is a `reg',
   then the mode indicates how large this part of the argument is.
   The second operand of the `expr_list' is a `const_int' which gives
   the offset in bytes into the entire argument where this part
   starts.

   The usual way to make the ANSI library `stdarg.h' work on a machine
   where some arguments are usually passed in registers, is to cause
   nameless arguments to be passed on the stack instead.  This is done
   by making `FUNCTION_ARG' return 0 whenever NAMED is 0.

   You may use the macro `MUST_PASS_IN_STACK (MODE, TYPE)' in the
   definition of this macro to determine if this argument is of a
   type that must be passed in the stack.  If `REG_PARM_STACK_SPACE'
   is not defined and `FUNCTION_ARG' returns non-zero for such an
   argument, the compiler will abort.  If `REG_PARM_STACK_SPACE' is
   defined, the argument will be computed in the stack and then
   loaded into a register.  */

typedef struct msp430_args {
  int nregs;			/* # registers available for passing */
  int regno;			/* next available register number */
} CUMULATIVE_ARGS;
/* A C type for declaring a variable that is used as the first
   argument of `FUNCTION_ARG' and other related values.  For some
   target machines, the type `int' suffices and can hold the number
   of bytes of argument so far.

   There is no need to record in `CUMULATIVE_ARGS' anything about the
   arguments that have been passed on the stack.  The compiler has
   other variables to keep track of that.  For target machines on
   which all arguments are passed on the stack, there is no need to
   store anything in `CUMULATIVE_ARGS'; however, the data structure
   must exist and should not be empty, so use `int'.  */


/*
	Correct DWARF2 unwind info generation requires prologue and epilogue
	to be expanded into series of INSNs, so that DWARF2 info generator
	can analyze those INSNs. Currently, prologue and epilogue are printed
	to the output file as raw text, so, unwind info cannot be generated.
*/

#define DWARF2_UNWIND_INFO 1
#define INCOMING_RETURN_ADDR_RTX gen_rtx_MEM (VOIDmode, gen_rtx_REG (VOIDmode, STACK_POINTER_REGNUM))

#define INCOMING_FRAME_SP_OFFSET ((cfun && cfun->machine->is_interrupt) ? 4 : 2)

#define EPILOGUE_USES(REGNO) msp430_epilogue_uses(REGNO)
   
#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, N_NAMED_ARGS ) \
init_cumulative_args (&(CUM), FNTYPE, LIBNAME, FNDECL)

#define  INIT_CUMULATIVE_INCOMING_ARGS(CUM, FNTYPE, LIBNAME) \
init_cumulative_incoming_args(&(CUM), FNTYPE, LIBNAME)

#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED)	\
  (function_arg_advance (&CUM, MODE, TYPE, NAMED))

#define FUNCTION_ARG_REGNO_P(r) (r >= 12 && r <= 15)

/* A C statement (sans semicolon) for initializing the variable CUM
   for the state at the beginning of the argument list.  The variable
   has type `CUMULATIVE_ARGS'.  The value of FNTYPE is the tree node
   for the data type of the function which will receive the args, or 0
   if the args are to a compiler support library function.  The value
   of INDIRECT is nonzero when processing an indirect call, for
   example a call through a function pointer.  The value of INDIRECT
   is zero for a call to an explicitly named function, a library
   function call, or when `INIT_CUMULATIVE_ARGS' is used to find
   arguments for the function being compiled.
   
   When processing a call to a compiler support library function,
   LIBNAME identifies which one.  It is a `symbol_ref' rtx which
   contains the name of the function, as a string.  LIBNAME is 0 when
   an ordinary C function call is being processed.  Thus, each time
   this macro is called, either LIBNAME or FNTYPE is nonzero, but
   never both of them at once.   */

#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED)	\
  (function_arg_advance (&CUM, MODE, TYPE, NAMED))

/* A C statement (sans semicolon) to update the summarizer variable
   CUM to advance past an argument in the argument list.  The values
   MODE, TYPE and NAMED describe that argument.  Once this is done,
   the variable CUM is suitable for analyzing the *following*
   argument with `FUNCTION_ARG', etc.
   
   This macro need not do anything if the argument in question was
   passed on the stack.  The compiler knows how to track the amount
   of stack space used for arguments without any special help. */

#define FUNCTION_ARG_REGNO_P(r) (r >= 12 && r <= 15)
/* A C expression that is nonzero if REGNO is the number of a hard
   register in which function arguments are sometimes passed.  This
   does *not* include implicit arguments such as the static chain and
   the structure-value address.  On many machines, no registers can be
   used for this purpose since all function arguments are pushed on
   the stack.  */

extern int msp430_reg_order[];

#define RET_REGISTER  15 /* msp430_ret_register ()*/

#define FUNCTION_VALUE(VALTYPE, FUNC) msp430_function_value (VALTYPE, FUNC)
/* A C expression to create an RTX representing the place where a
   function returns a value of data type VALTYPE.  VALTYPE is a tree
   node representing a data type.  Write `TYPE_MODE (VALTYPE)' to get
   the machine mode used to represent that type.  On many machines,
   only the mode is relevant.  (Actually, on most machines, scalar
   values are returned in the same place regardless of mode).

   The value of the expression is usually a `reg' RTX for the hard
   register where the return value is stored.  The value can also be a
   `parallel' RTX, if the return value is in multiple places.  See
   `FUNCTION_ARG' for an explanation of the `parallel' form.

   If `PROMOTE_FUNCTION_RETURN' is defined, you must apply the same
   promotion rules specified in `PROMOTE_MODE' if VALTYPE is a scalar
   type.

   If the precise function being called is known, FUNC is a tree node
   (`FUNCTION_DECL') for it; otherwise, FUNC is a null pointer.  This
   makes it possible to use a different value-returning convention
   for specific functions when all their calls are known.

   `FUNCTION_VALUE' is not used for return vales with aggregate data
   types, because these are returned in another way.  See
   `STRUCT_VALUE_REGNUM' and related macros, below.  */

#define LIBCALL_VALUE(MODE)  msp430_libcall_value (MODE)
/* A C expression to create an RTX representing the place where a
   library function returns a value of mode MODE.  If the precise
   function being called is known, FUNC is a tree node
   (`FUNCTION_DECL') for it; otherwise, FUNC is a null pointer.  This
   makes it possible to use a different value-returning convention
   for specific functions when all their calls are known.

   Note that "library function" in this context means a compiler
   support routine, used to perform arithmetic, whose name is known
   specially by the compiler and was not mentioned in the C code being
   compiled.

   The definition of `LIBRARY_VALUE' need not be concerned aggregate
   data types, because none of the library functions returns such
   types.  */

#define FUNCTION_VALUE_REGNO_P(N) ((N) == RET_REGISTER)
/* A C expression that is nonzero if REGNO is the number of a hard
   register in which the values of called function may come back.

   A register whose use for returning values is limited to serving as
   the second of a pair (for a value of type `double', say) need not
   be recognized by this macro.  So for most machines, this definition
   suffices:

   #define FUNCTION_VALUE_REGNO_P(N) ((N) == 0)

   If the machine has register windows, so that the caller and the
   called function use different registers for the return value, this
   macro should recognize only the caller's register numbers.  */

#define RETURN_IN_MEMORY(TYPE) ((TYPE_MODE (TYPE) == BLKmode)	\
				? int_size_in_bytes (TYPE) > 8	\
				: 0)
/* A C expression which can inhibit the returning of certain function
   values in registers, based on the type of value.  A nonzero value
   says to return the function value in memory, just as large
   structures are always returned.  Here TYPE will be a C expression
   of type `tree', representing the data type of the value.

   Note that values of mode `BLKmode' must be explicitly handled by
   this macro.  Also, the option `-fpcc-struct-return' takes effect
   regardless of this macro.  On most systems, it is possible to
   leave the macro undefined; this causes a default definition to be
   used, whose value is the constant 1 for `BLKmode' values, and 0
   otherwise.

   Do not use this macro to indicate that structures and unions
   should always be returned in memory.  You should instead use
   `DEFAULT_PCC_STRUCT_RETURN' to indicate this.  */

#define DEFAULT_PCC_STRUCT_RETURN 0
/* Define this macro to be 1 if all structure and union return values
   must be in memory.  Since this results in slower code, this should
   be defined only if needed for compatibility with other compilers
   or with an ABI.  If you define this macro to be 0, then the
   conventions used for structure and union return values are decided
   by the `RETURN_IN_MEMORY' macro.

   If not defined, this defaults to the value 1.  */

#define STRUCT_VALUE 0
/* If the structure value address is not passed in a register, define
   `STRUCT_VALUE' as an expression returning an RTX for the place
   where the address is passed.  If it returns 0, the address is
   passed as an "invisible" first argument.  */

/*
#define HAVE_PRE_INCREMENT 	1
*/

#define HAVE_POST_INCREMENT 	1

/*
#define HAVE_PRE_DECREMENT 	1
#define HAVE_PRE_INCREMENT 	1
#define HAVE_POST_DECREMENT 	1  
*/
/* Similar for other kinds of addressing.  */

#define CONSTANT_ADDRESS_P(X) CONSTANT_P (X)

/*
(GET_CODE (X) == LABEL_REF 	\
|| GET_CODE (X) == SYMBOL_REF   \
|| GET_CODE (X) == CONST_INT    \
|| GET_CODE (X) == CONST)
*/

/*
#define CANONICALIZE_COMPARISON(CODE, OP0, OP1) \
do{CODE = msp430_canonicalize_comparison(CODE, &OP0, &OP1); }while(0)
*/

/* A C expression that is 1 if the RTX X is a constant which is a
   valid address.  On most machines, this can be defined as
   `CONSTANT_P (X)', but a few machines are more restrictive in which
   constant addresses are supported.

   `CONSTANT_P' accepts integer-values expressions whose values are
   not explicitly known, such as `symbol_ref', `label_ref', and
   `high' expressions and `const' arithmetic expressions, in addition
   to `const_int' and `const_double' expressions.  */

#define MAX_REGS_PER_ADDRESS 1
/* A number, the maximum number of registers that can appear in a
   valid memory address.  Note that it is up to you to specify a
   value equal to the maximum number that `GO_IF_LEGITIMATE_ADDRESS'
   would ever accept.  */

#ifdef REG_OK_STRICT
#  define GO_IF_LEGITIMATE_ADDRESS(mode, operand, ADDR)	\
{							\
  if (legitimate_address_p (mode, operand, 1))		\
    goto ADDR;						\
}
#  else
#  define GO_IF_LEGITIMATE_ADDRESS(mode, operand, ADDR)	\
{							\
  if (legitimate_address_p (mode, operand, 0))		\
    goto ADDR;						\
}
#endif
/* A C compound statement with a conditional `goto LABEL;' executed
   if X (an RTX) is a legitimate memory address on the target machine
   for a memory operand of mode MODE.

   It usually pays to define several simpler macros to serve as
   subroutines for this one.  Otherwise it may be too complicated to
   understand.

   This macro must exist in two variants: a strict variant and a
   non-strict one.  The strict variant is used in the reload pass.  It
   must be defined so that any pseudo-register that has not been
   allocated a hard register is considered a memory reference.  In
   contexts where some kind of register is required, a pseudo-register
   with no hard register must be rejected.

   The non-strict variant is used in other passes.  It must be
   defined to accept all pseudo-registers in every context where some
   kind of register is required.

   Compiler source files that want to use the strict variant of this
   macro define the macro `REG_OK_STRICT'.  You should use an `#ifdef
   REG_OK_STRICT' conditional to define the strict variant in that
   case and the non-strict variant otherwise.

   Subroutines to check for acceptable registers for various purposes
   (one for base registers, one for index registers, and so on) are
   typically among the subroutines used to define
   `GO_IF_LEGITIMATE_ADDRESS'.  Then only these subroutine macros
   need have two variants; the higher levels of macros may be the
   same whether strict or not.

   Normally, constant addresses which are the sum of a `symbol_ref'
   and an integer are stored inside a `const' RTX to mark them as
   constant.  Therefore, there is no need to recognize such sums
   specifically as legitimate addresses.  Normally you would simply
   recognize any `const' as legitimate.

   Usually `PRINT_OPERAND_ADDRESS' is not prepared to handle constant
   sums that are not marked with  `const'.  It assumes that a naked
   `plus' indicates indexing.  If so, then you *must* reject such
   naked constant sums as illegitimate addresses, so that none of
   them will be given to `PRINT_OPERAND_ADDRESS'.

   On some machines, whether a symbolic address is legitimate depends
   on the section that the address refers to.  On these machines,
   define the macro `ENCODE_SECTION_INFO' to store the information
   into the `symbol_ref', and then check for it here.  When you see a
   `const', you will have to look inside it to find the `symbol_ref'
   in order to determine the section.  *Note Assembler Format::.

   The best way to modify the name string is by adding text to the
   beginning, with suitable punctuation to prevent any ambiguity.
   Allocate the new name in `saveable_obstack'.  You will have to
   modify `ASM_OUTPUT_LABELREF' to remove and decode the added text
   and output the name accordingly, and define `STRIP_NAME_ENCODING'
   to access the original name string.

   You can check the information stored here into the `symbol_ref' in
   the definitions of the macros `GO_IF_LEGITIMATE_ADDRESS' and
   `PRINT_OPERAND_ADDRESS'. */

/* `REG_OK_FOR_BASE_P (X)'
   A C expression that is nonzero if X (assumed to be a `reg' RTX) is
   valid for use as a base register.  For hard registers, it should
   always accept those which the hardware permits and reject the
   others.  Whether the macro accepts or rejects pseudo registers
   must be controlled by `REG_OK_STRICT' as described above.  This
   usually requires two variant definitions, of which `REG_OK_STRICT'
   controls the one actually used.  */

#define REG_OK_FOR_BASE_NOSTRICT_P(X) \
  (REGNO (X) >= FIRST_PSEUDO_REGISTER || REG_OK_FOR_BASE_STRICT_P(X))

#define REG_OK_FOR_BASE_STRICT_P(X) REGNO_OK_FOR_BASE_P (REGNO (X))

#ifdef REG_OK_STRICT
#  define REG_OK_FOR_BASE_P(X) REG_OK_FOR_BASE_STRICT_P (X)
#else
#  define REG_OK_FOR_BASE_P(X) REG_OK_FOR_BASE_NOSTRICT_P (X)
#endif

/* A C expression that is just like `REG_OK_FOR_BASE_P', except that
   that expression may examine the mode of the memory reference in
   MODE.  You should define this macro if the mode of the memory
   reference affects whether a register may be used as a base
   register.  If you define this macro, the compiler will use it
   instead of `REG_OK_FOR_BASE_P'.  */


#define REG_OK_FOR_INDEX_P(X) 0 /*( REGNO(X)!=3 && REGNO(X)!=2 &&REGNO(X)<=15) */
/*   A C expression that is nonzero if X (assumed to be a `reg' RTX) is
   valid for use as an index register.

   The difference between an index register and a base register is
   that the index register may be scaled.  If an address involves the
   sum of two registers, neither one of them scaled, then either one
   may be labeled the "base" and the other the "index"; but whichever
   labeling is used must fit the machine's constraints of which
   registers may serve in each capacity.  The compiler will try both
   labelings, looking for one that is valid, and will reload one or
   both registers only if neither labeling works.  */

#define LEGITIMIZE_ADDRESS(X, OLDX, MODE, WIN)				\
GO_IF_LEGITIMATE_ADDRESS (MODE, X, WIN)

/*{									\
									\
  (X) = legitimize_address (X, OLDX, MODE);				\
  if (memory_address_p (MODE, X))					\
    goto WIN;								\
									\
} */

/* A C compound statement that attempts to replace X with a valid
   memory address for an operand of mode MODE.  WIN will be a C
   statement label elsewhere in the code; the macro definition may use

   GO_IF_LEGITIMATE_ADDRESS (MODE, X, WIN);

   to avoid further processing if the address has become legitimate.

   X will always be the result of a call to `break_out_memory_refs',
   and OLDX will be the operand that was given to that function to
   produce X.

   The code generated by this macro should not alter the substructure
   of X.  If it transforms X into a more legitimate form, it should
   assign X (which will always be a C variable) a new value.

   It is not necessary for this macro to come up with a legitimate
   address.  The compiler has standard ways of doing so in all cases.
   In fact, it is safe for this macro to do nothing.  But often a
   machine-dependent strategy can generate better code.  */

/* A C compound statement that attempts to replace X, which is an
   address that needs reloading, with a valid memory address for an
   operand of mode MODE.  WIN will be a C statement label elsewhere
   in the code.  It is not necessary to define this macro, but it
   might be useful for performance reasons.

   For example, on the i386, it is sometimes possible to use a single
   reload register instead of two by reloading a sum of two pseudo
   registers into a register.  On the other hand, for number of RISC
   processors offsets are limited so that often an intermediate
   address needs to be generated in order to address a stack slot.
   By defining LEGITIMIZE_RELOAD_ADDRESS appropriately, the
   intermediate addresses generated for adjacent some stack slots can
   be made identical, and thus be shared.

   *Note*: This macro should be used with caution.  It is necessary
   to know something of how reload works in order to effectively use
   this, and it is quite easy to produce macros that build in too
   much knowledge of reload internals.

   *Note*: This macro must be able to reload an address created by a
   previous invocation of this macro.  If it fails to handle such
   addresses then the compiler may generate incorrect code or abort.

   The macro definition should use `push_reload' to indicate parts
   that need reloading; OPNUM, TYPE and IND_LEVELS are usually
   suitable to be passed unaltered to `push_reload'.

   The code generated by this macro must not alter the substructure of
   X.  If it transforms X into a more legitimate form, it should
   assign X (which will always be a C variable) a new value.  This
   also applies to parts that you change indirectly by calling
   `push_reload'.

   The macro definition may use `strict_memory_address_p' to test if
   the address has become legitimate.

   If you want to change only a part of X, one standard way of doing
   this is to use `copy_rtx'.  Note, however, that is unshares only a
   single level of rtl.  Thus, if the part to be changed is not at the
   top level, you'll need to replace first the top leve It is not
   necessary for this macro to come up with a legitimate address;
   but often a machine-dependent strategy can generate better code.  */
	
#define GO_IF_MODE_DEPENDENT_ADDRESS(ADDR,LABEL)			\
{}
/* A C statement or compound statement with a conditional `goto
   LABEL;' executed if memory address X (an RTX) can have different
   meanings depending on the machine mode of the memory reference it
   is used for or if the address is valid for some modes but not
   others.

   Autoincrement and autodecrement addresses typically have
   mode-dependent effects because the amount of the increment or
   decrement is the size of the operand being addressed.  Some
   machines have other mode-dependent addresses.  Many RISC machines
   have no mode-dependent addresses.

   You may assume that ADDR is a valid address for the machine.  */

#define LEGITIMATE_CONSTANT_P(X) 1
/* A C expression that is nonzero if X is a legitimate constant for
   an immediate operand on the target machine.  You can assume that X
   satisfies `CONSTANT_P', so you need not check this.  In fact, `1'
   is a suitable definition for this macro on machines where anything
   `CONSTANT_P' is valid.  */

/*#define CONST_COSTS(x,CODE,OUTER_CODE)		\
    case CONST_INT:				\
      if (OUTER_CODE == PLUS			\
	  || OUTER_CODE == IOR			\
	  || OUTER_CODE == AND			\
	  || OUTER_CODE == MINUS		\
	  || OUTER_CODE == SET			\
	  || INTVAL (x) == 0			\
          || INTVAL (x) == 1			\
          || INTVAL (x) == 2			\
          || INTVAL (x) == 4			\
          || INTVAL (x) == 8			\
          || INTVAL (x) == -1)			\
        return 0;				\
    case CONST:					\
      return 0;					\
    case LABEL_REF:				\
      return 1;					\
    case SYMBOL_REF:				\
      return 2;					\
    case CONST_DOUBLE:				\
      return 4;*/

/* Like `CONST_COSTS' but applies to nonconstant RTL expressions.
   This can be used, for example, to indicate how costly a multiply
   instruction is.  In writing this macro, you can use the construct
   `COSTS_N_INSNS (N)' to specify a cost equal to N fast
   instructions.  OUTER_CODE is the code of the expression in which X
   is contained.

   This macro is optional; do not define it if the default cost
   assumptions are adequate for the target machine.  */

#define REGISTER_MOVE_COST(MODE, FROM, TO) ((MODE)==QImode ? 1 :   \
					    (MODE)==HImode ? 1 :   \
					    (MODE)==SImode ? 2 :   \
					    (MODE)==SFmode ? 2 : 4)

/* A C expression for the cost of moving data from a register in class
   FROM to one in class TO.  The classes are expressed using the
   enumeration values such as `GENERAL_REGS'.  A value of 2 is the
   default; other values are interpreted relative to that.

   It is not required that the cost always equal 2 when FROM is the
   same as TO; on some machines it is expensive to move between
   registers if they are not general registers.

   If reload sees an insn consisting of a single `set' between two
   hard registers, and if `REGISTER_MOVE_COST' applied to their
   classes returns a value of 2, reload does not check to ensure that
   the constraints of the insn are met.  Setting a cost of other than
   2 will allow reload to verify that the constraints are met.  You
   should do this if the `movM' pattern's constraints do not allow
   such copying.  */

#define MEMORY_MOVE_COST(MODE,CLASS,IN) ((MODE)==QImode ? 2 :	\
					 (MODE)==HImode ? 2 :	\
					 (MODE)==SImode ? 4 :	\
					 (MODE)==SFmode ? 4 : 8)
/* A C expression for the cost of moving data of mode M between a
   register and memory.  A value of 4 is the default; this cost is
   relative to those in `REGISTER_MOVE_COST'.

   If moving between registers and memory is more expensive than
   between two registers, you should define this macro to express the
   relative cost.  */

#if GCC_VERSION_INT >= 0x440
#define BRANCH_COST(a,b) 0
#else
#define BRANCH_COST 0
#endif

/* A C expression for the cost of a branch instruction.  A value of 1
   is the default; other values are interpreted relative to that.

   Here are additional macros which do not specify precise relative
   costs, but only that certain actions are more expensive than GCC would
   ordinarily expect.  */

#define SLOW_BYTE_ACCESS 0
/* Define this macro as a C expression which is nonzero if accessing
   less than a word of memory (i.e. a `char' or a `short') is no
   faster than accessing a word of memory, i.e., if such access
   require more than one instruction or if there is no difference in
   cost between byte and (aligned) word loads.

   When this macro is not defined, the compiler will access a field by
   finding the smallest containing object; when it is defined, a
   fullword load will be used if alignment permits.  Unless bytes
   accesses are faster than word accesses, using word accesses is
   preferable since it may eliminate subsequent memory access if
   subsequent accesses occur to other fields in the same word of the
   structure, but to different bytes.

   `SLOW_ZERO_EXTEND'
   Define this macro if zero-extension (of a `char' or `short' to an
   `int') can be done faster if the destination is a register that is
   known to be zero.

   If you define this macro, you must have instruction patterns that
   recognize RTL structures like this:

   (set (strict_low_part (subreg:QI (reg:SI ...) 0)) ...)

   and likewise for `HImode'.

   `SLOW_UNALIGNED_ACCESS'
   Define this macro to be the value 1 if unaligned accesses have a
   cost many times greater than aligned accesses, for example if they
   are emulated in a trap handler.

   When this macro is non-zero, the compiler will act as if
   `STRICT_ALIGNMENT' were non-zero when generating code for block
   moves.  This can cause significantly more instructions to be
   produced.  Therefore, do not set this macro non-zero if unaligned
   accesses only add a cycle or two to the time for a memory access.

   If the value of this macro is always zero, it need not be defined.

   `DONT_REDUCE_ADDR'
   Define this macro to inhibit strength reduction of memory
   addresses.  (On some machines, such strength reduction seems to do
   harm rather than good.)

   `MOVE_RATIO'
   The number of scalar move insns which should be generated instead
   of a string move insn or a library call.  Increasing the value
   will always make code faster, but eventually incurs high cost in
   increased code size.

   If you don't define this, a reasonable default is used.  */

#define NO_FUNCTION_CSE
/* Define this macro if it is as good or better to call a constant
   function address than to call an address kept in a register.  */

#define TEXT_SECTION_ASM_OP "\t.text"
/* A C expression whose value is a string containing the assembler
   operation that should precede instructions and read-only data.
   Normally `"\t.text"' is right.  */

#define DATA_SECTION_ASM_OP "\t.data"
/* A C expression whose value is a string containing the assembler
   operation to identify the following data as writable initialized
   data.  Normally `"\t.data"' is right.  */

#define TARGET_ASM_NAMED_SECTION default_elf_asm_named_section

/*#define EXTRA_SECTIONS in_bootloader, in_infomem*/
/* A list of names for sections other than the standard two, which are
   `in_text' and `in_data'.  You need not define this macro on a
   system with no other sections (that GCC needs to use).  */

#if 0
#define EXTRA_SECTION_FUNCTIONS						      \
									      \
void									      \
bootloader_section (void)							\
{									      \
  if (in_section != in_bootloader)						\
    {									      \
      fprintf (asm_out_file,						      \
	       "\t.section .bootloader, \"ax\", @progbits\n");	      		\
      /* Should already be aligned, this is just to be safe if it isn't.  */  \
      fprintf (asm_out_file, "\t.p2align 1\n");				      \
      in_section = in_bootloader;						\
    }									      \
}										\
									      \
void									      \
infomem_section (void)							      \
{									      \
  if (in_section != in_infomem)						      \
    {									      \
      fprintf (asm_out_file,						      \
	       "\t.section .infomem, \"a\", @progbits\n");	      		\
      /* Should already be aligned, this is just to be safe if it isn't.  */  \
      fprintf (asm_out_file, "\t.p2align 1\n");				      \
      in_section = in_infomem;						      \
    }									      \
}
#endif

/* Define the pseudo-ops used to switch to the .ctors and .dtors sections.
   There are no shared libraries on this target, and these sections are
   placed in the read-only program memory, so they are not writable.  */

#undef CTORS_SECTION_ASM_OP
#define CTORS_SECTION_ASM_OP "\t.global\t__do_global_ctors\n\t.section .ctors,\"a\",@progbits"

#undef DTORS_SECTION_ASM_OP
#define DTORS_SECTION_ASM_OP "\t.global\t__do_global_dtors\n\t.section .dtors,\"a\",@progbits"

#define TARGET_ASM_CONSTRUCTOR default_ctor_section_asm_out_constructor

#define TARGET_ASM_DESTRUCTOR default_dtor_section_asm_out_destructor

/* `EXTRA_SECTION_FUNCTIONS'
   One or more functions to be defined in `varasm.c'.  These
   functions should do jobs analogous to those of `text_section' and
   `data_section', for your additional sections.  Do not define this
   macro if you do not define `EXTRA_SECTIONS'.  */

/*
#define READONLY_DATA_SECTION data_section
   On most machines, read-only variables, constants, and jump tables
   are placed in the text section.  If this is not the case on your
   machine, this macro should be defined to be the name of a function
   (either `data_section' or a function defined in `EXTRA_SECTIONS')
   that switches to the section to be used for read-only items.

   If these items should be placed in the text section, this macro
   should not be defined.  */

/* `SELECT_SECTION (EXP, RELOC)'
   A C statement or statements to switch to the appropriate section
   for output of EXP.  You can assume that EXP is either a `VAR_DECL'
   node or a constant of some sort.  RELOC indicates whether the
   initial value of EXP requires link-time relocations.  Select the
   section by calling `text_section' or one of the alternatives for
   other sections.

   Do not define this macro if you put all read-only variables and
   constants in the read-only data section (usually the text section).  */

/* `SELECT_RTX_SECTION (MODE, RTX)'
   A C statement or statements to switch to the appropriate section
   for output of RTX in mode MODE.  You can assume that RTX is some
   kind of constant in RTL.  The argument MODE is redundant except in
   the case of a `const_int' rtx.  Select the section by calling
   `text_section' or one of the alternatives for other sections.

   Do not define this macro if you put all constants in the read-only
   data section.  */

#define JUMP_TABLES_IN_TEXT_SECTION 0
/* Define this macro if jump tables (for `tablejump' insns) should be
   output in the text section, along with the assembler instructions.
   Otherwise, the readonly data section is used.

   This macro is irrelevant if there is no separate readonly data
   section.  */

/* ???? 
#define ENCODE_SECTION_INFO(DECL)  encode_section_info(DECL)
   Define this macro if references to a symbol must be treated
   differently depending on something about the variable or function
   named by the symbol (such as what section it is in).

   The macro definition, if any, is executed immediately after the
   rtl for DECL has been created and stored in `DECL_RTL (DECL)'.
   The value of the rtl will be a `mem' whose address is a
   `symbol_ref'.

   The usual thing for this macro to do is to record a flag in the
   `symbol_ref' (such as `SYMBOL_REF_FLAG') or to store a modified
   name string in the `symbol_ref' (if one bit is not enough
   information).  */


/* `UNIQUE_SECTION_P (DECL)'
   A C expression which evaluates to true if DECL should be placed
   into a unique section for some target-specific reason.  If you do
   not define this macro, the default is `0'.  Note that the flag
   `-ffunction-sections' will also cause functions to be placed into
   unique sections.  */

#define ASM_COMMENT_START " ; "
/* A C string constant describing how to begin a comment in the target
   assembler language.  The compiler assumes that the comment will
   end at the end of the line.  */

#define ASM_APP_ON "/* #APP */\n"
/* A C string constant for text to be output before each `asm'
   statement or group of consecutive ones.  Normally this is
   `"#APP"', which is a comment that has no effect on most assemblers
   but tells the GNU assembler that it must check the lines that
   follow for all valid assembler constructs.  */

#define ASM_APP_OFF "/* #NOAPP */\n"
/* A C string constant for text to be output after each `asm'
   statement or group of consecutive ones.  Normally this is
   `"#NO_APP"', which tells the GNU assembler to resume making the
   time-saving assumptions that are valid for ordinary compiler
   output.  */

#define ASM_OUTPUT_DOUBLE(STREAM, VALUE) fprintf (STREAM, "no double float %.20e\n", VALUE)
#define ASM_OUTPUT_FLOAT(STREAM, VALUE) asm_output_float (STREAM, VALUE)
/* `ASM_OUTPUT_LONG_DOUBLE (STREAM, VALUE)'
   `ASM_OUTPUT_THREE_QUARTER_FLOAT (STREAM, VALUE)'
   `ASM_OUTPUT_SHORT_FLOAT (STREAM, VALUE)'
   `ASM_OUTPUT_BYTE_FLOAT (STREAM, VALUE)'
   A C statement to output to the stdio stream STREAM an assembler
   instruction to assemble a floating-point constant of `TFmode',
   `DFmode', `SFmode', `TQFmode', `HFmode', or `QFmode',
   respectively, whose value is VALUE.  VALUE will be a C expression
   of type `REAL_VALUE_TYPE'.  Macros such as
   `REAL_VALUE_TO_TARGET_DOUBLE' are useful for writing these
   definitions.  */


#define ASM_OUTPUT_INT(FILE, VALUE)			\
 ( fprintf (FILE, "\t.long "),				\
   output_addr_const (FILE, (VALUE)),			\
   fputs ("\n", FILE))

 /* Likewise for `short' and `char' constants.   */

#define ASM_OUTPUT_SHORT(FILE,VALUE) asm_output_short(FILE,VALUE)
#define ASM_OUTPUT_CHAR(FILE,VALUE) asm_output_char(FILE,VALUE)

/* `ASM_OUTPUT_QUADRUPLE_INT (STREAM, EXP)'
   A C statement to output to the stdio stream STREAM an assembler
   instruction to assemble an integer of 16, 8, 4, 2 or 1 bytes,
   respectively, whose value is VALUE.  The argument EXP will be an
   RTL expression which represents a constant value.  Use
   `output_addr_const (STREAM, EXP)' to output this value as an
   assembler expression.

   For sizes larger than `UNITS_PER_WORD', if the action of a macro
   would be identical to repeatedly calling the macro corresponding to
   a size of `UNITS_PER_WORD', once for each word, you need not define
   the macro.  */


#define ASM_OUTPUT_BYTE(FILE,VALUE) asm_output_byte (FILE,VALUE)
/* A C statement to output to the stdio stream STREAM an assembler
   instruction to assemble a single byte containing the number VALUE.  */

#define ASM_BYTE_OP "\t.byte "
/* A C string constant giving the pseudo-op to use for a sequence of
   single-byte constants.  If this macro is not defined, the default
   is `"\t.byte\t"'.  */

#define ASM_OUTPUT_ASCII(FILE, P, SIZE)	 gas_output_ascii (FILE,P,SIZE)
/* `ASM_OUTPUT_ASCII (STREAM, PTR, LEN)'
   output_ascii (FILE, P, SIZE)
   A C statement to output to the stdio stream STREAM an assembler
   instruction to assemble a string constant containing the LEN bytes
   at PTR.  PTR will be a C expression of type `char *' and LEN a C
   expression of type `int'.

   If the assembler has a `.ascii' pseudo-op as found in the Berkeley
   Unix assembler, do not define the macro `ASM_OUTPUT_ASCII'.  */

#if GCC_VERSION_INT >= 0x430
#define IS_ASM_LOGICAL_LINE_SEPARATOR(C, STR) ((C) == '\n')
#else
#define IS_ASM_LOGICAL_LINE_SEPARATOR(C) ((C) == '\n')
#endif
/* Define this macro as a C expression which is nonzero if C is used
   as a logical line separator by the assembler.

   If you do not define this macro, the default is that only the
   character `;' is treated as a logical line separator.  */

/*
#define ASM_OPEN_PAREN "("
#define ASM_CLOSE_PAREN ")"
 These macros are defined as C string constant, describing the
   syntax in the assembler for grouping arithmetic expressions.  The
   following definitions are correct for most assemblers:

   #define ASM_OPEN_PAREN "("
   #define ASM_CLOSE_PAREN ")"

   These macros are provided by `real.h' for writing the definitions of
   `ASM_OUTPUT_DOUBLE' and the like:  */

/* Here we much catch r0 - r15  variable names */

#define ASM_OUTPUT_LABELREF(FILE,NAME) 			\
do{							\
	char *p = NAME;					\
	while(*p == '_') p++;				\
	if(*p == 'r' || *p == 'R')			\
	{						\
	    int val;					\
	    char *endptr;				\
	    p++;					\
	    val = strtol (p, &endptr, 10);		\
	    if(val >= 0 && val <= 15 &&			\
	    	*endptr == 0 )				\
	    {						\
	    	asm_fprintf ((FILE), "_%U%s", (NAME));	\
	    }						\
	    else					\
	    	asm_fprintf ((FILE), "%U%s", (NAME));   \
	}						\
	else						\
		asm_fprintf ((FILE), "%U%s", (NAME));	\
} while(0)


#define ASM_OUTPUT_COMMON(STREAM, NAME, SIZE, ROUNDED)	\
do {							\
     const char *p = NAME;					\
     if(*p == '*' || *p == '@' ) p++;			\
     if(*p >= '0' && *p <= '9' ) break;			\
     fputs ("\t.comm ", (STREAM));			\
     assemble_name ((STREAM), (NAME));			\
     fprintf ((STREAM), ",%d%s", (SIZE), (SIZE)>1?",2\n":"\n");\
} while (0)

#define ASM_OUTPUT_LOCAL(STREAM, NAME, SIZE, ROUNDED)	\
do {							\
     const char *p = NAME;					\
     if(*p == '*' || *p == '@' ) p++;			\
     if(*p >= '0' && *p <= '9' ) break;			\
     fputs ("\t.local ", (STREAM));			\
     assemble_name ((STREAM), (NAME));			\
     fputs ("\n",(STREAM));				\
     fputs ("\t.comm ", (STREAM));                     	\
     assemble_name ((STREAM), (NAME));			\
     fprintf ((STREAM), ",%d%s", (SIZE),(SIZE)>1?",2\n":"\n");\
} while (0)

#define ASM_OUTPUT_BSS(FILE, DECL, NAME, SIZE, ROUNDED)                 \
do { \
  const char *p = (NAME); \
  if(*p == '*' || *p == '@' ) p++; \
  if(*p >= '0' && *p <= '9' ) break; \
  asm_output_bss ((FILE), (DECL), (NAME), (SIZE), (ROUNDED)); \
} while(0)

/* macros to output uninitialized variable definitions */

/* Return a non-zero value if DECL has a section attribute.  */
#define IN_NAMED_SECTION(DECL)                                           \
  ((TREE_CODE (DECL) == FUNCTION_DECL || TREE_CODE (DECL) == VAR_DECL)    \
   && DECL_SECTION_NAME (DECL) != NULL_TREE)

#define BSS_SECTION_ASM_OP	"\t.section\t.bss"
/* If defined, a C expression whose value is a string containing the
   assembler operation to identify the following data as
   uninitialized global data.  If not defined, and neither
   'ASM_OUTPUT_BSS' nor 'ASM_OUTPUT_ALIGNED_BSS' are defined,
   uninitialized global data will be output in the data section if
   -fno-common' is passed, otherwise 'ASM_OUTPUT_COMMON' will be
   used.
*/


#define ASM_OUTPUT_LABEL(STREAM, NAME)		\
{						\
  int __msp430_data_name_workaround = (((NAME)[0] == '*') && ((NAME)[1] == '0'));  \
  if (__msp430_data_name_workaround) \
  { \
    static int repeatable_pseudolabels_resolver = 0;\
	fprintf (STREAM, "__");			\
    assemble_name (STREAM, NAME);			\
	fprintf (STREAM, "_%d", ++repeatable_pseudolabels_resolver);			\
  } \
  else \
    assemble_name (STREAM, NAME);			\
  fprintf (STREAM, ":\n");			\
}
/* A C statement (sans semicolon) to output to the stdio stream
   STREAM the assembler definition of a label named NAME.  Use the
   expression `assemble_name (STREAM, NAME)' to output the name
   itself; before and after that, output the additional assembler
   syntax for defining the name, and a newline.  */

#undef TYPE_ASM_OP
#undef SIZE_ASM_OP
#undef WEAK_ASM_OP
#define TYPE_ASM_OP	"\t.type\t"
#define SIZE_ASM_OP	"\t.size\t"
#define WEAK_ASM_OP	"\t.weak\t"
/* Define the strings used for the special svr4 .type and .size directives.
   These strings generally do not vary from one system running svr4 to
   another, but if a given system (e.g. m88k running svr) needs to use
   different pseudo-op names for these, they may be overridden in the
   file which includes this one.  */


#undef TYPE_OPERAND_FMT
#define TYPE_OPERAND_FMT	"@%s"
/* The following macro defines the format used to output the second
   operand of the .type assembler directive.  Different svr4 assemblers
   expect various different forms for this operand.  The one given here
   is just a default.  You may need to override it in your machine-
   specific tm.h file (depending upon the particulars of your assembler).  */


#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL) \
asm_declare_function_name (FILE, NAME, DECL)


/* A C statement (sans semicolon) to output to the stdio stream
   STREAM any text necessary for declaring the name NAME of a
   function which is being defined.  This macro is responsible for
   outputting the label definition (perhaps using
   `ASM_OUTPUT_LABEL').  The argument DECL is the `FUNCTION_DECL'
   tree node representing the function.

   If this macro is not defined, then the function name is defined in
   the usual manner as a label (by means of `ASM_OUTPUT_LABEL').  */


#define ASM_DECLARE_FUNCTION_SIZE(FILE, FNAME, DECL)			\
  do {									\
    if (!flag_inhibit_size_directive)					\
      {									\
        char label[256];						\
	static int labelno;						\
	labelno++;							\
	ASM_GENERATE_INTERNAL_LABEL (label, "Lfe", labelno);		\
	fprintf(FILE, ".%s%d:\n", "Lfe", labelno) ;		\
	fprintf (FILE, "%s", SIZE_ASM_OP);				\
	assemble_name (FILE, (FNAME));					\
        fprintf (FILE, ",");						\
	assemble_name (FILE, label);					\
        fprintf (FILE, "-");						\
	assemble_name (FILE, (FNAME));					\
	fprintf (FILE,"\n;; End of function \n\n");	\
      }									\
  } while (0)

#define ASM_DECLARE_OBJECT_NAME(FILE, NAME, DECL)			  \
do {									  \
      int __msp430_data_name_workaround = (((NAME)[0] == '*') && ((NAME)[1] == '0'));  \
	  if (__msp430_data_name_workaround) \
	    fprintf (FILE, "#"); \
      fprintf (FILE, "%s", TYPE_ASM_OP);				  \
      assemble_name (FILE, NAME);					  \
      putc (',', FILE);							  \
      fprintf (FILE, TYPE_OPERAND_FMT, "object");			  \
      putc ('\n', FILE);						  \
      size_directive_output = 0;					  \
      if (!flag_inhibit_size_directive && DECL_SIZE (DECL))		  \
	{								  \
	  size_directive_output = 1;					  \
	  if (__msp430_data_name_workaround) \
	    fprintf (FILE, "#"); \
	  fprintf (FILE, "%s", SIZE_ASM_OP);				  \
	  assemble_name (FILE, NAME);					  \
	  fprintf (FILE, ",%ld\n",  int_size_in_bytes (TREE_TYPE (DECL))); \
    }									  \
  ASM_OUTPUT_LABEL(FILE, NAME);						  \
} while (0)

#define ASM_FINISH_DECLARE_OBJECT(FILE, DECL, TOP_LEVEL, AT_END)	 \
do {									 \
     const char *name = XSTR (XEXP (DECL_RTL (DECL), 0), 0);		 \
     if (!flag_inhibit_size_directive && DECL_SIZE (DECL)		 \
         && ! AT_END && TOP_LEVEL					 \
	 && DECL_INITIAL (DECL) == error_mark_node			 \
	 && !size_directive_output)					 \
       {								 \
	 size_directive_output = 1;					 \
	 fprintf (FILE, "%s", SIZE_ASM_OP);				 \
	 assemble_name (FILE, name);					 \
	 fprintf (FILE, ",%d\n",  int_size_in_bytes (TREE_TYPE (DECL))); \
       }								 \
   } while (0)


#define ESCAPES \
"\1\1\1\1\1\1\1\1btn\1fr\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\\\0\0\0\
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\
\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1"
/* A table of bytes codes used by the ASM_OUTPUT_ASCII and
   ASM_OUTPUT_LIMITED_STRING macros.  Each byte in the table
   corresponds to a particular byte value [0..255].  For any
   given byte value, if the value in the corresponding table
   position is zero, the given character can be output directly.
   If the table value is 1, the byte must be output as a \ooo
   octal escape.  If the tables value is anything else, then the
   byte value should be output as a \ followed by the value
   in the table.  Note that we can use standard UN*X escape
   sequences for many control characters, but we don't use
   \a to represent BEL because some svr4 assemblers (e.g. on
   the i386) don't know about that.  Also, we don't use \v
   since some versions of gas, such as 2.2 did not accept it.  */

#define STRING_LIMIT	((unsigned) 64)
#define STRING_ASM_OP	"\t.string\t"
/* Some svr4 assemblers have a limit on the number of characters which
   can appear in the operand of a .string directive.  If your assembler
   has such a limitation, you should define STRING_LIMIT to reflect that
   limit.  Note that at least some svr4 assemblers have a limit on the
   actual number of bytes in the double-quoted string, and that they
   count each character in an escape sequence as one byte.  Thus, an
   escape sequence like \377 would count as four bytes.

   If your target assembler doesn't support the .string directive, you
   should define this to zero.  */

#define GLOBAL_ASM_OP ".global\t"
     
#define ASM_WEAKEN_LABEL(FILE, NAME) 	\
  do					\
    {					\
      fputs ("\t.weak\t", (FILE));	\
      assemble_name ((FILE), (NAME)); 	\
      fputc ('\n', (FILE));		\
    }					\
  while (0)

/* A C statement (sans semicolon) to output to the stdio stream
   STREAM some commands that will make the label NAME weak; that is,
   available for reference from other files but only used if no other
   definition is available.  Use the expression `assemble_name
   (STREAM, NAME)' to output the name itself; before and after that,
   output the additional assembler syntax for making that name weak,
   and a newline.

   If you don't define this macro, GNU CC will not support weak
   symbols and you should not define the `SUPPORTS_WEAK' macro.
*/

#define SUPPORTS_WEAK 1
/* A C expression which evaluates to true if the target supports weak
   symbols.

   If you don't define this macro, `defaults.h' provides a default
   definition.  If `ASM_WEAKEN_LABEL' is defined, the default
   definition is `1'; otherwise, it is `0'.  Define this macro if you
   want to control weak symbol support with a compiler flag such as
   `-melf'.

   `MAKE_DECL_ONE_ONLY'
   A C statement (sans semicolon) to mark DECL to be emitted as a
   public symbol such that extra copies in multiple translation units
   will be discarded by the linker.  Define this macro if your object
   file format provides support for this concept, such as the `COMDAT'
   section flags in the Microsoft Windows PE/COFF format, and this
   support requires changes to DECL, such as putting it in a separate
   section.

   `SUPPORTS_WEAK'
   A C expression which evaluates to true if the target supports
   one-only semantics.

   If you don't define this macro, `varasm.c' provides a default
   definition.  If `MAKE_DECL_ONE_ONLY' is defined, the default
   definition is `1'; otherwise, it is `0'.  Define this macro if you
   want to control weak symbol support with a compiler flag, or if
   setting the `DECL_ONE_ONLY' flag is enough to mark a declaration to
   be emitted as one-only.  */

/* A C statement to output to the stdio stream STREAM a label whose
   name is made from the string PREFIX and the number NUM.

   It is absolutely essential that these labels be distinct from the
   labels used for user-level functions and variables.  Otherwise,
   certain programs will have name conflicts with internal labels.

   It is desirable to exclude internal labels from the symbol table
   of the object file.  Most assemblers have a naming convention for
   labels that should be excluded; on many systems, the letter `L' at
   the beginning of a label has this effect.  You should find out what
   convention your system uses, and follow it.

   The usual definition of this macro is as follows:

   fprintf (STREAM, "L%s%d:\n", PREFIX, NUM)  */

#define ASM_GENERATE_INTERNAL_LABEL(STRING, PREFIX, NUM)	\
sprintf (STRING, "*.%s%d", PREFIX, NUM)
/* A C statement to store into the string STRING a label whose name
   is made from the string PREFIX and the number NUM.

   This string, when output subsequently by `assemble_name', should
   produce the output that `ASM_OUTPUT_INTERNAL_LABEL' would produce
   with the same PREFIX and NUM.

   If the string begins with `*', then `assemble_name' will output
   the rest of the string unchanged.  It is often convenient for
   `ASM_GENERATE_INTERNAL_LABEL' to use `*' in this way.  If the
   string doesn't start with `*', then `ASM_OUTPUT_LABELREF' gets to
   output the string, and may change it.  (Of course,
   `ASM_OUTPUT_LABELREF' is also part of your machine description, so
   you should know what it does on your machine.)  */

#define ASM_FORMAT_PRIVATE_NAME(OUTPUT, NAME, LABELNO)	\
( (OUTPUT) = (char *) alloca (strlen ((NAME)) + 10),	\
  sprintf ((OUTPUT), "%s.%d", (NAME), (LABELNO)))

/* A C expression to assign to OUTVAR (which is a variable of type
   `char *') a newly allocated string made from the string NAME and
   the number NUMBER, with some suitable punctuation added.  Use
   `alloca' to get space for the string.

   The string will be used as an argument to `ASM_OUTPUT_LABELREF' to
   produce an assembler label for an internal static variable whose
   name is NAME.  Therefore, the string must be such as to result in
   valid assembler code.  The argument NUMBER is different each time
   this macro is executed; it prevents conflicts between
   similarly-named internal static variables in different scopes.

   Ideally this string should not be a valid C identifier, to prevent
   any conflict with the user's own symbols.  Most assemblers allow
   periods or percent signs in assembler symbols; putting at least
   one of these between the name and the number will suffice.  */

/* `ASM_OUTPUT_WEAK_ALIAS (STREAM, NAME, VALUE)'
   A C statement to output to the stdio stream STREAM assembler code
   which defines (equates) the weak symbol NAME to have the value
   VALUE.

   Define this macro if the target only supports weak aliases; define
   ASM_OUTPUT_DEF instead if possible.  */

#define HAS_INIT_SECTION 1
/* If defined, `main' will not call `__main' as described above.
   This macro should be defined for systems that control the contents
   of the init section on a symbol-by-symbol basis, such as OSF/1,
   and should not be defined explicitly for systems that support
   `INIT_SECTION_ASM_OP'.  */

#define REGISTER_NAMES {				\
  "r0","r1","r2","r3","r4","r5","r6","r7",		\
  "r8","r9","r10","r11","r12","r13","r14","r15","argp"		\
}
/* A C initializer containing the assembler's names for the machine
   registers, each one as a C string constant.  This is what
   translates register numbers in the compiler into assembler
   language.  */

#define FINAL_PRESCAN_INSN(insn, operand, nop) final_prescan_insn (insn, operand,nop)
/* If defined, a C statement to be executed just prior to the output
   of assembler code for INSN, to modify the extracted operands so
   they will be output differently.

   Here the argument OPVEC is the vector containing the operands
   extracted from INSN, and NOPERANDS is the number of elements of
   the vector which contain meaningful data for this insn.  The
   contents of this vector are what will be used to convert the insn
   template into assembler code, so you can change the assembler
   output by changing the contents of the vector.

   This macro is useful when various assembler syntaxes share a single
   file of instruction patterns; by defining this macro differently,
   you can cause a large class of instructions to be output
   differently (such as with rearranged operands).  Naturally,
   variations in assembler syntax affecting individual insn patterns
   ought to be handled by writing conditional output routines in
   those patterns.

   If this macro is not defined, it is equivalent to a null statement.  */

#define PRINT_OPERAND(STREAM, X, CODE) print_operand (STREAM, X, CODE)
/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand X.  X is an RTL
   expression.

   CODE is a value that can be used to specify one of several ways of
   printing the operand.  It is used when identical operands must be
   printed differently depending on the context.  CODE comes from the
   `%' specification that was used to request printing of the
   operand.  If the specification was just `%DIGIT' then CODE is 0;
   if the specification was `%LTR DIGIT' then CODE is the ASCII code
   for LTR.

   If X is a register, this macro should print the register's name.
   The names can be found in an array `reg_names' whose type is `char
   *[]'.  `reg_names' is initialized from `REGISTER_NAMES'.

   When the machine description has a specification `%PUNCT' (a `%'
   followed by a punctuation character), this macro is called with a
   null pointer for X and the punctuation character for CODE.  */

#define PRINT_OPERAND_PUNCT_VALID_P(CODE) ((CODE) == '~')
/* A C expression which evaluates to true if CODE is a valid
   punctuation character for use in the `PRINT_OPERAND' macro.  If
   `PRINT_OPERAND_PUNCT_VALID_P' is not defined, it means that no
   punctuation characters (except for the standard one, `%') are used
   in this way.  */

#define PRINT_OPERAND_ADDRESS(STREAM, X) print_operand_address(STREAM, X)
/* A C compound statement to output to stdio stream STREAM the
   assembler syntax for an instruction operand that is a memory
   reference whose address is X.  X is an RTL expression.

   On some machines, the syntax for a symbolic address depends on the
   section that the address refers to.  On these machines, define the
   macro `ENCODE_SECTION_INFO' to store the information into the
   `symbol_ref', and then check for it here.  *Note Assembler
   Format::.  */

#define USER_LABEL_PREFIX ""
/* `LOCAL_LABEL_PREFIX'
   `REGISTER_PREFIX'
   `IMMEDIATE_PREFIX'
   If defined, C string expressions to be used for the `%R', `%L',
   `%U', and `%I' options of `asm_fprintf' (see `final.c').  These
   are useful when a single `md' file must support multiple assembler
   formats.  In that case, the various `tm.h' files can define these
   macros differently.  */

#define ASSEMBLER_DIALECT MSP430_HAS_HWMUL_INTERNAL
/* If your target supports multiple dialects of assembler language
  (such as different opcodes), define this macro as a C expression
  that gives the numeric index of the assembler language dialect to
  use, with zero as the first variant.

  If this macro is defined, you may use constructs of the form
  `{option0|option1|option2...}' in the output templates of patterns
  (*note Output Template::.) or in the first argument of
  `asm_fprintf'.  This construct outputs `option0', `option1' or
  `option2', etc., if the value of `ASSEMBLER_DIALECT' is zero, one
  or two, etc.  Any special characters within these strings retain
  their usual meaning.

  If you do not define this macro, the characters `{', `|' and `}'
  do not have any special meaning when used in templates or operands
  to `asm_fprintf'.

  Define the macros `REGISTER_PREFIX', `LOCAL_LABEL_PREFIX',
  `USER_LABEL_PREFIX' and `IMMEDIATE_PREFIX' if you can express the
  variations in assembler language syntax with that mechanism.
  Define `ASSEMBLER_DIALECT' and use the `{option0|option1}' syntax
  if the syntax variant are larger and involve such things as
  different opcodes or operand order.  */

#define ASM_OUTPUT_REG_PUSH(STREAM, REGNO)	\
{						\
  if (REGNO > 15)				\
    abort ();					\
  fprintf (STREAM, "\tpush\tr%d", REGNO);	\
}
/* A C expression to output to STREAM some assembler code which will
   push hard register number REGNO onto the stack.  The code need not
   be optimal, since this macro is used only when profiling.  */

#define ASM_OUTPUT_REG_POP(STREAM, REGNO)	\
{						\
  if (REGNO > 15)				\
    abort ();					\
  fprintf (STREAM, "\tpop\tr%d", REGNO);	\
}
/* A C expression to output to STREAM some assembler code which will
   pop hard register number REGNO off of the stack.  The code need
   not be optimal, since this macro is used only when profiling.  */

#define ASM_OUTPUT_ADDR_VEC_ELT(STREAM, VALUE)		\
  msp430_output_addr_vec_elt(STREAM, VALUE)
/* This macro should be provided on machines where the addresses in a
   dispatch table are absolute.

   The definition should be a C statement to output to the stdio
   stream STREAM an assembler pseudo-instruction to generate a
   reference to a label.  VALUE is the number of an internal label
   whose definition is output using `ASM_OUTPUT_INTERNAL_LABEL'.  For
   example,

   fprintf (STREAM, "\t.word L%d\n", VALUE)  */

/*
#define ASM_OUTPUT_CASE_LABEL(STREAM, PREFIX, NUM, TABLE) \
   progmem_section (), ASM_OUTPUT_INTERNAL_LABEL (STREAM, PREFIX, NUM)

   `ASM_OUTPUT_CASE_LABEL (STREAM, PREFIX, NUM, TABLE)'
   Define this if the label before a jump-table needs to be output
   specially.  The first three arguments are the same as for
   `ASM_OUTPUT_INTERNAL_LABEL'; the fourth argument is the jump-table
   which follows (a `jump_insn' containing an `addr_vec' or
   `addr_diff_vec').

   This feature is used on system V to output a `swbeg' statement for
   the table.

   If this macro is not defined, these labels are output with
   `ASM_OUTPUT_INTERNAL_LABEL'.  */

/* `ASM_OUTPUT_CASE_END (STREAM, NUM, TABLE)'
   Define this if something special must be output at the end of a
   jump-table.  The definition should be a C statement to be executed
   after the assembler code for the table is written.  It should write
   the appropriate code to stdio stream STREAM.  The argument TABLE
   is the jump-table insn, and NUM is the label-number of the
   preceding label.

   If this macro is not defined, nothing special is output at the end
   of the jump-table.  */

#ifndef SET_ASM_OP
#define SET_ASM_OP "\t.set\t"
#endif
  
#define ASM_OUTPUT_SKIP(STREAM, N)		\
fprintf (STREAM, "\t.skip %d,0\n", N)
/* A C statement to output to the stdio stream STREAM an assembler
   instruction to advance the location counter by NBYTES bytes.
   Those bytes should be zero when loaded.  NBYTES will be a C
   expression of type `int'.  */

#define ASM_OUTPUT_ALIGN(STREAM, POWER) \
fprintf (STREAM, "\t.p2align %d,0\n", POWER)
/* A C statement to output to the stdio stream STREAM an assembler
   command to advance the location counter to a multiple of 2 to the
   POWER bytes.  POWER will be a C expression of type `int'.  */

#define CASE_VECTOR_MODE HImode
/* An alias for a machine mode name.  This is the machine mode that
   elements of a jump-table should have.  */

extern struct rtx_def *msp430_compare_op0, *msp430_compare_op1;


extern int msp430_case_values_threshold;

#define CASE_VALUES_THRESHOLD msp430_case_values_threshold
/* `CASE_VALUES_THRESHOLD'
   Define this to be the smallest number of different values for
   which it is best to use a jump-table instead of a tree of
   conditional branches.  The default is four for machines with a
   `casesi' instruction and five otherwise.  This is best for most
   machines.  */

#undef WORD_REGISTER_OPERATIONS
/* Define this macro if operations between registers with integral
   mode smaller than a word are always performed on the entire
   register.  Most RISC machines have this property and most CISC
   machines do not.  */

/*
#define EASY_DIV_EXPR TRUNC_DIV_EXPR
 An alias for a tree code that is the easiest kind of division to
   compile code for in the general case.  It may be `TRUNC_DIV_EXPR',
   `FLOOR_DIV_EXPR', `CEIL_DIV_EXPR' or `ROUND_DIV_EXPR'.  These four
   division operators differ in how they round the result to an
   integer.  `EASY_DIV_EXPR' is used when it is permissible to use
   any of those kinds of division and the choice should be made on
   the basis of efficiency.  */

#define MOVE_MAX 2
/* The maximum number of bytes that a single instruction can move
   quickly between memory and registers or between two memory
   locations.  */

#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1
/* A C expression which is nonzero if on this machine it is safe to
   "convert" an integer of INPREC bits to one of OUTPREC bits (where
   OUTPREC is smaller than INPREC) by merely operating on it as if it
   had only OUTPREC bits.

   On many machines, this expression can be 1.

   When `TRULY_NOOP_TRUNCATION' returns 1 for a pair of sizes for
   modes for which `MODES_TIEABLE_P' is 0, suboptimal code can result.
   If this is the case, making `TRULY_NOOP_TRUNCATION' return 0 in
   such cases may improve things.  */

#define Pmode HImode
/* An alias for the machine mode for pointers.  On most machines,
   define this to be the integer mode corresponding to the width of a
   hardware pointer; `SImode' on 32-bit machine or `DImode' on 64-bit
   machines.  On some machines you must define this to be one of the
   partial integer modes, such as `PSImode'.

   The width of `Pmode' must be at least as large as the value of
   `POINTER_SIZE'.  If it is not equal, you must define the macro
   `POINTERS_EXTEND_UNSIGNED' to specify how pointers are extended to
   `Pmode'.  */

#define FUNCTION_MODE HImode
/* An alias for the machine mode used for memory references to
   functions being called, in `call' RTL expressions.  On most
   machines this should be `QImode'.  */
     /*                            1        3 */
/*
#define VALID_MACHINE_DECL_ATTRIBUTE(DECL, ATTRIBUTES, IDENTIFIER, ARGS) \
valid_machine_decl_attribute (DECL, ATTRIBUTES, IDENTIFIER, ARGS)
 `VALID_MACHINE_DECL_ATTRIBUTE (DECL, ATTRIBUTES, IDENTIFIER, ARGS)'
   If defined, a C expression whose value is nonzero if IDENTIFIER
   with arguments ARGS is a valid machine specific attribute for DECL.
   The attributes in ATTRIBUTES have previously been assigned to DECL.  */

/*
#define VALID_MACHINE_TYPE_ATTRIBUTE(TYPE, ATTRIBUTES, IDENTIFIER, ARGS) \
     valid_machine_type_attribute(TYPE, ATTRIBUTES, IDENTIFIER, ARGS)
   `VALID_MACHINE_TYPE_ATTRIBUTE (TYPE, ATTRIBUTES, IDENTIFIER, ARGS)'
   If defined, a C expression whose value is nonzero if IDENTIFIER
   with arguments ARGS is a valid machine specific attribute for TYPE.
   The attributes in ATTRIBUTES have previously been assigned to TYPE.  */

#define DOLLARS_IN_IDENTIFIERS 0
/* Define this macro to control use of the character `$' in identifier
   names.  0 means `$' is not allowed by default; 1 means it is
   allowed.  1 is the default; there is no need to define this macro
   in that case.  This macro controls the compiler proper; it does
   not affect the preprocessor.  */

#define NO_DOLLAR_IN_LABEL 1
/* Define this macro if the assembler does not accept the character
   `$' in label names.  By default constructors and destructors in
   G++ have `$' in the identifiers.  If this macro is defined, `.' is
   used instead.  */

#define TRAMPOLINE_TEMPLATE(FILE) msp430_trampoline_template((FILE))

#define TRAMPOLINE_SIZE 8
#define TRAMPOLINE_ALIGNMENT 16


#define INITIALIZE_TRAMPOLINE(TRAMP, FNADDR, CXT)	\
	msp430_initialize_trampoline (TRAMP, FNADDR, CXT)


/* Store in cc_status the expressions
   that the condition codes will describe
   after execution of an instruction whose pattern is EXP.
   Do not alter them if the instruction would not alter the cc's.  */

#define NOTICE_UPDATE_CC(EXP, INSN) notice_update_cc(EXP, INSN)

/* The add insns don't set overflow in a usable way.  */
#define CC_OVERFLOW_UNUSABLE 01000
/* The mov,and,or,xor insns don't set carry.  That's ok though as the
   Z bit is all we need when doing unsigned comparisons on the result of
   these insns (since they're always with 0).  However, conditions.h has
   CC_NO_OVERFLOW defined for this purpose.  Rename it to something more
   understandable.  */
#define CC_NO_CARRY CC_NO_OVERFLOW


/* Output assembler code to FILE to increment profiler label # LABELNO
   for profiling a function entry.  */

#define FUNCTION_PROFILER(FILE, LABELNO)  \
  fprintf (FILE, "/* profiler %d */", (LABELNO))

/* `FIRST_INSN_ADDRESS'
   When the `length' insn attribute is used, this macro specifies the
   value to be assigned to the address of the first insn in a
   function.  If not specified, 0 is used.  */

#define ADJUST_INSN_LENGTH(INSN, LENGTH) (LENGTH =\
					  adjust_insn_length (INSN, LENGTH))
/* If defined, modifies the length assigned to instruction INSN as a
   function of the context in which it is used.  LENGTH is an lvalue
   that contains the initially computed length of the insn and should
   be updated with the correct length of the insn.  If updating is
   required, INSN must not be a varying-length insn.

   This macro will normally not be required.  A case in which it is
   required is the ROMP.  On this machine, the size of an `addr_vec'
   insn must be increased by two to compensate for the fact that
   alignment may be required.  */

#define CPP_SPEC "\
%{!mmcu*|mmcu=msp1:%(cpp_msp1)} \
%{mmcu=msp2:%(cpp_msp2)} \
%{mmcu=msp3:%(cpp_msp3)} \
%{mmcu=msp4:%(cpp_msp4)} \
%{mmcu=msp5:%(cpp_msp5)} \
%{mmcu=msp6:%(cpp_msp6)} \
%{mmcu=msp430x110:%(cpp_msp1) -D__MSP430_110__} \
%{mmcu=msp430x112:%(cpp_msp1) -D__MSP430_112__} \
%{mmcu=msp430x1101:%(cpp_msp1) -D__MSP430_1101__} \
%{mmcu=msp430x1111:%(cpp_msp1) -D__MSP430_1111__} \
%{mmcu=msp430x1121:%(cpp_msp1) -D__MSP430_1121__} \
%{mmcu=msp430x1122:%(cpp_msp1) -D__MSP430_1122__} \
%{mmcu=msp430x1132:%(cpp_msp1) -D__MSP430_1132__} \
%{mmcu=msp430x122:%(cpp_msp1) -D__MSP430_122__} \
%{mmcu=msp430x123:%(cpp_msp1) -D__MSP430_123__} \
%{mmcu=msp430x1222:%(cpp_msp1) -D__MSP430_1222__} \
%{mmcu=msp430x1232:%(cpp_msp1) -D__MSP430_1232__} \
%{mmcu=msp430x133:%(cpp_msp1) -D__MSP430_133__} \
%{mmcu=msp430x135:%(cpp_msp1) -D__MSP430_135__} \
%{mmcu=msp430x1331:%(cpp_msp1) -D__MSP430_1331__} \
%{mmcu=msp430x1351:%(cpp_msp1) -D__MSP430_1351__} \
%{mmcu=msp430x147:%(cpp_msp2) -D__MSP430_147__} \
%{mmcu=msp430x148:%(cpp_msp2) -D__MSP430_148__} \
%{mmcu=msp430x149:%(cpp_msp2) -D__MSP430_149__} \
%{mmcu=msp430x1471:%(cpp_msp2) -D__MSP430_1471__} \
%{mmcu=msp430x1481:%(cpp_msp2) -D__MSP430_1481__} \
%{mmcu=msp430x1491:%(cpp_msp2) -D__MSP430_1491__} \
%{mmcu=msp430x155:%(cpp_msp1) -D__MSP430_155__} \
%{mmcu=msp430x156:%(cpp_msp1) -D__MSP430_156__} \
%{mmcu=msp430x157:%(cpp_msp1) -D__MSP430_157__} \
%{mmcu=msp430x167:%(cpp_msp2) -D__MSP430_167__} \
%{mmcu=msp430x168:%(cpp_msp2) -D__MSP430_168__} \
%{mmcu=msp430x169:%(cpp_msp2) -D__MSP430_169__} \
%{mmcu=msp430x1610:%(cpp_msp2) -D__MSP430_1610__} \
%{mmcu=msp430x1611:%(cpp_msp2) -D__MSP430_1611__} \
%{mmcu=msp430x1612:%(cpp_msp2) -D__MSP430_1612__} \
%{mmcu=msp430x2001:%(cpp_msp1) -D__MSP430_2001__} \
%{mmcu=msp430x2011:%(cpp_msp1) -D__MSP430_2011__} \
%{mmcu=msp430x2002:%(cpp_msp1) -D__MSP430_2002__} \
%{mmcu=msp430x2012:%(cpp_msp1) -D__MSP430_2012__} \
%{mmcu=msp430x2003:%(cpp_msp1) -D__MSP430_2003__} \
%{mmcu=msp430x2013:%(cpp_msp1) -D__MSP430_2013__} \
%{mmcu=msp430x2101:%(cpp_msp1) -D__MSP430_2101__} \
%{mmcu=msp430x2111:%(cpp_msp1) -D__MSP430_2111__} \
%{mmcu=msp430x2121:%(cpp_msp1) -D__MSP430_2121__} \
%{mmcu=msp430x2131:%(cpp_msp1) -D__MSP430_2131__} \
%{mmcu=msp430x2112:%(cpp_msp1) -D__MSP430_2112__} \
%{mmcu=msp430x2122:%(cpp_msp1) -D__MSP430_2122__} \
%{mmcu=msp430x2132:%(cpp_msp1) -D__MSP430_2132__} \
%{mmcu=msp430x2201:%(cpp_msp1) -D__MSP430_2201__} \
%{mmcu=msp430x2211:%(cpp_msp1) -D__MSP430_2211__} \
%{mmcu=msp430x2221:%(cpp_msp1) -D__MSP430_2221__} \
%{mmcu=msp430x2231:%(cpp_msp1) -D__MSP430_2231__} \
%{mmcu=msp430x2232:%(cpp_msp1) -D__MSP430_2232__} \
%{mmcu=msp430x2252:%(cpp_msp1) -D__MSP430_2252__} \
%{mmcu=msp430x2272:%(cpp_msp1) -D__MSP430_2272__} \
%{mmcu=msp430x2234:%(cpp_msp1) -D__MSP430_2234__} \
%{mmcu=msp430x2254:%(cpp_msp1) -D__MSP430_2254__} \
%{mmcu=msp430x2274:%(cpp_msp1) -D__MSP430_2274__} \
%{mmcu=msp430x233:%(cpp_msp2) -D__MSP430_233__} \
%{mmcu=msp430x235:%(cpp_msp2) -D__MSP430_235__} \
%{mmcu=msp430x2330:%(cpp_msp2) -D__MSP430_2330__} \
%{mmcu=msp430x2350:%(cpp_msp2) -D__MSP430_2350__} \
%{mmcu=msp430x2370:%(cpp_msp2) -D__MSP430_2370__} \
%{mmcu=msp430x247:%(cpp_msp2) -D__MSP430_247__} \
%{mmcu=msp430x248:%(cpp_msp2) -D__MSP430_248__} \
%{mmcu=msp430x249:%(cpp_msp2) -D__MSP430_249__} \
%{mmcu=msp430x2410:%(cpp_msp2) -D__MSP430_2410__} \
%{mmcu=msp430x2471:%(cpp_msp2) -D__MSP430_2471__} \
%{mmcu=msp430x2481:%(cpp_msp2) -D__MSP430_2481__} \
%{mmcu=msp430x2491:%(cpp_msp2) -D__MSP430_2491__} \
%{mmcu=msp430x2416:%(cpp_msp3) -D__MSP430_2416__} \
%{mmcu=msp430x2417:%(cpp_msp3) -D__MSP430_2417__} \
%{mmcu=msp430x2418:%(cpp_msp3) -D__MSP430_2418__} \
%{mmcu=msp430x2419:%(cpp_msp3) -D__MSP430_2419__} \
%{mmcu=msp430x2616:%(cpp_msp3) -D__MSP430_2616__} \
%{mmcu=msp430x2617:%(cpp_msp3) -D__MSP430_2617__} \
%{mmcu=msp430x2618:%(cpp_msp3) -D__MSP430_2618__} \
%{mmcu=msp430x2619:%(cpp_msp3) -D__MSP430_2619__} \
%{mmcu=msp430x311:%(cpp_msp1) -D__MSP430_311__} \
%{mmcu=msp430x312:%(cpp_msp1) -D__MSP430_312__} \
%{mmcu=msp430x313:%(cpp_msp1) -D__MSP430_313__} \
%{mmcu=msp430x314:%(cpp_msp1) -D__MSP430_314__} \
%{mmcu=msp430x315:%(cpp_msp1) -D__MSP430_315__} \
%{mmcu=msp430x323:%(cpp_msp1) -D__MSP430_323__} \
%{mmcu=msp430x325:%(cpp_msp1) -D__MSP430_325__} \
%{mmcu=msp430x336:%(cpp_msp2) -D__MSP430_336__} \
%{mmcu=msp430x337:%(cpp_msp2) -D__MSP430_337__} \
%{mmcu=msp430x412:%(cpp_msp1) -D__MSP430_412__} \
%{mmcu=msp430x413:%(cpp_msp1) -D__MSP430_413__} \
%{mmcu=msp430x415:%(cpp_msp1) -D__MSP430_415__} \
%{mmcu=msp430x417:%(cpp_msp1) -D__MSP430_417__} \
%{mmcu=msp430x423:%(cpp_msp2) -D__MSP430_423__} \
%{mmcu=msp430x425:%(cpp_msp2) -D__MSP430_425__} \
%{mmcu=msp430x427:%(cpp_msp2) -D__MSP430_427__} \
%{mmcu=msp430x4250:%(cpp_msp1) -D__MSP430_4250__} \
%{mmcu=msp430x4260:%(cpp_msp1) -D__MSP430_4260__} \
%{mmcu=msp430x4270:%(cpp_msp1) -D__MSP430_4270__} \
%{mmcu=msp430xG4250:%(cpp_msp1) -D__MSP430_G4250__} \
%{mmcu=msp430xG4260:%(cpp_msp1) -D__MSP430_G4260__} \
%{mmcu=msp430xG4270:%(cpp_msp1) -D__MSP430_G4270__} \
%{mmcu=msp430xE423:%(cpp_msp2) -D__MSP430_E423__} \
%{mmcu=msp430xE425:%(cpp_msp2) -D__MSP430_E425__} \
%{mmcu=msp430xE427:%(cpp_msp2) -D__MSP430_E427__} \
%{mmcu=msp430xE4232:%(cpp_msp2) -D__MSP430_E4232__} \
%{mmcu=msp430xE4242:%(cpp_msp2) -D__MSP430_E4242__} \
%{mmcu=msp430xE4252:%(cpp_msp2) -D__MSP430_E4252__} \
%{mmcu=msp430xE4272:%(cpp_msp2) -D__MSP430_E4272__} \
%{mmcu=msp430xW423:%(cpp_msp1) -D__MSP430_W423__} \
%{mmcu=msp430xW425:%(cpp_msp1) -D__MSP430_W425__} \
%{mmcu=msp430xW427:%(cpp_msp1) -D__MSP430_W427__} \
%{mmcu=msp430xG437:%(cpp_msp1) -D__MSP430_G437__} \
%{mmcu=msp430xG438:%(cpp_msp1) -D__MSP430_G438__} \
%{mmcu=msp430xG439:%(cpp_msp1) -D__MSP430_G439__} \
%{mmcu=msp430x435:%(cpp_msp1) -D__MSP430_435__} \
%{mmcu=msp430x436:%(cpp_msp1) -D__MSP430_436__} \
%{mmcu=msp430x437:%(cpp_msp1) -D__MSP430_437__} \
%{mmcu=msp430x4351:%(cpp_msp1) -D__MSP430_4351__} \
%{mmcu=msp430x4361:%(cpp_msp1) -D__MSP430_4361__} \
%{mmcu=msp430x4371:%(cpp_msp1) -D__MSP430_4371__} \
%{mmcu=msp430x447:%(cpp_msp2) -D__MSP430_447__} \
%{mmcu=msp430x448:%(cpp_msp2) -D__MSP430_448__} \
%{mmcu=msp430x449:%(cpp_msp2) -D__MSP430_449__} \
%{mmcu=msp430xG4616:%(cpp_msp3) -D__MSP430_G4616__} \
%{mmcu=msp430xG4617:%(cpp_msp3) -D__MSP430_G4617__} \
%{mmcu=msp430xG4618:%(cpp_msp3) -D__MSP430_G4618__} \
%{mmcu=msp430xG4619:%(cpp_msp3) -D__MSP430_G4619__} \
%{mmcu=msp430x4783:%(cpp_msp4) -D__MSP430_4783__} \
%{mmcu=msp430x4784:%(cpp_msp4) -D__MSP430_4784__} \
%{mmcu=msp430x4793:%(cpp_msp4) -D__MSP430_4793__} \
%{mmcu=msp430x4794:%(cpp_msp4) -D__MSP430_4794__} \
%{mmcu=msp430x47166:%(cpp_msp5) -D__MSP430_47166__} \
%{mmcu=msp430x47176:%(cpp_msp5) -D__MSP430_47176__} \
%{mmcu=msp430x47186:%(cpp_msp5) -D__MSP430_47186__} \
%{mmcu=msp430x47196:%(cpp_msp5) -D__MSP430_47196__} \
%{mmcu=msp430x47167:%(cpp_msp5) -D__MSP430_47167__} \
%{mmcu=msp430x47177:%(cpp_msp5) -D__MSP430_47177__} \
%{mmcu=msp430x47187:%(cpp_msp5) -D__MSP430_47187__} \
%{mmcu=msp430x47197:%(cpp_msp5) -D__MSP430_47197__} \
%{mmcu=msp430x5418:%(cpp_msp6) -D__MSP430_5418__} \
%{mmcu=msp430x5419:%(cpp_msp6) -D__MSP430_5419__} \
%{mmcu=msp430x5435:%(cpp_msp6) -D__MSP430_5435__} \
%{mmcu=msp430x5436:%(cpp_msp6) -D__MSP430_5436__} \
%{mmcu=msp430x5437:%(cpp_msp6) -D__MSP430_5437__} \
%{mmcu=msp430x5438:%(cpp_msp6) -D__MSP430_5438__} \
%{mmcu=msp430x5510:%(cpp_msp6) -D__MSP430_5510__} \
%{mmcu=msp430x5513:%(cpp_msp6) -D__MSP430_5513__} \
%{mmcu=msp430x5514:%(cpp_msp6) -D__MSP430_5514__} \
%{mmcu=msp430x5515:%(cpp_msp6) -D__MSP430_5515__} \
%{mmcu=msp430x5517:%(cpp_msp6) -D__MSP430_5517__} \
%{mmcu=msp430x5519:%(cpp_msp6) -D__MSP430_5519__} \
%{mmcu=msp430x5521:%(cpp_msp6) -D__MSP430_5521__} \
%{mmcu=msp430x5522:%(cpp_msp6) -D__MSP430_5522__} \
%{mmcu=msp430x5524:%(cpp_msp6) -D__MSP430_5524__} \
%{mmcu=msp430x5525:%(cpp_msp6) -D__MSP430_5525__} \
%{mmcu=msp430x5526:%(cpp_msp6) -D__MSP430_5526__} \
%{mmcu=msp430x5527:%(cpp_msp6) -D__MSP430_5527__} \
%{mmcu=msp430x5528:%(cpp_msp6) -D__MSP430_5528__} \
%{mmcu=msp430x5529:%(cpp_msp6) -D__MSP430_5529__} \
%{mmcu=msp430x6638:%(cpp_msp6) -D__MSP430_6638__} \
%{mmcu=cc430x5133:%(cpp_msp6) -D__CC430_5133__} \
%{mmcu=cc430x5125:%(cpp_msp6) -D__CC430_5125__} \
%{mmcu=cc430x6125:%(cpp_msp6) -D__CC430_6125__} \
%{mmcu=cc430x6135:%(cpp_msp6) -D__CC430_6135__} \
%{mmcu=cc430x6126:%(cpp_msp6) -D__CC430_6126__} \
%{mmcu=cc430x5137:%(cpp_msp6) -D__CC430_5137__} \
%{mmcu=cc430x6127:%(cpp_msp6) -D__CC430_6127__} \
%{mmcu=cc430x6137:%(cpp_msp6) -D__CC430_6137__} \
%{posix:-D_POSIX_SOURCE} %{mIAR:-D_IAR_ASSEMBLER_}"

/* A C string constant that tells the GNU CC driver program options to
   pass to CPP.  It can also specify how to translate options you
   give to GNU CC into options for GNU CC to pass to the CPP.

   Do not define this macro if it does not need to do anything.  */

#define CC1_SPEC "%{profile:-p}"

#define CC1PLUS_SPEC "-fno-rtti -fno-exceptions"

/* A C string constant that tells the GNU CC driver program options to
   pass to `cc1'.  It can also specify how to translate options you
   give to GNU CC into options for GNU CC to pass to the `cc1'.

   Do not define this macro if it does not need to do anything.  */

#define ASM_SPEC "%{mmcu=*:-mmcu=%*}"
/* A C string constant that tells the GNU CC driver program options to
   pass to the assembler.  It can also specify how to translate
   options you give to GNU CC into options for GNU CC to pass to the
   assembler.  See the file `sun3.h' for an example of this.

   Do not define this macro if it does not need to do anything.  */

#define ASM_FINAL_SPEC ""
/* A C string constant that tells the GNU CC driver program how to
   run any programs which cleanup after the normal assembler.
   Normally, this is not needed.  See the file `mips.h' for an
   example of this.

   Do not define this macro if it does not need to do anything.  */

#define LINK_SPEC "\
%{!mmcu*:-m msp430x110} \
%{mmcu=msp1:-m msp430x110} \
%{mmcu=msp2:-m msp430x147} \
%{mmcu=msp3:-m msp430xG4616} \
%{mmcu=msp4:-m msp430x4783} \
%{mmcu=msp5:-m msp430x47166} \
%{mmcu=msp6:-m msp430x5418} \
%{mmcu=msp430x110:-m msp430x110 } \
%{mmcu=msp430x112:-m msp430x112 } \
%{mmcu=msp430x1101:-m msp430x1101 } \
%{mmcu=msp430x1111:-m msp430x1111 } \
%{mmcu=msp430x1121:-m msp430x1121 } \
%{mmcu=msp430x1122:-m msp430x1122 } \
%{mmcu=msp430x1132:-m msp430x1132 } \
%{mmcu=msp430x122:-m msp430x122 } \
%{mmcu=msp430x123:-m msp430x123 } \
%{mmcu=msp430x1222:-m msp430x1222 } \
%{mmcu=msp430x1232:-m msp430x1232 } \
%{mmcu=msp430x133:-m msp430x133 } \
%{mmcu=msp430x135:-m msp430x135 } \
%{mmcu=msp430x1331:-m msp430x1331 } \
%{mmcu=msp430x1351:-m msp430x1351 } \
%{mmcu=msp430x147:-m msp430x147 } \
%{mmcu=msp430x148:-m msp430x148 } \
%{mmcu=msp430x149:-m msp430x149 } \
%{mmcu=msp430x1471:-m msp430x1471 } \
%{mmcu=msp430x1481:-m msp430x1481 } \
%{mmcu=msp430x1491:-m msp430x1491 } \
%{mmcu=msp430x155:-m msp430x155 } \
%{mmcu=msp430x156:-m msp430x156 } \
%{mmcu=msp430x157:-m msp430x157 } \
%{mmcu=msp430x167:-m msp430x167 } \
%{mmcu=msp430x168:-m msp430x168 } \
%{mmcu=msp430x169:-m msp430x169 } \
%{mmcu=msp430x1610:-m msp430x1610 } \
%{mmcu=msp430x1611:-m msp430x1611 } \
%{mmcu=msp430x1612:-m msp430x1612 } \
%{mmcu=msp430x2001:-m msp430x2001 } \
%{mmcu=msp430x2011:-m msp430x2011 } \
%{mmcu=msp430x2002:-m msp430x2002 } \
%{mmcu=msp430x2012:-m msp430x2012 } \
%{mmcu=msp430x2003:-m msp430x2003 } \
%{mmcu=msp430x2013:-m msp430x2013 } \
%{mmcu=msp430x2101:-m msp430x2101 } \
%{mmcu=msp430x2111:-m msp430x2111 } \
%{mmcu=msp430x2121:-m msp430x2121 } \
%{mmcu=msp430x2131:-m msp430x2131 } \
%{mmcu=msp430x2112:-m msp430x2112 } \
%{mmcu=msp430x2122:-m msp430x2122 } \
%{mmcu=msp430x2132:-m msp430x2132 } \
%{mmcu=msp430x2201:-m msp430x2201 } \
%{mmcu=msp430x2211:-m msp430x2211 } \
%{mmcu=msp430x2221:-m msp430x2221 } \
%{mmcu=msp430x2231:-m msp430x2231 } \
%{mmcu=msp430x2232:-m msp430x2232 } \
%{mmcu=msp430x2252:-m msp430x2252 } \
%{mmcu=msp430x2272:-m msp430x2272 } \
%{mmcu=msp430x2234:-m msp430x2234 } \
%{mmcu=msp430x2254:-m msp430x2254 } \
%{mmcu=msp430x2274:-m msp430x2274 } \
%{mmcu=msp430x233:-m msp430x233 } \
%{mmcu=msp430x235:-m msp430x235 } \
%{mmcu=msp430x2330:-m msp430x2330 } \
%{mmcu=msp430x2350:-m msp430x2350 } \
%{mmcu=msp430x2370:-m msp430x2370 } \
%{mmcu=msp430x247:-m msp430x247 } \
%{mmcu=msp430x248:-m msp430x248 } \
%{mmcu=msp430x249:-m msp430x249 } \
%{mmcu=msp430x2410:-m msp430x2410 } \
%{mmcu=msp430x2471:-m msp430x2471 } \
%{mmcu=msp430x2481:-m msp430x2481 } \
%{mmcu=msp430x2491:-m msp430x2491 } \
%{mmcu=msp430x2416:-m msp430x2416 } \
%{mmcu=msp430x2417:-m msp430x2417 } \
%{mmcu=msp430x2418:-m msp430x2418 } \
%{mmcu=msp430x2419:-m msp430x2419 } \
%{mmcu=msp430x2616:-m msp430x2616 } \
%{mmcu=msp430x2617:-m msp430x2617 } \
%{mmcu=msp430x2618:-m msp430x2618 } \
%{mmcu=msp430x2619:-m msp430x2619 } \
%{mmcu=msp430x311:-m msp430x311 } \
%{mmcu=msp430x312:-m msp430x312 } \
%{mmcu=msp430x313:-m msp430x313 } \
%{mmcu=msp430x314:-m msp430x314 } \
%{mmcu=msp430x315:-m msp430x315 } \
%{mmcu=msp430x323:-m msp430x323 } \
%{mmcu=msp430x325:-m msp430x325 } \
%{mmcu=msp430x336:-m msp430x336 } \
%{mmcu=msp430x337:-m msp430x337 } \
%{mmcu=msp430x412:-m msp430x412 } \
%{mmcu=msp430x413:-m msp430x413 } \
%{mmcu=msp430x415:-m msp430x415 } \
%{mmcu=msp430x417:-m msp430x417 } \
%{mmcu=msp430x423:-m msp430x423 } \
%{mmcu=msp430x425:-m msp430x425 } \
%{mmcu=msp430x427:-m msp430x427 } \
%{mmcu=msp430x4250:-m msp430x4250 } \
%{mmcu=msp430x4260:-m msp430x4260 } \
%{mmcu=msp430x4270:-m msp430x4270 } \
%{mmcu=msp430xG4250:-m msp430xG4250 } \
%{mmcu=msp430xG4260:-m msp430xG4260 } \
%{mmcu=msp430xG4270:-m msp430xG4270 } \
%{mmcu=msp430xE423:-m msp430xE423 } \
%{mmcu=msp430xE425:-m msp430xE425 } \
%{mmcu=msp430xE427:-m msp430xE427 } \
%{mmcu=msp430xE4232:-m msp430xE4232 } \
%{mmcu=msp430xE4242:-m msp430xE4242 } \
%{mmcu=msp430xE4252:-m msp430xE4252 } \
%{mmcu=msp430xE4272:-m msp430xE4272 } \
%{mmcu=msp430xW423:-m msp430xW423 } \
%{mmcu=msp430xW425:-m msp430xW425 } \
%{mmcu=msp430xW427:-m msp430xW427 } \
%{mmcu=msp430xG437:-m msp430xG437 } \
%{mmcu=msp430xG438:-m msp430xG438 } \
%{mmcu=msp430xG439:-m msp430xG439 } \
%{mmcu=msp430x435:-m msp430x435 } \
%{mmcu=msp430x436:-m msp430x436 } \
%{mmcu=msp430x437:-m msp430x437 } \
%{mmcu=msp430x4351:-m msp430x4351 } \
%{mmcu=msp430x4361:-m msp430x4361 } \
%{mmcu=msp430x4371:-m msp430x4371 } \
%{mmcu=msp430x447:-m msp430x447 } \
%{mmcu=msp430x448:-m msp430x448 } \
%{mmcu=msp430x449:-m msp430x449 } \
%{mmcu=msp430xG4616:-m msp430xG4616 } \
%{mmcu=msp430xG4617:-m msp430xG4617 } \
%{mmcu=msp430xG4618:-m msp430xG4618 } \
%{mmcu=msp430xG4619:-m msp430xG4619 } \
%{mmcu=msp430x4783:-m msp430x4783 } \
%{mmcu=msp430x4784:-m msp430x4784 } \
%{mmcu=msp430x4793:-m msp430x4793 } \
%{mmcu=msp430x4794:-m msp430x4794 } \
%{mmcu=msp430x47166:-m msp430x47166 } \
%{mmcu=msp430x47176:-m msp430x47176 } \
%{mmcu=msp430x47186:-m msp430x47186 } \
%{mmcu=msp430x47196:-m msp430x47196 } \
%{mmcu=msp430x47167:-m msp430x47167 } \
%{mmcu=msp430x47177:-m msp430x47177 } \
%{mmcu=msp430x47187:-m msp430x47187 } \
%{mmcu=msp430x47197:-m msp430x47197 } \
%{mmcu=msp430x5418:-m msp430x5418 } \
%{mmcu=msp430x5419:-m msp430x5419 } \
%{mmcu=msp430x5435:-m msp430x5435 } \
%{mmcu=msp430x5436:-m msp430x5436 } \
%{mmcu=msp430x5437:-m msp430x5437 } \
%{mmcu=msp430x5438:-m msp430x5438 } \
%{mmcu=msp430x5510:-m msp430x5510 } \
%{mmcu=msp430x5513:-m msp430x5513 } \
%{mmcu=msp430x5514:-m msp430x5514 } \
%{mmcu=msp430x5515:-m msp430x5515 } \
%{mmcu=msp430x5517:-m msp430x5517 } \
%{mmcu=msp430x5519:-m msp430x5519 } \
%{mmcu=msp430x5521:-m msp430x5521 } \
%{mmcu=msp430x5522:-m msp430x5522 } \
%{mmcu=msp430x5524:-m msp430x5524 } \
%{mmcu=msp430x5525:-m msp430x5525 } \
%{mmcu=msp430x5526:-m msp430x5526 } \
%{mmcu=msp430x5527:-m msp430x5527 } \
%{mmcu=msp430x5528:-m msp430x5528 } \
%{mmcu=msp430x5529:-m msp430x5529 } \
%{mmcu=msp430x6638:-m msp430x6638 } \
%{mmcu=cc430x5133:-m cc430x5133 } \
%{mmcu=cc430x5125:-m cc430x5125 } \
%{mmcu=cc430x6125:-m cc430x6125 } \
%{mmcu=cc430x6135:-m cc430x6135 } \
%{mmcu=cc430x6126:-m cc430x6126 } \
%{mmcu=cc430x5137:-m cc430x5137 } \
%{mmcu=cc430x6127:-m cc430x6127 } \
%{mmcu=cc430x6137:-m cc430x6137 }"

#define LIB_SPEC "-lc"
#define LIBGCC_SPEC "-lgcc"
#define LIBSTDCXX "-lgcc"

#define STARTFILE_SPEC "%(crt_binutils)"
/* Another C string constant used much like `LINK_SPEC'.  The
   difference between the two is that `STARTFILE_SPEC' is used at the
   very beginning of the command given to the linker.

   If this macro is not defined, a default is provided that loads the
   standard C startup file from the usual place.  See `gcc.c'.  */

#define ENDFILE_SPEC ""
/* Another C string constant used much like `LINK_SPEC'.  The
   difference between the two is that `ENDFILE_SPEC' is used at the
   very end of the command given to the linker.

   Do not define this macro if it does not need to do anything.  */

#define CRT_BINUTILS_SPECS "\
%{!mmcu=*|mmcu=msp430x110|mmcu=msp1:crt430x110.o%s} \
%{mmcu=msp430x112:crt430x112.o%s} \
%{mmcu=msp430x1101:crt430x1101.o%s} \
%{mmcu=msp430x1111:crt430x1111.o%s} \
%{mmcu=msp430x1121:crt430x1121.o%s} \
%{mmcu=msp430x1122:crt430x1122.o%s} \
%{mmcu=msp430x1132:crt430x1132.o%s} \
%{mmcu=msp430x122:crt430x122.o%s} \
%{mmcu=msp430x123:crt430x123.o%s} \
%{mmcu=msp430x1222:crt430x1222.o%s} \
%{mmcu=msp430x1232:crt430x1232.o%s} \
%{mmcu=msp430x133:crt430x133.o%s} \
%{mmcu=msp430x135:crt430x135.o%s} \
%{mmcu=msp430x1331:crt430x1331.o%s} \
%{mmcu=msp430x1351:crt430x1351.o%s} \
%{mmcu=msp430x147:crt430x147.o%s} \
%{mmcu=msp430x148:crt430x148.o%s} \
%{mmcu=msp430x149:crt430x149.o%s} \
%{mmcu=msp430x1471:crt430x1471.o%s} \
%{mmcu=msp430x1481:crt430x1481.o%s} \
%{mmcu=msp430x1491:crt430x1491.o%s} \
%{mmcu=msp430x155:crt430x155.o%s} \
%{mmcu=msp430x156:crt430x156.o%s} \
%{mmcu=msp430x157:crt430x157.o%s} \
%{mmcu=msp430x167:crt430x167.o%s} \
%{mmcu=msp430x168:crt430x168.o%s} \
%{mmcu=msp430x169:crt430x169.o%s} \
%{mmcu=msp430x1610:crt430x1610.o%s} \
%{mmcu=msp430x1611:crt430x1611.o%s} \
%{mmcu=msp430x1612:crt430x1612.o%s} \
%{mmcu=msp430x2001:crt430x2001.o%s} \
%{mmcu=msp430x2011:crt430x2011.o%s} \
%{mmcu=msp430x2002:crt430x2002.o%s} \
%{mmcu=msp430x2012:crt430x2012.o%s} \
%{mmcu=msp430x2003:crt430x2003.o%s} \
%{mmcu=msp430x2013:crt430x2013.o%s} \
%{mmcu=msp430x2101:crt430x2101.o%s} \
%{mmcu=msp430x2111:crt430x2111.o%s} \
%{mmcu=msp430x2121:crt430x2121.o%s} \
%{mmcu=msp430x2131:crt430x2131.o%s} \
%{mmcu=msp430x2112:crt430x2112.o%s} \
%{mmcu=msp430x2122:crt430x2122.o%s} \
%{mmcu=msp430x2132:crt430x2132.o%s} \
%{mmcu=msp430x2201:crt430x2201.o%s} \
%{mmcu=msp430x2211:crt430x2211.o%s} \
%{mmcu=msp430x2221:crt430x2221.o%s} \
%{mmcu=msp430x2231:crt430x2231.o%s} \
%{mmcu=msp430x2232:crt430x2232.o%s} \
%{mmcu=msp430x2252:crt430x2252.o%s} \
%{mmcu=msp430x2272:crt430x2272.o%s} \
%{mmcu=msp430x2234:crt430x2234.o%s} \
%{mmcu=msp430x2254:crt430x2254.o%s} \
%{mmcu=msp430x2274:crt430x2274.o%s} \
%{mmcu=msp430x233:crt430x233.o%s} \
%{mmcu=msp430x235:crt430x235.o%s} \
%{mmcu=msp430x2330:crt430x2330.o%s} \
%{mmcu=msp430x2350:crt430x2350.o%s} \
%{mmcu=msp430x2370:crt430x2370.o%s} \
%{mmcu=msp430x247:crt430x247.o%s} \
%{mmcu=msp430x248:crt430x248.o%s} \
%{mmcu=msp430x249:crt430x249.o%s} \
%{mmcu=msp430x2410:crt430x2410.o%s} \
%{mmcu=msp430x2471:crt430x2471.o%s} \
%{mmcu=msp430x2481:crt430x2481.o%s} \
%{mmcu=msp430x2491:crt430x2491.o%s} \
%{mmcu=msp430x2416:crt430x2416.o%s} \
%{mmcu=msp430x2417:crt430x2417.o%s} \
%{mmcu=msp430x2418:crt430x2418.o%s} \
%{mmcu=msp430x2419:crt430x2419.o%s} \
%{mmcu=msp430x2616:crt430x2616.o%s} \
%{mmcu=msp430x2617:crt430x2617.o%s} \
%{mmcu=msp430x2618:crt430x2618.o%s} \
%{mmcu=msp430x2619:crt430x2619.o%s} \
%{mmcu=msp430x311:crt430x311.o%s} \
%{mmcu=msp430x312:crt430x312.o%s} \
%{mmcu=msp430x313:crt430x313.o%s} \
%{mmcu=msp430x314:crt430x314.o%s} \
%{mmcu=msp430x315:crt430x315.o%s} \
%{mmcu=msp430x323:crt430x323.o%s} \
%{mmcu=msp430x325:crt430x325.o%s} \
%{mmcu=msp430x336|mmcu=msp2:crt430x336.o%s} \
%{mmcu=msp430x337:crt430x337.o%s} \
%{mmcu=msp430x412:crt430x412.o%s} \
%{mmcu=msp430x413:crt430x413.o%s} \
%{mmcu=msp430x415:crt430x415.o%s} \
%{mmcu=msp430x417:crt430x417.o%s} \
%{mmcu=msp430x423:crt430x423.o%s} \
%{mmcu=msp430x425:crt430x425.o%s} \
%{mmcu=msp430x427:crt430x427.o%s} \
%{mmcu=msp430x4250:crt430x4250.o%s} \
%{mmcu=msp430x4260:crt430x4260.o%s} \
%{mmcu=msp430x4270:crt430x4270.o%s} \
%{mmcu=msp430xG4250:crt430xG4250.o%s} \
%{mmcu=msp430xG4260:crt430xG4260.o%s} \
%{mmcu=msp430xG4270:crt430xG4270.o%s} \
%{mmcu=msp430xE423:crt430xE423.o%s} \
%{mmcu=msp430xE425:crt430xE425.o%s} \
%{mmcu=msp430xE427:crt430xE427.o%s} \
%{mmcu=msp430xE4232:crt430xE4232.o%s} \
%{mmcu=msp430xE4242:crt430xE4242.o%s} \
%{mmcu=msp430xE4252:crt430xE4252.o%s} \
%{mmcu=msp430xE4272:crt430xE4272.o%s} \
%{mmcu=msp430xW423:crt430xW423.o%s} \
%{mmcu=msp430xW425:crt430xW425.o%s} \
%{mmcu=msp430xW427:crt430xW427.o%s} \
%{mmcu=msp430xG437:crt430xG437.o%s} \
%{mmcu=msp430xG438:crt430xG438.o%s} \
%{mmcu=msp430xG439:crt430xG439.o%s} \
%{mmcu=msp430x435:crt430x435.o%s} \
%{mmcu=msp430x436:crt430x436.o%s} \
%{mmcu=msp430x437:crt430x437.o%s} \
%{mmcu=msp430x4351:crt430x4351.o%s} \
%{mmcu=msp430x4361:crt430x4361.o%s} \
%{mmcu=msp430x4371:crt430x4371.o%s} \
%{mmcu=msp430x447:crt430x447.o%s} \
%{mmcu=msp430x448:crt430x448.o%s} \
%{mmcu=msp430x449:crt430x449.o%s} \
%{mmcu=msp430xG4616|mmcu=msp3:crt430xG4616.o%s} \
%{mmcu=msp430xG4617:crt430xG4617.o%s} \
%{mmcu=msp430xG4618:crt430xG4618.o%s} \
%{mmcu=msp430xG4619:crt430xG4619.o%s} \
%{mmcu=msp430x4783|mmcu=msp4:crt430x4783.o%s} \
%{mmcu=msp430x4784:crt430x4784.o%s} \
%{mmcu=msp430x4793:crt430x4793.o%s} \
%{mmcu=msp430x4794:crt430x4794.o%s} \
%{mmcu=msp430x47166|mmcu=msp5:crt430x47166.o%s} \
%{mmcu=msp430x47176:crt430x47176.o%s} \
%{mmcu=msp430x47186:crt430x47186.o%s} \
%{mmcu=msp430x47196:crt430x47196.o%s} \
%{mmcu=msp430x47167:crt430x47167.o%s} \
%{mmcu=msp430x47177:crt430x47177.o%s} \
%{mmcu=msp430x47187:crt430x47187.o%s} \
%{mmcu=msp430x47197:crt430x47197.o%s} \
%{mmcu=msp430x5418|mmcu=msp6:crt430x5418.o%s} \
%{mmcu=msp430x5419:crt430x5419.o%s} \
%{mmcu=msp430x5435:crt430x5435.o%s} \
%{mmcu=msp430x5436:crt430x5436.o%s} \
%{mmcu=msp430x5437:crt430x5437.o%s} \
%{mmcu=msp430x5438:crt430x5438.o%s} \
%{mmcu=msp430x5510:crt430x5510.o%s} \
%{mmcu=msp430x5513:crt430x5513.o%s} \
%{mmcu=msp430x5514:crt430x5514.o%s} \
%{mmcu=msp430x5515:crt430x5515.o%s} \
%{mmcu=msp430x5517:crt430x5517.o%s} \
%{mmcu=msp430x5519:crt430x5519.o%s} \
%{mmcu=msp430x5521:crt430x5521.o%s} \
%{mmcu=msp430x5522:crt430x5522.o%s} \
%{mmcu=msp430x5524:crt430x5524.o%s} \
%{mmcu=msp430x5525:crt430x5525.o%s} \
%{mmcu=msp430x5526:crt430x5526.o%s} \
%{mmcu=msp430x5527:crt430x5527.o%s} \
%{mmcu=msp430x5528:crt430x5528.o%s} \
%{mmcu=msp430x5529:crt430x5529.o%s} \
%{mmcu=msp430x6638:crt430x6638.o%s} \
%{mmcu=cc430x5133:crtcc430x5133.o%s} \
%{mmcu=cc430x5125:crtcc430x5125.o%s} \
%{mmcu=cc430x6125:crtcc430x6125.o%s} \
%{mmcu=cc430x6135:crtcc430x6135.o%s} \
%{mmcu=cc430x6126:crtcc430x6126.o%s} \
%{mmcu=cc430x5137:crtcc430x5137.o%s} \
%{mmcu=cc430x6127:crtcc430x6127.o%s} \
%{mmcu=cc430x6137:crtcc430x6137.o%s}"

#define CPP_MSP1_SPEC " -DMSP430_NO_HW_MUL "
#define CPP_MSP2_SPEC " -DMSP430_HAS_HW_MUL -DMSP430_HAS_HWMUL "
#define CPP_MSP3_SPEC " -DMSP430_HAS_HW_MUL -DMSP430_HAS_HWMUL -D__MSP430X__"
#define CPP_MSP4_SPEC " -DMSP430_HAS_HW_MUL -DMSP430_HAS_HWMUL"
#define CPP_MSP5_SPEC " -DMSP430_HAS_HW_MUL -DMSP430_HAS_HWMUL -D__MSP430X__"
#define CPP_MSP6_SPEC " -DMSP430_HAS_HW_MUL -DMSP430_HAS_HWMUL -D__MSP430X2__"

#define EXTRA_SPECS                           	\
{"cpp_msp1",CPP_MSP1_SPEC},			\
{"cpp_msp2",CPP_MSP2_SPEC},			\
{"cpp_msp3",CPP_MSP3_SPEC},			\
{"cpp_msp4",CPP_MSP4_SPEC},			\
{"cpp_msp5",CPP_MSP5_SPEC},			\
{"cpp_msp6",CPP_MSP6_SPEC},			\
{"crt_binutils", CRT_BINUTILS_SPECS},

/* Define this macro to provide additional specifications to put in
   the `specs' file that can be used in various specifications like
   `CC1_SPEC'.

   The definition should be an initializer for an array of structures,
   containing a string constant, that defines the specification name,
   and a string constant that provides the specification.

   Do not define this macro if it does not need to do anything.

   `EXTRA_SPECS' is useful when an architecture contains several
   related targets, which have various `..._SPECS' which are similar
   to each other, and the maintainer would like one central place to
   keep these definitions.

   For example, the PowerPC System V.4 targets use `EXTRA_SPECS' to
   define either `_CALL_SYSV' when the System V calling sequence is
   used or `_CALL_AIX' when the older AIX-based calling sequence is
   used.

   The `config/rs6000/rs6000.h' target file defines:

   #define EXTRA_SPECS \
   { "cpp_sysv_default", CPP_SYSV_DEFAULT },

   #define CPP_SYS_DEFAULT ""

   The `config/rs6000/sysv.h' target file defines:
   #undef CPP_SPEC
   #define CPP_SPEC \
   "%{posix: -D_POSIX_SOURCE } \
   %{mcall-sysv: -D_CALL_SYSV } %{mcall-aix: -D_CALL_AIX } \
   %{!mcall-sysv: %{!mcall-aix: %(cpp_sysv_default) }} \
   %{msoft-float: -D_SOFT_FLOAT} %{mcpu=403: -D_SOFT_FLOAT}"

   #undef CPP_SYSV_DEFAULT
   #define CPP_SYSV_DEFAULT "-D_CALL_SYSV"

   while the `config/rs6000/eabiaix.h' target file defines
   `CPP_SYSV_DEFAULT' as:

   #undef CPP_SYSV_DEFAULT
   #define CPP_SYSV_DEFAULT "-D_CALL_AIX"  */


#define MULTILIB_DEFAULTS { "mmcu=msp430x110" }

#define TEST_HARD_REG_CLASS(CLASS, REGNO) \
  TEST_HARD_REG_BIT (reg_class_contents[ (int) (CLASS)], REGNO)

/* Note that the other files fail to use these
   in some of the places where they should.  */

#if defined(__STDC__) || defined(ALMOST_STDC)
#define AS2(a,b,c) #a " " #b "," #c
#define AS2C(b,c) " " #b "," #c
#define AS3(a,b,c,d) #a " " #b "," #c "," #d
#define AS1(a,b) #a " " #b
#else
#define AS1(a,b) "a	b"
#define AS2(a,b,c) "a	b,c"
#define AS2C(b,c) " b,c"
#define AS3(a,b,c,d) "a	b,c,d"
#endif
#define OUT_AS1(a,b) output_asm_insn (AS1(a,b), operands)
#define OUT_AS2(a,b,c) output_asm_insn (AS2(a,b,c), operands)
#define CR_TAB "\n\t"

#define TARGET_FLOAT_FORMAT IEEE_FLOAT_FORMAT

#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG
#define DWARF2_DEBUGGING_INFO 1
#define OBJECT_FORMAT_ELF

#define DBX_REGISTER_NUMBER(r) (r)

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

struct machine_function GTY(())
{
  /* 'true' - if the current function is a leaf function.  */
  int is_leaf;

  /* 'true' - if current function is a naked function.  */
  int is_naked;

  /* 'true' - if current function is an interrupt function 
     as specified by the "interrupt" attribute.  */
  int is_interrupt;

  /* 'true' - if current function is a 'task' function 
     as specified by the "OS_task" attribute.  */
  int is_OS_task;
 
  int is_noint_hwmul;
  
  int is_critical;
  
  int is_reenterant;
  
  int is_wakeup;
  
  int is_signal;
};

#ifdef RTX_CODE
extern int default_rtx_costs (rtx X ATTRIBUTE_UNUSED, enum rtx_code code, enum rtx_code outer_code ATTRIBUTE_UNUSED);
#endif
