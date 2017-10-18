project('ash', 'cpp', version : '0.0.1')

# Global configuration data.
config_data = configuration_data()
config_data.set('VERSION', meson.project_version())
config_data.set('TOP_SRCDIR', meson.source_root())
config_data.set('TOP_BUILDDIR', meson.build_root())

# Determine compiler flags.
cpp_compiler = meson.get_compiler('cpp')
cpp_no_exceptions_flag = cpp_compiler.first_supported_argument(['/EHs-', '-fno-exceptions'])
cpp_reenable_exceptions_flag = cpp_compiler.first_supported_argument(['/EHs', '-fexceptions'])
cpp_no_rtti_flag = cpp_compiler.first_supported_argument(['/GR-', '-fno-rtti'])
add_global_arguments(cpp_no_exceptions_flag, cpp_no_rtti_flag, language : 'cpp')
add_global_link_arguments(cpp_no_exceptions_flag, cpp_no_rtti_flag, language : 'cpp')

# For reference in the docs target.
readme_file = files('README.md')

# Enter the project subdirectories.
subdir('include')
subdir('src')
subdir('test')
subdir('doc')

# Declare this project as a dependency for others to use.
proj_dep = declare_dependency(include_directories : includes)