%define target msp430
# The top-level _version_tag.txt file from the mspgcc4 repository
%global VERSION_TAG r4.20100210
# Marks the RPM spec file release.  Resets to 1 when VERSION_TAG
# changes.  As long as the version tag remains constant for a specific
# version of gcc, ordinality of releases should be satisfied.
%global RELEASE_NUMBER 2
# Match the mspgcc4 tested version
%global mpfr_version 2.4.1

Name:		%{target}-gcc
Version:	4.4.3
# There has been no release, so this is a snapshot
Release:	%{RELEASE_NUMBER}.%{VERSION_TAG}%{?dist}
Summary:	Cross Compiling GNU GCC targeted at %{target}
Group:		Development/Languages
License:	GPLv2+
URL:		http://mspgcc4.sourceforge.net/
Source0:	ftp://ftp.gnu.org/gnu/gcc/gcc-%{version}/gcc-core-%{version}.tar.bz2
Source1:	ftp://ftp.gnu.org/gnu/gcc/gcc-%{version}/gcc-g++-%{version}.tar.bz2
Source2:	msp430-%{VERSION_TAG}-gcc-%{version}.tar.bz2
Source3: http://www.mpfr.org/mpfr-%{mpfr_version}/mpfr-%{mpfr_version}.tar.bz2
Patch0:		gcc-%{version}-msp430-%{VERSION_TAG}.patch
Patch1:		msp430-%{VERSION_TAG}-chipcat.patch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	%{target}-binutils >= 2.19
BuildRequires:	zlib-devel, gettext, bison, flex, texinfo
BuildRequires: gmp-devel >= 4.1.2-8
# BuildRequires: mpfr-devel >= 2.2.1
Requires:	%{target}-binutils

%global gcc_version %{version}

%description
This is a cross compiling version of GNU GCC, which can be used to
compile for the %{target} platform, instead of for the native %{_arch} 
platform.

%package -n %{target}-c++
Summary: C++ support for GCC
Group: Development/Languages
Requires: %{target}-gcc = %{version}-%{release}
Autoreq: true

%description -n %{target}-c++
This package adds C++ support to the GNU Compiler Collection.
It includes support for most of the current C++ specification,
including templates and exception handling.

%prep
%setup -q -c
%setup -q -T -D -a 1
%setup -T -D -a 2
%setup -T -D -a 3
%patch0 -p0 -b .msp430~
cd gcc-%{version}
%patch1 -p3 -b .chipcat~
cd ..

%build
rm -fr build
mkdir -p build
cd build

mkdir mpfr mpfr-install
cd mpfr
../../mpfr-%{mpfr_version}/configure --disable-shared \
  CFLAGS="${CFLAGS:-%optflags}" CXXFLAGS="${CXXFLAGS:-%optflags}" \
  --prefix=`cd ..; pwd`/mpfr-install
make %{?_smp_mflags}
make install
cd ..

CC="%{__cc} ${RPM_OPT_FLAGS}" ../gcc-%{version}/configure --prefix=%{_prefix} \
	--libdir=%{_libdir} --mandir=%{_mandir} --infodir=%{_infodir} \
	--target=%{target} --disable-nls \
	--enable-version-specific-runtime-libs \
	--enable-languages=c,c++ \
        --with-mpfr=`pwd`/mpfr-install/ \
	--with-pkgversion=MSPGCC4_%{VERSION_TAG}
make %{?_smp_mflags}


%install
rm -fr %{buildroot}
cd build

make DESTDIR=%{buildroot} install

FULLPATH=%{buildroot}%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}
FULLEPATH=%{buildroot}%{_prefix}/libexec/gcc/%{target}/%{gcc_version}

