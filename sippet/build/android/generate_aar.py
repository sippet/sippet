#!/usr/bin/env python
#
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generates an AAR package a GN target."""

import argparse
import os
import fnmatch
import shutil
import shlex
import sys
import tempfile
import xml.dom.minidom
import zipfile

_BUILD_ANDROID = os.path.join(os.path.dirname(__file__),
        os.pardir, os.pardir, os.pardir, 'build', 'android')
sys.path.append(_BUILD_ANDROID)
import devil_chromium
from devil.utils import run_tests_helper
from pylib import constants
from pylib.constants import host_paths

sys.path.append(os.path.join(_BUILD_ANDROID, 'gyp'))
import jinja_template
from util import build_utils


def _ZipDir(input_dir, output):
  inputs = build_utils.FindInDirectory(input_dir, '*')
  with zipfile.ZipFile(output, 'w') as outfile:
    for f in inputs:
      outfile.write(f, os.path.relpath(f, input_dir))


def _ParseArgs(args):
  parser = argparse.ArgumentParser()
  parser.add_argument('--android-abi',
                      help='Android architecture to use for native libraries')
  parser.add_argument('--native-libs',
                      action='append',
                      help='GYP-list of native libraries to include. '
                           'Can be specified multiple times.',
                      default=[])
  parser.add_argument('--android-manifest',
                      help='AndroidManifest.xml file path')
  parser.add_argument('--R-txt',
                      help='R.txt file path')
  parser.add_argument('--jar-path',
                      help='Path to the main jar.')
  parser.add_argument('--extra-jar-libs',
                      action='append',
                      help='GYP-list of extra .jar libraries to include. '
                           'Can be specified multiple times.',
                      default=[])
  parser.add_argument('--aar-path', help='AAR package output path.')

  options = parser.parse_args(args)

  if not options.android_manifest:
    raise Exception('Must specify --android-manifest')
  elif not os.path.exists(options.android_manifest):
    raise Exception('Path passed to --android-manifest does not exist')

  if not options.R_txt:
    raise Exception('Must specify --R-txt')
  elif not os.path.exists(options.R_txt):
    raise Exception('Path passed to --R-txt does not exist')

  if not options.jar_path:
    raise Exception('Must specify --jar-path')
  elif not os.path.exists(options.jar_path):
    raise Exception('Path passed to --jar-path does not exist')

  if not options.aar_path:
    raise Exception('Must specify --aar-path')
  if not options.android_abi and options.native_libs:
    raise Exception('Must specify --android-abi with --native-libs')

  native_libs = []
  for gyp_list in options.native_libs:
    native_libs.extend(build_utils.ParseGnList(gyp_list))
  options.native_libs = native_libs

  extra_jar_libs = []
  for gyp_list in options.extra_jar_libs:
    extra_jar_libs.extend(build_utils.ParseGnList(gyp_list))
  options.extra_jar_libs = extra_jar_libs

  return options


def _InPatternList(file, patterns):
  for pattern in patterns:
    if fnmatch.fnmatch(file, pattern):
      return True
  return False


def main(args):
  args = build_utils.ExpandFileArgs(args)
  options = _ParseArgs(args)

  temp_dir = tempfile.mkdtemp()

  excluded_jars = [
    '**/support-annotations-*.jar',
    '**/support-vector-drawable-*.jar',
    '**/support-v4-*.jar',
    '**/appcompat-v7-*.jar',
    '**/design-*.jar',
    '**/support-fragment-*.jar',
    '**/support-compat-*.jar',
    '**/support-media-compat-*.jar',
    '**/animated-vector-drawable-*.jar',
    '**/transition-*.jar',
    '**/support-core-ui-*.jar',
    '**/support-core-utils-*.jar',
    '**/recyclerview-v7-*.jar',
    '**/jsr_305_javalib.jar',
  ]
  
  try:
    shutil.copy(options.android_manifest,
                os.path.join(temp_dir, 'AndroidManifest.xml'))
    shutil.copy(options.jar_path, os.path.join(temp_dir, 'classes.jar'))

    # Create an empty res/.keep file (this is mandatory)
    res_dir = os.path.join(temp_dir, 'res')
    os.mkdir(res_dir)
    open(os.path.join(res_dir, '.keep'), 'a').close()

    shutil.copy(options.R_txt, os.path.join(temp_dir, 'R.txt'))

    if options.extra_jar_libs:
      libs_dir = os.path.join(temp_dir, 'libs')
      os.mkdir(libs_dir)
      for input_jar in options.extra_jar_libs:
        if not _InPatternList(input_jar, excluded_jars):
          shutil.copy(input_jar, libs_dir)

    if options.native_libs:
      jni_dir = os.path.join(temp_dir, 'jni')
      os.mkdir(jni_dir)
      android_abi_dir = os.path.join(jni_dir, options.android_abi)
      os.mkdir(android_abi_dir)
      for native_lib in options.native_libs:
        shutil.copy(native_lib, android_abi_dir)

    _ZipDir(temp_dir, options.aar_path)

  finally:
    shutil.rmtree(temp_dir)


if __name__ == '__main__':
  main(sys.argv[1:])
