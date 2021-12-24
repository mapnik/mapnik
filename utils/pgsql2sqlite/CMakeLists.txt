find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)
find_package(SQLite3 REQUIRED)
find_package(PostgreSQL REQUIRED)

add_executable(pgsql2sqlite 
    main.cpp
    sqlite.cpp
)

target_include_directories(pgsql2sqlite PRIVATE ../../plugins/input/postgis)
target_link_libraries(pgsql2sqlite PRIVATE 
    SQLite::SQLite3
    PostgreSQL::PostgreSQL
    Boost::program_options
    mapnik::mapnik
)

set_target_properties(pgsql2sqlite PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
)

mapnik_install_utility(pgsql2sqlite)
