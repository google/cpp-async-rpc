find_program_path = find_program('find') 

# Find all source files to be linked into a library.

find_sources_output = run_command(find_program_path.path(), meson.current_source_dir(), '-name', '*.cpp', '-printf', '%P\n')
sources_list = find_sources_output.stdout().strip().split('\n')

ash_library = library('ash', sources_list, include_directories : includes)
