set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_library(utils INTERFACE)

target_include_directories(utils INTERFACE
    ${include_dirs}
)