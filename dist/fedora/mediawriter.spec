Name:           mediawriter
Version:        4.0
Release:        1%{?dist}
Summary:        Fedora Media Writer

License:        GPLv2+
URL:            https://github.com/MartinBriza/MediaWriter
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  gettext
#BuildRequires:  libappstream-glib
BuildRequires:  gcc-c++

Requires:       qt5-qtbase%{?_isa}
Requires:       qt5-qtquickcontrols%{?_isa} >= 5.3.0
Requires:       polkit%{?_isa}
%if 0%{?fedora} && 0%{?fedora} < 25
Requires: udisks2%{?_isa}
%else
Requires: storaged%{?_isa}
%endif

%description
A tool to write images of Fedora media to portable drives
like flash drives or memory cards.

%prep
%autosetup -p1 -n MediaWriter-%{commit}
mkdir %{_target_platform}

%build
pushd %{_target_platform}
%{qmake_qt5} PREFIX=%{_prefix} MEDIAWRITER_VERSION=%{version}-%{dist} ..
popd
%make_build -C %{_target_platform}

%install
make install INSTALL_ROOT=%{buildroot} -C %{_target_platform}


%files
%{_bindir}/%{name}
%{_libexecdir}/%{name}/

%changelog
* Tue Aug 9 2016 Martin Bříza <mbriza@redhat.com> 4.0
- Initial release

