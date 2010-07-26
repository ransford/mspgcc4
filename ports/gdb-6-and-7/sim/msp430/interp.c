/* Simulator for msp430 TI MCU
   Copyright (C) 2002 Free Software Foundation, Inc.
 
This file is part of GDB, the GNU debugger.
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include "sysdep.h"
#if !defined(__MINGW32__)
#include <sys/times.h>
#endif
#include <sys/param.h>
#if !defined(__MINGW32__)
#include <netinet/in.h>	/* for byte ordering macros */
#endif
#include "bfd.h"
#include "libiberty.h"
#include "gdb/callback.h"
#include "gdb/remote-sim.h"

/* Mingw32 doesn't have definitions for the all the Unix signals, so we need
   a fudge things by defining a few here */
#if defined(__MINGW32__)
#define SIGQUIT     3
#define SIGTRAP     5
#define SIGBUS      7
#endif

#define FIRST_PSEUDO_REG	16
#define MEM_SIZE		65536

struct msp430_alu
  {
    short 		regs[FIRST_PSEUDO_REG];
    short		saved_sr;
    unsigned long	cycles;
    unsigned long	insns;
    unsigned long	interrupts;
    char 		mem[MEM_SIZE];
    char 		prog_name[1024];
    int		flags;
#define CPUOFF		0x10
#define OSCOFF		0x20
#define SCG0		0x40
#define SCG1		0x80
    int		signal;
  };


static struct msp430_alu	alu;


#define		pc	(alu.regs[0])
#define		sp	(alu.regs[1])
#define		sr	(alu.regs[2])
#define		cg1	(alu.regs[2])
#define		cg2	(alu.regs[3])


#define SET_C	sr |= 0x0001
#define SET_Z	sr |= 0x0002
#define SET_N   sr |= 0x0004
#define SET_V   sr |= 0x0100

#define CLR_C   sr &= ~0x0001
#define CLR_Z   sr &= ~0x0002
#define CLR_N   sr &= ~0x0004
#define CLR_V   sr &= ~0x0100

#define SR_MASK	(0x0100|0x0004|0x0002|0x0001)


#define	C	((unsigned long)(sr & 0x0001)>>0)
#define Z       ((unsigned long)(sr & 0x0002)>>1)
#define N       ((unsigned long)(sr & 0x0004)>>2)
#define V       ((unsigned long)(sr & 0x0100)>>8)



enum msp430_modes {
  REGISTER,
  INDEXED, 	/* x(Rn) */
  SYMBOLIC,	/* addr: (pc+x) (mind pc) */
  ABSOLUTE,	/* &addr (mind r2) */
  INDERECT,	/* @Rn */
  INDERECTINC,	/* @Rn+ */
  IMMEDIATE	/* #XXXX (@pc+) */
};

struct msp430_op_mode
  {
    int rs;
    int rd;
    enum msp430_modes ms;
    enum msp430_modes md;
    char as;
    char ad;
    char byte;
  };


struct msp430_ops
  {
    int d_reg;
    short src;
    short dst;
    short insn;
    short dval;
    char byte;
  };


/******************************************************************/
/*********** HARDWARE MULTIPLIER **********************************/

short msp430_fetch_integer(unsigned int loc);


#define __MPY 		0x130
#define __MPYS		0x132
#define __MAC		0x134 
#define __MACS		0x136
#define __OP2		0x138 
#define __RESLO		0x13a
#define __RESHI		0x13c
#define __SUMEXT	0x13e

static struct __mpl {
	int op1;
	int op2;
	int mode;
	union {
		unsigned int u;
		int s;
	      } acc;
	int res;
	int len;
	int sumext;
} mpl;

void 
msp430_multiplier(unsigned int loc, int len)
{
    int sres;
    unsigned int ures;
    unsigned long long ull;
    long long sll;
    
    if (loc == __OP2)	/* perform an action */
    {
    	mpl.op2 = (int) msp430_fetch_integer(loc&0xffff);
    	if (mpl.mode == __MPYS || mpl.mode == __MACS)
    	{
    	    if (len == 1 && (mpl.op2&0x80) )
    	    	mpl.op2 = -((~mpl.op2 + 1) & 0xff);
    	    else if (len == 2 && (mpl.op2&0x8000) )
    	    	mpl.op2 = -((~mpl.op2 + 1) & 0xffff);
    	}
    	else
    	{
    	    if (len == 1) 
    	    	mpl.op2 &= 0xff;
    	    else if (len==2) 
    	    	mpl.op2 &= 0xffff;
    	}
    	
    	switch(mpl.mode)
    	{
    	    case __MPY:
    	    	mpl.acc.u = (unsigned)mpl.op2 * (unsigned)mpl.op1;
    	    	mpl.sumext = 0;
    	    	break;

    	    case __MPYS:
    	    	mpl.acc.s = mpl.op2 * mpl.op1;
    	    	if ((mpl.op1 < 0 && mpl.op2 >= 0) 
    	    	   || (mpl.op1 >= 0 && mpl.op2 < 0))
    	    	   	mpl.sumext = 0xffff;
    	    	else
    	    		mpl.sumext = 0;
    	    	break;

    	    case __MAC:

    	    	ures = (unsigned)mpl.op2 * (unsigned)mpl.op1;
    	    	ull = (unsigned long long) mpl.acc.u + ures;
    	    	mpl.acc.u = ull&0xfffffffful;
    	    	
    	    	if (ull > 0x00000000ffffffffull)
    	    		mpl.sumext = 1;
    		else
    			mpl.sumext = 0;
    		
    		break;
    	    
    	    case __MACS:
    	    	sres = mpl.op2 * mpl.op1;
    	    	sll = (long long) mpl.acc.s + sres;
    	        mpl.acc.s = sll&0xfffffffful;
    		
    		if (sll > 0x000000007fffffffll)
    			mpl.sumext = 0xffff;
    		else
    			mpl.sumext = 0;
    		
    		break;
    	    
    	    default:
    	    	break;	
    	}
    	
    	/**/
    	
    	
    }
    else if (loc == __RESLO)
    {
    	mpl.acc.u = (mpl.acc.u & 0xffff0000) | 
    		(0xfffful&((int)msp430_fetch_integer(loc&0xffff)));
    }
    else if (loc == __RESHI)
    {
    	mpl.acc.u = (mpl.acc.u & 0xffff) |
    		(((int)msp430_fetch_integer(loc&0xffff)) << 16);
    }
    else if (loc == __SUMEXT)
    {
    	return;	/* read only location */
    }
    else
    {
    	mpl.mode = loc;
    	mpl.op1 = (int) msp430_fetch_integer(loc&0xffff);

    	if (mpl.mode == __MPYS || mpl.mode == __MACS)
    	{
    	    if (len == 1 && (mpl.op1 & 0x80) )
    	    	mpl.op1 = -((~mpl.op1 + 1) & 0xff);
    	    else if (len == 2 && (mpl.op1 & 0x8000) )
    	    	mpl.op1 = -((~mpl.op1 + 1) & 0xffff);
    	}
    	else
    	{
    	    if (len == 1) 
    	    	mpl.op1 &= 0xff;
    	    else if (len==2) 
    	    	mpl.op1 &= 0xffff;
    	}
    	
    	/* return, cause this will write initial locs only. */
    	return;
    }

    /* finaly stuff the results address */
    
    alu.mem[__SUMEXT] = mpl.sumext&0xff;
    alu.mem[__SUMEXT+1] = (mpl.sumext>>8)&0xff;
    
    alu.mem[__RESLO] = mpl.acc.u&0xff;
    alu.mem[__RESLO+1] = (mpl.acc.u>>8)&0xff;
    alu.mem[__RESHI] = (mpl.acc.u>>16)&0xff;
    alu.mem[__RESHI+1] = (mpl.acc.u>>24)&0xff;
    
}



