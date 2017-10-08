project('ash', 'cpp', version : '0.0.1')

# Global configuration data.
config_data = configuration_data()
config_data.set('VERSION', meson.project_version())
config_data.set('TOP_SRCDIR', meson.source_root())
config_data.set('TOP_BUILDDIR', meson.build_root())

# For reference in the docs target.
readme_file = files('README.md')

# Enter the project subdirectories.
subdir('include')
subdir('src')
subdir('test')
subdir('doc')

# Declare this project as a dependency for others to use.
proj_dep = declare_dependency(include_directories : includes)