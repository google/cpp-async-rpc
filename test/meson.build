# *** Meson build configuration for ASH tests.
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

# This is a lot of machinery to automatically list test files
# at any subdirectory depth level and running them.

all_test_includes = [includes, third_party_includes, third_party_test_includes]

find_tests_output = run_command(find_program_path.path(), meson.current_source_dir(), '-name', '*_test.cpp', '-printf', '%P\n')
tests_list = find_tests_output.stdout().strip().split('\n')
foreach test_source : tests_list
  test_source_components = test_source.split('.')
  test_binary_name = test_source_components[0].underscorify()
  test_executable = executable(
    test_binary_name,
    test_source,
    include_directories : all_test_includes,
    cpp_args: [cpp_catch2_config_main_flag],
    link_args : '-lpthread',
    link_with: ash_library
  )
  test(test_binary_name, test_executable)
endforeach
