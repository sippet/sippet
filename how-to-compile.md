---
layout: page
title: Compile for desktop platforms
---

This documentation was based on the same Chrome instructions:
[Windows](http://dev.chromium.org/developers/how-tos/build-instructions-windows),
[Mac OS X](http://code.google.com/p/chromium/wiki/MacBuildInstructions),
[Linux](http://code.google.com/p/chromium/wiki/LinuxBuildInstructions),
[Android](http://code.google.com/p/chromium/wiki/AndroidBuildInstructions), and
[iOS](http://dev.chromium.org/developers/how-tos/build-instructions-ios)

# Building Sippet

## Install depot_tools

Follow the documentation from Chromium build: [Install
depot_tools](http://dev.chromium.org/developers/how-tos/install-depot-tools).


## Checkout the sources

### Windows

Create a directory to hold your source code. This example assumes `c:\sippet`,
but other names are fine.

**Important:** Make sure the full directory path has no spaces.

In a shell window, execute the following commands:

    cd c:\sippet
    gclient config --name=src git://github.com/sippet/sippet.git

If you intend to use Visual Studio 2013, do the following before the next
commands:

    set GYP_GENERATORS=ninja,msvs-ninja
    set GYP_MSVS_VERSION=2013

**Note:** The generator 'msvs' is not supported anymore, so stick to
'msvs-ninja', and don't forget to add depot_tools to your Visual Studio path
before executing it.

There is an extra environment variable to set, if you're not a Google developer
(otherwise I assume you know what to do at this point):

    set DEPOT_TOOLS_WIN_TOOLCHAIN=0

Now, to download the initial code, update your checkout as described in next
section.

### Linux

Pick a directory for your build.  We will call this directory `~/sippet` below.

Check out the code:

    cd ~/sippet
    gclient config --name=src git://github.com/sippet/sippet.git

To download the initial code, update your checkout as described in next
section.

### Mac OSX

Create a directory to hold the code.  This example assumes the directory is
`~/sippet`, but other names are fine.  Note that if you have FileVault enabled,
you'll want to either disable it or put the code in a folder outside the home
directory, as otherwise Xcode will be very slow or hang.  You also probably
want to disable Spotlight indexing for that folder (System Preferences ->
Spotlight -> mark the folder as private).

From a shell in the Terminal, execute the following commands: 

    cd ~/sippet
    gclient config --name=src git://github.com/sippet/sippet.git

To download the initial code, update your checkout as described in next section.


## Update to the latest revision

The first time you execute gclient, there will be a delay (a minute or so)
while it updates the depot tools.

In a shell window, execute the following commands:

    gclient sync

and the `DEPS` file will make sure you get the other directories in their
matching forms.

### Linux build requisites

Just run:

    ./src/build/install-build-deps.sh


## Create the build

The build is created using `gclient 'runhooks'` command:

    gclient runhooks

This will generate the build, usually for Ninja.

## Compile

Now that you have the build set up, just start compiling. Typically, on any
operating system, you would do just:

    ninja -C src/out/Release

Take a coffee. The output will be located in `src/out/Release`.


# Building 64-bit Sippet

## Linux

Run the following command prior to `gclient runhooks`:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES': 'target_arch=x64', }
    EOF

## Mac OS X

Run the following command prior to `gclient runhooks`:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES': 'target_arch=x64 host_arch=x64', }
    EOF

## Windows

Run the following command prior to `gclient runhooks`:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES': 'target_arch=x64', }
    EOF

Build either the `Release_x64` or `Debug_x64` target using ninja:

    ninja -C src\out\Debug_x64

