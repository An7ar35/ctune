# Maintainer: E.A.Davison

pkgname=ctune-git
pkgver=v1.0.0
pkgrel=1
pkgdesc="NCurses internet radio player for Linux."
arch=('x86_64' 'aarch64')
url="https://github.com/An7ar35/ctune"

depends=('ncurses'
         'openssl'
         'curl'
         'ffmpeg'
         'alsa-lib'
         'pulse')

makedepends=('cmake'
             'git')

optdepends=('sdl2: for SDL2 output plugin support'
            'sndio: for SNDIO output plugin support'
            'vlc: for VLC player plugin support')

conflicts=('ctune')
provides=('ctune')
license=('AGPLv3')
source=(git://github.com/An7ar35/ctune)
sha512sums=('SKIP')

_gitname=ctune

build() { //TODO
  cd "$_gitname"
  ./configure prefix=/usr
  make
}

package() { //TODO
  cd "$_gitname"
  make DESTDIR="$pkgdir" install
  install -Dm644 contrib/ctune.bash-completion "$pkgdir"/usr/share/bash-completion/completions/ctune
}
