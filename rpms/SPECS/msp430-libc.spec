# This package is noarch, and supplies some foreign binaries for msp430.
# See https://www.redhat.com/archives/fedora-devel-list/2009-February/msg02261.html
%global _binaries_in_noarch_packages_terminate_build 0

Name:		msp430-libc
Version:	20100430
Release:	1%{?dist}
Summary:	C library for use with GCC on Texas Instruments MSP430 microcontrollers
Group:		Development/Libraries
License:	BSD
URL:		http://mspgcc4.sourceforge.net/
Source0:	http://sourceforge.net/projects/mspgcc4/files/msp430-libc/msp430-libc-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:	msp430-gcc
Requires:	msp430-gcc
BuildArch:	noarch

%description
A C library for use on Texas Instruments MSP430 microcontrollers with
the mspgcc toolchain.

%prep
%setup -q

%build
cd src
rm -rf build
CFLAGS="%{optflags}" make %{?_smp_mflags} PREFIX=%{_prefix}

%install
rm -rf %{buildroot}
cd src
mkdir -p %{buildroot}%{_prefix}/msp430/lib
make install PREFIX=%{buildroot}%{_prefix}

# despite us being noarch redhat-rpm-config insists on stripping our files
%define __os_install_post /usr/lib/rpm/redhat/brp-compress

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc doc/{devheaders.txt,volatil}
%dir %{_prefix}/msp430/lib/msp*
%dir %{_prefix}/msp430/include

%{_prefix}/msp430/include/*
%{_prefix}/msp430/lib/*.o
%{_prefix}/msp430/lib/*.a
%{_prefix}/msp430/lib/msp*/*.a


%changelog
* Sat May  1 2010 Peter A. Bigot pab@peoplepowerco.com  - 20100430-1
- Update to version 20100430.

* Sat Apr  3 2010 Peter A. Bigot pab@peoplepowerco.com  - 20100403-1
- Update for mspgcc4

* Sun Jul 26 2009 Rob Spanton rspanton@zepler.net 0-3.20090726cvs
- Use later version from CVS

* Mon Jul 20 2009 Rob Spanton rspanton@zepler.net 0-2.20090620cvs
- Own /usr/msp430/include

* Sat Jun 28 2009 Rob Spanton rspanton@zepler.net 0-1.20090620cvs
- Initial release
