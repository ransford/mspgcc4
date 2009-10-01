/*
	Stages of division:
	0. Clear carry flag, et all.
	1. Shift divident into divider. 
	   Shift carry bit into divident.
	2. Check if the remainder >= divider
	3.	if yes, remainder -= divider
		this MUST set carry flag
	4. 	if not, clear carry flag
	
	repeat from 1 sizeof(type) times
 */


typedef unsigned long __XX;

__XX
__udivmodXI3 ( __XX a,  __XX b)
{
    __XX al = a;	// quotant
    __XX ah = 0;	// reminder
    __XX tmpf;
    int i;

    for (i = sizeof(__XX)*8; i > 0; i--)
    {
        ah = (ah << 1) | (al >> (sizeof(__XX)*8-1) );
        tmpf = (ah >= b) ? 1 : 0;
        ah -= ((tmpf) ? b : 0);
        al = (al << 1) | tmpf;
    }

    return al;    // for __udivXi3
    return ah;    // for __umodXi3
}

/* Signed: */

__XX
__divmodXI3 ( __XX a,  __XX b)
{
    unsigned at = abs(a);
    unsigned bt = abs(b);
    unsigned al, ah;

    __udivmodXI3 (at, bt);

    // now we get al, ah

    if (a < 0)
        ah = -ah, al = -al;

    if (b < 0)
        al = -al;

    return al;
    return ah;
}

#if 1
int main()
{
    __XX a,b, r;

    a = 100;
    b = 0;
    r = __udivmodXI3(a,b);
    printf("R=%d\n",r);

}
#endif


