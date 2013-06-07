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

  "chromium_revision": "187216",
  "webrtc_revision": "3561",
  "libjingle_revision": "321",
  "v8_revision": "13671",
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

  "googleurl":
    (Var("googlecode_url") % "google-url") + "/trunk@183",

  "crypto":
    Var("chromium_trunk") + "/src/crypto@" + Var("chromium_revision"),

  "net":
    Var("chromium_trunk") + "/src/net@" + Var("chromium_revision"),

  "sdch":
    Var("chromium_trunk") + "/src/sdch@" + Var("chromium_revision"),

  "sdch/open-vcdiff":
    (Var("googlecode_url") % "open-vcdiff") + "/trunk@42",

  "v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),

  "chrome/app/policy":
    Var("chromium_trunk") + "/src/chrome/app/policy@" + Var("chromium_revision"),

  "chrome/browser/policy/proto":
    Var("chromium_trunk") + "/src/chrome/browser/policy/proto@" + Var("chromium_revision"),

  "chrome/tools/build":
    Var("chromium_trunk") + "/src/chrome/tools/build@" + Var("chromium_revision"),

  "testing":
    Var("chromium_trunk") + "/src/testing@" + Var("chromium_revision"),

  "testing/gtest":
    (Var("googlecode_url") % "googletest") + "/trunk@629",

  "testing/gmock":
    (Var("googlecode_url") % "googlemock") + "/trunk@410",

  "third_party/libjingle/source":
    (Var("googlecode_url") % "libjingle") + "/trunk@" + Var("libjingle_revision"),

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

  "tools/clang":
    Var("chromium_trunk") + "/src/tools/clang@" + Var("chromium_revision"),

  "tools/gyp":
    From("chromium_deps", "src/tools/gyp"),

  "tools/python":
    Var("chromium_trunk") + "/src/tools/python@" + Var("chromium_revision"),

  "tools/valgrind":
    Var("chromium_trunk") + "/src/tools/valgrind@" + Var("chromium_revision"),

  "tools/grit":
    (Var("googlecode_url") % "grit-i18n") + "/trunk@107",

  "tools/gritsettings":
    Var("chromium_trunk") + "/src/tools/gritsettings@" + Var("chromium_revision"),

  "tools/protoc_wrapper":
    Var("chromium_trunk") + "/src/tools/protoc_wrapper@" + Var("chromium_revision"),

  # Needed by build/common.gypi.
  "tools/win/supalink":
    Var("chromium_trunk") + "/src/tools/win/supalink@" + Var("chromium_revision"),
}


deps_os = {
  "win": {
    # Use our own, stripped down, version of Cygwin (required by GYP).
    "third_party/cygwin":
      (Var("googlecode_url") % "webrtc") + "/deps/third_party/cygwin@2672",

    "third_party/winsdk_samples":
      (Var("googlecode_url") % "webrtc") + "/stable/third_party/winsdk_samples@" + Var("webrtc_revision"),

    "third_party/winsdk_samples/src":
      (Var("googlecode_url") % "webrtc") + "/deps/third_party/winsdk_samples_v71@3145",

    "tools/win/toolchain":
      Var("chromium_trunk") + "/src/tools/win/toolchain@" + Var("chromium_revision"),

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
    "third_party/gold":
      From("chromium_deps", "src/third_party/gold"),

    "third_party/openssl":
      From("chromium_deps", "src/third_party/openssl"),
  },

  "android": {
    "third_party/android_tools":
      From("chromium_deps", "src/third_party/android_tools"),

    "third_party/openssl":
      From("chromium_deps", "src/third_party/openssl"),
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
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/gyp_chromium",
               "--depth=" + Var("root_dir"),
               Var("root_dir") + "/sippet/all.gyp", Var("extra_gyp_flag")],
  },
]

