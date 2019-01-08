project('ash', 'cpp', default_options : ['cpp_std=c++17'], version : '0.0.1')

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
subdir('include')
subdir('src')
subdir('test')
# subdir('doc')

# Declare this project as a dependency for others to use.
proj_dep = declare_dependency(include_directories : includes)
