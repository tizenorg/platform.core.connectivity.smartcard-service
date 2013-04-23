Name:       smartcard-service
Summary:    Smartcard Service FW
Version:    0.1.18
Release:    0
Group:      libs
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
#IFNDEF USE_AUTOSTART
#Source1:    smartcard-service-server.init
#ENDIF
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: cmake
BuildRequires: gettext-tools

Requires(post):   /sbin/ldconfig
Requires(post):   /usr/bin/vconftool
requires(postun): /sbin/ldconfig

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
Requires:   %{name} = %{version}-%{release}

%description -n smartcard-service-common
common smartcard service.

%package -n smartcard-service-common-devel
Summary:    common smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   smartcard-service-common = %{version}-%{release}

%description -n smartcard-service-common-devel
common smartcard service.

%package -n smartcard-service-server
Summary:    server smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description -n smartcard-service-server
smartcard service.

%build
mkdir obj-arm-limux-qnueabi
cd obj-arm-limux-qnueabi
#IFNDEF USE_AUTOSTART
#cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix}
#ELSE
%cmake .. -DUSE_AUTOSTART=1 # daemon will be started when client makes instance by DBUS
#ENDIF
#make %{?jobs:-j%jobs}

%install
cd obj-arm-limux-qnueabi
%make_install
#IFNDEF USE_AUTOSTART
#%__mkdir -p  %{buildroot}/etc/init.d/
#%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
#%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
#%__cp -af %SOURCE1 %{buildroot}/etc/init.d/smartcard-service-server
#chmod 755 %{buildroot}/etc/init.d/smartcard-service-server
#ENDIF
mkdir -p %{buildroot}/usr/share/license
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-service %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-service-common %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/smartcard-service-server %{buildroot}/usr/share/license/

%post
/sbin/ldconfig
#IFNDEF USE_AUTOSTART
#ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc3.d/S79smartcard-service-server
#ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc5.d/S79smartcard-service-server
#ENDIF

%postun
/sbin/ldconfig
#IFNDEF USE_AUTOSTART
#rm -f /etc/rc.d/rc3.d/S79smartcard-service-server
#rm -f /etc/rc.d/rc5.d/S79smartcard-service-server
#ENDIF

%files
%manifest smartcard-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service.so.*
/usr/share/license/smartcard-service

%files  devel
%manifest smartcard-service-devel.manifest
%defattr(-,root,root,-)
%{_includedir}/smartcard-service/*
%{_libdir}/libsmartcard-service.so
%{_libdir}/pkgconfig/smartcard-service.pc

%files -n smartcard-service-common
%manifest smartcard-service-common.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service-common.so.*
/usr/share/license/smartcard-service-common

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
#/usr/bin/smartcard-test-client
#IFNDEF USE_AUTOSTART
#/etc/init.d/smartcard-service-server
#ELSE
/usr/share/dbus-1/services/org.tizen.smartcard_service.service
#ENDIF
/usr/share/license/smartcard-service-server
