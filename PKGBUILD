pkgname=noted
pkgver=1.0.0
pkgrel=1
pkgdesc="A terminal note-taking app with a built-in TUI"
arch=('x86_64')
license=('MIT')
depends=('libsodium')
source=("main.c" "Makefile")
md5sums=('SKIP' 'SKIP')

build() {
  cd "$srcdir"
  make
}

package() {
  cd "$srcdir"
  install -Dm755 noted "$pkgdir/usr/bin/noted"
}
