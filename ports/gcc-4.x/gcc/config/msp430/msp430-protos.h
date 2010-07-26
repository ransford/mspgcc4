/* Prototypes for exported functions defined in msp430.c
   
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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


extern void   bootloader_section PARAMS ((void));
extern void   infomem_section PARAMS ((void));

extern void   asm_file_start            PARAMS ((FILE *file));
extern void   asm_file_end              PARAMS ((FILE *file));
extern void   msp430_init_once          PARAMS ((void));
extern void   msp430_override_options   PARAMS ((void));
/*extern void   function_prologue         PARAMS ((FILE *file, int size));
extern void   function_epilogue         PARAMS ((FILE *file, int size));*/
extern void   gas_output_limited_string PARAMS ((FILE *file, const char *str));
extern void   gas_output_ascii          PARAMS ((FILE *file, const char *str,
							 size_t length));
extern void   order_regs_for_local_alloc PARAMS ((void));
extern void msp430_trampoline_template PARAMS ((FILE *fd));


extern int frame_pointer_required_p PARAMS ((void));
extern int msp430_empty_epilogue PARAMS ((void));

int msp430_regno_ok_for_base_p PARAMS ((int));


#ifdef HAVE_MACHINE_MODES
extern int    msp430_hard_regno_mode_ok PARAMS ((int regno,
					     enum machine_mode mode));
#endif

extern int initial_elimination_offset PARAMS ((int, int));



#ifdef TREE_CODE
extern void   asm_output_external          PARAMS ((FILE *file, tree decl,
						   char *name));
extern void   unique_section               PARAMS ((tree decl, int reloc));
extern void   encode_section_info          PARAMS ((tree decl));
extern void   asm_output_section_name      PARAMS ((FILE *file, tree decl,
						   const char *name,
						   int reloc));
extern int    valid_machine_type_attribute PARAMS ((tree type, tree attributes,
						   tree identifier,
						   tree args));
extern int    valid_machine_decl_attribute PARAMS ((tree decl, tree attributes,
						   tree attr, tree args));
extern void asm_declare_function_name PARAMS ((FILE *, const char *, tree));
unsigned int msp430_section_type_flags PARAMS (( tree DECL, const char *NAME, int RELOC));


#ifdef RTX_CODE /* inside TREE_CODE */
extern rtx    msp430_function_value          PARAMS ((tree type, tree func));
extern void   init_cumulative_args           PARAMS ((CUMULATIVE_ARGS *cum,
						   tree fntype, rtx libname,
						   int indirect));
extern rtx    function_arg         PARAMS ((CUMULATIVE_ARGS *cum,
					   enum machine_mode mode,
					   tree type, int named));
extern void   init_cumulative_incoming_args           PARAMS ((CUMULATIVE_ARGS *cum,
						   tree fntype, rtx libname));
extern rtx    function_incoming_arg         PARAMS ((CUMULATIVE_ARGS *cum,
					   enum machine_mode mode,
					   tree type, int named));



#endif /* RTX_CODE inside TREE_CODE */

#ifdef HAVE_MACHINE_MODES /* inside TREE_CODE */
extern void   function_arg_advance PARAMS ((CUMULATIVE_ARGS *cum,
					   enum machine_mode mode, tree type,
					   int named));
#endif /* HAVE_MACHINE_MODES inside TREE_CODE*/
#endif /* TREE_CODE */

#ifdef RTX_CODE


extern enum rtx_code msp430_canonicalize_comparison PARAMS ((enum rtx_code,rtx *,rtx *));


extern void msp430_emit_cbranch PARAMS ((enum rtx_code, rtx));
extern void msp430_emit_cset PARAMS ((enum rtx_code, rtx));

extern int dead_or_set_in_peep PARAMS ((int, rtx, rtx));
extern void msp430_initialize_trampoline PARAMS ((rtx,rtx,rtx));   


extern enum reg_class msp430_reg_class_from_letter PARAMS ((int));
extern enum reg_class preferred_reload_class PARAMS ((rtx,enum reg_class));
enum reg_class msp430_regno_reg_class PARAMS ((int));

