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