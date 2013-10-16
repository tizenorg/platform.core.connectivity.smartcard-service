# FOR COMMENTING DEFINITION, MUST USE %% instead of %
%global use_autostart "-DUSE_AUTOSTART=1"
#%%global test_client "-DTEST_CLIENT=1"

Name:       smartcard-service
Summary:    Smartcard Service
Version:    0.1.29
Release:    0
Group:      Network & Connectivity/Service
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
%if 0%{!?use_autostart:1}
Source1:    smartcard-service-server.init
%endif
BuildRequires: cmake
Source1001:	%{name}.manifest
Source1002:	%{name}-devel.manifest
Source1003:	smartcard-service-common.manifest
Source1004:	smartcard-service-common-devel.manifest
Source1005:	smartcard-service-server.manifest
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: python
BuildRequires: python-xml

Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig
Requires:         smartcard-service-common = %{version}-%{release}


%description
A library for Smartcard applications.


%prep
%setup -q
cp %{SOURCE1001} %{SOURCE1002} %{SOURCE1003} %{SOURCE1004} %{SOURCE1005} .


%package    devel
Summary:    Smartcard service
Group:      Network & Connectivity/Development
Requires:   %{name} = %{version}-%{release}

%description devel
For developing Smartcard applications.


%package -n smartcard-service-common
Summary:    Common smartcard service
Group:      Network & Connectivity/Service

%description -n smartcard-service-common
Common smartcard service for developing internally


%package -n smartcard-service-common-devel
Summary:    Common smartcard service
Group:      Network & Connectivity/Development
Requires:   smartcard-service-common = %{version}-%{release}

%description -n smartcard-service-common-devel
For developing smartcard services internally.


%package -n smartcard-service-server
Summary:    Smartcard service server
Group:      Network & Connectivity/Service
Requires:   smartcard-service-common = %{version}-%{release}

%description -n smartcard-service-server
Server for smartcard service


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . %{?use_autostart} %{?test_client} -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

%install
%make_install
%if 0%{!?use_autostart:1}
	%__mkdir -p  %{buildroot}/etc/init.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
	%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
	%__cp -af %SOURCE1 %{buildroot}/etc/init.d/smartcard-service-server
	chmod 755 %{buildroot}/etc/init.d/smartcard-service-server
%endif

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

%post -n smartcard-service-common
/sbin/ldconfig

%postun -n smartcard-service-common
/sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmartcard-service.so.*
%license LICENSE.APLv2


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
%license LICENSE.APLv2


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
	%{_datadir}/dbus-1/system-services/org.tizen.smartcard_service.service
%else
	%{_sysconfdir}/init.d/smartcard-service-server
%endif
%license LICENSE.APLv2
