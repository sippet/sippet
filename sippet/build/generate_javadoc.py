#!/usr/bin/env python
#
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import fnmatch
import optparse
import os
import sys

REPOSITORY_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), os.pardir, os.pardir))

sys.path.append(os.path.join(REPOSITORY_ROOT, 'build', 'android', 'gyp'))
from util import build_utils


def GenerateJavadoc(options, args):
  source_dir = options.source_dir
  output_dir = options.output_dir

  build_utils.DeleteDirectory(output_dir)
  build_utils.MakeDirectory(output_dir)
  javadoc_cmd = ['javadoc', '-sourcepath', source_dir,
             '-d', os.path.abspath(output_dir) ] + args
  build_utils.CheckOutput(javadoc_cmd)


def main():
  parser = optparse.OptionParser()
  parser.add_option('--source-dir', help='Source directory')
  parser.add_option('--output-dir', help='Directory to put javadoc')

  options, args = parser.parse_args()

  GenerateJavadoc(options, args)

if __name__ == '__main__':
  sys.exit(main())

