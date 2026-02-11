#!/bin/bash
set -e

# Script to build RPM package for TreeMk
# Usage: VERSION=0.5.0 ./build-rpm.sh

if [ -z "$VERSION" ]; then
    echo "ERROR: VERSION environment variable must be set"
    exit 1
fi

echo "Building RPM for version: $VERSION"

# Setup RPM build environment
rpmdev-setuptree

# Generate changelog date
CHANGELOG_DATE=$(date "+%a %b %d %Y")

# Create spec file
cat > ~/rpmbuild/SPECS/treemk.spec <<EOF
Name:           treemk
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        Wiki-style markdown editor with live preview
License:        Custom
URL:            https://github.com/jailop/treemk
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtwebengine-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  md4c-devel

Requires:       qt6-qtbase
Requires:       qt6-qtwebengine
Requires:       qt6-qtsvg
Requires:       md4c

%description
TreeMk is a Markdown editor with wiki-link support, featuring a multi-window
architecture, file management, and real-time preview capabilities.

%prep
%setup -q

%build
cmake -B build \\
      -DCMAKE_BUILD_TYPE=Release \\
      -DCMAKE_INSTALL_PREFIX=/usr \\
      -DCMAKE_INSTALL_LIBDIR=%{_libdir}
cmake --build build --parallel

%install
DESTDIR=%{buildroot} cmake --install build

# Install desktop file
mkdir -p %{buildroot}%{_datadir}/applications
install -Dm644 resources/treemk.desktop \\
    %{buildroot}%{_datadir}/applications/treemk.desktop

# Install icon
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/256x256/apps
install -Dm644 resources/treemk.png \\
    %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/treemk.png

%files
%{_bindir}/treemk
%{_datadir}/applications/treemk.desktop
%{_datadir}/icons/hicolor/256x256/apps/treemk.png
%license LICENSE

%changelog
* ${CHANGELOG_DATE} TreeMk CI <noreply@github.com> - ${VERSION}-1
- Automated build for version ${VERSION}
EOF

echo "Generated spec file:"
cat ~/rpmbuild/SPECS/treemk.spec

# Create source tarball
mkdir -p ~/rpmbuild/SOURCES

TEMP_DIR=$(mktemp -d)
mkdir -p ${TEMP_DIR}/treemk-${VERSION}

echo "Copying files to temporary directory..."
cp -R . ${TEMP_DIR}/treemk-${VERSION}/

echo "Creating tarball..."
tar --exclude='.git' --exclude='build' --exclude='tmp' \
    --exclude='treemk_autogen' --exclude='CMakeFiles' \
    --exclude='CMakeCache.txt' --exclude='Makefile' \
    -czf ~/rpmbuild/SOURCES/treemk-${VERSION}.tar.gz \
    -C ${TEMP_DIR} treemk-${VERSION}

rm -rf ${TEMP_DIR}

echo "Tarball contents (first 20 files):"
tar -tzf ~/rpmbuild/SOURCES/treemk-${VERSION}.tar.gz | head -20

# Build RPM
echo "Building RPM package..."
rpmbuild -ba ~/rpmbuild/SPECS/treemk.spec

# Copy RPM to workspace
echo "Copying RPM to workspace..."
cp ~/rpmbuild/RPMS/x86_64/treemk-*.rpm .

echo "RPM build complete!"
ls -lh treemk-*.rpm
