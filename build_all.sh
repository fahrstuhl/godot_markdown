#!/usr/bin/env bash

env -i HOME="$HOME" bash -l -c "scons platform=linux target=release build_libcmark_gfm=yes"
env -i HOME="$HOME" bash -l -c "scons platform=windows target=release build_libcmark_gfm=yes"
env -i HOME="$HOME" bash -l -c "scons platform=android target=release arch=armv7a build_libcmark_gfm=yes"
env -i HOME="$HOME" bash -l -c "scons platform=android target=release arch=armv8a build_libcmark_gfm=yes"
env -i HOME="$HOME" bash -l -c "scons platform=android target=release arch=x86_64 build_libcmark_gfm=yes"

cd demo
rm release.zip
zip -r release.zip fonts bin
# TODO: osx and 32bit platforms
