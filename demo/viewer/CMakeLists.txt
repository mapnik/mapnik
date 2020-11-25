project(mapnik-viewer)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(mapnik-viewer 
    about_dialog.cpp
    info_dialog.cpp
    layerdelegate.cpp
    layerlistmodel.cpp
    layerwidget.cpp
    layer_info_dialog.cpp
    main.cpp
    mainwindow.cpp
    mapwidget.cpp
    styles_model.cpp
    forms/about.ui
    forms/info.ui
    forms/layer_info.ui    
    mapnik_viewer.qrc
)

set_target_properties(mapnik-viewer PROPERTIES
    AUTOUIC_SEARCH_PATHS forms
    AUTORCC ON    
    AUTOUIC ON
    AUTOMOC ON
)


target_link_libraries(mapnik-viewer PRIVATE 
    Qt5::Widgets
    mapnik::headers
    mapnik::agg
    mapnik::mapnik
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/viewer.ini 
"[mapnik]
plugins_dir=${PLUGINS_INSTALL_DIR}
fonts/1/dir=${FONTS_INSTALL_DIR}"
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/viewer.ini DESTINATION bin)
install(TARGETS mapnik-viewer 
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)
