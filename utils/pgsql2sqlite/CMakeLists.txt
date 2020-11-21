
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
    mapnik::headers
    mapnik::mapnik
)

install(TARGETS pgsql2sqlite
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)