# fix some things
ln -sf gcc %{buildroot}%{_prefix}/bin/%{target}-cc
mkdir -p %{buildroot}/lib
ln -sf ..%{_prefix}/bin/%{target}-cpp %{buildroot}/lib/%{target}-cpp
rm -f %{buildroot}%{_infodir}/dir
gzip -9 %{buildroot}%{_infodir}/*.info*

for f in `find %{buildroot}%{_prefix}/include/c++/%{gcc_version}/%{target}/ -name c++config.h`; do
  for i in 1 2 4 8; do
    sed -i -e 's/#define _GLIBCXX_ATOMIC_BUILTINS_'$i' 1/#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_'$i'\
&\

#endif/' $f
  done
done

if [ -n "$FULLLPATH" ]; then
  mkdir -p $FULLLPATH
else
  FULLLPATH=$FULLPATH
fi

find %{buildroot} -name \*.la | xargs rm -f
mkdir -p $FULLPATH

pushd $FULLPATH

# Strip debug info from Fortran/ObjC/Java static libraries
%{target}-strip -g `find . \( -name libgcc.a -o -name libgcov.a \) -a -type f`
popd

mv $FULLPATH/include-fixed/syslimits.h $FULLPATH/include/syslimits.h
mv $FULLPATH/include-fixed/limits.h $FULLPATH/include/limits.h
for h in `find $FULLPATH/include -name \*.h`; do
  if grep -q 'It has been auto-edited by fixincludes from' $h; then
    rh=`grep -A2 'It has been auto-edited by fixincludes from' $h | tail -1 | sed 's|^.*"\(.*\)".*$|\1|'`
    diff -up $rh $h || :
    rm -f $h
  fi
done

cat > %{buildroot}%{_prefix}/bin/%{target}-c89 <<"EOF"
#!/bin/sh
fl="-std=c89"
for opt; do
  case "$opt" in
    -ansi|-std=c89|-std=iso9899:1990) fl="";;
    -std=*) echo "`basename $0` called with non ANSI/ISO C option $opt" >&2
	    exit 1;;
  esac
done
exec %{target}-gcc $fl ${1+"$@"}
EOF
cat > %{buildroot}%{_prefix}/bin/%{target}-c99 <<"EOF"
#!/bin/sh
fl="-std=c99"
for opt; do
  case "$opt" in
    -std=c99|-std=iso9899:1999) fl="";;
    -std=*) echo "`basename $0` called with non ISO C99 option $opt" >&2
	    exit 1;;
  esac
done
exec %{target}-gcc $fl ${1+"$@"}
EOF
chmod 755 %{buildroot}%{_prefix}/bin/%{target}-c?9

cd ..

# Remove binaries we will not be including, so that they don't end up in
# gcc-debuginfo
rm -f %{buildroot}%{_prefix}/%{_lib}/{libffi*,libiberty.a}
rm -f $FULLEPATH/install-tools/{mkheaders,fixincl}
rm -f %{buildroot}%{_prefix}/lib/{32,64}/libiberty.a
rm -f %{buildroot}%{_prefix}/%{_lib}/libssp*
rm -f %{buildroot}%{_prefix}/bin/gnative2ascii

# Remove other stuff not installed that rpm whines about
rm -rf ${FULLPATH}/include-fixed
rm -rf ${FULLPATH}/install-tools
rm -rf ${FULLEPATH}/install-tools
rm -rf %{buildroot}%{_mandir}/man7
rm -rf %{buildroot}%{_infodir}

( cat << \EOF
%__os_install_post
EOF
) | sed \
    -e 's@/usr/bin/strip@/usr/bin/msp430-strip@' \
    -e 's@/usr/bin/objdump@/usr/bin/msp430-objdump@' \
  > os_install_post
echo %define __os_install_post . ./os_install_post

%clean
rm -fr %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/%{target}-*
%{_mandir}/man1/%{target}-*.1*
#%{_infodir}/gcc*
%dir %{_prefix}/%{_lib}/gcc
%dir %{_prefix}/%{_lib}/gcc/%{target}
%dir %{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{target}
%dir %{_prefix}/libexec/gcc/%{target}/%{gcc_version}
%dir %{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/stddef.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/stdarg.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/stdfix.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/varargs.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/float.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/stdbool.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/iso646.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/limits.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/tgmath.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/syslimits.h
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/include/unwind.h
%{_prefix}/libexec/gcc/%{target}/%{gcc_version}/collect2
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/libgcc.a
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/libgcov.a
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/msp*/libgcc.a
%{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}/msp*/libgcov.a
# %doc gcc/README* rpm.doc/changelogs/gcc/ChangeLog* gcc/COPYING*
/lib/%{target}-cpp
# %{_infodir}/cpp*
%{_prefix}/libexec/gcc/%{target}/%{gcc_version}/cc1

%files -n %{target}-c++
%defattr(-,root,root,-)
%{_prefix}/bin/%{target}-*++
%{_mandir}/man1/%{target}-*++.1*
%dir %{_prefix}/%{_lib}/gcc
%dir %{_prefix}/%{_lib}/gcc/%{target}
%dir %{_prefix}/%{_lib}/gcc/%{target}/%{gcc_version}
%dir %{_prefix}/libexec/gcc
%dir %{_prefix}/libexec/gcc/%{target}
%dir %{_prefix}/libexec/gcc/%{target}/%{gcc_version}
%{_prefix}/libexec/gcc/%{target}/%{gcc_version}/cc1plus
# %doc rpm.doc/changelogs/gcc/cp/ChangeLog*

%changelog
* Fri Apr 30 2010 Peter A. Bigot <pab@peoplepowerco.com> - 2.r4.20100210
- Build mpfr locally
- Fix for paths on 64-bit systems

* Sat Apr  3 2010 Peter A. Bigot <pab@peoplepowerco.com>  - 1.r4.20100210
- Update for mspgcc4
- Add chipcat patch for consistency with binutils

* Sat Jul 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.2.3-3.20090210cvs
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Sat Feb 21 2009 Rob Spanton <rspanton@zepler.net> 3.2.3-2.20090210cvs
- Use setup macro to do cleaner decompressing.
- Own libdir/gcc-lib dirs.
- Rename man files rather than delete them.

* Tue Feb 10 2009 Rob Spanton <rspanton@zepler.net> 3.2.3-1.20090210cvs 
- Initial release

