# This file contains dependencies for Sippet that are not shared with Chromium.
# If you wish to add a dependency that is present in Chromium's src/DEPS or a
# directory from the Chromium checkout, you should add it to setup_links.py
# instead.

vars = {
  'sippet_git': 'https://github.com/sippet',
  'chromium_git': 'https://chromium.googlesource.com',
  'oldfisher_git': 'https://github.com/OldFisher',
  'balena_git': 'https://github.com/balena',
  'googlei18n_git': 'https://github.com/googlei18n',
  'chromium_revision': 'b45a4a07e612794f91ff922968f40f3b48bfb7ae',  # 60.0.3112.97
  'pjsip_revision': '3dc02a36513e14d8b79139ddc353faa48dcc430b', # version 2.4.5
}

deps = {
  'src/third_party/gflags/src':
    (Var('chromium_git')) + '/external/gflags/src@e7390f9185c75f8d902c05ed7d20bb94eb914d0c', # from svn revision 82

  # RadixDB is used for some specific embedded databases
  'src/third_party/radixdb/source':
    (Var('balena_git')) + '/radixdb.git@' + Var('radixdb_revision'),

  # PJSIP is used for testing purposes
  'src/third_party/pjsip/source':
    (Var('sippet_git')) + '/pjproject.git@' + Var('pjsip_revision'),

  # This is part of the PJSIP build
  'src/third_party/speex':
    (Var('chromium_git')) + '/chromium/third_party/speex@45535c64629edeb9b53ec3d73c98dd4543b93956', # latest revision
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
]