/******************************************************************/


void
msp430_set_pc(unsigned int x)
{
  pc = x;
}

unsigned int
msp430_get_pc()
{
  return (unsigned int)pc;
}


struct msp430_op_mode
msp430_get_op_mode_single(short insn)
  {
    struct msp430_op_mode m;
    int s_reg;
    int d_reg;

    alu.saved_sr = sr;


    m.as = (insn>>4) & 3;
    m.byte = (insn>>6) & 1;
    m.rd = m.rs = d_reg = s_reg = insn & 0xf;

    switch(m.as)
      {
      case 0:
        /* cg1 - reg.mode */
        cg2 = 0;
        m.ms = REGISTER;
        break;

      case 1:
        /* cg1 - abs.addr.mode */

        if (s_reg == 2)
          m.ms = ABSOLUTE;	/* &addr */
        else if (s_reg == 0)
          m.ms = SYMBOLIC;	/* addr (pc + x) */
        else if (s_reg == 3)
          m.ms = REGISTER;
        else
          m.ms = INDEXED;		/* X(Rn) */

        cg2 = 1;
        break;

      case 2:
        m.ms = INDERECT;
        if (s_reg == 2 || s_reg == 3) m.ms = REGISTER;
        cg1 = 4;
        cg2 = 2;
        break;

      case 3:
        if (s_reg == 0)
          m.ms = IMMEDIATE;
        else
          m.ms = INDERECTINC;
        if (s_reg == 2 || s_reg == 3) m.ms = REGISTER;
        cg1 = 8;
        cg2 = -1;
        break;
      }

    return m;
  }


struct msp430_op_mode
msp430_get_op_mode_double(short insn)
  {
    struct msp430_op_mode m;
    int s_reg;
    int d_reg;

    alu.saved_sr = sr;


    m.as = (insn>>4) & 3;
    m.ad = (insn>>7) & 1;
    m.byte = (insn>>6) & 1;
    s_reg = (insn>>8) & 0xf;
    d_reg = insn & 0xf;

    m.rs = s_reg;
    m.rd = d_reg;

    if (m.ad == 0)
      m.md = REGISTER;
    else
      {
        if (d_reg == 2)
          m.md = ABSOLUTE;	/* &addr */
        else if (d_reg == 0)
          m.md = SYMBOLIC;	/* addr (pc + x) */
        else
          m.md = INDEXED;		/* X(Rn) */
      }

    switch(m.as)
      {
      case 0:
        /* cg1 - reg.mode */
        cg2 = 0;
        m.ms = REGISTER;
        break;

      case 1:
        /* cg1 - abs.addr.mode */

        if (s_reg == 2)
          m.ms = ABSOLUTE;	/* &addr */
        else if (s_reg == 0)
          m.ms = SYMBOLIC;	/* addr (pc + x) */
        else if (s_reg == 3)
          m.ms = REGISTER;
        else
          m.ms = INDEXED;		/* X(Rn) */

        cg2 = 1;
        break;

      case 2:
        m.ms = INDERECT;
        if (s_reg == 2 || s_reg == 3) m.ms = REGISTER;
        cg1 = 4;
        cg2 = 2;
        break;

      case 3:
        if (s_reg == 0)
          m.ms = IMMEDIATE;
        else
          m.ms = INDERECTINC;
        if (s_reg == 2 || s_reg == 3) m.ms = REGISTER;
        cg1 = 8;
        cg2 = -1;
        break;
      }

    return m;
  }

static char wpr[0xffff];       /* catch read */
static char wpw[0xffff];       /* catch write */


short
msp430_fetch_integer(unsigned int loc)
{
  char *m = &alu.mem[0xffff&loc];
  short res;

  if (wpr[0xffff&loc]) alu.signal = SIGTRAP;

  res = (0xff&(*m)) | (short)(*(m+1))<<8;
  return res;
}


