%define	ver	%VERSION
%define	RELEASE	1
%define rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define	prefix	/usr

Name: %NAME
Summary: Generate a core dump of a running program without crashing
Version: %ver
Release: %rel
Group: Development/Libraries
URL: http://goog-coredumper.sourceforge.net
Copyright: BSD
Vendor: Google
Packager: Google <opensource@google.com>
Source: http://goog-coredumper.sourceforge.net/%{NAME}-%{PACKAGE_VERSION}.tar.gz
Distribution: Redhat 7 and above.
Buildroot: %{_tmppath}/%{name}-root
Docdir: %prefix/doc
Prefix: %prefix

%description
The %name utility allows a running program to generate a core
file without actually crashing.  This serves to allow the programmer
to generate a snapshot of a running program's state at any time.

%package devel
Summary: Generate a core dump of a running program without crashing
Group: Development/Libraries

%description devel
The %name-devel package contains static and debug libraries and header
files for developing applications that use the %name utility.

%changelog
    * Fri Feb 11 2005 <opensource@google.com>
    - First draft

%prep
%setup

%build
./configure
make prefix=%prefix

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%doc AUTHORS COPYING ChangeLog INSTALL NEWS README TODO examples

%{prefix}/lib/libcoredumper.so.0
%{prefix}/lib/libcoredumper.so.0.0.0


%files devel
%defattr(-,root,root)

%{prefix}/include/google
%{prefix}/lib/debug
%{prefix}/lib/libcoredumper.a
%{prefix}/lib/libcoredumper.la
%{prefix}/lib/libcoredumper.so
