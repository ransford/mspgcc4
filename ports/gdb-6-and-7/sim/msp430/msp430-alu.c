
#include <stdio.h>
#include <signal.h>


#define FIRST_PSEUDO_REG	16
#define MEM_SIZE		65536

struct msp430_alu {
	short 		regs[FIRST_PSEUDO_REG];
	unsigned long	cycles;
	unsigned long	insns;
	unsigned long	interrupts;
	char 		mem[MEM_SIZE];
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



void msp430_set_pc(unsigned int x)
{
	pc = x;
}

unsigned int msp430_get_pc()
{
	return (unsigned int)pc;
}



struct msp430_op_mode
msp430_get_op_mode(short insn)
{
	struct msp430_op_mode m;
	int s_reg;
	int d_reg;
	
	
	m.as = (insn>>4) & 3;
	m.ad = (insn>>7) & 1;
	m.byte = (insn>>6) & 1;
	s_reg = (insn>>8) & 0xf;
	d_reg = insn & 0xf;
	
	m.rs = s_reg;
	m.rd = d_reg;
		
	if(m.ad == 0) 
		m.md = REGISTER;
	else
	{
		if(d_reg == 2)
			m.md = ABSOLUTE;	/* &addr */
		else if(d_reg == 0)
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
			
			if(s_reg == 2)
				m.ms = ABSOLUTE;	/* &addr */
			else if(s_reg == 0)
				m.ms = SYMBOLIC;	/* addr (pc + x) */
			else if(s_reg == 3)
				m.ms = REGISTER;
			else
				m.ms = INDEXED;		/* X(Rn) */
			
			cg2 = 1;
			break;

		case 2:
			m.ms = INDERECT;
			if(s_reg == 2 || s_reg == 3) m.ms = REGISTER;
			cg1 = 4;
			cg2 = 2;
			break;

		case 3:
			if(s_reg == 0) 
				m.ms = IMMEDIATE;
			else
				m.ms = INDERECTINC;
			if(s_reg == 2 || s_reg == 3) m.ms = REGISTER;
			cg1 = 8;
			cg2 = -1;
			break;
	}
		
	return m;
}

short msp430_fetch_integer(unsigned int loc)
{
	char *m = &alu.mem[0xffff&loc];
	short res;
	
	res = (*m) | (short)(*(m+1))<<8;
	return res;
}


void msp430_write_integer(int loc, int val, int len)
{
	if(!len ) return;
	
	if(loc&1 && len != 1)
	{
		alu.signal = SIGBUS;
		return;
	}

	while(len--)
	{
		alu.mem[loc] = 0xff & val;
		loc++;
		val >>= 8;
	}	
}


struct msp430_ops
msp430_double_operands( short insn )
{
	short src_val;
	struct msp430_op_mode m = msp430_get_op_mode(insn);
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
			cycles += (m.rd!=0) ? 1:2;
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
			cycles += 2 + (m.rd) ? 0 : 1;
			ret.dst = alu.regs[m.rs];
			src_val = msp430_fetch_integer(alu.regs[m.rs]);
			break;
		case INDERECTINC:
			cycles += 2 + (m.rd) ? 0 : 1;
			ret.dst = alu.regs[m.rs];
			src_val = msp430_fetch_integer(alu.regs[m.rs]);
			if(m.rs != 1 && m.rs != 0) 
				alu.regs[m.rs] += m.byte ? 1 : 2;
			else
				alu.regs[m.rs] += 2;
			break;
		case IMMEDIATE:
			cycles += 2 + (m.rd) ? 0 : 1;
			pc += 2;
			src_val = msp430_fetch_integer(pc);
			break;
	}

	switch(m.md)
	{
		case REGISTER:
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
	
	
	
	return ret;
}

struct msp430_ops
msp430_single_operands( short insn )
{
	short src_val;
	struct msp430_op_mode m = msp430_get_op_mode(insn);
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
			ret.d_reg = m.rs;
			cycles += (m.rd) ? 1: 2;
			break;
		case INDEXED:
			len += 1;
			cycles += 3;
			pc += 2;
			ret.dst = tmp = msp430_fetch_integer(pc) + alu.regs[m.rs];
			src_val = msp430_fetch_integer(tmp);
			break;
		case SYMBOLIC:
			len += 1;
			pc += 2;
			cycles += 3;
			ret.dst = tmp = msp430_fetch_integer(pc) + pc;
			src_val = msp430_fetch_integer(tmp);
			break;
		case ABSOLUTE:
			len += 1;
			pc += 2;
			cycles += 3;
			ret.dst = tmp = msp430_fetch_integer(pc);
			src_val = msp430_fetch_integer(tmp);
			break;
		case INDERECT:
			cycles += 2 + (m.rd) ? 0 : 1;
			ret.dst = alu.regs[m.rs];
			src_val = msp430_fetch_integer(alu.regs[m.rs]);
			break;
		case INDERECTINC:
			cycles += 2 + (m.rd) ? 0 : 1;
			ret.dst = alu.regs[m.rs];
			src_val = msp430_fetch_integer(alu.regs[m.rs]);
			if(m.rs != 1 && m.rs != 0) 
				alu.regs[m.rs] += m.byte ? 1 : 2;
			else
				alu.regs[m.rs] += 2;
			break;
		case IMMEDIATE:
			cycles += 2 + (m.rd) ? 0 : 1;
			pc += 2;
			src_val = msp430_fetch_integer(pc);
			break;
	}
	
