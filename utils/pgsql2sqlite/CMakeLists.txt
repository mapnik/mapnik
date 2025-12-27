mapnik_find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)
mapnik_find_package(SQLite3 REQUIRED)
mapnik_find_package(PostgreSQL REQUIRED)

add_executable(pgsql2sqlite
    main.cpp
    sqlite.cpp
)

target_include_directories(pgsql2sqlite PRIVATE ../../plugins/input/postgis+pgraster)
target_link_libraries(pgsql2sqlite PRIVATE
    SQLite::SQLite3
    PostgreSQL::PostgreSQL
    Boost::program_options
    mapnik::mapnik
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(pgsql2sqlite)
