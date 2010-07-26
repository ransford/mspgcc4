#!/bin/perl
#Workaround for buggy MinGW name mapping mechanism
#As a result of compilation, will produce stable native Win32 binaries for GCC
#Run this script inside GCC build directory: <...>/mingw-gccwa.pl <TARGET DIRECTORY>

die "Usage: mingw-gccwa.pl <GCC TARGET DIRECTORY>" if $ARGV[0] eq '';

print "Running initial make...\n";
system "mingw32-make";


unless (-e "Makefile.mgwbk")
{
	print "Fixing Makefile includes...\n";
	rename "gcc/Makefile", "gcc/Makefile.mgwbk";

	open S, "gcc/Makefile.mgwbk";
	open D, ">gcc/Makefile";
	foreach(<S>)
	{
	s/-I\/([a-zA-Z])\//-I$1:\//g;
	print D $_;
	}
	close S;
	close D;
}

print "Running make from GCC subdir...\n";
system "mingw32-make -C gcc";

print "Restarting global make...\n";
system "mingw32-make";

print "Installing xgcc...\n";
system "mingw32-make -C gcc install";

open F, "msp430/libgcc/config.log" || die "Cannot open msp430/libgcc/config.log";
foreach (<F>)
{
	if (/^  \$ (.*)$/)
	{
		$cmdline = $1;
		last;
	}
}
close F;

die "Cannot retrieve libgcc configure invocation command line\n" if $cmdline eq '';
$cmdline =~ s/\\/\//g;

printf "Adding $ARGV[0] to path...\n";
$ENV{PATH} = $ARGV[0] . "/bin:" . $ENV{PATH};

chdir "msp430/libgcc";
print "Running configuration script ...\n";

system $cmdline;

printf "Patching Makefile...\n";
open F, "Makefile";
@MAKEFILE = <F>;
close F;

open D, ">Makefile";
print D "ifeq (\$(MINGW_WORKAROUND),)
all:
install:
else\n";
print D @MAKEFILE;
print D "\nendif";

close D;


printf "Building libgcc...\n";
system "mingw32-make MINGW_WORKAROUND=1";

printf "Installing libgcc...\n";
system "make install MINGW_WORKAROUND=1";

chdir "../..";
system "mingw32-make";
system "mingw32-make install";