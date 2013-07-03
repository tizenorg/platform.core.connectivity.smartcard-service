# FOR COMMENTING DEFINITION, MUST USE %% instead of %
%global use_autostart "-DUSE_AUTOSTART=1"
%global use_gdbus "-DUSE_GDBUS=1"
%global test_client "-DTEST_CLIENT=1"

Name:       smartcard-service
Summary:    Smartcard Service FW
Version:    0.1.20
Release:    0
Group:      libs
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
%if 0%{!?use_autostart:1}
Source1:    smartcard-service-server.init
%endif
BuildRequires: cmake
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
%if 0%{!?use_gdbus:1}
BuildRequires: pkgconfig(dbus-glib-1)
%endif
# for gdbus
BuildRequires: python
BuildRequires: python-xml

Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:         smartcard-service-common = %{version}-%{release}


%description
Smartcard Service FW.


%prep
%setup -q


%package    devel
Summary:    smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}


%description devel
smartcard service.


%package -n smartcard-service-common
Summary:    common smartcard service
Group:      Development/Libraries


%description -n smartcard-service-common
common smartcard service.


%package -n smartcard-service-common-devel
Summary:    common smartcard service
Group:      Development/Libraries
Requires:   smartcard-service-common = %{version}-%{release}


%description -n smartcard-service-common-devel
common smartcard service.


%package -n smartcard-service-server
Summary:    server smartcard service
Group:      Development/Libraries
Requires:   smartcard-service-common = %{version}-%{release}


%description -n smartcard-service-server
smartcard service.


%build
mkdir obj-arm-limux-qnueabi
cd obj-arm-limux-qnueabi
%cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix} %{?use_autostart} %{?use_gdbus} %{?test_client}
#make %{?jobs:-j%jobs}


%install
cd obj-arm-limux-qnueabi
%make_install
%if 0%{!?use_autostart:1}
	%__mkdir -p  %{buildroot}/etc/init.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
	%__cp -af %SOURCE1 %{buildroot}/etc/init.d/smartcard-service-server
	chmod 755 %{buildroot}/etc/init.d/smartcard-service-server
%endif
mkdir -p %{buildroot}/usr/share/license
cp -af %{_builddir}/%{name}-%{version}/packaging/%{name} %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-service-common %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-service-server %{buildroot}/usr/share/license/


%post
/sbin/ldconfig
%if 0%{!?use_autostart:1}
	ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc3.d/S79smartcard-service-server
	ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc5.d/S79smartcard-service-server
%endif


%postun
/sbin/ldconfig
%if 0%{!?use_autostart:1}
	rm -f /etc/rc.d/rc3.d/S79smartcard-service-server
	rm -f /etc/rc.d/rc5.d/S79smartcard-service-server
%endif


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service.so.*
%{_datadir}/license/%{name}


%files  devel
%manifest smartcard-service-devel.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}/*
%{_libdir}/libsmartcard-service.so
%{_libdir}/pkgconfig/%{name}.pc


%files -n smartcard-service-common
%manifest smartcard-service-common.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service-common.so.*
%{_datadir}/license/smartcard-service-common


%files -n smartcard-service-common-devel
%manifest smartcard-service-common-devel.manifest
%defattr(-,root,root,-)
%{_includedir}/smartcard-service-common/*
%{_libdir}/libsmartcard-service-common.so
%{_libdir}/pkgconfig/smartcard-service-common.pc


%files -n smartcard-service-server
%manifest smartcard-service-server.manifest
%defattr(-,root,root,-)
%{_bindir}/smartcard-daemon
%{_datadir}/packages/smartcard-service-server.xml
%if 0%{?test_client:1}
	%{_bindir}/smartcard-test-client
%endif
%if 0%{?use_autostart:1}
	%{_datadir}/dbus-1/services/org.tizen.smartcard_service.service
%else
	%{_sysconfdir}/init.d/smartcard-service-server
%endif
%{_datadir}/license/smartcard-service-server
