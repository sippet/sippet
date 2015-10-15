---
layout: page
title: Build for iOS
description: Build the Sippet library for iOS
---

**NOTE:** this guide is closely related to the original [Build Instructions
(iOS)](https://www.chromium.org/developers/how-tos/build-instructions-ios) and
[WebRTC build for iOS](https://sites.google.com/site/webrtc/native-code/ios?),
from Google Chromium and WebRTC respectively.

# Prerequisites

An OS X machine is required for iOS development.

The following sections assume that you already have configured gclient as
documented in [Compile for desktop platforms](/how-to-compile/).

# Get the code

After configuring gclient, you'll likely need to do:

    echo "target_os = ['ios']" >> .gclient
    gclient sync


# Configure GYP

GYP is used to generate build instructions for ninja from the relevant .gyp
files. Ninja is used to compile the source using the previously generated
instructions. In order to configure GYP to generate build files for iOS certain
environment variables need to be set. Those variables can be edited for the
various build configurations as needed.

Building for ARMv7:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES':
        'OS=ios target_arch=arm arm_version=7 target_subarch=arm32 enable_webrtc=1 chromium_ios_signing=0',
      'GYP_GENERATORS':
        'ninja'
    }
    EOF

Building for ARM64:

    cat << EOF > sippet.gyp_env
    { 'GYP_DEFINES':
        'OS=ios target_arch=arm64 target_subarch=arm64 enable_webrtc=1 chromium_ios_signing=0',
      'GYP_GENERATORS':
        'ninja'
    }
    EOF

**NOTE:** If you are using the `GYP_DEFINES` environment variable, it will
override any settings in this file. Either clear it or set it to the values
above before running gclient runhooks.

Once `sippet.gyp_env` is ready, you need to run the following commands to
update projects from gyp files. You may need to run this again when you have
added new files, updated gyp files, or sync'ed your repository.

    gclient runhooks


# Build

The following commands will generate all needed modules for your application,
depending on which target you need.

Building for iPhone device:

    ninja -C out/Release-iphoneos all

Building for iPhone simulator:

    ninja -C out/Release-iphonesimulator all

Replace 'Release' with 'Debug' above if you want to generate a Debug library.
