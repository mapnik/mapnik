function(mapnik_find_target_location)
  set(options)
  set(multiValueArgs TARGETS)
  cmake_parse_arguments( WIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

endfunction()

function(mapnik_copy_dependencies)
  if(COPY_LIBRARIES_FOR_EXECUTABLES AND WIN32)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs TARGETS PLUGINS)
    cmake_parse_arguments(MAPNIK_CP_DEPS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    include(CopyDllsForDebug)
    foreach(TARGET IN LISTS MAPNIK_CP_DEPS_TARGETS)
      add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mapnik::mapnik>" ${CMAKE_CURRENT_BINARY_DIR})
    endforeach()

    set(LIBS "")
    foreach(PLUGIN IN LISTS MAPNIK_CP_DEPS_PLUGINS)
      if(TARGET ${PLUGIN}) # only copy plugins that are be build
        list(APPEND LIBS "$<TARGET_FILE:${PLUGIN}>")
      endif()
    endforeach()
    copy_dlls_for_debug(TARGETS ${MAPNIK_CP_DEPS_TARGETS} LIBS ${LIBS} DIRS ${ADDITIONAL_LIBARIES_PATHS})
  endif()
endfunction()

function(mapnik_copy_plugins)
  if(COPY_FONTS_AND_PLUGINS_FOR_EXECUTABLES)
    set(options)
    set(oneValueArgs TARGET DESTINATION)
    set(multiValueArgs PLUGINS)
    cmake_parse_arguments(MAPNIK_CP_PLG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # copy_if_different requires a existing directory.
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MAPNIK_CP_PLG_DESTINATION})
    foreach(PLUGIN IN LISTS MAPNIK_CP_PLG_PLUGINS)
      #message(STATUS "copying plugin ${PLUGIN} to path: ${CMAKE_CURRENT_BINARY_DIR}/${MAPNIK_CP_PLG_DESTINATION}")
      if(TARGET ${PLUGIN})
        add_custom_command(TARGET ${MAPNIK_CP_PLG_TARGET} POST_BUILD COMMAND 
          ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${PLUGIN}>" ${CMAKE_CURRENT_BINARY_DIR}/${MAPNIK_CP_PLG_DESTINATION}/)
      else()
        message(NOTICE "${MAPNIK_CP_PLG_TARGET} requires plugin ${PLUGIN} but it isn't build. Check USE_PLUGIN_INPUT_ options to enable the plugin.")
      endif()
    endforeach()
  endif()
endfunction()

function(mapnik_require_fonts)
  if(COPY_FONTS_AND_PLUGINS_FOR_EXECUTABLES)
    set(options)
    set(oneValueArgs TARGET DESTINATION)
    set(multiValueArgs)
    cmake_parse_arguments(MAPNIK_REQUIRE_FONTS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_command(TARGET ${MAPNIK_REQUIRE_FONTS_TARGET} POST_BUILD COMMAND 
        ${CMAKE_COMMAND} -E copy_directory ${mapnik_SOURCE_DIR}/fonts ${CMAKE_CURRENT_BINARY_DIR}/${MAPNIK_REQUIRE_FONTS_DESTINATION}/)
  endif()
endfunction()
