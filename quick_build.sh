#!/bin/bash
# Quick build + install for GSF development (incremental, no cmake reconfigure)
set -e
BUILDDIR=/aifs/user/data/zhangcg/gsfdev/CEPCSW/build.105.0.0.x86_64-el9-gcc11-opt
rm -f "$BUILDDIR/lib/libRecGsfTracking.so"
cd "$BUILDDIR" && make -j 8 && cmake --install . && cd -
echo "Done."