cmake_minimum_required(VERSION 3.28)
project(learn_metal)

function(build_shaders LIB_NAME)
    message(STATUS "Building shader lib: ${LIB_NAME}")

    set(SHADER_AIRS "")
    foreach (SHADER_PATH IN LISTS ARGN)
        get_filename_component(SHADER_FILENAME ${SHADER_PATH} NAME_WLE)
        message(STATUS "  Compiling: ${SHADER_FILENAME}")

        set(SHADER_SRC "${CMAKE_SOURCE_DIR}/${SHADER_PATH}")
        get_filename_component(LIB_NAME_WLE ${LIB_NAME} NAME_WLE)
        set(SHADER_AIR "${LIB_NAME_WLE}_${SHADER_FILENAME}.air")

        list(APPEND SHADER_AIRS ${SHADER_AIR})
        add_custom_command(
                OUTPUT ${SHADER_AIR}
                COMMAND xcrun -sdk macosx metal -o ${SHADER_AIR} -c ${SHADER_SRC}
                DEPENDS ${SHADER_SRC}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endforeach ()

    message(STATUS "  Building: ${SHADER_AIRS}")
    add_custom_command(
            OUTPUT ${LIB_NAME}
            COMMAND xcrun -sdk macosx metallib -o ${LIB_NAME} ${SHADER_AIRS}
            DEPENDS ${SHADER_AIRS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -fno-objc-arc")
set(CMAKE_EXE_LINKER_FLAGS "-framework Metal -framework Foundation -framework Cocoa -framework CoreGraphics -framework MetalKit")

include_directories(metal-cmake/metal-cpp)
include_directories(metal-cmake/metal-cpp-extensions)

include_directories(src/common)

add_subdirectory(metal-cmake)  # Library definition

set(COMMON_SOURCE_FILES
        src/common/app-delegate.cpp
        src/common/view-delegate.cpp
        src/common/utils.cpp
        src/common/matrices.cpp
        src/common/matrices.hpp)

add_executable(00-window
        src/00-window/main.cpp
        ${COMMON_SOURCE_FILES}
)
target_link_libraries(00-window metal_cpp)

build_shaders(01-hello-triangle.metallib src/01-hello-triangle/shaders.metal)
add_executable(01-hello-triangle
        src/01-hello-triangle/main.cpp
        01-hello-triangle.metallib
        ${COMMON_SOURCE_FILES}
)
target_link_libraries(01-hello-triangle metal_cpp)

build_shaders(02-hello-3d.metallib src/02-hello-3d/shaders.metal)
add_executable(02-hello-3d
        src/02-hello-3d/main.cpp
        02-hello-3d.metallib
        ${COMMON_SOURCE_FILES}
)
target_link_libraries(02-hello-3d metal_cpp)
