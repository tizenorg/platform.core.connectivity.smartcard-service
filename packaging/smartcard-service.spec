Name:       smartcard-service
Summary:    Smartcard Service FW
Version:    0.1.19
Release:    1
Group:      libs
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
#Source1:    smartcard-service-server.init

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
Summary:    Smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
smartcard service.

%package -n smartcard-service-common
Summary:    Common smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description -n smartcard-service-common
common smartcard service.

%package -n smartcard-service-common-devel
Summary:    Common smartcard service
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   smartcard-service-common = %{version}-%{release}

%description -n smartcard-service-common-devel
common smartcard service.

%package -n smartcard-service-server
Summary:    Server smartcard service
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
%__mkdir -p  %{buildroot}/etc/init.d/
%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
#IFNDEF USE_AUTOSTART
#%__cp -af %SOURCE1 %{buildroot}/etc/init.d/smartcard-service-server
#chmod 755 %{buildroot}/etc/init.d/smartcard-service-server
#ENDIF

%post
/sbin/ldconfig
ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc3.d/S79smartcard-service-server
ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc5.d/S79smartcard-service-server

%postun
/sbin/ldconfig
rm -f /etc/rc.d/rc3.d/S79smartcard-service-server
rm -f /etc/rc.d/rc5.d/S79smartcard-service-server


%post -n smartcard-service-common
/sbin/ldconfig

%postun -n smartcard-service-common
/sbin/ldconfig


%files
%manifest smartcard-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service.so.*

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