void
msp430_write_integer(int loc, int val, int len)
{
  int sloc = loc;
  int slen = len;
  
  if (!len ) return;

  if (loc&1 && len != 1)
    {
      alu.signal = SIGBUS;
      return;
    }

  while(len--)
    {
      alu.mem[0xffff&loc] = 0xff & val;
      if (wpw[0xffff&loc]) alu.signal = SIGTRAP;
      loc++;
      val >>= 8;
    }
  
  if (sloc >=0x0130 && sloc <= 0x013f) msp430_multiplier(sloc, slen);
}

int msp430_lookup_symbol(char *symname)
{
  int symaddr = 0;
  fprintf(stderr,"Sorry, address only at the moment\n");
  return (symaddr);
}



void
msp430_set_watchpoint(char *mode, char *name)
{
  int addr = 0;
  char *endptr;

  if (strcmp(name,"all") )addr = -1;
  /* is it an address ? */
  addr = strtol(name, &endptr, 0);

  /* nope - name */
  if (*endptr!='\n' && *endptr!='\t' && *endptr!=' ' && *endptr!=0)
    {
      addr = msp430_lookup_symbol(name);
    }

  if (!addr || (addr < 0 && addr != -1) || addr > 0xffff)
    {
      fprintf(stderr,"Error: canont find '%s' symbol\n", name);
      return;
    }

  switch(*mode)
    {
    case 'r':
      wpr[addr] = 1;
      break;
    case 'w':
      wpw[addr] = 1;
      break;
    case 'a':
      wpw[addr] = 1;
      wpr[addr] = 1;
      break;
    case 'd':
      wpw[addr] = 0;
      wpr[addr] = 0;
      break;
    case 'c':
      if (addr == -1)
      {
      	memset(wpw, 0, sizeof(wpw));
      	memset(wpr, 0, sizeof(wpr));
      }
      
    default:
      fprintf(stderr,"Error: Invalid mode: %s\n", mode);
      return;
    }

  if (*mode != 'd')
    {
      fprintf(stderr,"Add symbol %s at 0x%04x to watchpoins list\n", name, addr);
      return;
    }
  else
    {
      fprintf(stderr,"Remove watchpoint at 0x%04x\n", addr);
    }

  return;
}


struct msp430_ops
msp430_double_operands( short insn )
  {
    short src_val = 0;
    struct msp430_op_mode m = msp430_get_op_mode_double(insn);
    unsigned int tmp;
    int len = 0;
    int cycles = 0;
    struct msp430_ops ret;

    ret.byte = m.byte;
    ret.d_reg = -1;

    switch (m.ms)
      {
      case REGISTER:
        src_val = alu.regs[m.rs];
        cycles += 1;
        break;
      case INDEXED:
        len += 1;
        cycles += 3;
        pc += 2;
        tmp = msp430_fetch_integer(pc) + alu.regs[m.rs];
        src_val = msp430_fetch_integer(tmp);
        break;
      case SYMBOLIC:
        len += 1;
        pc += 2;
        cycles += 3;
        tmp = msp430_fetch_integer(pc) + pc;
        src_val = msp430_fetch_integer(tmp);
        break;
      case ABSOLUTE:
        len += 1;
        pc += 2;
        cycles += 3;
        tmp = msp430_fetch_integer(pc);
        src_val = msp430_fetch_integer(tmp);
        break;
      case INDERECT:
        cycles += 2;
        ret.dst = alu.regs[m.rs];
        src_val = msp430_fetch_integer(alu.regs[m.rs]);
        break;
      case INDERECTINC:
        cycles += 2;
        ret.dst = alu.regs[m.rs];
        src_val = msp430_fetch_integer(alu.regs[m.rs]);
        if (m.rs != 1 && m.rs != 0)
          alu.regs[m.rs] += m.byte ? 1 : 2;
        else
          alu.regs[m.rs] += 2;
        break;
      case IMMEDIATE:
        cycles += 2;
        pc += 2;
        src_val = msp430_fetch_integer(pc);
        break;
      }

    ret.src = src_val;

    switch(m.md)
      {
      case REGISTER:
        cycles += (m.ms==REGISTER 
                   || m.ms == INDERECT 
                   || m.ms == INDERECTINC 
                   || m.ms == IMMEDIATE) ? ((m.rd) ? 0 : 1) : 0;
        ret.dval = alu.regs[m.rd];
        ret.d_reg = m.rd;
        break;

      case ABSOLUTE:
        len += 1;
        pc += 2;
        cycles += 3;
        ret.dst = tmp = msp430_fetch_integer(pc);
        ret.dval = msp430_fetch_integer(tmp);
        break;

      case SYMBOLIC:
        len += 1;
        pc += 2;
        cycles += 3;
        ret.dst = tmp = msp430_fetch_integer(pc) + pc;
        ret.dval = msp430_fetch_integer(tmp);
        break;

      case INDEXED:
        len += 1;
        pc += 2;
        cycles += 3;
        ret.dst = tmp = msp430_fetch_integer(pc) + alu.regs[m.rd];
        ret.dval = msp430_fetch_integer(tmp);
        break;
      default:
        alu.signal = SIGILL;
      }

    pc += 2;
    sr = alu.saved_sr;
    
    alu.cycles += cycles;

    return ret;
  }

