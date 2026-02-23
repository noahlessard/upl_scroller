cmake_minimum_required (VERSION 3.8)


project ("upl_scroller")

# make output go to root dir, so next to fonts folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

# don't need to find the path, because it's already in default paths
find_library(NOTCURSES_LIBRARY NAMES notcurses)
find_library(NOTCURSES_CORE_LIBRARY NAMES notcurses-core)

# Add source to this project's executable.
file(GLOB SOURCES "src/*.cpp" "src/*.h")
add_executable(${PROJECT_NAME} ${SOURCES})

# don't need to add directories, since we install thru apt-get, its already in default paths
target_link_libraries(${PROJECT_NAME} notcurses notcurses-core)

# add dynamic reference to fonts path
target_compile_definitions(${PROJECT_NAME} PRIVATE FONTS_PATH="${CMAKE_SOURCE_DIR}/fonts")