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

  # WebRTC repository for recovering the resources dir
  # Using the testfile32kHz.pcm for testing
  'webrtc_trunk': 'http://webrtc.googlecode.com/svn/trunk',
  'resources_revision': '2280',

  # Siphon G.729 codec was forked here 
  'siphon_trunk': 'http://siphon.googlecode.com/svn/trunk',
  'siphon_revision': '793',
}

# NOTE: Prefer revision numbers to tags for svn deps. Use http rather than
# https; the latter can cause problems for users behind proxies.
deps = {
  'src/third_party/pjsip/source':
    Var('pjsip_trunk') + '@' + Var('pjsip_revision'),

  'src/third_party/gflags/src':
    Var('chromium_git') + '/external/gflags/src@e7390f9185c75f8d902c05ed7d20bb94eb914d0c', # from svn revision 82

  'src/resources/g729a':
    Var('siphon_trunk') + '/g729a/Data@' + Var('siphon_revision'),

  'src/resources/audio_coding':
    Var('webrtc_trunk') + '/data/audio_coding@' + Var('resources_revision'),
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

