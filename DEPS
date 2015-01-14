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

  "chromium_revision": "238260",

  # PJSIP is used for testing purposes
  "pjsip_trunk": "http://svn.pjsip.org/repos/pjproject/trunk",
  "pjsip_revision": "4796",  # Corresponds to version 2.2.1

  "openssl_revision": "236537",
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

  "url":
    Var("chromium_trunk") + "/src/url@" + Var("chromium_revision"),

  "chrome/tools/build":
    Var("chromium_trunk") + "/src/chrome/tools/build@" + Var("chromium_revision"),

  "testing":
    Var("chromium_trunk") + "/src/testing@" + Var("chromium_revision"),

  "testing/gtest":
    From("chromium_deps", "src/testing/gtest"),

  "testing/gmock":
    From("chromium_deps", "src/testing/gmock"),

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

  "third_party/pjsip/source":
    Var("pjsip_trunk") + "@" + Var("pjsip_revision"),

  "third_party/openssl/openssl":
    Var("chromium_trunk") + "/deps/third_party/openssl/openssl@" + Var("openssl_revision"),

  "third_party/speex":
    From("chromium_deps", "src/third_party/speex"),

  "tools":
    File(Var("chromium_trunk") + "/src/tools/find_depot_tools.py@" + Var("chromium_revision")),

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
      From("chromium_deps", "src/third_party/cygwin"),

    # NSS, for SSLClientSocketNSS.
    "third_party/nss":
      From("chromium_deps", "src/third_party/nss"),
  },

  "mac": {
    # NSS, for SSLClientSocketNSS.
    "third_party/nss":
      From("chromium_deps", "src/third_party/nss"),

    "third_party/libevent":
      Var("chromium_trunk") + "/src/third_party/libevent@" + Var("chromium_revision"),

    "third_party/mach_override":
      Var("chromium_trunk") + "/src/third_party/mach_override@" + Var("chromium_revision"),

    "third_party/apple_apsl":
      Var("chromium_trunk") + "/src/third_party/apple_apsl@" + Var("chromium_revision"),

    "ipc":
      Var("chromium_trunk") + "/src/ipc@" + Var("chromium_revision"),
  },

  "ios": {
    # NSS, for SSLClientSocketNSS.
    "third_party/nss":
      From("chromium_deps", "src/third_party/nss"),

    "net/third_party/nss":
      Var("chromium_trunk") + "/src/net/third_party/nss@" + Var("chromium_revision"),

    # class-dump utility to generate header files for undocumented SDKs.
    "testing/iossim/third_party/class-dump":
      From("chromium_deps", "src/testing/iossim/third_party/class-dump"),

    # Helper for running under the simulator.
    "testing/iossim":
      Var("chromium_trunk") + "/src/testing/iossim@" + Var("chromium_revision"),
  },

  "unix": {
    "third_party/gold":
      From("chromium_deps", "src/third_party/gold"),
  },

  "android": {
    "tools/android":
      Var("chromium_trunk") + "/src/tools/android@" + Var("chromium_revision"),

    "third_party/libevent":
      Var("chromium_trunk") + "/src/third_party/libevent@" + Var("chromium_revision"),

    "third_party/ashmem":
      Var("chromium_trunk") + "/src/third_party/ashmem@" + Var("chromium_revision"),

    "third_party/jsr-305":
      Var("chromium_trunk") + "/src/third_party/jsr-305@" + Var("chromium_revision"),

    "third_party/jsr-305/src":
      From("chromium_deps", "src/third_party/jsr-305/src"),

    "third_party/android_tools":
      From("chromium_deps", "src/third_party/android_tools"),

    "third_party/android_testrunner":
      Var("chromium_trunk") + "/src/third_party/android_testrunner@" + Var("chromium_revision"),

    "ipc":
      Var("chromium_trunk") + "/src/ipc@" + Var("chromium_revision"),
  },
}


hooks = [
  {
    # Pull clang on mac. If nothing changed, or on non-mac platforms, this takes
    # zero seconds to run. If something changed, it downloads a prebuilt clang.
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/tools/clang/scripts/update.py",
               "--mac-only"],
  },
  {
    # Update the cygwin mount on Windows.
    # This is necessary to get the correct mapping between e.g. /bin and the
    # cygwin path on Windows. Without it we can't run bash scripts in actions.
    # Ideally this should be solved in "pylib/gyp/msvs_emulation.py".
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/win/setup_cygwin_mount.py",
               "--win-only"],
  },
  {
    # Update LASTCHANGE. This is also run by export_tarball.py in
    # src/tools/export_tarball - please keep them in sync.
    "name": "lastchange",
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/util/lastchange.py",
               "-s", Var("root_dir") + "/sippet",
               "-o", Var("root_dir") + "/build/util/LASTCHANGE"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/gyp_chromium",
               "--depth=" + Var("root_dir"),
               Var("root_dir") + "/sippet/all.gyp", Var("extra_gyp_flag")],
  },
]