struct msp430_ops
msp430_single_operands( short insn )
  {
    short src_val = 0;
    struct msp430_op_mode m = msp430_get_op_mode_single(insn);
    unsigned int tmp;
    int len = 0;
    int cycles = 0;
    struct msp430_ops ret;
    int i_push = 0, i_call = 0;

    ret.byte = m.byte;
    ret.d_reg = -1;
    
    if ((insn & ~0x7f) == 0x1280) i_call = 1;
    if ((insn & ~0x7f) == 0x1200) i_push = 1;

    switch (m.ms)
      {
      case REGISTER:
        src_val = alu.regs[m.rs];
        ret.d_reg = m.rs;
        cycles += 1 + (i_push?2:0) + (i_call?3:0) ;
        break;
      case INDEXED:
        len += 1;
        cycles += 4 + (i_push?1:0) + (i_call?1:0);
        pc += 2;
        ret.dst = tmp = msp430_fetch_integer(pc) + alu.regs[m.rs];
        src_val = msp430_fetch_integer(tmp);
        break;
      case SYMBOLIC:
        len += 1;
        pc += 2;
        cycles += 4 + (i_push?1:0) + (i_call?1:0);
        ret.dst = tmp = msp430_fetch_integer(pc) + pc;
        src_val = msp430_fetch_integer(tmp);
        break;
      case ABSOLUTE:
        len += 1;
        pc += 2;
        cycles += 4 + (i_push?1:0) + (i_call?1:0);
        ret.dst = tmp = msp430_fetch_integer(pc);
        src_val = msp430_fetch_integer(tmp);
        break;
      case INDERECT:
        cycles += 3 + (i_push?1:0) + (i_call?1:0);
        ret.dst = alu.regs[m.rs];
        src_val = msp430_fetch_integer(alu.regs[m.rs]);
        break;
      case INDERECTINC:
        cycles += 3 + (i_push?1:0) + (i_call?2:0);
        ret.dst = alu.regs[m.rs];
        src_val = msp430_fetch_integer(alu.regs[m.rs]);
        if (m.rs != 1 && m.rs != 0)
          alu.regs[m.rs] += m.byte ? 1 : 2;
        else
          alu.regs[m.rs] += 2;
        break;
      case IMMEDIATE:
        cycles += 3 + (i_push?1:0) + (i_call?2:0);
        pc += 2;
        src_val = msp430_fetch_integer(pc);
        break;
      }

    pc += 2;
    ret.src  = ret.dval = src_val;
    sr = alu.saved_sr;
    alu.cycles += cycles;
    return ret;
  }


short
msp430_offset(short insn)
{
  short res = insn & 0x3ff;

  if (res & 0x200) res |= ~0x3ff;

  alu.cycles += 2;
  
  return res << 1;
}

void
msp430_check_bp(unsigned int l)
{
  static unsigned int last;

  last = l;
}


/* c: ZNCV */

void
msp430_set_status(int r, char b, char *c)
{
  int tmp = b ? (r&0xff) : (r&0xffff);

  /* Zero flag:*/
  if (*c == 'x')
    {
      if (!tmp) SET_Z;
      else CLR_Z;
    }
  else if (*c == '0')
    {
      CLR_Z;
    }
  else if (*c == '1')
    {
      SET_Z;
    }

  c++;
  /* negative flag */
  if (*c == 'x')
    {
      if ( tmp & (b ? 0x80 : 0x8000) ) SET_N;
      else CLR_N;
    }
  else if (*c == '0')
    {
      CLR_N;
    }
  else if (*c == '1')
    {
      SET_N;
    }

  c++;
  /* carry flag:*/
  if (*c == 'x')
    {
      if (r<0) SET_C;
      else if (b && (r&0xffffff00ul)) SET_C;
      else if (!b && (r&0xffff0000ul)) SET_C;
      else CLR_C;
    }
  else if (*c == '0')
    {
      CLR_C;
    }
  else if (*c == '1')
    {
      SET_C;
    }

  c++;
  /* overflow flag: */
  if (*c == '0')
    {
      CLR_V;
    }
  else if (*c == '1')
    {
      SET_V;
    }

  /* check low power modes */
  if (sr & 0x00f0)
    {
      alu.signal = SIGQUIT;
    }
}

#undef AND

#define	MOV	0x4
#define	ADD	0x5
#define	ADDC	0x6
#define	SUBC	0x7
#define	SUB	0x8
#define	CMP	0x9
#define	DADD	0xa
#define	BIT	0xb
#define	BIC	0xc
#define	BIS	0xd
#define	XOR	0xe
#define	AND	0xf

#define	JMP	0x3c
#define	JL	0x38
#define	JGE	0x34
#define	JN	0x30
#define	JC	0x2c
#define	JNC	0x28
#define	JZ	0x24
#define	JNZ	0x20

#define	SXT	0x118
#define	RETI	0x130
#define	CALL	0x128
#define	PUSH	0x120
#define	RRA	0x110
#define	SWPB	0x108
#define	RRC	0x100

unsigned int
extract_opcode(short insn)
{
  unsigned int tmp;

  /* debug trap */
  if (insn == 1) return 1;

  tmp = 0x0000f000ul & (int)insn;
  tmp >>= 12;

  /* double operands */
  if (tmp>3) return tmp;

  /* jumps */
  if (tmp ==3 || tmp == 2) return ((unsigned int)insn >> 8) & ~3;

  /* single operands or TRAP */
  return ((unsigned int)insn >> 4) & ~7;
}


/* DO NOT PUT BRACES AROUND CAUSE OF SOME '~' OPERATIONS!!! */
#define READ_SRC(ss)	ss.src & (ss.byte?0xff:0xffff)
#define READ_DST(ss)    (ss.dval & (ss.byte?0xff:0xffff))

#define WRITE_BACK(ss,x)				\
do {							\
if (ss.d_reg != -1) 					\
{							\
alu.regs[ss.d_reg] = tmp & (ss.byte? 0xff : 0xffff);	\
}							\
else							\
msp430_write_integer(ss.dst,tmp, (ss.byte?1:2) );	\
} while(0)


#define MASK_SR(x)	\
do { if (x.d_reg != 2) alu.regs[2] &= SR_MASK; } while(0)


