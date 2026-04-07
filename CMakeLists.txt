cmake_minimum_required(VERSION 3.8)
project("upl_scroller")

# Output binary next to the source (same location as before)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO    REQUIRED cairo)
pkg_check_modules(FREETYPE REQUIRED freetype2)

file(GLOB SOURCES "src/*.cpp" "src/*.h")
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CAIRO_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    ${CAIRO_LIBRARIES}
    ${FREETYPE_LIBRARIES})

# Paths baked into the binary at compile time
target_compile_definitions(${PROJECT_NAME} PRIVATE
    FONTS_PATH="${CMAKE_SOURCE_DIR}/fonts"
    FONT_TTF="${CMAKE_SOURCE_DIR}/pix.ttf")
