function(create_pkg_config_file _target _lib_name _description)
    string(CONFIGURE [[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

Name: @_lib_name@
Description: @_description@
Version: @MAPNIK_VERSION@
Libs: -L"${libdir}" -l$<TARGET_FILE_BASE_NAME:@_target@>$<TARGET_PROPERTY:@_target@,$<CONFIG>_POSTFIX>
Cflags: -I"${includedir}" ]]
    _contents @ONLY)

    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}-$<CONFIG>.pc
        CONTENT "${_contents}"
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}-$<CONFIG>.pc
        DESTINATION ${MAPNIK_PKGCONF_DIR}
        RENAME ${_lib_name}.pc
    )
endfunction()

function(create_pkg_config_file_mapnik _lib_name _description)
    get_target_property(m_compile_defs core INTERFACE_COMPILE_DEFINITIONS)
    string(JOIN " -D" m_str_compile_defs ${m_compile_defs})
    if(m_str_compile_defs)
        set(m_str_compile_defs "-D${m_str_compile_defs}")
    endif()

    set(m_requires
        libmapnikwkt
        libmapnikjson
        icu-uc
        icu-i18n
        harfbuzz
        freetype2
    )
    if(USE_LIBXML2)
        list(APPEND m_requires libxml-2.0)
    endif()
    if(USE_PNG)
        list(APPEND m_requires libpng)
    endif()
    if(USE_JPEG)
        list(APPEND m_requires libjpeg)
    endif()
    if(USE_TIFF)
        list(APPEND m_requires libtiff-4)
    endif()
    if(USE_WEBP)
        list(APPEND m_requires libwebp)
    endif()
    if(USE_CAIRO)
        list(APPEND m_requires cairo)
    endif()
    if(USE_PROJ)
        list(APPEND m_requires "proj >= ${PROJ_MIN_VERSION}")
    endif()
    string(JOIN " " m_requires ${m_requires})
    string(CONFIGURE [[
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

Name: @_lib_name@
Description: @_description@
Version: @MAPNIK_VERSION@
Requires: @m_requires@
Libs: -L"${libdir}" -l$<TARGET_FILE_BASE_NAME:mapnik>$<TARGET_PROPERTY:mapnik,$<CONFIG>_POSTFIX>
Cflags: -I"${includedir}" @m_str_compile_defs@]]
    _contents @ONLY)
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}-$<CONFIG>.pc
        CONTENT "${_contents}"
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${_lib_name}-$<CONFIG>.pc
        DESTINATION ${MAPNIK_PKGCONF_DIR}
        RENAME ${_lib_name}.pc
    )
endfunction()


create_pkg_config_file(wkt libmapnikwkt "wkt library")
create_pkg_config_file(json libmapnikjson "json library")
create_pkg_config_file_mapnik("libmapnik" "mapnik library")