void
flow()
{
  short insn;
  unsigned int opcode;
  struct msp430_ops srp;
  struct msp430_ops drp;
  int tmp;

  do
    {
      tmp = 0;

      insn = msp430_fetch_integer(pc);
      opcode = extract_opcode(insn);

      if (!opcode)
        {
          alu.signal = SIGTRAP;
          return;
        }
      else if (opcode == 1)	/* debug trap */
      {
      	alu.signal = SIGTRAP;
      	pc += 2;
      	return;
      }
      else if (opcode < 16)
        {
          drp = msp430_double_operands( insn );
        }
      else if (opcode < 0x3d)
        {
          ;
          ; /* jump insn */
        }
      else if (opcode&0x100)
        {
			if ((opcode != CALL) && (opcode != PUSH))
				srp = msp430_single_operands( insn );
        }

      alu.insns += 1;
      
      switch(opcode)
        {
        default:
          fprintf(stderr,"Unknown code 0x%04x\n", insn);

          /* single operands */
        case RRC:
          tmp = (C<<(srp.byte?8:16)) | (READ_SRC(srp));
          tmp >>= 1;
          WRITE_BACK(srp,tmp);
          MASK_SR(srp);
          CLR_C;
          if (srp.src & 1) SET_C;
          msp430_set_status(tmp, srp.byte, "xx-0");
          break;
        case SWPB:
          tmp = ((srp.src << 8)&0xff00) | ((srp.src >> 8)&0x00ff);
          WRITE_BACK(srp,tmp);
          MASK_SR(srp);
          break;
        case RRA:
          tmp = READ_SRC(srp);
          tmp = (tmp & (srp.byte ? 0x80 : 0x8000)) | ((tmp>>1) & (srp.byte ? 0x7f : 0x7fff) );
          WRITE_BACK(srp,tmp);
          MASK_SR(srp);
          CLR_C;
          if (srp.src & 1) SET_C;
          msp430_set_status(tmp, srp.byte, "xx-0");
          break;
        case SXT:
          tmp = srp.src & 0xff;
          if (tmp&0x80) tmp |= 0xff00;
          WRITE_BACK(srp,tmp);
          MASK_SR(srp);
          msp430_set_status(tmp, srp.byte, "xx-0");
          break;
        case PUSH:
          sp -= 2;
          srp = msp430_single_operands( insn );
          tmp = READ_SRC(srp);
          msp430_write_integer(sp,tmp, 2);
          break;
        case CALL:
          sp -= 2;
          srp = msp430_single_operands( insn );
          tmp = READ_SRC(srp);
          msp430_write_integer(sp,pc, 2);
          pc = tmp;
          break;
        case RETI:
          sr = msp430_fetch_integer(sp);
          sp += 2;
          msp430_set_status(0, 0, "----");
          pc = msp430_fetch_integer(sp);
          sp += 2;
          break;

          /* double operand */

	case DADD:
	  {
	    unsigned int imc = C;
	    unsigned int ss = drp.src;
	    unsigned int dd = drp.dval;
	    unsigned int rr = 0;
	    int i = 0;
	    tmp = 0;
	    
	    for(i=0; i<(drp.byte?2:4); i++ )
	    {
	      rr = (ss&0xf) + (dd&0xf) + imc;
	      
	      if (rr >= 10) 
	      {
		rr -= 10;
		imc = 1;
	      }
	      else
		imc = 0;
	      
	      ss >>= 4;
	      dd >>= 4;
	      tmp |= rr << (i*4);
	    }
	    
	    WRITE_BACK(drp,tmp);
	    msp430_set_status(tmp, drp.byte, "xx--");
	    if (imc) 
	      SET_C;
	    else
	      CLR_C; 
	  }
	  break;
	case MOV:
          tmp = READ_SRC(drp);
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          msp430_set_status(0, 0, "----");
          break;
        case ADD:
        case ADDC:
          tmp = (READ_SRC(drp)) + READ_DST(drp) + ((opcode==ADDC) ? C : 0) ;
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          {
            int b = drp.byte ?0x80:0x8000;
            int f1,f2;
            int s = READ_SRC(drp),d = READ_DST(drp), r=tmp;
            
            f1 = ((d&b) == 0) && ((s&b) == 0) && (r&b);
            f2 = (d&b) && (s&b) && ((r&b) == 0);
            
            if (f1 || f2) SET_V;
            else CLR_V;
          }
          msp430_set_status(tmp, drp.byte, "xxx-");
          break;
        case SUB:
        case SUBC:
          tmp = (~READ_SRC(drp)) + READ_DST(drp) + ((opcode==SUBC) ? C : 1) ;
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          {
            int b = drp.byte ?0x80:0x8000;
            int f1,f2;
	    int s = READ_SRC(drp),d = READ_DST(drp), r=tmp;
	    
	    f1 = ((d&b) == 0) && (s&b) && (r&b);
            f2 = (d&b) && ((s&b) == 0) &&((r&b) == 0);
            
            if (f1 || f2) SET_V;
            else CLR_V;
          }
          msp430_set_status(tmp, drp.byte, "xxx-");
          break;
        case CMP:
          tmp = (~READ_SRC(drp)) + READ_DST(drp) + 1;
          MASK_SR(drp);
          {
            int b = drp.byte ?0x80:0x8000;
            int f1,f2;
	    int s = READ_SRC(drp),d = READ_DST(drp), r=tmp;
	    
	    f1 = ((d&b) == 0) && (s&b) && (r&b);
            f2 = (d&b) && ((s&b) == 0) &&((r&b) == 0);
            
            if (f1 || f2) SET_V;
            else CLR_V;
          }
          msp430_set_status(tmp, drp.byte, "xxx-");
          break;
        case BIT:
          tmp = (READ_SRC(drp)) & READ_DST(drp);
          MASK_SR(drp);
          msp430_set_status(tmp, drp.byte, "xx-0");
          if (tmp) SET_C;
          else CLR_C;
          break;
        case BIC:
          tmp = (~READ_SRC(drp)) & READ_DST(drp);
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          msp430_set_status(tmp, drp.byte, "----");
          break;
        case BIS:
          tmp = (READ_SRC(drp)) | READ_DST(drp);
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          msp430_set_status(tmp, drp.byte, "----");
          break;
        case XOR:
          tmp = (READ_SRC(drp)) ^ READ_DST(drp);
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          msp430_set_status(tmp, drp.byte, "xx--");
          if (tmp) SET_C;
          else CLR_C;
          {
            int b = drp.byte ?0x80:0x8000;
            int f;
            f = ((READ_SRC(drp))&b) && (READ_DST(drp)&b);
            if (f) SET_V;
            else CLR_V;
          }
          break;
        case AND:
          tmp = (READ_SRC(drp)) & READ_DST(drp);
          WRITE_BACK(drp,tmp);
          MASK_SR(drp);
          msp430_set_status(tmp, drp.byte, "xx-0");
          if (tmp) SET_C;
          else CLR_C;
          break;


          /* jumps */
        case JNZ:
          tmp = msp430_offset(insn);
          pc += 2 + ((!Z) ? tmp:0);
          break;
        case JZ:
          tmp = msp430_offset(insn);
          pc += 2 + ((Z) ? tmp:0);
          break;
        case JC:
          tmp = msp430_offset(insn);
          pc += 2 + ((C) ? tmp:0);
          break;
        case JNC:
          tmp = msp430_offset(insn);
          pc += 2 + ((!C) ? tmp:0);
          break;
        case JN:
          tmp = msp430_offset(insn);
          pc += 2 + ((N) ? tmp:0);
          break;
        case JGE:
          tmp = msp430_offset(insn);
          pc += 2 + ((!(N^V)) ? tmp:0);
          break;
        case JL:
          tmp = msp430_offset(insn);
          pc += 2 + ((N^V) ? tmp:0);
          break;
        case JMP:
          pc += msp430_offset(insn) + 2;
          break;
        } /* case */

    }
  while(alu.signal == 0 ) ;
}


