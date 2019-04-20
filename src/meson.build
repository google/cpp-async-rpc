# *** Meson build configuration for lasr main sources.
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

includes = include_directories('.')

all_src_includes = [includes, third_party_includes]

find_program_path = find_program('find') 

# This is a lot of machinery to automatically list header files
# at any subdirectory depth level (so that they can be listed as
# Doxygen's dependencies) and installing them at the matching
# subdirectory.

find_includes_output = run_command(find_program_path.path(), meson.current_source_dir(), '-name', '*.h', '-printf', '%P\n')
includes_list = find_includes_output.stdout().strip().split('\n')
all_includes = []
foreach include : includes_list
  include_file = files(include)
  all_includes += include_file
  include_components = include.split('/')
  subdir_components = []
  foreach path_component : include_components
    if subdir_components.length() < include_components.length() - 1
      subdir_components += path_component
    endif
  endforeach
  include_subdir = ''.join(subdir_components)
  install_headers(include_file, subdir : include_subdir)
endforeach


# Find all source files to be linked into a library.

find_sources_output = run_command(find_program_path.path(), meson.current_source_dir(), '-name', '*.cpp', '-printf', '%P\n')
sources_list = find_sources_output.stdout().strip().split('\n')

lasr_library = static_library('lasr', sources_list, include_directories : all_src_includes)
