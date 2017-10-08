doxygen = find_program('doxygen', required : true)
doxyfile = configure_file(input : 'Doxyfile.in', output : 'Doxyfile', configuration : config_data, install : false)
docs_install_dir = join_paths(get_option('datadir'), 'doc', 'ash')
html_target = custom_target('docs', input : [doxyfile, readme_file, all_includes], output : 'html',
                            command : [doxygen, doxyfile], install : true, install_dir : docs_install_dir)