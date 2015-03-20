---
layout: page
title: Android build instructions
---

**NOTE:** this guide is closely related to the original [Android Build
Instructions](https://code.google.com/p/chromium/wiki/AndroidBuildInstructions),
from Google Chromium.

# Prerequisites

A **Linux 64-bit build machine** (Ubuntu 64-bit is recommended) capable of
building Sippet for Linux. Other (Mac/Windows) platforms are not supported for
Android.

The following sections assume that you already have configured gclient as
documented in [Compile for desktop platforms](/how-to-compile/).

# Get the code

After configuring gclient, you'll likely need to do:

    echo "target_os = ['android']" >> .gclient
    gclient sync


# Configure GYP

GYP is the meta-makefile system used in Sippet to generate build files for the
various platforms (ninja in the case of Android). Next to the `.gclient` file,
create a file called `sippet.gyp_env`:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES': 'OS=android', }
    EOF

If you wish to build for x86 targets:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES': 'OS=android target_arch=ia32', }
    EOF

**NOTE:** If you are using the `GYP_DEFINES` environment variable, it will
override any settings in this file. Either clear it or set it to the values
above before running gclient runhooks.

Once `sippet.gyp_env` is ready, you need to run the following commands to
update projects from gyp files. You may need to run this again when you have
added new files, updated gyp files, or sync'ed your repository.

    . src/build/android/envsetup.sh
    gclient runhooks


# Install Java JDK

You should be able to build Sippet for Android with Open JDK 1.6 or 1.7. By the
way, it's recommended to use **Oracle JDK 1.8**.

On Ubuntu, do the following:

    sudo add-apt-repository ppa:webupd8team/java
    sudo apt-get update
    sudo apt-get install oracle-java8-installer

And follow screen instructions.


# Install build dependencies

Update the system packages required to build by running:

    ~/sippet$ src/build/install-build-deps-android.sh


# Build and install the APKs

Unfortunately, at the moment, there's no full support to a fully featured build
of the library (WIP). However, most of the code can be compiled into static
libraries (WIP).


## Build the library

The following command will generate all needed modules for your application:

    ninja -C out/Release all

Replace 'Release' with 'Debug' above if you want to generate a Debug library.

If you use custom out dir instead of standard `out/` dir, use
`CHROMIUM_OUT_DIR` env.

    export CHROMIUM_OUT_DIR=out_android

# TO-DO

Document the Java API of the Sippet library.