extern RTX_CODE followed_compare_condition PARAMS ((rtx));

extern const char * msp430_movesi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_movedi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_addsi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_subsi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_andsi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_iorsi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_xorsi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_adddi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_subdi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_anddi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_iordi_code PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_xordi_code PARAMS ((rtx insn, rtx operands[], int *l));


extern int zero_shifted PARAMS ((rtx ));
extern int indexed_location PARAMS ((rtx ));


extern int regsi_ok_safe PARAMS ((rtx operands[]));
extern int regsi_ok_clobber PARAMS ((rtx operands[]));
extern int regdi_ok_safe PARAMS ((rtx operands[]));
extern int regdi_ok_clobber PARAMS ((rtx operands[]));
extern int sameoperand PARAMS ((rtx operands[], int));

extern int general_operand_msp430 PARAMS ((rtx, enum machine_mode )); 
extern int nonimmediate_operand_msp430 PARAMS ((rtx, enum machine_mode ));
extern int memory_operand_msp430 PARAMS ((rtx, enum machine_mode ));
extern int halfnibble_constant PARAMS ((rtx, enum machine_mode ));
extern int halfnibble_integer PARAMS ((rtx, enum machine_mode ));
extern int halfnibble_constant_shift PARAMS ((rtx, enum machine_mode ));
extern int halfnibble_integer_shift PARAMS ((rtx, enum machine_mode ));
extern int which_nibble PARAMS ((int));
extern int which_nibble_shift PARAMS ((int));


extern void   asm_output_external_libcall PARAMS ((FILE *file, rtx symref));
extern int    legitimate_address_p    PARAMS ((enum machine_mode mode, rtx x,
					int strict));
extern int    compare_diff_p  PARAMS ((rtx insn));

extern int    emit_indexed_arith PARAMS ((rtx insn, rtx operands[], int, const char *, int));

extern const char * msp430_emit_abssi    PARAMS ((rtx insn, rtx operands[], int *l));
extern const char * msp430_emit_absdi    PARAMS ((rtx insn, rtx operands[], int *l));

extern const char * msp430_emit_indexed_add2 PARAMS ((rtx insn, rtx op[], int *l));
extern const char * msp430_emit_indexed_add4 PARAMS ((rtx insn, rtx op[], int *l));

extern const char * msp430_emit_indexed_sub2 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_sub4 PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * msp430_emit_indexed_and2 PARAMS ((rtx insn, rtx op[], int *l));
extern const char * msp430_emit_indexed_and4 PARAMS ((rtx insn, rtx op[], int *l));
extern const char * msp430_emit_immediate_and2 PARAMS ((rtx insn, rtx op[], int *l));
extern const char * msp430_emit_immediate_and4 PARAMS ((rtx insn, rtx op[], int *l));

extern const char * msp430_emit_indexed_ior2 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_ior4 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_immediate_ior2 PARAMS ((rtx insn, rtx op[], int *l));
extern const char * msp430_emit_immediate_ior4 PARAMS ((rtx insn, rtx op[], int *l));


extern int msp430_emit_indexed_mov PARAMS ((rtx insn, rtx operands[], int len, const char *));       
extern const char * movstrsi_insn PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * clrstrsi_insn PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * movstrhi_insn PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * clrstrhi_insn PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_mov2 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_mov4 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * movsisf_regmode PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * movdidf_regmode PARAMS ((rtx insn, rtx operands[], int *len));


extern int is_shift_better_in_reg PARAMS ((rtx operands[]));
extern int msp430_emit_shift_cnt PARAMS ((int (*funct)(rtx, int, int), const char *, rtx insn, rtx operands[], int *len, int));
extern const char * msp430_emit_ashlqi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashlhi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashlsi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashldi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashrqi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashrhi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashrsi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_ashrdi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_lshrqi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_lshrhi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_lshrsi3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_lshrdi3 PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * signextendqihi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * signextendqisi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * signextendqidi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * signextendhisi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * signextendhidi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * signextendsidi PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * msp430_emit_indexed_sub2 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_sub4 PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * msp430_emit_indexed_xor2 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_xor4 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_xor2_3 PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_indexed_xor4_3 PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * zeroextendqihi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * zeroextendqisi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * zeroextendqidi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * zeroextendhisi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * zeroextendhidi PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * zeroextendsidi PARAMS ((rtx insn, rtx operands[], int *len));

