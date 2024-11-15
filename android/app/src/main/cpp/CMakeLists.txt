cmake_minimum_required(VERSION 3.22.1)

project("javascriptnative")

# Set path to Skia build directory
set(SKIA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../skia)

# Add Skia include directories
include_directories(
        ${SKIA_DIR}
        ${SKIA_DIR}/include
        ${SKIA_DIR}/include/core
        ${SKIA_DIR}/include/config
)

# Create the native library
add_library(
        skia_wrapper
        SHARED
        skia_wrapper.cpp
)

# Set C++ standard and flags
set_target_properties(
        skia_wrapper
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

# Find required Android libraries
find_library(android-lib android)
find_library(log-lib log)

# Add Skia library
add_library(skia SHARED IMPORTED)
set_target_properties(
        skia
        PROPERTIES
        IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libskia.so
)

# Add compiler definitions and flags
target_compile_definitions(
        skia_wrapper
        PRIVATE
        SK_RELEASE
)

target_compile_options(
        skia_wrapper
        PRIVATE
        -fno-exceptions
        -fno-rtti
        -Wno-error=format-security
)

# Link all libraries
target_link_libraries(
        skia_wrapper
        skia
        ${android-lib}
        ${log-lib}
)