#!/usr/bin/env perl -w
#! Replaces all binaries in msp430/bin with gcclauncher.exe

die "Usage: stripwin.pl <mspgcc prefix>" if $ARGV[0] eq '';
die "stripwin.pl works only under Win32" if $ENV{SYSTEMROOT} eq '';

foreach(split(/\n/, `ls $ARGV[0]/msp430/bin`))
{
	if (-e "$ARGV[0]/bin/msp430-$_")
	{
		print "Replacing $ARGV[0]/msp430/bin/$_\n";
		system "cp -r launcher/gcclauncher.exe $ARGV[0]/msp430/bin/$_" ;
	}
}