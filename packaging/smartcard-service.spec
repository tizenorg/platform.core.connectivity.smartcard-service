# FOR COMMENTING DEFINITION, MUST USE %% instead of %
%global use_autostart "-DUSE_AUTOSTART=1"
#%%global test_client "-DTEST_CLIENT=1"

################################################################################
# package : smartcard-service                                                  #
################################################################################
Name:       smartcard-service
Summary:    Smartcard Service FW
Version:    0.1.48
Release:    0
Group:      libs
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
%if 0%{!?use_autostart:1}
Source1:    %{name}-server.init
%endif
BuildRequires: cmake
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(libcrypto)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(cynara-creds-gdbus)
BuildRequires: pkgconfig(cynara-session)

BuildRequires: python
BuildRequires: python-xml
BuildRequires: hash-signer

Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:         %{name}-common = %{version}-%{release}
Requires: security-config


%description
Smartcard Service FW.


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so.*
%{_datadir}/license/%{name}


%post
/sbin/ldconfig


%postun
/sbin/ldconfig


################################################################################
# package : smartcard-service-devel                                            #
################################################################################
%package    devel
Summary:    smartcard service devel
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}


%description devel
smartcard service.


%files  devel
%manifest %{name}-devel.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}/*
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc


################################################################################
# package : smartcard-service-common                                           #
################################################################################
%package    common
Summary:    common smartcard service
Group:      Development/Libraries


%description common
common smartcard service.


%files common
%manifest %{name}-common.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}-common.so.*
%{_datadir}/license/%{name}-common


################################################################################
# package : smartcard-service-common-devel                                     #
################################################################################
%package    common-devel
Summary:    common smartcard service
Group:      Development/Libraries
Requires:   %{name}-common = %{version}-%{release}


%description common-devel
common smartcard service.


%files common-devel
%manifest %{name}-common-devel.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}-common/*
%{_libdir}/lib%{name}-common.so
%{_libdir}/pkgconfig/%{name}-common.pc


################################################################################
# package : smartcard-service-server                                           #
################################################################################
%global bindir /usr/apps/%{name}-server

%package          server
Summary:          server smartcard service
Group:            Development/Libraries
Requires:         %{name}-common = %{version}-%{release}


%description server
smartcard service server


%post server
/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform %{bindir}
%if 0%{!?use_autostart:1}
	ln -sf /etc/init.d/%{name}-server /etc/rc.d/rc3.d/S79%{name}-server
	ln -sf /etc/init.d/%{name}-server /etc/rc.d/rc5.d/S79%{name}-server
%endif
ln -sf /usr/apps/%{name}-server/bin/smartcard-daemon /usr/bin/smartcard-daemon
%if 0%{?test_client:1}
	ln -sf /usr/apps/%{name}-server/bin/smartcard-test-client /usr/bin/smartcard-test-client
%endif


%postun server
%if 0%{!?use_autostart:1}
	rm -f /etc/rc.d/rc3.d/S79%{name}-server
	rm -f /etc/rc.d/rc5.d/S79%{name}-server
%endif
rm -f /usr/bin/smartcard-daemon


%files server
%manifest %{name}-server.manifest
%defattr(-,root,root,-)
%{bindir}/bin/smartcard-daemon
%if 0%{?test_client:1}
	%{bindir}/bin/smartcard-test-client
%endif
%if 0%{?use_autostart:1}
	%{_datadir}/dbus-1/system-services/org.tizen.SmartcardService.service
%else
	%{_sysconfdir}/init.d/%{name}-server
%endif
%{bindir}/%{name}-server
%{bindir}/author-signature.xml
%{bindir}/signature1.xml
/etc/dbus-1/system.d/org.tizen.SmartcardService.conf

################################################################################
# common...                                                                    #
################################################################################
%prep
%setup -q


%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
export LDFLAGS+="-Wl,-Bsymbolic-functions"
mkdir obj-arm-limux-qnueabi
cd obj-arm-limux-qnueabi
%cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix} %{?use_autostart} %{?use_gdbus} %{?test_client}

%install
cd obj-arm-limux-qnueabi
%make_install
%if 0%{!?use_autostart:1}
	%__mkdir -p  %{buildroot}/etc/init.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
	%__cp -af %SOURCE1 %{buildroot}/etc/init.d/%{name}-server
	chmod 755 %{buildroot}/etc/init.d/%{name}-server
%endif
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/etc/dbus-1/system.d/
cp -af %{_builddir}/%{name}-%{version}/packaging/%{name} %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/%{name}-common %{buildroot}/usr/share/license/
cp -af %{_builddir}/%{name}-%{version}/packaging/%{name}-server %{buildroot}%{bindir}
cp -af %{_builddir}/%{name}-%{version}/packaging/org.tizen.SmartcardService.conf %{buildroot}/etc/dbus-1/system.d/

%define tizen_sign 1
%define tizen_sign_base %{bindir}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1
