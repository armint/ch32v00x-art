cmake_minimum_required(VERSION 3.20)

project(ch32v00x-art LANGUAGES C CXX ASM)

file(GLOB_RECURSE PROJECT_SOURCES
        "src/*.c"
        "src/*.cpp"
)

add_library(ch32v00x-art STATIC ${PROJECT_SOURCES})

target_include_directories(ch32v00x-art
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(ch32v00x-art
        PRIVATE
        art-core
        etl::etl
)

# Use ch32v00x-system interface
target_include_directories(ch32v00x-art
        PUBLIC $<TARGET_PROPERTY:ch32v00x-system,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(ch32v00x-art
        PUBLIC $<TARGET_PROPERTY:ch32v00x-system,INTERFACE_COMPILE_DEFINITIONS>
)

target_compile_features(ch32v00x-art PUBLIC cxx_std_23)

if (TARGET art-build-config)
    target_link_libraries(ch32v00x-art PRIVATE $<BUILD_INTERFACE:art-build-config>)
endif ()

# Ensure the exported interface does not pull non-exported targets.
set_property(TARGET ch32v00x-art PROPERTY INTERFACE_LINK_LIBRARIES "")
