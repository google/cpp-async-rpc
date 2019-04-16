examples_includes = include_directories('.')

all_examples_includes = [includes, examples_includes, third_party_includes]

main = executable('main',
                  'main.cpp',
                  include_directories : all_examples_includes,
                  link_args : '-lpthread',
                  link_with : ash_library)
                  
modules = executable('modules',
                     'modules.cpp', 'module1.cpp', 'module2.cpp',
                     include_directories : all_examples_includes,
                     link_args : '-lpthread',
                     link_with : ash_library)
