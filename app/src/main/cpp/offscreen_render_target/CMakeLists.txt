set(include_dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        )

file(GLOB_RECURSE srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        )

add_library(offscreen_rt STATIC ${srcs})

target_include_directories(offscreen_rt PUBLIC
        ${include_dirs}
        )

target_include_directories(offscreen_rt SYSTEM PUBLIC ${LIBYUV_INCLUDE_DIRS})
target_include_directories(offscreen_rt PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(offscreen_rt
        GLESv3
        EGL
        ogl_utils
        )