	pc += 2;
	ret.src  = ret.dval = src_val;
	return ret;
}


short msp430_offset(short insn)
{
	short res = insn & 0x3ff;
	
	if(res & 0x200) res |= ~0x3ff;
	
	return res << 1;
}

void msp430_check_bp(unsigned int l)
{
	static unsigned int last;
	
	last = l;
}


/* C: ZNCV */

void msp430_set_status(int r, char b, char *c)
{
	int tmp = b ? (r&0xff) : (r&0xffff);
	
	/* Zero flag:*/
	if(*c == 'x')
	{
		if(!tmp) SET_Z; else CLR_Z;
	}
	else if(*c == '0')
	{
		CLR_Z;
	}
	else if(*c == '1')
	{
		SET_Z;
	}
	
	c++;
	/* negative flag */
	if(*c == 'x')
	{
		if( tmp & (b ? 0x80 : 0x8000) ) SET_N; else CLR_N;
	}
	else if(*c == '0')
	{
		CLR_N;
	}
	else if(*c == '1')
	{
		SET_N;
	}
	
	c++;
	/* carry flag:*/
	if(*c == 'x')
	{
		if(r<0) SET_C;
		else if(b && (r&0xffffff00ul)) SET_C;
		else if(!b && (r&0xffff0000ul)) SET_C;
		else CLR_C;
	}
	else if(*c == '0')
	{
		CLR_C;
	}
	else if(*c == '1')
	{
		SET_C;
	}
	
	c++;
	/* overflow flag: */
	if(*c == '0')
	{
		CLR_V;
	}
	else if(*c == '1')
	{
		SET_V;
	}
	
	/* check low power modes */
	if(sr & 0x00f0)
	{
		alu.signal = SIGQUIT;
	}
}

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

unsigned int extract_opcode(short insn)
{
	unsigned int tmp;
	
	tmp = 0x0000f000ul & (int)insn;
	tmp >>= 12;
	
	/* double operands */
	if(tmp>3) return tmp;
	
	/* jumps */
	if(tmp ==3 || tmp == 2) return ((unsigned int)insn >> 8) & ~3;
	
	/* single operands or TRAP */
	return ((unsigned int)insn >> 4) & ~5;
}



#define READ_SRC(ss)	(ss.src & (ss.byte?0xff:0xffff))
#define READ_DST(ss)    (ss.dval & (ss.byte?0xff:0xffff))

#define WRITE_BACK(ss,x)				\
do {							\
if(ss.d_reg != -1) 					\
alu.regs[ss.d_reg] = tmp & (ss.byte? 0xff : 0xffff);	\
else							\
msp430_write_integer(ss.dst,tmp, (ss.byte?1:2) );	\
} while(0)

