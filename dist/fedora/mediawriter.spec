Name:           mediawriter
Version:        4.0
Release:        1%{?dist}
Summary:        Fedora Media Writer

License:        GPLv2
URL:            https://github.com/MartinBriza/MediaWriter
Source0:        https://github.com/MartinBriza/MediaWriter/archive/%{version}.tar.gz

BuildRequires:  qt5-qtbase-devel
BuildRequires:  gettext
BuildRequires:  libappstream-glib

Requires:       qt5-qtbase
Requires:       qt5-qtquickcontrols >= 5.3.0
Requires:       udisks2
Requires:       polkit

%description
A tool to write images of Fedora media to portable drives like flash drives or memory cards.

%prep
%autosetup -p1 -n %{version}

%build
mkdir %{_target_platform}
pushd %{_target_platform}
%{qmake_qt5} ..
popd
make %{?_smp_mflags} -C %{_target_platform}

%install
make install INSTALL_ROOT=%{buildroot} -C %{_target_platform}
ls -l %{buildroot}


%files
%defattr(-,root,root,-)
#%doc README.rst LICENSE.txt
#%{python_sitelib}/*
%{_bindir}/mediawriter
#%{_datadir}/applications/*liveusb-creator.desktop
#%{_datadir}/pixmaps/fedorausb.png
#%{_datadir}/appdata/%{name}.appdata.xml
#%{_datadir}/locale/*/LC_MESSAGES/liveusb-creator.mo
#%{_datadir}/polkit-1/actions/org.fedoraproject.pkexec.run-liveusb-creator.policy

%changelog
* Mon Apr 1 2016 Martin Bříza <mbriza@redhat.com> 4.0-1
- Initial release
