solutions = [{
  'name': 'src',
  'url': 'https://chromium.googlesource.com/chromium/src.git',
  'deps_file': '.DEPS.git',
  'managed': False,
  'custom_deps': {
    # Skip syncing some large dependencies Sippet will never need.
    'src/chrome/tools/test/reference_build/chrome_linux': None,
    'src/chrome/tools/test/reference_build/chrome_mac': None,
    'src/chrome/tools/test/reference_build/chrome_win': None,
    'src/native_client': None,
    'src/third_party/ffmpeg': None,
    'src/third_party/junit/src': None,
    'src/third_party/WebKit': None,
  },
  'safesync_url': ''
}]

cache_dir = None
