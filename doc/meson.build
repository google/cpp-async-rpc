# *** Meson build configuration for the ASH Doxygen documentation.
#
# Copyright 2019 by Google LLC. All Rights Reserved.
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

doxygen = find_program('doxygen', required : true)
doxyfile = configure_file(input : 'Doxyfile.in', output : 'Doxyfile', configuration : config_data, install : false)
docs_install_dir = join_paths(get_option('datadir'), 'doc', 'ash')
html_target = custom_target('docs', build_by_default : false,
                            input : [doxyfile, readme_file, license_file, all_includes], output : 'html',
                            command : [doxygen, doxyfile], install : true, install_dir : docs_install_dir)
