# This file contains dependencies for Sippet that are not shared with Chromium.
# If you wish to add a dependency that is present in Chromium's src/DEPS or a
# directory from the Chromium checkout, you should add it to setup_links.py
# instead.

vars = {
  'extra_gyp_flag': '-Dextra_gyp_flag=0',
  'chromium_git': 'https://chromium.googlesource.com',
  'chromium_revision': 'fad7fb9e0b0f1a29b71e45f27c007ecbf4870498', # 40.0.2214.93

  # PJSIP is used for testing purposes
  'pjsip_trunk': 'http://svn.pjsip.org/repos/pjproject/trunk',
  'pjsip_revision': '4796',  # Corresponds to version 2.2.1
}

# NOTE: Prefer revision numbers to tags for svn deps. Use http rather than
# https; the latter can cause problems for users behind proxies.
deps = {
  'src/third_party/pjsip/source':
    Var('pjsip_trunk') + '@' + Var('pjsip_revision'),
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
    "pattern": ".",
    "action": ["python", Var("root_dir") + "/build/gyp_sippet",
               "--depth=" + Var("root_dir"),
               Var("root_dir") + "/sippet/all.gyp", Var("extra_gyp_flag")],
  },
]

