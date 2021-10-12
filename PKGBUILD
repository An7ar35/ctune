# Maintainer: E.A.Davison <eadavison at protonmail dot com>

pkgname=ctune-git
pkgver=v1.0.7
pkgrel=0
pkgdesc="NCurses internet radio player for Linux."
arch=('x86_64' 'aarch64')
url="https://github.com/An7ar35/ctune"

depends=('ncurses'
         'openssl'
         'curl'
         'ffmpeg'
         'alsa-lib'
         'pulseaudio')

makedepends=('cmake'
             'git')

optdepends=('sdl2: for SDL2 output plugin support'
            'sndio: for SNDIO output plugin support'
            'vlc: for VLC player plugin support')

conflicts=('ctune')
provides=('ctune')
license=('AGPL3')
source=(git://github.com/An7ar35/ctune)
sha512sums=('SKIP')

build() {
    cmake -B ctune_build -S "ctune" \
          -DCMAKE_BUILD_TYPE='Release' \
          -DCMAKE_INSTALL_PREFIX='/usr' \
          -Wno-dev
    cmake --build ctune_build
}

package() {
    DESTDIR="$pkgdir" cmake --install ctune_build
}