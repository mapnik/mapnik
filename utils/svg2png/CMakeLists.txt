add_executable(svg2png 
    svg2png.cpp
)

target_link_libraries(svg2png PRIVATE mapnik::headers mapnik::mapnik)
install(TARGETS svg2png
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)