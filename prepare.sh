pushd fuse-3
meson build
set -e
pushd build
ninja
sudo ninja install

