Notes for building MSPGCC4 on MinGW:

1. Binutils, libc and GDB are built automatically without any problems

2. Due to the unreasonable double path system (/c/xxx vs. c:/xxx) the following
   workaround steps are required to build GCC:

  2.1. The MPFR's libtool should be replaced by a wrapper around MSys libtool:
       (already done by build scripts)
	#! c:/msys/bin/sh.exe
	exec libtool "$@"

  2.2. The MPFR include path can be set incorrectly. If this happens,
       replace all '-I/c/' with '-Ic:/' in gcc-build/gcc/Makefile

  2.3. If a nested call to mingw32-make in gcc-build/gcc fails with strange
       error messages involving incorrect '\', just restart it manually from
       "gcc" subfolder

  2.4. The xgcc.exe cannot correctly invoke cc1 and ld. Thus, when libgcc
       compilation fails, the following needs to be done:

	* Go to gcc-build/gcc directory and run 'mingw32-make install' to
	  install xgcc in its 'prefix' location

	* Set PATH so it covers the directory containing msp430-xxx binaries

	* Go back to gcc-build/msp430/libgcc and relaunch "configure" using the
	  information from config.log
	* Install the libgcc using msys make (make install), not the MinGW make
	  (mingw32-make install)
