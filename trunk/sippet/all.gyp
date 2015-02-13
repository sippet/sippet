# Copyright (c) 2014 The Sippet Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'All',
      'type': 'none',
      'dependencies': [
        'sippet.gyp:*',
      ],
      'conditions': [
        ['OS != "ios"', {
          'dependencies': [
            'sippet_tests.gyp:*',
            'sippet_examples.gyp:*',
          ],
        }],
      ],
    },
  ],
}