void flow()
{
	short insn;
	unsigned int opcode;
	struct msp430_ops srp;
	struct msp430_ops drp;
	int tmp;
	
	while(alu.signal == 0 )
	{
		tmp = 0;
		
		insn = msp430_fetch_integer(pc);
		opcode = extract_opcode(insn);
		
		if(!opcode)
		{
			alu.signal = SIGTRAP;
			return;
		}
		else if(opcode < 16)
		{
			drp = msp430_double_operands( insn );
		}
		else if(opcode < 0x3d)
		{
			;; /* jump insn */
		}
		else if(opcode&0x100)
		{
			srp = msp430_single_operands( insn );
		}
		
		switch(opcode)
		{
			default:
				fprintf(stderr,"Unknown code 0x%04x\n", insn);
			
			/* single operands */
			case RRC:
				tmp = (C<<(srp.byte?8:16))|READ_SRC(srp);
				tmp >>= 1;
				if(srp.src & 1) SET_C;
				WRITE_BACK(srp,tmp);
				msp430_set_status(tmp, srp.byte, "xx-0");
				break;
			case SWPB:
				tmp = ((srp.src << 8)&0xff00) | ((srp.src >> 8)&0x00ff);
				WRITE_BACK(srp,tmp);
				break;
			case RRA:
				tmp = READ_SRC(srp);
				if(srp.src & 1) SET_C;
				tmp = (tmp & (srp.byte ? 0xff : 0xffff)) | ((tmp>>1) & (srp.byte ? 0x7f : 0x7fff) );
				WRITE_BACK(srp,tmp);
				msp430_set_status(tmp, srp.byte, "xx-0");
				break;
			case SXT:
				tmp = srp.src & 0xff;
				if(tmp&0x80) tmp |= 0xff00;
				WRITE_BACK(srp,tmp);
				msp430_set_status(tmp, srp.byte, "xx-0");
				break;
			case PUSH:
				tmp = READ_SRC(srp);
				msp430_write_integer(sp,tmp, 2);
				sp -= 2;
				break;
			case CALL:
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
			
			case MOV:
				tmp = READ_SRC(drp);
				WRITE_BACK(drp,tmp);
				msp430_set_status(0, 0, "----");
				break;
			case ADD:
			case ADDC:
				tmp = READ_SRC(drp) + READ_DST(drp) + ((opcode==ADDC) ? C : 0) ;
				WRITE_BACK(drp,tmp);
				{
					int b = drp.byte ?0x80:0x8000;
					int f;
					
					f = (!(tmp & b) 
					     && (READ_SRC(drp) & b) 
					     && (READ_DST(drp) & b))
					    || ((tmp & b)
					    	&& !(READ_SRC(drp) & b)
					    	&& !(READ_DST(drp) & b));
					if(f) SET_V; else CLR_V;
				}
				msp430_set_status(tmp, drp.byte, "xxx-");
				break;
			case SUB:
			case SUBC:
				tmp = ~READ_SRC(drp) + READ_DST(drp) + ((opcode==SUBC) ? C : 1) ;
				WRITE_BACK(drp,tmp);
				{
					int b = drp.byte ?0x80:0x8000;
					int f;
					
					f = (!(tmp & b) 
					     && (READ_SRC(drp) & b) 
					     && (READ_DST(drp) & b))
					    || ((tmp & b)
					    	&& !(READ_SRC(drp) & b)
					    	&& !(READ_DST(drp) & b));
					if(f) SET_V; else CLR_V;
				}
				msp430_set_status(tmp, drp.byte, "xxx-");
				break;
			case CMP:
				tmp = ~READ_SRC(drp) + READ_DST(drp) + 1;
				{
					int b = drp.byte ?0x80:0x8000;
					int f;
					
					f = (!(tmp & b) 
					     && (READ_SRC(drp) & b) 
					     && (READ_DST(drp) & b))
					    || ((tmp & b)
					    	&& !(READ_SRC(drp) & b)
					    	&& !(READ_DST(drp) & b));
					if(f) SET_V; else CLR_V;
				}
				msp430_set_status(tmp, drp.byte, "xxx-");
				break;
			case BIT:
				tmp = READ_SRC(drp) & READ_DST(drp);
				msp430_set_status(tmp, drp.byte, "-x-0");
				if(tmp) SET_C; else CLR_C;
				break;
			case BIC:
				tmp = ~READ_SRC(drp) & READ_DST(drp);
				WRITE_BACK(drp,tmp);
				msp430_set_status(tmp, drp.byte, "----");
				break;
			case BIS:
				tmp = READ_SRC(drp) | READ_DST(drp);
				WRITE_BACK(drp,tmp);
				msp430_set_status(tmp, drp.byte, "----");
				break;
			case XOR:
				tmp = READ_SRC(drp) ^ READ_DST(drp);
				msp430_set_status(tmp, drp.byte, "xx--");  
				if(tmp) SET_C; else CLR_C;
				{
					int b = drp.byte ?0x80:0x8000;
					int f;
					f = (READ_SRC(drp)&b) && (READ_DST(drp)&b);
					if(f) SET_V; else CLR_V;
				}
				break;
			case AND:
				tmp = READ_SRC(drp) & READ_DST(drp);
				WRITE_BACK(drp,tmp);
				msp430_set_status(tmp, drp.byte, "xx-0");  
				if(tmp) SET_C; else CLR_C;
				break;
			/* jumps */
			case JNZ:
				pc += 2 + (!Z) ? msp430_offset(insn):0;
				break;
			case JZ:
				pc += 2 + (Z) ? msp430_offset(insn):0;
				break;
			case JC:
				pc += 2 + (C) ? msp430_offset(insn):0;
				break;
			case JNC:
				pc += 2 + (!C) ? msp430_offset(insn):0;
				break;
			case JN:
				pc += 2 + (N) ? msp430_offset(insn):0;
				break;
			case JGE:
				pc += 2 + (!(N^V)) ? msp430_offset(insn):0;  
				break;
			case JL:
				pc += 2 + (N^V) ? msp430_offset(insn):0;
				break;
			case JMP:
				pc += msp430_offset(insn);
				break;	
		} /* case */
		
	} /* while */
}


















