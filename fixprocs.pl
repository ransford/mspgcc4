# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

#restrict processor types used by LIBC to set supported by binutils
foreach (<STDIN>)
{
	chomp;
	$TYPES{$1}=1 if (/{"msp(.*)",[ \t]+MSP430.*, [a-zA-Z0-9_]+},/);
}

@MF = `cat mspgcc/msp430-libc/src/Makefile`;
open F, ">mspgcc/msp430-libc/src/Makefile";
foreach (@MF)
{
	s/crt(430x[^\.]+)\.o/$TYPES{$1}?"crt$1.o":""/ge;
	print F;
}
close F;
