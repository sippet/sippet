# This file contains dependencies for Sippet that are not shared with Chromium.
# If you wish to add a dependency that is present in Chromium's src/DEPS or a
# directory from the Chromium checkout, you should add it to setup_links.py
# instead.

vars = {
  'extra_gyp_flag': '-Dextra_gyp_flag=0',
  'chromium_git': 'https://chromium.googlesource.com',
  'chromium_revision': '9070a8059b513108b09d30f96576b5ce11d0857a',

  # PJSIP is used for testing purposes
  'pjsip_trunk': 'http://svn.pjsip.org/repos/pjproject/trunk',
  'pjsip_revision': '4796',  # Corresponds to version 2.2.1

  # G.729 open source for testing purposes
  'g729_trunk': 'http://siphon.googlecode.com/svn/trunk/',
  'g729_revision': '793',  # Jan 14, 2015
}

# NOTE: Prefer revision numbers to tags for svn deps. Use http rather than
# https; the latter can cause problems for users behind proxies.
deps = {
  'src/third_party/pjsip/source':
    Var('pjsip_trunk') + '@' + Var('pjsip_revision'),

  'src/third_party/g729/source':
    Var('g729_trunk') + '/g729a/Sources@' + Var('g729_revision'),
  'src/third_party/g729/include':
    Var('g729_trunk') + '/g729a/Headers@' + Var('g729_revision'),
}

hooks = [
  {
    # Check for legacy named top-level dir (named 'trunk').
    'name': 'check_root_dir_name',
    'pattern': '.',
    'action': ['python','-c',
               ('import os,sys;'
                'script = os.path.join("trunk","check_root_dir.py");'
                '_ = os.system("%s %s" % (sys.executable,script)) '
                'if os.path.exists(script) else 0')],
  },
  {
    # Clone chromium and its deps.
    'name': 'sync chromium',
    'pattern': '.',
    'action': ['python', '-u', 'src/sync_chromium.py',
               '--target-revision', Var('chromium_revision')],
  },
  {
    # Create links to shared dependencies in Chromium.
    'name': 'setup_links',
    'pattern': '.',
    'action': ['python', 'src/setup_links.py'],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    'name': 'gyp',
    'pattern': '.',
    'action': ['python', 'src/sippet/build/gyp_sippet',
               '-Dinclude_internal_audio_device=1',
               Var('extra_gyp_flag')],
  },
]