int 
msp430_interrupt(int vector)
{
  int ivec;

  if (vector < 0xffe0 || vector > 0xfffful || (vector&1)) return -1;

  /* get ISR address */
  ivec = msp430_fetch_integer(vector);

  /* push pc*/
  sp -= 2;
  msp430_write_integer(sp,pc, 2);
  /* push sr */
  sp -= 2;
  msp430_write_integer(sp,sr, 2);

  /* load pc */
  pc = ivec;
  
  alu.interrupts += 1;

  return 0;
}



#ifndef NUM_ELEM
#define NUM_ELEM(A) (sizeof (A) / sizeof (A)[0])
#endif


typedef short int           word;
typedef unsigned short int  uword;

static unsigned long  heap_ptr = 0;
host_callback *       callback;


static int issue_messages = 10;

unsigned long
msp430_extract_unsigned_integer (addr, len)
unsigned char * addr;
int len;
{
  unsigned long retval = 0, i = 0;
  unsigned char * p;
  unsigned char * startaddr = (unsigned char *)addr;

  if (len > (int) sizeof (unsigned long))
    printf ("That operation is not available on integers of more than %d bytes.",
            sizeof (unsigned long));
  /* msp430 is a little-endian beastie */
  p = startaddr;
  while(len)
    {
      retval |= (*p) << (i*8) ;
      p++;
      i++;
      len--;
    }
  return retval;
}

void
sim_size (power)
int power;
{
  fprintf(stderr,"Simulator required size id %ld\n", (1l<<power));
}

static void
init_pointers ()
{}





void
msp430_store_unsigned_integer (addr, len, val)
unsigned char * addr;
int len;
unsigned long val;
{
  unsigned char * p;
  unsigned char * startaddr = (unsigned char *)addr;
  unsigned char * endaddr = startaddr + len;

  for (p = startaddr; p < endaddr;)
    {
      * p ++ = val & 0xff;
      val >>= 8;
    }
}

#define	MEM_SIZE_FLOOR	64

static void
set_initial_gprs ()
{
  int i;

  for(i=0; i<16; i++) alu.regs[i] = 0;
  alu.cycles = 0;
  alu.insns = 0;
  alu.interrupts = 0;
  memset(wpr, 0, sizeof(wpr));
  memset(wpw, 0, sizeof(wpw));
}

static void
interrupt ()
{
  alu.signal = SIGINT;
}

/* Functions so that trapped open/close don't interfere with the
   parent's functions.  We say that we can't close the descriptors
   that we didn't open.  exit() and cleanup() get in trouble here,
   to some extent.  That's the price of emulation.  */


static int tracing = 0;

void
sim_resume (sd, step, siggnal)
SIM_DESC sd;
int step, siggnal;
{
  void (* sigsave)();

  //  fprintf(stderr,"%s\n", step?"Step flow":"Free run");
  sigsave = signal (SIGINT, interrupt);
  alu.signal = step ? SIGTRAP : 0;
  flow();
  signal (SIGINT, sigsave);
}


int
sim_write (sd, addr, buffer, size)
SIM_DESC sd;
SIM_ADDR addr;
unsigned char * buffer;
int size;
{
  memcpy (& alu.mem[(addr&0xfffful)], buffer, size);
  return size;
}

int
sim_read (sd, addr, buffer, size)
SIM_DESC sd;
SIM_ADDR addr;
unsigned char * buffer;
int size;
{
  memcpy (buffer, & alu.mem[(addr&0xfffful)], size);
  return size;
}


int
sim_store_register (sd, rn, memory, length)
SIM_DESC sd;
int rn;
unsigned char * memory;
int length;
{
  init_pointers ();

  if (rn < 16 && rn >= 0)
    {
      if (length == 2)
        {
          long ival;

          /* misalignment safe */
          ival = msp430_extract_unsigned_integer (memory, 2);
          alu.regs[rn] = ival;
          return 2;
        }
      else if (length == 1)
        {
          long ival;
          ival = msp430_extract_unsigned_integer (memory, 2);
          alu.regs[rn] = ival&0xff;
          return 1;
        }
    }
  else
    return 0;

  return 0;
}

