cmake_minimum_required(VERSION 4.0.1)
project(services_tesseract)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(NOT DEFINED BUILD_SERVICES_SHARED)
    set(BUILD_SERVICES_SHARED OFF CACHE BOOL "Build services as shared libraries")
endif()
if(BUILD_SERVICES_SHARED)
    set(_svc_type SHARED)
else()
    set(_svc_type STATIC)
endif()

add_library(services_tesseract ${_svc_type} ${SRCS})

target_include_directories(services_tesseract PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(services_tesseract PUBLIC cxx_std_20)

find_package(Tesseract REQUIRED CONFIG)
if(Tesseract_FOUND)
    target_link_libraries(services_tesseract PUBLIC Tesseract::libtesseract)
endif()

# Link to api so API headers are available
target_link_libraries(services_tesseract PUBLIC api)

# link to core/debug
target_link_libraries(services_tesseract PUBLIC core_lib debug_lib)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(services_tesseract PRIVATE "${CMAKE_SOURCE_DIR}/include/pch.h")
    endif()
endif()

# Ensure consistent, config-specific output directories (bin/Release etc.)
set_target_properties(services_tesseract PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

install(TARGETS services_tesseract ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
