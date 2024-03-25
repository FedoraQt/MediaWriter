Name:           mediawriter
Version:        5.0.0
Release:        1%{?dist}
Summary:        AOSC Media Writer

License:        GPLv2+
URL:            https://github.com/AOSC-Dev/MediaWriter
Source0:        https://github.com/AOSC-Dev/MediaWriter/archive/MediaWriter-%{version}.tar.gz

Provides:       liveusb-creator = %{version}-%{release}
Obsoletes:      liveusb-creator <= 3.95.4-2

BuildRequires:  gcc-c++
BuildRequires:  gettext
BuildRequires:  cmake
BuildRequires:  make
BuildRequires:  libappstream-glib
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  xz-devel

Requires:       qt6-qtsvg
Requires:       qt6-qtdeclarative

%if !0%{?flatpak}
Requires:       polkit
%endif
Requires:       xz-libs

%if !0%{?flatpak}
%if 0%{?fedora} && 0%{?fedora} != 25
Requires: storaged
%else
Requires: udisks
%endif
%endif

%description
A tool to write images of AOSC media to portable drives
like flash drives or memory cards.

%prep
%autosetup -p1 -n MediaWriter-%{version}

%build
%cmake

%cmake_build

%install
%cmake_install

%check
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/metainfo/org.fedoraproject.MediaWriter.appdata.xml

%files
%{_bindir}/%{name}
%{_libexecdir}/%{name}/
%{_datadir}/metainfo/org.fedoraproject.MediaWriter.appdata.xml
%{_datadir}/applications/org.fedoraproject.MediaWriter.desktop
%{_datadir}/icons/hicolor/16x16/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/22x22/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/24x24/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/32x32/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/48x48/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/64x64/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/128x128/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/256x256/apps/org.fedoraproject.MediaWriter.png
%{_datadir}/icons/hicolor/512x512/apps/org.fedoraproject.MediaWriter.png

%changelog
* Mon May 09 06 2022 Jan Grulich <jgrulich@redhat.com> - 5.0.0-1
- 5.0.0
