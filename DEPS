use_relative_paths = True

vars = {
  # Override root_dir in your .gclient's custom_vars to specify a custom root
  # folder name.
  "root_dir": "trunk",
  "extra_gyp_flag": "-Dextra_gyp_flag=0",

  # Use this googlecode_url variable only if there is an internal mirror for it.
  # If you do not know, use the full path while defining your new deps entry.
  "googlecode_url": "http://%s.googlecode.com/svn",
  "sourceforge_url": "http://%(repo)s.svn.sourceforge.net/svnroot/%(repo)s",
  "chromium_trunk": "http://src.chromium.org/chrome/trunk",

  "chromium_revision": "272606",
}

# NOTE: Prefer revision numbers to tags for svn deps. Use http rather than
# https; the latter can cause problems for users behind proxies.
deps = {
  "../chromium_deps":
    File(Var("chromium_trunk") + "/src/DEPS@" + Var("chromium_revision")),

  "build":
    Var("chromium_trunk") + "/src/build@" + Var("chromium_revision"),

  # Needed by common.gypi.
  "google_apis/build":
    Var("chromium_trunk") + "/src/google_apis/build@" + Var("chromium_revision"),

  "base":
    Var("chromium_trunk") + "/src/base@" + Var("chromium_revision"),

  "crypto":
    Var("chromium_trunk") + "/src/crypto@" + Var("chromium_revision"),

  "dbus":
    Var("chromium_trunk") + "/src/dbus@" + Var("chromium_revision"),

  "net":
    Var("chromium_trunk") + "/src/net@" + Var("chromium_revision"),

  "sdch":
    Var("chromium_trunk") + "/src/sdch@" + Var("chromium_revision"),

  "sdch/open-vcdiff":
    From("chromium_deps", "src/sdch/open-vcdiff"),

  "v8":
    From("chromium_deps", "src/v8"),

  "gin":
    Var("chromium_trunk") + "/src/gin@" + Var("chromium_revision"),

  "url":
    Var("chromium_trunk") + "/src/url@" + Var("chromium_revision"),

  "chrome/tools/build":
    Var("chromium_trunk") + "/src/chrome/tools/build@" + Var("chromium_revision"),

  "chrome":
    File(Var("chromium_trunk") + "/src/chrome/VERSION@" + Var("chromium_revision")),

  "testing":
    Var("chromium_trunk") + "/src/testing@" + Var("chromium_revision"),

  "testing/gtest":
    From("chromium_deps", "src/testing/gtest"),

  "testing/gmock":
    From("chromium_deps", "src/testing/gmock"),

  "third_party/libjingle/source/talk":
    From("chromium_deps", "src/third_party/libjingle/source/talk"),

  "third_party/icu":
    From("chromium_deps", "src/third_party/icu"),

  "third_party/sqlite":
    Var("chromium_trunk") + "/src/third_party/sqlite@" + Var("chromium_revision"),

  "third_party/modp_b64":
    Var("chromium_trunk") + "/src/third_party/modp_b64@" + Var("chromium_revision"),

  "third_party/tcmalloc":
    Var("chromium_trunk") + "/src/third_party/tcmalloc@" + Var("chromium_revision"),

  "third_party/jemalloc":
    Var("chromium_trunk") + "/src/third_party/jemalloc@" + Var("chromium_revision"),

  "third_party/protobuf":
    Var("chromium_trunk") + "/src/third_party/protobuf@" + Var("chromium_revision"),

  "third_party/zlib":
    Var("chromium_trunk") + "/src/third_party/zlib@" + Var("chromium_revision"),

  "third_party/libxml":
    Var("chromium_trunk") + "/src/third_party/libxml@" + Var("chromium_revision"),

  "third_party/openssl":
    From("chromium_deps", "src/third_party/openssl"),

  "third_party/clang_format":
    Var("chromium_trunk") + "/src/third_party/clang_format@" + Var("chromium_revision"),

  "third_party/clang_format/script":
    From("chromium_deps", "src/third_party/clang_format/script"),

  "tools/clang":
    Var("chromium_trunk") + "/src/tools/clang@" + Var("chromium_revision"),

  "tools/gyp":
    From("chromium_deps", "src/tools/gyp"),

  "tools/python":
    Var("chromium_trunk") + "/src/tools/python@" + Var("chromium_revision"),

  "tools/valgrind":
    Var("chromium_trunk") + "/src/tools/valgrind@" + Var("chromium_revision"),

  "tools/grit":
    From("chromium_deps", "src/tools/grit"),

  "tools/gritsettings":
    Var("chromium_trunk") + "/src/tools/gritsettings@" + Var("chromium_revision"),

  "tools/gn":
    Var("chromium_trunk") + "/src/tools/gn@" + Var("chromium_revision"),

  "tools/protoc_wrapper":
    Var("chromium_trunk") + "/src/tools/protoc_wrapper@" + Var("chromium_revision"),

  # Needed by build/common.gypi.
  "tools/win/supalink":
    Var("chromium_trunk") + "/src/tools/win/supalink@" + Var("chromium_revision"),
}


