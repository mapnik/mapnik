function(create_pkg_config_file _target _lib_name _description)
    string(CONFIGURE [[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

Name: @_lib_name@
Description: @_description@
Version: @MAPNIK_VERSION@
Libs: -L"${libdir}" -l$<TARGET_FILE_PREFIX:@_target@>$<TARGET_FILE_BASE_NAME:@_target@>$<TARGET_PROPERTY:@_target@,$<CONFIG>_POSTFIX>
Cflags: -I"${includedir}" ]] 
    _contents @ONLY)

    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}.pc
        CONTENT "${_contents}"
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}.pc
        DESTINATION ${MAPNIK_PKGCONF_DIR}
    )
endfunction()

function(create_pkg_config_file_mapnik _lib_name _description)
    get_target_property(_compile_defs core INTERFACE_COMPILE_DEFINITIONS)
    string(JOIN " -D" _str_compile_defs ${_compile_defs})
    if(_str_compile_defs) 
        set(_str_compile_defs "-D${_str_compile_defs}")
    endif()
    string(CONFIGURE [[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

Name: @_lib_name@
Description: @_description@
Version: @MAPNIK_VERSION@
Requires: libmapnikwkt libmapnikjson
Libs: -L"${libdir}" -l$<TARGET_FILE_PREFIX:mapnik>$<TARGET_FILE_BASE_NAME:mapnik>$<TARGET_PROPERTY:mapnik,$<CONFIG>_POSTFIX> -l$<TARGET_FILE_PREFIX:json>$<TARGET_FILE_BASE_NAME:json>$<TARGET_PROPERTY:json,$<CONFIG>_POSTFIX> -l$<TARGET_FILE_PREFIX:wkt>$<TARGET_FILE_BASE_NAME:wkt>$<TARGET_PROPERTY:wkt,$<CONFIG>_POSTFIX>
Cflags: -I"${includedir}" @_str_compile_defs@]] 
    _contents @ONLY)
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}.pc
        CONTENT "${_contents}"
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}.pc
        DESTINATION ${MAPNIK_PKGCONF_DIR}
    )
endfunction()


create_pkg_config_file(wkt libmapnikwkt "wkt library")
create_pkg_config_file(json libmapnikjson "json library")
create_pkg_config_file_mapnik("libmapnik" "mapnik library")
