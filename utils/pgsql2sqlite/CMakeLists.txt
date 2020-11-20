add_executable(pgsql2sqlite 
    main.cpp
    sqlite.cpp
)

target_link_libraries(pgsql2sqlite PRIVATE mapnik::mapnik)
install(TARGETS pgsql2sqlite
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)