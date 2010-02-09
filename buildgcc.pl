#!/bin/perl
# This work is partially financed by the European Commission under the
# Framework 6 Information Society Technologies Project
#  "Wirelessly Accessible Sensor Populations (WASP)".

$g_DialogPresent = (`which dialog 2>/dev/null` ne '');

sub CallDialog
{
	my $ansfile = "/tmp/dialog-$$.ans";
	system("dialog $_[0] 2>$ansfile");
	if ($?)
	{
		unlink($ansfile);
		exit;
	}
	my $answer = `cat $ansfile` || return -1;
	unlink($ansfile);
	return $answer;
}

#int SelectFromList(defaultIdx, Title, Option1, Option2, ...)
sub SelectFromList
{
	my $default = (shift @_) + 1;
	my $title = shift @_, $optionCount = 0;
	if ($g_DialogPresent)
	{
		my $options = '';
		my $width = length($title) + 4;
		$options .= ' ' . ++$optionCount . " \"$_\"" foreach (@_);
		my $res = CallDialog("--default-item $default --menu \"$title\" ".($optionCount + 7)." $width $optionCount $options");
		return $res - 1;
	}
	else
	{
		print "\n$title\n";
		print ++$optionCount . ") $_\n" foreach @_;
		print "Enter your choice (1-$optionCount) [$default]: ";
		my $r = <STDIN>;
		chomp $r;
		$r = $default if $r eq '';
		return $r-1;
	}
}

#string AskForString (prompt, width, default)
sub AskForString
{
	if ($g_DialogPresent)
	{
		return CallDialog("--inputbox \"$_[0]\" 7 $_[1] \"$_[2]\"");
	}
	else
	{
		print "\n$_[0]\n[$_[2]]: ";
		my $r = <STDIN>;
		chomp $r;
		$r = $_[2] if $r eq '';
		return $r;
	}
}

#bool AskYesNo(prompt, default)
sub AskYesNo
{
	if ($g_DialogPresent)
	{
		my $def;
		my $lines = 4;
		my $maxlen = 0;
		foreach(split(/\n/, $_[0]))
		{
			$lines++;
			$maxlen = length($_) if length($_) > $maxlen;
		}
		$def = " --defaultno" if $_[1] eq '0';
		system("dialog$def --yesno \"$_[0]\" $lines ".($maxlen + 6));
		return $? ? 0 : 1;
	}
	else
	{
		my $def = ($_[1] eq '0') ? 'n' : 'y';
		print "\n$_[0] (y/n)\n[$def]";
		my $r = <STDIN>;
		chomp $r;
		$r = $def if $r eq '';
		return ($r eq 'y') ? 1 : 0;
	}
}

