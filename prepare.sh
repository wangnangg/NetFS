cd fuse-3
meson build
set -e
cd build
meson configure -Dprefix=/usr
ninja reconfigure
ninja
sudo ninja install
