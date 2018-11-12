# Import catch.
catch_subproject = subproject('catch2')
catch_dependency = catch_subproject.get_variable('catch2_dep')

# This is a lot of machinery to automatically list test files
# at any subdirectory depth level and running them.

find_tests_output = run_command(find_program_path.path(), meson.current_source_dir(), '-name', '*_test.cpp', '-printf', '%P\n')
tests_list = find_tests_output.stdout().strip().split('\n')
foreach test_source : tests_list
  test_source_components = test_source.split('.')
  test_binary_name = test_source_components[0].underscorify()
  test_executable = executable(
    test_binary_name,
    test_source,
    include_directories : includes,
    dependencies : [catch_dependency],
    cpp_args: [cpp_catch2_no_exceptions_flag, cpp_catch2_config_main_flag]
  )
  test(test_binary_name, test_executable)
endforeach