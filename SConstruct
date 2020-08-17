#!/usr/bin/env python
import os, subprocess, shutil

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['d', 'debug', 'r', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx', 'android']))
opts.Add(EnumVariable('arch', "Compilation architecture", 'x86_64', ['x86_64', 'armv7a', 'armv8a']))
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'x11', 'linux', 'osx', 'android']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'demo/bin/'))
opts.Add(PathVariable('target_name', 'The library name.', 'libmarkdown', PathVariable.PathAccept))
opts.Add(PathVariable('android_ndk', 'Path to the Android NDK installation', '/opt/android-ndk/', PathVariable.PathAccept))
opts.Add(BoolVariable('build_libcmark_gfm', "Build and deploy libcmark-gfm for current platform.", 'no'))


# only support 64 at this time..
bits = 64

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['p'] != '':
    env['platform'] = env['p']

if env['platform'] == '':
    print("No valid target platform selected.")
    quit();

platform = ""
arch = ""
ccflags = ""
linkflags = ""
# Check our platform specifics
if env['platform'] == "osx":
    platform = "osx"
    arch = "x86_64"
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-g','-O2', '-arch', 'x86_64'])
        env.Append(LINKFLAGS = ['-arch', 'x86_64'])
    else:
        env.Append(CCFLAGS = ['-g','-O3', '-arch', 'x86_64', '-std=c11'])
        env.Append(LINKFLAGS = ['-arch', 'x86_64'])

elif env['platform'] in ('x11', 'linux'):
    platform = "x11"
    arch = "x86_64"
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-fPIC', '-g3','-Og', '-std=c11'])
    else:
        env.Append(CCFLAGS = ['-fPIC', '-g','-O3', '-std=c11'])

elif env['platform'] == "windows":
    platform = "windows"
    arch = "x86_64"
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV = os.environ)

    env.Append(CCFLAGS = ['-DWIN32', '-D_WIN32', '-D_WINDOWS', '-W3', '-GR', '-D_CRT_SECURE_NO_WARNINGS'])
    env.Append(LIBS=['Advapi32'])
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-EHsc', '-D_DEBUG', '-MDd'])
    else:
        env.Append(CCFLAGS = ['-O2', '-EHsc', '-DNDEBUG', '-MD'])

elif env['platform'] == "android":
    platform = "android"
    arch = env['arch']
    env['CC'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi30-clang"
    env['CXX'] = env['android_ndk'] + "/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi30-clang++"
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-fPIC', '-g3','-Og', '-std=c11'])
    else:
        env.Append(CCFLAGS = ['-fPIC', '-g','-O3', '-std=c11'])

if env['target'] in ('debug', 'd'):
    target = "debug"
else:
    target = "release"

# Local dependency paths, adapt them to your setup
suffix = "{}/{}/{}".format(platform, arch, target)
godot_headers_path = "godot_headers/"
target = os.path.join(env['target_path'], suffix, env['target_name'])
libcmark_gfm_lib_path = "demo/bin/{}".format(suffix)
libcmark_gfm_include_paths = [
        "godot_markdown/thirdparty/cmark-gfm/src/", # for cmark-gfm.h
        "godot_markdown/thirdparty/cmark-gfm/extensions/", # for all extension headers
        "godot_markdown/thirdparty/cmark-gfm/build_{}_{}/src/".format(platform, arch), # for export.h and version.h
        "godot_markdown/thirdparty/cmark-gfm/build_{}_{}/extensions/".format(platform, arch), # for extensions_export.h
        ]
print(suffix)

env.Append(CPPPATH=['.', godot_headers_path])
env.Append(CPPPATH=['godot_markdown/src/', *libcmark_gfm_include_paths])
env.Append(LIBPATH=[libcmark_gfm_lib_path])
env.Append(LIBS=['cmark-gfm'])

sources = Glob('godot_markdown/src/*.c')

library = env.SharedLibrary(target=target, source=sources)

if env['build_libcmark_gfm']:
    cwd = os.getcwd()
    os.chdir("godot_markdown/thirdparty/cmark-gfm")

    lib_target_path = os.path.join(cwd, libcmark_gfm_lib_path)
    build_dir = "build_{}_{}".format(platform, arch)

    os.makedirs(lib_target_path, exist_ok=True)
    os.makedirs(build_dir, exist_ok=True)

    os.chdir(build_dir)
    if platform == 'android':
        cmake = [
                "cmake",
                "-DCMAKE_TOOLCHAIN_FILE=/opt/android-ndk/build/cmake/android.toolchain.cmake",
                "-DANDROID_ABI=armeabi-v7a",
                "-DANDROID_NATIVE_API_LEVEL=",
                "..",
                ]
    else:
        cmake = ["cmake", ".."]
    make = ["make"]
    subprocess.run(cmake)
    subprocess.run(make)
    shutil.copy2("src/libcmark-gfm.so", lib_target_path)
    shutil.copy2("extensions/libcmark-gfm-extensions.so", lib_target_path)
    os.chdir(cwd)

Default(library)

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
