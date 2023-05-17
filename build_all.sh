#!/usr/bin/env bash

# only build linux on a distro with an old libc like Debian 11 or Ubuntu 20.04 for compatibility
env -i HOME="$HOME" bash -l -c "scons platform=linux target=template_debug build_libcmark_gfm=yes"
env -i HOME="$HOME" bash -l -c "scons platform=linux target=template_release build_libcmark_gfm=yes"
# env -i HOME="$HOME" bash -l -c "scons platform=windows target=template_release build_libcmark_gfm=yes"
env -i ANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" HOME="$HOME" bash -l -c "scons platform=android target=template_release arch=armv7a build_libcmark_gfm=yes android_ndk=$ANDROID_NDK_ROOT"
env -i ANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" HOME="$HOME" bash -l -c "scons platform=android target=template_release arch=armv8a build_libcmark_gfm=yes android_ndk=$ANDROID_NDK_ROOT"
env -i ANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" HOME="$HOME" bash -l -c "scons platform=android target=template_release arch=x86_64 build_libcmark_gfm=yes android_ndk=$ANDROID_NDK_ROOT"
env -i ANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" HOME="$HOME" bash -l -c "scons platform=android target=template_debug arch=armv8a build_libcmark_gfm=yes android_ndk=$ANDROID_NDK_ROOT"

cd demo
rm release.zip
zip -r release.zip fonts bin
# TODO: osx and 32bit platforms
