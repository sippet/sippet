#!/usr/bin/env python
#
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import shutil
import shlex
import sys
import tempfile
import xml.dom.minidom
import zipfile

script_dir = os.path.dirname(os.path.realpath(__file__))
checkout_root = os.path.abspath(os.path.join(script_dir, os.pardir, os.pardir, os.pardir, os.pardir))

sys.path.insert(0, os.path.join(checkout_root, 'build', 'android', 'gyp'))
from util import build_utils

def ZipDir(input_dir, output):
  inputs = build_utils.FindInDirectory(input_dir, '*')
  with zipfile.ZipFile(output, 'w') as outfile:
    for f in inputs:
      outfile.write(f, os.path.relpath(f, input_dir))

def MakeEmptyDir(d):
  if os.path.exists(d):
    shutil.rmtree(d)
  os.mkdir(d)

def main():
  parser = optparse.OptionParser()

  parser.add_option('--aar-path', help='Aar output path.')
  parser.add_option('--out-dir', help='Output directory.')
  parser.add_option('--jar-path', help='Source jar.')
  parser.add_option('--resource-dir', help='Resource directory.')
  parser.add_option('--native-libs-dir', help='Native library directory.')
  parser.add_option('--android-manifest-path', help='Android manifest path.')
  parser.add_option('--asset-dir', help='Asset directory.')
  parser.add_option('--input-jars-paths', help='Input jar paths.')
  parser.add_option('--additional-resource-arr-paths',
      help='Resource aar dependencies.')

  options, _ = parser.parse_args()
  r_text_path = os.path.join(options.out_dir, 'gen', 'R.txt')

  resource_aar_dir = os.path.join(options.out_dir, "resource_aar")
  MakeEmptyDir(resource_aar_dir)
  for resource_aar in shlex.split(options.additional_resource_arr_paths):
    shutil.copy(resource_aar, resource_aar_dir)

  temp_dir = tempfile.mkdtemp()

  try:
    if os.path.exists(r_text_path):
      shutil.copy(r_text_path, temp_dir)
    else:
      # Create an empty R.txt file if there's none
      open(os.path.join(temp_dir, 'R.txt'), 'a').close()
    shutil.copy(options.jar_path, os.path.join(temp_dir, 'classes.jar'))
    shutil.copytree(options.resource_dir, os.path.join(temp_dir, 'res'))
    if os.path.exists(options.native_libs_dir):
      shutil.copytree(options.native_libs_dir, os.path.join(temp_dir, 'jni'))
    if os.path.exists(options.asset_dir):
      shutil.copytree(options.asset_dir, os.path.join(temp_dir, 'assets'))
    libs_dir = os.path.join(temp_dir, 'libs')
    os.mkdir(libs_dir)

    for input_jar in shlex.split(options.input_jars_paths):
      shutil.copy(input_jar, libs_dir)

    manifest_dom = xml.dom.minidom.parse(options.android_manifest_path)
    manifest_node = manifest_dom.getElementsByTagName("manifest")[0]
    package = manifest_node.getAttribute("package")

    # Generate a stripped down AndroidManifest in the aar file so that users
    # of the aar file don't get unexpected services or activities running
    # as part of their applications.
    with open(os.path.join(temp_dir, 'AndroidManifest.xml'), 'w') as outfile:
      outfile.write("""<?xml version="1.0" encoding="utf-8"?>
<!--
  Copyright (c) 2014 The Chromium Authors. All rights reserved.  Use of this
  source code is governed by a BSD-style license that can be found in the
  LICENSE file.
-->

<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="%s">

    <uses-sdk android:minSdkVersion="14" android:targetSdkVersion="19" />

</manifest>
""" % (package))

    ZipDir(temp_dir, options.aar_path)

  finally:
    shutil.rmtree(temp_dir)

if __name__ == '__main__':
  sys.exit(main())

