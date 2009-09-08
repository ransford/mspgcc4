;; Predicate definitions for MSP430
;; Copyright (C) 2006 Free Software Foundation, Inc.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published
;; by the Free Software Foundation; either version 2, or (at your
;; option) any later version.

;; GCC is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING.  If not, write to
;; the Free Software Foundation, 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;; <==== Comment by Ivan Shcherbakov ====>
;; All predicates here were checked againist latest port of GCC 3.2.3
(define_special_predicate "equality_operator"
  (match_code "eq,ne")
{
  return ((mode == VOIDmode || GET_MODE (op) == mode) && (GET_CODE (op) == EQ || GET_CODE (op) == NE));
})

(define_special_predicate "inequality_operator"
  (match_code "ge,gt,le,lt,geu,gtu,leu,ltu")
{
  return comparison_operator (op, mode) && !equality_operator (op, mode);
})

(define_predicate "nonimmediate_operand_msp430"
  (match_code "reg,subreg,mem")
{
  int save_volatile_ok = volatile_ok;
  int niop = 0;

  if (!TARGET_NVWA)
    volatile_ok = 1;
  niop = nonimmediate_operand (op, mode);
  volatile_ok = save_volatile_ok;

  return niop;
})

(define_predicate "memory_operand_msp430"
  (match_code "subreg,mem")
{
  int save_volatile_ok = volatile_ok;
  int mop = 0;

  if (!TARGET_NVWA)
    volatile_ok = 1;
  mop = memory_operand (op, mode);
  volatile_ok = save_volatile_ok;
  return mop;
})

(define_predicate "general_operand_msp430"
  (match_code "const_int,const_double,const,symbol_ref,label_ref,subreg,reg,mem")
{
  int save_volatile_ok = volatile_ok;
  int gop = 0;

  if (!TARGET_NVWA)
    volatile_ok = 1;
  gop = general_operand (op, mode);
  volatile_ok = save_volatile_ok;
  return gop;
})

(define_predicate "three_operands_msp430"
  (match_code "plus,minus,and,ior,xor")
{
  enum rtx_code code = GET_CODE (op);
  if (GET_MODE (op) != mode)
    return 0;

  return (code == PLUS || code == MINUS || code == AND || code == IOR || code == XOR);
})

