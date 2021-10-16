macro(mapnik_option option_name option_description option_default_value)
    option(${option_name} "${option_description}" ${option_default_value})
    add_feature_info(${option_name} ${${option_name}} "${option_description}")
endmacro()
