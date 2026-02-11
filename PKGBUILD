# Maintainer: Jaime Lopez <jaime.lopez@datainquiry.dev>
pkgname=treemk-git
pkgver=r100.f670c30
pkgrel=1
pkgdesc="Wiki-style markdown editor with live preview and hierarchical task management (git version)"
arch=('x86_64')
url="https://github.com/jailop/treemk"
license=('custom')
depends=('qt6-base' 'qt6-webengine' 'md4c')
makedepends=('cmake' 'git' 'qt6-tools')
optdepends=('pandoc: for exporting to various formats')
provides=('treemk')
conflicts=('treemk')
source=("git+https://github.com/jailop/treemk.git")
sha256sums=('SKIP')

pkgver() {
    cd "${srcdir}/treemk"
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd "${srcdir}/treemk"
    
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib
    
    cmake --build build
}

package() {
    cd "${srcdir}/treemk"
    
    DESTDIR="${pkgdir}" cmake --install build
    
    # Install license
    install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
    
    # Install desktop file
    install -Dm644 resources/treemk.desktop \
        "${pkgdir}/usr/share/applications/treemk.desktop"
    
    # Install icon
    install -Dm644 resources/treemk.png \
        "${pkgdir}/usr/share/icons/hicolor/256x256/apps/treemk.png"
}
