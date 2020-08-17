#!/usr/bin/env bash

scons platform=linux target=release build_libcmark_gfm=yes
scons platform=windows target=release build_libcmark_gfm=yes
scons platform=android target=release arch=armv7a build_libcmark_gfm=yes
scons platform=android target=release arch=armv8a build_libcmark_gfm=yes
scons platform=android target=release arch=x86_64 build_libcmark_gfm=yes

# TODO: osx and 32bit platforms
