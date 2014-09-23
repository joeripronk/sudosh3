Summary: Complete logging for sudo
Name: sudosh
Version: 3.2.3
Release: 3
License: GPL
Group: Applications/System
URL: http://sudosh.sourceforge.net
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%description
sudosh allows complete session logging of shells run under sudo.
Individual sudo commands are still logged as normal but running a shell
under sudosh records the entire session as well as session timings for complete playback later.

%prep
%setup -q
%configure --prefix=/usr

%build
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall
install -d $RPM_BUILD_ROOT/etc/profile.d/
install -m 0733 -d $RPM_BUILD_ROOT/var/log/sudosh
install $RPM_BUILD_DIR/%{name}-%{version}/contrib/sudosh.sh $RPM_BUILD_ROOT/etc/profile.d/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/*
/etc/profile.d/sudosh.sh
%doc AUTHORS COPYING INSTALL NEWS PLATFORMS README ChangeLog
%attr(733,root,root) %dir /var/log/sudosh

%changelog
* Sat Sep 20 2014 Joeri Pronk <joeri.pronk@adyen.com>
- Timing perfections
* Sat Sep 20 2014 Joeri Pronk <joeri.pronk@adyen.com>
- Timing improvements
* Fri Sep 19 2014 Joeri Pronk <joeri.pronk@adyen.com>
- Add xterm auto resize support
* Fri Sep 19 2014 Joeri Pronk <joeri.pronk@adyen.com>
- Add saving of columns & rows for more accurate replays
- No need to scan all files to replay just one
- Fix unused variables
* Thu Apr 19 2012 Joeri Pronk <joeri.pronk@adyen.com>
- small fix for count_dur which caused "too many open files" errors in sudosh-replay
* Tue Mar 01 2005 Chris MacLeod <cmacleod@redhat.com>
- Initial build.
