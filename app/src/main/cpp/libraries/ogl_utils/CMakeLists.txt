set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

file(GLOB_RECURSE srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

add_library(ogl_utils STATIC ${srcs})

target_include_directories(ogl_utils PUBLIC
    ${include_dirs}
)

target_link_libraries(ogl_utils
    banuba
)