#Convert GNU versions to comparable integers (e.g. 1.2.3 => 0x01020300, 1.2 => 0x01020000)
sub GNUVersionToInt
{
	my @vn = split(/[\.-]/, $_[0]);
	push @vn, 0 while $#vn < 3;
	my $ret = 0;
	while ($#vn != -1)
	{
		$ret <<= 8;
		$ret += shift @vn;
	}
	return $ret;
}

#----------------------------------------------------------------------------------------------------------------------------------

$BINUTILS_VERSION = "2.20";
$GNU_MIRROR="ftp.uni-kl.de";
$BUILD_DIR="build";
$GMP_VERSION="4.3.1";
$MPFR_VERSION="2.4.1";

@GCC_VERSIONS = ( 
				  {'ver' => '4.4.3', 'config' => '4.x'}, 
				  {'ver' => '4.4.2', 'config' => '4.x'}, 
				  {'ver' => '4.3.4', 'config' => '4.x'},
				  {'ver' => '4.2.4', 'config' => '4.x'},
				  {'ver' => '3.3.6', 'config' => '3.4'},
				  {'ver' => '3.2.3', 'config' => '3.3'},
				  );
				  
@GDB_VERSIONS = grep(/^gdb-(.*)\.patch/, split("\n", `ls -1 -r`));
s/gdb-(.*)\.patch/$1/ foreach(@GDB_VERSIONS);
@GDB_VERSIONS = sort{GNUVersionToInt($b) <=> GNUVersionToInt($a)}(@GDB_VERSIONS);

@INSIGHT_VERSIONS = grep(/^insight-(.*)\.patch/, split("\n", `ls -1 -r`));
@INSIGHT_VERSIONS = sort{GNUVersionToInt($b) <=> GNUVersionToInt($a)}(@INSIGHT_VERSIONS);
s/insight-(.*)\.patch/$1/ foreach(@INSIGHT_VERSIONS);

%GCCRELEASE = %{$GCC_VERSIONS[SelectFromList(0, "Select GCC version to build:", (map{'gcc-'.$$_{'ver'}}(@GCC_VERSIONS)), "none")]};


$GDBVERSION = $GDB_VERSIONS[SelectFromList(0, "Select GDB version to build:", (map{'gdb-'.$_}(@GDB_VERSIONS)), "none")];
$INSIGHTVERSION = $INSIGHT_VERSIONS[SelectFromList(0, "Select Insight version to build:", (map{'insight-'.$_}(@INSIGHT_VERSIONS)), "none")];

$TARGETPATH = AskForString("Enter target toolchain path", 50, ((`uname -o` eq "Msys\n") ? "/c" : "/opt") ."/msp430-gcc-$GCCRELEASE{ver}");

if (AskYesNo("Create binary package after build?", 1))
{
	$BINPACKAGE = AskForString("Enter binary package name", 50, "msp430-gcc-$GCCRELEASE{ver}".(($GDBVERSION eq '') ? '' : "_gdb_$GDBVERSION").".tbz");
}

$SKIP_BINUTILS = AskYesNo("Looks like binutils are already installed in $TARGETPATH. Skip building binutils?") if (-e "$TARGETPATH/bin/msp430-as");
$BASEDIR = `pwd`;
chomp $BASEDIR;

$startnow = AskYesNo("Selected GCC $GCCRELEASE{ver}
GDB version: $GDBVERSION
Insight version: $INSIGHTVERSION
Target location: $TARGETPATH
Binary package name: $BINPACKAGE
-------------------------------------
Do you want to start build right now?", 0);

if (!$startnow)
{
	$SCRIPTFILE = AskForString("Enter script name to generate", 50, "buildgcc-$GCCRELEASE{ver}".(($GDBVERSION eq '') ? '' : "_gdb_$GDBVERSION").".sh");
}

@COMMANDS = ();
if ($GCCRELEASE{ver} ne '')
{
	push @COMMANDS, "sh do-binutils.sh \"$TARGETPATH\" \"$BINUTILS_VERSION\" \"$GNU_MIRROR\" \"$BUILD_DIR\"" unless $SKIP_BINUTILS;
	push @COMMANDS, "sh do-gcc.sh \"$TARGETPATH\" \"$GCCRELEASE{ver}\" \"$GNU_MIRROR\" \"$BUILD_DIR\" \"gcc-$GCCRELEASE{config}\" \"$GMP_VERSION\" \"$MPFR_VERSION\"";
	push @COMMANDS, "sh do-libc.sh \"$TARGETPATH\" \"$BUILD_DIR\"";
}
	
push @COMMANDS, "sh do-gdb.sh \"$TARGETPATH\" \"$GDBVERSION\" \"$GNU_MIRROR\" \"$BUILD_DIR\" gdb" if $GDBVERSION ne '';
push @COMMANDS, "sh do-gdb.sh \"$TARGETPATH\" \"$INSIGHTVERSION\" \"$GNU_MIRROR\" \"$BUILD_DIR\" insight" if $INSIGHTVERSION ne '';

if ($BINPACKAGE ne '')
{
	push @COMMANDS, "echo \"Creating binary package...\"";
	push @COMMANDS, "cd \"$TARGETPATH\"";
	push @COMMANDS, "tar cjf \"$BASEDIR/$BINPACKAGE\" *";
	push @COMMANDS, "ls -ldq \"$BASEDIR/$BINPACKAGE_NAME\"";
	push @COMMANDS, "cd \"$BASEDIR\"";
}

if ($SCRIPTFILE ne '')
{
	mkdir $BUILD_DIR;
	open F, ">$BUILD_DIR/$SCRIPTFILE" || die "Cannot open script $BUILD_DIR/$SCRIPTFILE for writing";
	print F "#!/bin/sh\ncd ..\nset -eu\n\n";
	print F "$_\n" foreach @COMMANDS;
	close F;
	if (AskYesNo("$SCRIPTFILE created successfully. Run it now"))
	{
		chdir $BUILD_DIR;
		system("sh $SCRIPTFILE");
	}
	printf "Run $BUILD_DIR/$SCRIPTFILE to start build process\n";
}
else
{
	foreach(@COMMANDS)
	{
		print "Running $_\n";
		system($_) || die "Failed to execute $_";
	}
}