# *** Top-level Meson build configuration for cpp-async-rpc.
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

project('arpc', 'cpp', default_options : ['cpp_std=c++17'], version : '0.0.1')

# Third party includes.
third_party_includes = include_directories('third_party/function2/include')
third_party_test_includes = include_directories('third_party/catch2/single_include')

# Global configuration data.
config_data = configuration_data()
config_data.set('VERSION', meson.project_version())
config_data.set('TOP_SRCDIR', meson.source_root())
config_data.set('TOP_BUILDDIR', meson.build_root())

# Determine compiler flags.
cpp_compiler = meson.get_compiler('cpp')
cpp_no_rtti_flag = cpp_compiler.first_supported_argument(['-fno-rtti', '/GR'])
cpp_catch2_config_main_flag = cpp_compiler.first_supported_argument(['-DCATCH_CONFIG_MAIN', '/DCATCH_CONFIG_MAIN'])
add_project_arguments(cpp_no_rtti_flag, language : 'cpp')
add_project_link_arguments(cpp_no_rtti_flag, language : 'cpp')

# For reference in the docs target.
readme_file = files('README.md')
license_file = files('LICENSE.md')

# Enter the project subdirectories.
subdir('src')
subdir('examples')
subdir('test')
subdir('doc')

# Declare this project as a dependency for others to use.
proj_dep = declare_dependency(include_directories : includes, link_with : arpc_library)
