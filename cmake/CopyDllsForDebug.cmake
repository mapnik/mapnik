# This is a helper script to run BundleUtilities fixup_bundle as postbuild
# for a target. The primary use case is to copy .DLLs to the build directory for
# the Windows platform. It allows generator expressions to be used to determine
# the binary location
#
# Usage : copy_dlls_for_debug TARGET LIBS DIRS
# - TARGET : A cmake target
# - See fixup_bundle for LIBS and DIRS arguments

if(RUN_IT)
# Script ran by the add_custom_command
	include(BundleUtilities)
	include(InstallRequiredSystemLibraries)
	string (REPLACE " " ";" TO_FIXUP_LIBS "${TO_FIXUP_LIBS}")
	string (REPLACE " " ";" TO_FIXUP_DIRS "${TO_FIXUP_DIRS}")
	#message(STATUS "${TO_FIXUP_FILE} ${TO_FIXUP_LIBS} ${TO_FIXUP_DIRS}")
	fixup_bundle("${TO_FIXUP_FILE}" "${TO_FIXUP_LIBS}" "${TO_FIXUP_DIRS}")
# End of script ran by the add_custom_command
else()

set(THIS_FILE ${CMAKE_CURRENT_LIST_FILE})
function(copy_dlls_for_debug)
	set(options)
	set(oneValueArgs)
	set(multiValueArgs TARGETS LIBS DIRS)
	cmake_parse_arguments(MAPNIK_COPY_DLLS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if(WIN32)
		foreach(_target IN LISTS MAPNIK_COPY_DLLS_TARGETS)
			add_custom_command(
				TARGET ${_target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} ARGS -DRUN_IT:BOOL=ON -DTO_FIXUP_FILE="$<TARGET_FILE:${_target}>" -DTO_FIXUP_LIBS:STRING="${MAPNIK_COPY_DLLS_LIBS}" -DTO_FIXUP_DIRS="${MAPNIK_COPY_DLLS_DIRS}" -P "${THIS_FILE}"
				COMMENT "Fixing up dependencies for ${_target}"
			)
		endforeach()
	endif(WIN32)
endfunction()

endif()