extern const char * msp430_pushsisf PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_pushdi   PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_pushhi   PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_pushqi   PARAMS ((rtx insn, rtx operands[], int *len));
extern const char * msp430_emit_call (rtx operands[]);

extern const char * msp430_emit_return PARAMS ((rtx insn, rtx operands[], int *len));
extern const char *msp430_cbranch PARAMS ((rtx insn, rtx operands[], int *len, int is_cc0_branch));
extern const char *msp430_cset PARAMS ((rtx insn, rtx operands[], int *len));

extern void   notice_update_cc       PARAMS ((rtx body, rtx insn));
extern int    msp430_peep2_scratch_safe PARAMS ((rtx reg_rtx));
extern int    test_hard_reg_class    PARAMS ((enum reg_class class, rtx x));
extern void   machine_dependent_reorg PARAMS ((rtx first_insn));
extern void msp430_output_addr_vec_elt PARAMS ((FILE *stream, int value));
extern void   final_prescan_insn     PARAMS ((rtx insn, rtx *operand,
							int num_operands));
extern int    adjust_insn_length     PARAMS ((rtx insn, int len));


extern int    msp430_address_cost    PARAMS ((rtx x));
extern int    extra_constraint       PARAMS ((rtx x, int c));
extern rtx    legitimize_address     PARAMS ((rtx x, rtx oldx,
					     enum machine_mode mode));
extern rtx    msp430_libcall_value   PARAMS ((enum machine_mode mode));
extern int    default_rtx_costs      PARAMS ((rtx X, RTX_CODE code,
					     RTX_CODE outer_code));
extern void   asm_output_char        PARAMS ((FILE *file, rtx value));
extern void   asm_output_short       PARAMS ((FILE *file, rtx value));
extern void   asm_output_byte        PARAMS ((FILE *file, int value));

extern void   print_operand          PARAMS ((FILE *file, rtx x, int code));
extern void   print_operand_address  PARAMS ((FILE *file, rtx addr));
extern int    reg_unused_after       PARAMS ((rtx insn, rtx reg));
extern int    msp430_jump_dist       PARAMS ((rtx x, rtx insn));
extern int    call_insn_operand      PARAMS ((rtx op, enum machine_mode mode));
extern int    msp430_branch_mode     PARAMS ((rtx x, rtx insn));

extern int 	msp430_easy_mul PARAMS ((rtx [],int));
extern int	msp430_mul3_guard	PARAMS ((rtx [], int ));
extern int      msp430_umul3_guard       PARAMS ((rtx [], int ));
extern int	msp430_mulhisi_guard PARAMS ((rtx [] ));
extern int	msp430_umulhisi_guard	PARAMS ((rtx [] ));
extern int	msp430_ashlhi3 		PARAMS ((rtx [] ));
extern int      msp430_ashlsi3          PARAMS ((rtx [] ));
extern int      msp430_ashrhi3          PARAMS ((rtx [] ));
extern int      msp430_ashrsi3          PARAMS ((rtx [] ));
extern int      msp430_lshrhi3          PARAMS ((rtx [] ));
extern int      msp430_lshrsi3          PARAMS ((rtx [] ));

extern void expand_prologue (void);
extern void expand_epilogue (void);
extern int msp430_epilogue_uses (int regno);

#endif /* RTX_CODE */

#ifdef HAVE_MACHINE_MODES
extern int    class_max_nregs        PARAMS ((enum reg_class class,
					     enum machine_mode mode));
#endif /* HAVE_MACHINE_MODES */

#ifdef REAL_VALUE_TYPE

extern void   asm_output_float       PARAMS ((FILE *file, REAL_VALUE_TYPE n));

#endif


