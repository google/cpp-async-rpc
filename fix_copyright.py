#! /usr/bin/python3
# *** Tool to fix copyright headers in source files.
#
# Copyright 2019 by Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain a
# copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

import glob

CPP_HEADER_PREFIX = """///"""

CPP_HEADER = """\
/// \\file
/// \\brief {description}
///
/// \\copyright
///   Copyright 2019 by Google LLC.
///
/// \\copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \\copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \\copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.
"""

CPP_DESCRIPTION_PREFIX = """/// \\brief """

PY_HEADER_PREFIX = """#"""

PY_HEADER = """\
# *** {description}
#
# Copyright 2019 by Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain a
# copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
"""

PY_DESCRIPTION_PREFIX = """# *** """

EXCLUDES = ('third_party/', 'builddir/')

TRANSFORMS = (
    ('**/*.py', PY_HEADER_PREFIX, PY_DESCRIPTION_PREFIX, PY_HEADER),
    ('**/meson.build', PY_HEADER_PREFIX, PY_DESCRIPTION_PREFIX, PY_HEADER),
    ('**/component.mk', PY_HEADER_PREFIX, PY_DESCRIPTION_PREFIX, PY_HEADER),
    ('**/*.h', CPP_HEADER_PREFIX, CPP_DESCRIPTION_PREFIX, CPP_HEADER),
    ('**/*.c', CPP_HEADER_PREFIX, CPP_DESCRIPTION_PREFIX, CPP_HEADER),
    ('**/*.hpp', CPP_HEADER_PREFIX, CPP_DESCRIPTION_PREFIX, CPP_HEADER),
    ('**/*.cpp', CPP_HEADER_PREFIX, CPP_DESCRIPTION_PREFIX, CPP_HEADER),
)


def process_file(path, header_prefix, description_prefix, header):
    text = [l.rstrip('\n') for l in open(path, 'rt').readlines()]

    if len(text) > 0 and text[0].startswith('#!'):
        prefix = text[0] + '\n'
        text = text[1:]
    else:
        prefix = ""

    description = "NEEDS_DESCRIPTION"
    i = 0
    while i < len(text) and text[i].startswith(header_prefix):
        if text[i].startswith(description_prefix):
            description = text[i][len(description_prefix):]
        i += 1

    result = prefix + header.format(
        description=description) + '\n'.join(text[i:]) + '\n'
    open(path, 'wt').write(result)


for pattern, header_prefix, description_prefix, header in TRANSFORMS:
    for path in glob.iglob(pattern, recursive=True):
        for exclude in EXCLUDES:
            if path.startswith(exclude):
                break
        else:
            process_file(path, header_prefix, description_prefix, header)
