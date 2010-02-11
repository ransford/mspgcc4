#! /usr/bin/env perl -w

#This script helps testing compiler changes. It takes a disassembly dump
#and removes all absolute addresses in all forms from it.
#The stripped disassembly from old and new compiler versions can then be
#compared using a GUI tool (e.g. kdiff3) to see what exactly was changed.

use Carp;
use strict;

my ($lastFunctionName, $lastFunctionAddr, $out);

sub werr() {
	croak "Write error."
}

if ($#ARGV < 1)
{
	die "Usage: stripdump.pl <dump file> <output file>\nAny of the files can be - to mean stdin or stdout, respectively.\n";
}

open S, $ARGV[0] || die "Failed to open $ARGV[0] for reading: $!\n";
open D, ">$ARGV[1]" || die "Failed to open $ARGV[1] for writing: $!\n";

foreach (<S>)
{
	chomp;
	if (/([0-9a-fA-F]{8}) <(.*)>:/)
	{
		$lastFunctionAddr = hex($1);
		$lastFunctionName = $2;
		print D "$lastFunctionName:\n";
	}
	elsif (/^[ ]{4}([0-9a-fA-F]{4}):\t[^\t]+\t(.*)$/)
	{
		$out = sprintf("+%04x:\t$2\n", hex($1) - $lastFunctionAddr);
		$out =~ s/#0x[0-9a-fA-F]+/#(imm)/g;
		$out =~ s/#[0-9]+/#(imm)/g;
		$out =~ s/&0x[0-9a-fA-F]+/&(imm)/g;
		$out =~ s/#0x[0-9a-fA-F]+/#(imm)/g;
		$out =~ s/0x[0-9a-fA-F]+(\(r[0-9]{2}\))/off$1/g;
		$out =~ s/[0-9]+(\(r[0-9]{2}\))/off$1/g;
		$out =~ s/;abs 0x[0-9a-fA-F]+//g;
		print D $out or werr();
	}
	elsif (/^[ ]{4}([0-9a-fA-F]{4}):\t[^\t]+$/)
	{
		$out = sprintf("+%04x:\n", hex($1) - $lastFunctionAddr);
		print D $out or werr();
	}
	else
	{
		print D "$_\n" or werr();
	}
}

close S;
close D;
