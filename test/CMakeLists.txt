project(mapnik-test)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)
FetchContent_MakeAvailable(Catch2)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_executable(mapnik-test-unit 
    unit/run.cpp
    unit/color/css_color.cpp
)
target_link_libraries(mapnik-test-unit PUBLIC
    Catch 
    mapnik::headers
    mapnik::agg
    mapnik::mapnik
)

include(CTest)
include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
include(${catch2_SOURCE_DIR}/contrib/ParseAndAddCatchTests.cmake)

ParseAndAddCatchTests(mapnik-test-unit)