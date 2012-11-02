Name:       smartcard-service
Summary:    Smartcard Service FW
Version:    0.1.0
Release:    9
Group:      libs
License:    Samsung Proprietary License
Source0:    %{name}-%{version}.tar.gz
Source1:    smartcard-service-server.init
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(libssl)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dpl-wrt-dao-ro)
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
cmake .. -DCMAKE_INSTALL_PREFIX=%{_prefix}
#make %{?jobs:-j%jobs}

%install
cd obj-arm-limux-qnueabi
%make_install
%__mkdir -p  %{buildroot}/etc/init.d/
%__mkdir -p  %{buildroot}/etc/rc.d/rc3.d/
%__mkdir -p  %{buildroot}/etc/rc.d/rc5.d/
%__cp -af %SOURCE1 %{buildroot}/etc/init.d/smartcard-service-server
chmod 755 %{buildroot}/etc/init.d/smartcard-service-server

%post
/sbin/ldconfig

ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc3.d/S79smartcard-service-server
ln -sf /etc/init.d/smartcard-service-server /etc/rc.d/rc5.d/S79smartcard-service-server

%postun
/sbin/ldconfig

rm -f /etc/rc.d/rc3.d/S79smartcard-service-server
rm -f /etc/rc.d/rc5.d/S79smartcard-service-server

#%post
# -n nfc-common-lib -p /sbin/ldconfig

#%postun
# -n nfc-common-lib -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
/usr/lib/libsmartcard-service.so.*

%files  devel
%defattr(-,root,root,-)
/usr/include/smartcard-service/*
/usr/lib/libsmartcard-service.so
/usr/lib/pkgconfig/smartcard-service.pc

%files -n smartcard-service-common
%defattr(-,root,root,-)
/usr/lib/libsmartcard-service-common.so.*

%files -n smartcard-service-common-devel
%defattr(-,root,root,-)
/usr/include/smartcard-service-common/*
/usr/lib/libsmartcard-service-common.so
/usr/lib/pkgconfig/smartcard-service-common.pc

%files -n smartcard-service-server
%manifest smartcard-service-server.manifest
%defattr(-,root,root,-)
/usr/bin/smartcard-daemon
#/usr/bin/smartcard-test-client
/etc/init.d/smartcard-service-server
