includes = include_directories('.')

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

ash_library = library('ash', sources_list, include_directories : includes)
