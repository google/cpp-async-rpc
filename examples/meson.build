examples_includes = include_directories('.')

main = executable('main',
                  'main.cpp',
                  include_directories : [includes, examples_includes],
                  link_args : '-lpthread',
                  link_with : ash_library)
                  
main = executable('modules',
                  'modules.cpp', 'module1.cpp', 'module2.cpp',
                   include_directories : [includes, examples_includes],
                   link_args : '-lpthread',
                   link_with : ash_library)