deps_os = {
  "win": {
    "third_party/cygwin":
      (Var("googlecode_url") % "webrtc") + "/deps/third_party/cygwin@231940",

    # NSS, for SSLClientSocketNSS.
    "third_party/nss":
      From("chromium_deps", "src/third_party/nss"),
  },

  "mac": {
    # NSS, for SSLClientSocketNSS.
    "third_party/nss":
      From("chromium_deps", "src/third_party/nss"),
  },

  "unix": {
    "third_party/openssl":
      From("chromium_deps", "src/third_party/openssl"),

    "third_party/libevent":
      Var("chromium_trunk") + "/src/third_party/libevent@" + Var("chromium_revision"),

    "tools/xdisplaycheck":
      Var("chromium_trunk") + "/src/tools/xdisplaycheck@" + Var("chromium_revision"),

    "tools/generate_library_loader":
      Var("chromium_trunk") + "/src/tools/generate_library_loader@" + Var("chromium_revision"),
  },

  "android": {
    "third_party/android_tools":
      From("chromium_deps", "src/third_party/android_tools"),

    "third_party/openssl":
      From("chromium_deps", "src/third_party/openssl"),
  },
}


include_rules = [
  "+base",
  "+build",

  "+testing",
  "+third_party/icu/source/common/unicode",
  "+third_party/icu/source/i18n/unicode",
  "+url",
]


hooks = [
  {
    # This downloads binaries for Native Client's newlib toolchain.
    # Done in lieu of building the toolchain from scratch as it can take
    # anywhere from 30 minutes to 4 hours depending on platform to build.
    "name": "nacltools",
    "pattern": ".",
    "action": [
        "python", "trunk/build/download_nacl_toolchains.py",
        "--exclude", "arm_trusted",
    ],
  },
  {
    # Downloads an ARM sysroot image to trunk/arm-sysroot. This image updates
    # at about the same rate that the chrome build deps change.
    # This script is a no-op except for linux users who have
    # target_arch=arm in their GYP_DEFINES.
    "name": "sysroot",
    "pattern": ".",
    "action": ["python", "trunk/build/linux/install-arm-sysroot.py",
               "--linux-only"],
  },
  {
    # Pull clang if on Mac or clang is requested via GYP_DEFINES.
    "name": "clang",
    "pattern": ".",
    "action": ["python", "trunk/tools/clang/scripts/update.py", "--if-needed"],
  },
  {
    # Update the Windows toolchain if necessary.
    "name": "win_toolchain",
    "pattern": ".",
    "action": ["python", "trunk/build/vs_toolchain.py", "update"],
  },
  {
    # Update LASTCHANGE. This is also run by export_tarball.py in
    # trunk/tools/export_tarball - please keep them in sync.
    "name": "lastchange",
    "pattern": ".",
    "action": ["python", "trunk/build/util/lastchange.py",
               "-o", "trunk/build/util/LASTCHANGE"],
  },
  {
    # Update LASTCHANGE.blink. This is also run by export_tarball.py in
    # trunk/tools/export_tarball - please keep them in sync.
    "name": "lastchange",
    "pattern": ".",
    "action": ["python", "trunk/build/util/lastchange.py",
               "-s", "trunk/third_party/WebKit",
               "-o", "trunk/build/util/LASTCHANGE.blink"],
  },
  # Pull GN binaries. This needs to be before running GYP below.
  {
    "name": "gn_win",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=win32",
                "--no_auth",
                "--bucket", "chromium-gn",
                "-s", "trunk/tools/gn/bin/win/gn.exe.sha1",
    ],
  },
  {
    "name": "gn_mac",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=darwin",
                "--no_auth",
                "--bucket", "chromium-gn",
                "-s", "trunk/tools/gn/bin/mac/gn.sha1",
    ],
  },
  {
    "name": "gn_linux",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=linux*",
                "--no_auth",
                "--bucket", "chromium-gn",
                "-s", "trunk/tools/gn/bin/linux/gn.sha1",
    ],
  },
  {
    "name": "gn_linux32",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=linux*",
                "--no_auth",
                "--bucket", "chromium-gn",
                "-s", "trunk/tools/gn/bin/linux/gn32.sha1",
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    "name": "clang_format_win",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=win32",
                "--no_auth",
                "--bucket", "chromium-clang-format",
                "-s", "trunk/third_party/clang_format/bin/win/clang-format.exe.sha1",
    ],
  },
  {
    "name": "clang_format_mac",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=darwin",
                "--no_auth",
                "--bucket", "chromium-clang-format",
                "-s", "trunk/third_party/clang_format/bin/mac/clang-format.sha1",
    ],
  },
  {
    "name": "clang_format_linux",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=linux*",
                "--no_auth",
                "--bucket", "chromium-clang-format",
                "-s", "trunk/third_party/clang_format/bin/linux/clang-format.sha1",
    ],
  },
  # Pull eu-strip binaries using checked-in hashes.
  {
    "name": "eu-strip",
    "pattern": ".",
    "action": [ "download_from_google_storage",
                "--no_resume",
                "--platform=linux*",
                "--no_auth",
                "--bucket", "chromium-eu-strip",
                "-s", "trunk/build/linux/bin/eu-strip.sha1",
    ],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/gyp_chromium",
               "--depth=" + Var("root_dir"),
               Var("root_dir") + "/sippet/all.gyp", Var("extra_gyp_flag")],
  },
]

