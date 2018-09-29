pushd fuse-3
meson build
set -e
pushd build
ninja
popd
popd
configs="debug release"
for config in $configs
do
dir=build/$config/lib
mkdir -p $dir
cp fuse-3/build/lib/libfuse3.so* $dir
done

