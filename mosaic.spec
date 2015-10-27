Name:		mosaic
Version:	0.0.0
Release:	1%{?dist}
Summary:	A library to configure hybrid storage for CTs/VMs
Source:		mosaic-%{version}.tar.xz

Group:		System Environment/Libraries
License:	LGPL v2.1
URL:		https://github.com/xemul/mosaic

%description
Mosaic is a tool and library to configure hybrid storage to be used
in VM/CT environments. A storage is supposed to be suitable for both --
keeping lots of small files for VM/CT meta-data as well as fast and
scalable disks for VMs and CTs data.

%prep
%setup -q


%build
make %{?_smp_mflags} LIBDIR=%{_libdir}

%install
%make_install LIBDIR=%{_libdir}

%files
%{_sbindir}/moctl
%{_libdir}/libmosaic.so.*
%doc README.md COPYING doc/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%package devel
Summary: Development files for mosaic library
Requires: mosaic = %{version}-%{release}
Requires: pkgconfig
%description devel
Development files for mosaic library

%files devel
%dir %{_includedir}/mosaic
%{_includedir}/mosaic/mosaic.h
%{_libdir}/pkgconfig/mosaic.pc
%{_libdir}/libmosaic.so

%changelog
* Mon Oct 26 2015 Kir Kolyshkin <kir@openvz.org> 0.0.0-1
- Initial packaging
