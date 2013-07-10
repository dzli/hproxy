#/bin/sh

function do_debug
{
    cd jni
    ndk-build
    cd -
    cp libs/armeabi/hproxy assets
    cp jni/daemon/hproxy.conf assets
    ant debug 
}

case "$1" in
debug | d )
    do_debug
;;  
debug-deploy | dd )
    do_debug
    adb install -r  bin/UnlockProxy-debug.apk
;;  
* )
;;  
esac

