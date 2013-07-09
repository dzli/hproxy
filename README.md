hproxy
======

build proxy daemon:
1. goto jni directory and run ndk-build, then you can get the execuable ../libs/armeabi/hproxy
2. copy hproxy and jni/daemon/hproxy.conf to the device's /data/local/tmp
3. run terminal app on the device
4. goto /data/local/tmp and run ./hproxy start -f -c hproxy.conf
