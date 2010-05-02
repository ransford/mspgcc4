%define target msp430
# The top-level _version_tag.txt file from the mspgcc4 repository
%global VERSION_TAG r4.20100210
# Marks the RPM spec file release.  Resets to 1 when VERSION_TAG
# changes.
%global RELEASE_NUMBER 2

Name:		%{target}-binutils
Version:	2.20.1
Release:	%{RELEASE_NUMBER}.%{VERSION_TAG}%{?dist}
Summary:	Cross Compiling GNU binutils targeted at %{target}
Group:		Development/Tools
License:	GPLv2+
URL:		http://www.gnu.org/software/binutils/
Source0:	http://ftp.gnu.org/gnu/binutils/binutils-%{version}.tar.bz2
# I (rob spanton) have attempted to get this patch upstream.
# See mailing list post: http://article.gmane.org/gmane.comp.gnu.binutils/39694
# FSF's response was to request copyright release from all contributors.
Patch0:		binutils-2.20.1-msp430.patch
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	texinfo

%description
This is a Cross Compiling version of GNU binutils, which can be used to
assemble and link binaries for the %{target} platform, instead of for the
native %{_arch} platform.

%prep
%setup -q -c -n msp430-binutils
%patch0 -p0 -b .msp430~

%build
mkdir -p build
cd build
CFLAGS="$RPM_OPT_FLAGS" ../binutils-%{version}/configure --prefix=%{_prefix} \
	--libdir=%{_libdir} --mandir=%{_mandir} --infodir=%{_infodir} \
	--target=%{target} --disable-werror --disable-nls \
	--with-pkgversion=MSPGCC4_%{VERSION_TAG}

make %{?_smp_mflags}

%install
cd build
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
# these are for win targets only
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/%{target}-{dlltool,nlmconv,windres,windmc}.1
# we don't want these as this is a cross-compiler
rm -rf $RPM_BUILD_ROOT%{_infodir}
rm -f $RPM_BUILD_ROOT%{_libdir}/libiberty.a

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_prefix}/%{target}

%{_bindir}/%{target}-*
%{_mandir}/man1/%{target}-*.1.gz

%changelog
* Sat May  1 2010 Peter A. Bigot pab@peoplepowerco.com  - 2.20.1-2.r4.20100210
- Update to binutils 2.20.1

* Sat Apr  3 2010 Peter A. Bigot pab@peoplepowerco.com  - 2.20-1
- Update for mspgcc4

* Sat Jul 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.19.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Fri Jun 12 2009 Rob Spanton rspanton@zepler.net 2.19.1-1
- Bump up to binutils 2.19.1 

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.19-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Thu Jan 15 2009 Rob Spanton rspanton@zepler.net 2.19-2
- Add comment about getting patch upstream.

* Tue Jan 13 2009 Rob Spanton rspanton@zepler.net 2.19-1
- Bump up to binutils 2.19.
- Use the bundled 2.19 patch from mspgcc packaging.
- Don't install man pages about windows utilities.
- Reduce a number of lines from the files section to just one line.
- Don't delete the debug information.

* Thu Aug 28 2007 Rob Spanton rspanton@zepler.net 2.18-1
- Initial release
