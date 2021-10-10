macro(mapnik_option _name)
    option(${ARGN})
    add_feature_info("${ARGV0}" ${ARGV0} "${ARGV1}")
endmacro()
