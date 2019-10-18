Summary: Complete logging for sudo
Name: sudosh
Version: 3.3.0
Release: 5 
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
install -m 0733 -d $RPM_BUILD_ROOT/var/log/sudosh
#install -d $RPM_BUILD_ROOT/etc/profile.d/
#install $RPM_BUILD_DIR/%{name}-%{version}/contrib/sudosh.sh $RPM_BUILD_ROOT/etc/profile.d/

%clean
rm -rf $RPM_BUILD_ROOT

%pre
chattr -a /var/log/sudosh

%post
chattr +a /var/log/sudosh

%files
%defattr(-,root,root,-)
%{_bindir}/*
/etc/sudosh.conf
%doc AUTHORS COPYING INSTALL NEWS PLATFORMS README ChangeLog
%{_mandir}/man1/*
%{_mandir}/man5/*
%{_mandir}/man8/*
%attr(733,root,root) %dir /var/log/sudosh
%attr(4711,root,root) /usr/bin/sudosh

%changelog
* Fri Oct 18 2019 Joeri Pronk <joeri.pronk@adyen.com>
- merge reads to avoid partial (escape code) captures
  and to reduce time filesize
- only write term dimensions on a terminal resize
- fix compile warnings for snprintf truncations
- add header and tailer for time files
  includes TERM and LC_* environment
- fgets will now always read until an EOL
  discarding any additional unparsed data
- fix sudosh-replay for larger replay sizes
  created by merging reads
- make sscanf more tolerant on extra data
- -c arg now supports and should use complete path
  it automatically finds commands using basename:
  -c arg /usr/bin/rsync
  will be resolved to /usr/bin/rsync from rsync  
- will also log command input/output when it's a tty
- -c arg no longer uses a shell to execute the command
  closing the hole for using shell escapes
- update to newer autoconf/configure
* Fri May 26 2017 Joeri Pronk <joeri.pronk@adyen.com>
- fix child exit, but sudosh does not exit
* Tue Oct 07 2014 Joeri Pronk <joeri.pronk@adyen.com>
- fix homedir for setuid root sudosh
- remove escape characters from sudosh-replay dump
* Mon Oct 06 2014 Joeri Pronk <joeri.pronk@adyen.com>
- Add append_only flag, to avoid file truncation/deletion by non-root users
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