int
sim_fetch_register (sd, rn, memory, length)
SIM_DESC sd;
int rn;
unsigned char * memory;
int length;
{
  init_pointers ();

  if (rn < 16 && rn >= 0)
    {
      if (length == 2)
        {
          long ival;

          ival = alu.regs[rn];
          /* misalignment-safe */
          msp430_store_unsigned_integer (memory, 2, ival);
          return 2;
        }
      else if (length == 1)
        {
          long ival = alu.regs[rn];
          msp430_store_unsigned_integer (memory, 1, ival);
          return 1;
        }
    }
  else
    return 0;

  return 0;
}


int
sim_trace (sd)
SIM_DESC sd;
{
  tracing = 1;

  sim_resume (sd, 0, 0);

  tracing = 0;

  return 1;
}



static int stop_prog_addr;

void
sim_stop_reason (sd, reason, sigrc)
SIM_DESC sd;
enum sim_stop * reason;
int * sigrc;
{
  if (alu.signal == SIGQUIT)
    {
      char *name;
      int status = 0;
      
      if (stop_prog_addr && stop_prog_addr == (pc&0xfffful))
        {
          *sigrc = 0;
          *reason = sim_exited;
        }
      else
        {
          *sigrc = alu.signal;
          *reason = sim_stopped;
        }
    }
  else
    {
      *reason = sim_stopped;
      *sigrc = alu.signal;
    }
}


int
sim_stop (sd)
SIM_DESC sd;
{
  return 1;
}


void
sim_info (sd, verbose)
SIM_DESC sd;
int verbose;
{

  callback->printf_filtered (callback, "\n\n# instructions executed  %10d\n",
                             alu.insns);
  callback->printf_filtered (callback,     "# cycles                 %10d\n",
                             alu.cycles);
  callback->printf_filtered (callback,     "# Interrupts             %10d\n",
  			     alu.interrupts);

}

#define	LONG(x)		(((x)[0]<<24)|((x)[1]<<16)|((x)[2]<<8)|(x)[3])
#define	SHORT(x)	(((x)[0]<<8)|(x)[1])

SIM_DESC
sim_open (kind, cb, abfd, argv)
SIM_OPEN_KIND kind;
host_callback * cb;
struct bfd * abfd;
char ** argv;
{
  char *myname;

  myname = argv[0];
  callback = cb;

  if (kind == SIM_OPEN_STANDALONE)
    issue_messages = 1;

  /* Discard and reacquire memory -- start with a clean slate.  */

  set_initial_gprs ();	/* Reset the GPR registers.  */

  /* Fudge our descriptor for now.  */
  return (SIM_DESC) 1;
}

void
sim_close (sd, quitting)
SIM_DESC sd;
int quitting;
{
  /* nothing to do */
}

static SIM_OPEN_KIND sim_kind;


SIM_RC
sim_load (sd, prog, abfd, from_tty)
SIM_DESC sd;
char * prog;
bfd * abfd;
int from_tty;
{
  /* Do the right thing for ELF executables; this turns out to be
     just about the right thing for any object format that:
       - we crack using BFD routines
       - follows the traditional UNIX text/data/bss layout
       - calls the bss section ".bss".   */

  extern bfd * sim_load_file (); /* ??? Don't know where this should live.  */
  bfd * prog_bfd;
  char *myname = prog;

  strcpy(alu.prog_name, prog);

  {
    bfd * handle;
    asection * s_bss;

    handle = bfd_openr (prog, 0);	/* could be "msp430" */


    if (!handle)
      {
        printf("``%s'' could not be opened.\n", prog);
        return SIM_RC_FAIL;
      }

    /* Makes sure that we have an object file, also cleans gets the
       section headers in place.  */
    if (!bfd_check_format (handle, bfd_object))
      {
        /* wasn't an object file */
        bfd_close (handle);
        printf ("``%s'' is not appropriate object file.\n", prog);
        return SIM_RC_FAIL;
      }

    /* Look for that bss section.  */
    s_bss = bfd_get_section_by_name (handle, ".bss");

    if (!s_bss)
      {
        printf("``%s'' has no bss section.\n", prog);
        return SIM_RC_FAIL;
      }

    /* Appropriately paranoid would check that we have
       a traditional text/data/bss ordering within memory.  */

    /* figure the end of the bss section */
#if 0
    printf ("bss section at 0x%08x for 0x%08x bytes\n",
            (unsigned long) s_bss->vma , (unsigned long) s_bss->_cooked_size);
#endif
#if 0
    heap_ptr = (unsigned long) s_bss->vma + (unsigned long) s_bss->_cooked_size;
#endif
    /* Clean up after ourselves.  */
    bfd_close (handle);

    /* XXX: do we need to free the s_bss and handle structures? */
  }

  /* from sh -- dac */
  prog_bfd = sim_load_file (sd, myname, callback, prog, abfd,
                            sim_kind == SIM_OPEN_DEBUG,
                            0, sim_write);
  if (prog_bfd == NULL)
    return SIM_RC_FAIL;

  if (abfd == NULL)
    bfd_close (prog_bfd);

  return SIM_RC_OK;
}

int get_stop_addr PARAMS ((struct bfd * ));

