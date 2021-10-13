# to keep compability with SCons we need to read the file and replace the values.
file(READ mapnik-config.template.sh _template)

math(EXPR m_mapnik_version_expanded "${CMAKE_PROJECT_VERSION_MAJOR} * 100000 + ${CMAKE_PROJECT_VERSION_MINOR} * 100 + ${CMAKE_PROJECT_VERSION_PATCH}")
string(REPLACE "CONFIG_MAPNIK_VERSION=\"unknown\"" "CONFIG_MAPNIK_VERSION=\"${m_mapnik_version_expanded}\"" _template ${_template})
string(REPLACE "CONFIG_MAPNIK_VERSION_STRING=\"unknown\"" "CONFIG_MAPNIK_VERSION_STRING=\"${CMAKE_PROJECT_VERSION}\"" _template ${_template})
string(REPLACE "CONFIG_PREFIX=\"/usr/local\"" "CONFIG_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"" _template ${_template})

string(REPLACE "CONFIG_LIBDIR_SCHEMA=\"lib\"" "CONFIG_LIBDIR_SCHEMA=\"${MAPNIK_LIB_DIR}\"" _template ${_template})
string(REPLACE "CONFIG_MAPNIK_LIB_DIR=\"\${CONFIG_MAPNIK_LIB_BASE}/\${CONFIG_LIB_DIR_NAME}\"" "CONFIG_MAPNIK_LIB_DIR=\"\${CONFIG_MAPNIK_LIB_BASE}\"" _template ${_template})

string(REPLACE "CONFIG_MAPNIK_INPUT_PLUGINS=\"\${CONFIG_MAPNIK_LIB_DIR}/input\"" "CONFIG_MAPNIK_INPUT_PLUGINS=\"\${CONFIG_PREFIX}/${PLUGINS_INSTALL_DIR}\"" _template ${_template})
string(REPLACE "CONFIG_MAPNIK_FONTS=\"\${CONFIG_MAPNIK_LIB_DIR}/fonts\"" "CONFIG_MAPNIK_FONTS=\"\${CONFIG_PREFIX}/${FONTS_INSTALL_DIR}\"" _template ${_template})

get_filename_component(_cxx_compiler ${CMAKE_CXX_COMPILER} NAME_WE)
string(REPLACE "CONFIG_CXX=\"c++\"" "CONFIG_CXX=\"${_cxx_compiler}\"" _template ${_template})

get_target_property(_compile_defs core INTERFACE_COMPILE_DEFINITIONS)
string(JOIN " -D" _str_compile_defs ${_compile_defs})
if(_str_compile_defs) 
    set(_str_compile_defs "-D${_str_compile_defs}")
    string(REPLACE "CONFIG_DEFINES=" "CONFIG_DEFINES=\"${_str_compile_defs}\"" _template ${_template})
endif()

execute_process(COMMAND git rev-list --max-count=1 HEAD OUTPUT_VARIABLE _git_rev ERROR_VARIABLE _git_rev_err WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
execute_process(COMMAND git describe HEAD OUTPUT_VARIABLE _git_desc ERROR_VARIABLE _git_desc_err WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
message(STATUS "mapnik-config: _git_rev: ${_git_rev}")
message(STATUS "mapnik-config: _git_rev_err: ${_git_rev_err}")
message(STATUS "mapnik-config: _git_desc: ${_git_desc}")
message(STATUS "mapnik-config: _git_desc_err: ${_git_desc_err}")
if(NOT _git_rev_err)
    string(REGEX REPLACE "\n$" "" _git_rev "${_git_rev}")
    string(REPLACE "CONFIG_GIT_REVISION=\"N/A\"" "CONFIG_GIT_REVISION=\"${_git_rev}\"" _template ${_template})
endif()
if(NOT _git_desc_err)
    string(REGEX REPLACE "\n$" "" _git_desc "${_git_desc}")
    string(REPLACE "CONFIG_GIT_DESCRIBE=\"\${CONFIG_MAPNIK_VERSION_STRING}\"" "CONFIG_GIT_DESCRIBE=\"${_git_desc}\"" _template ${_template})
endif()


file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mapnik-config" ${_template})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mapnik-config" DESTINATION ${MAPNIK_BIN_DIR} COMPONENT MapnikRuntime)