SIM_RC
sim_create_inferior (sd, prog_bfd, argv, env)
SIM_DESC sd;
struct bfd * prog_bfd;
char ** argv;
char ** env;
{
  unsigned short ivec;
  unsigned char tmp;

  asection * s_vectors;
  asection * s_data;

  s_vectors = bfd_get_section_by_name (prog_bfd, ".vectors");
  bfd_get_section_contents (prog_bfd, s_vectors, &tmp, 30, 1);
  ivec = tmp;
  bfd_get_section_contents (prog_bfd, s_vectors, &tmp, 31, 1);
  ivec |= (unsigned short)tmp << 8;

  /* we need to do this cause gdb somehow
     puts ".data" into .bss */
  s_data = bfd_get_section_by_name (prog_bfd, ".data");

  if (s_data->flags & SEC_LOAD)
    {
      char *buffer;
#if 0
      // no longer available
      bfd_size_type size = bfd_get_section_size_before_reloc (s_data);
#endif
      bfd_size_type size = bfd_get_section_size (s_data);
      bfd_vma lma;

      buffer = malloc (size);

      /* need a rom image !!! */
      lma = bfd_section_lma (prog_bfd, s_data);
      bfd_get_section_contents (prog_bfd, s_data, buffer, 0, size);
      sim_write (sd, lma, buffer, size);
    }

  pc = ivec;
  
  stop_prog_addr = get_stop_addr(prog_bfd);

  return SIM_RC_OK;
}

void
sim_kill (sd)
SIM_DESC sd;
{
  /* nothing to do */
}

void
sim_do_command (sd, cmd)
SIM_DESC sd;
char * cmd;
{
  FILE *dumpfile;

  if (cmd != NULL)
    {
      char ** simargv = buildargv (cmd);

      if (strcmp (simargv[0], "watch") == 0)
        {
          if ((simargv[1] == NULL) || (simargv[2] == NULL))
            {
              fprintf (stderr, "Error: missing argument to watch cmd.\n");
              return;
            }
          msp430_set_watchpoint(simargv[1], simargv[2]);
          return;

        }
      else if (strcmp (simargv[0], "dumpmem") == 0)
        {

          if (simargv[1] == NULL)
            {
              fprintf (stderr, "Error: missing argument to dumpmem cmd.\n");
              return;
            }

          fprintf (stderr, "Writing dumpfile %s...",simargv[1]);

          dumpfile = fopen (simargv[1], "w");
          fclose (dumpfile);

          fprintf (stderr, "done.\n");
        }
      else if (strcmp (simargv[0], "clearstats") == 0)
        {
          alu.cycles = 0;
          alu.interrupts = 0;
          alu.insns = 0;
        }
      else if (strcmp (simargv[0], "verbose") == 0)
        {
          issue_messages = 2;
        }
      else if (strcmp (simargv[0], "interrupt") == 0)
        {
          int ivec;

          if (simargv[1] == 0)
            {
              fprintf (stderr, "Error: vector address required.\n");
              return;
            }

          if (strcmp (simargv[1], "reset") == 0)
            {
              msp430_interrupt(0xfffeUL);
              return;
            }

          ivec = (int) strtol( simargv[1], (char **)NULL, 0) ;
          if (msp430_interrupt(ivec))
            {
              fprintf (stderr, "Error: vector address 0x%04x (%s) is out of range.\n",
                       ivec, simargv[1]);
            }

          return;
        }
      else if (strcmp (simargv[0], "ww") == 0)
        {
          int addr;
          int val;

          if (simargv[1] == 0)
            {
              fprintf (stderr, "Error: address required.\n");
              return;
            }

          addr = (int) strtol( simargv[1], (char **)NULL, 0) ;

          if (simargv[2] == 0)
            {
              fprintf (stderr, "Warning: missing value. Assuming zero.\n");
              val = 0;
            }
          else
            {
              val = (int) strtol( simargv[2], (char **)NULL, 0) ;
            }
          msp430_write_integer(addr&0xffff, val&0xffff,2);
        }
      else if (strcmp (simargv[0], "wb") == 0)
        {
          int addr;
          int val;

          if (simargv[1] == 0)
            {
              fprintf (stderr, "Error: address required.\n");
              return;
            }

          addr = (int) strtol( simargv[1], (char **)NULL, 0) ;

          if (simargv[2] == 0)
            {
              fprintf (stderr, "Warning: missing value. Assuming zero.\n");
              val = 0;
            }
          else
            {
              val = (int) strtol( simargv[2], (char **)NULL, 0) ;
            }
          msp430_write_integer(addr&0xffff, val&0xff,1);
        }
      else if (strncmp (simargv[0], "stat", 4) == 0)
        {
          fprintf (stderr,"Cycles:	 %d\n", alu.cycles);
          fprintf (stderr,"Instructions: %d\n", alu.insns);
          fprintf (stderr,"Interrupts:	 %d\n", alu.interrupts);
        }
      else
        {
          fprintf (stderr,"Error: \"%s\" is not a valid MSP430 simulator command.\n",
                   cmd);
        }
    }
  else
    {
      fprintf (stderr, "MSP430 built-in simulator commands: \n");
      fprintf (stderr, "  watch [r/w/a/d] <addr>\n");
      fprintf (stderr, "  dumpmem <filename>\n");
      fprintf (stderr, "  clearstats\n");
      fprintf (stderr, "  verbose\n");
      fprintf (stderr, "  interrupt <vector>\n");
      fprintf (stderr, "  ww <addr> [value]\n");
      fprintf (stderr, "  wb <addr> [value]\n");
    }
}

void
sim_set_callbacks (ptr)
host_callback * ptr;
{
  callback = ptr;
}

int
process_symbol (asymbol *sym)
{
  int res;
  
  if ( strcmp(sym->name, "__stop_progExec__") ) return 0;
  
  res = sym->value + sym->section->vma + 4;
  
  return res;
}

int
get_stop_addr(struct bfd *abfd)
{
  long storage_needed;
  asymbol **symbol_table;
  long number_of_symbols;
  long i;
  int res;

  storage_needed = bfd_get_symtab_upper_bound (abfd);

  if (storage_needed < 0) return;

  if (storage_needed == 0) 
    {
      return ;
    }
  
  symbol_table = (asymbol **) xmalloc (storage_needed);
  number_of_symbols = bfd_canonicalize_symtab (abfd, symbol_table);

  if (number_of_symbols < 0) return;

  for (i = 0; i < number_of_symbols; i++) 
  {
      res = process_symbol (symbol_table[i]) ;
      if (res) return res;
  }
  
  return 0;
}














