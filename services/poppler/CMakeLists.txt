cmake_minimum_required(VERSION 4.0.1)
project(services_poppler)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Respect top-level option BUILD_SERVICES_SHARED (default OFF)
if(NOT DEFINED BUILD_SERVICES_SHARED)
    set(BUILD_SERVICES_SHARED OFF CACHE BOOL "Build services as shared libraries")
endif()
if(BUILD_SERVICES_SHARED)
    set(_svc_type SHARED)
else()
    set(_svc_type STATIC)
endif()

add_library(services_poppler ${_svc_type} ${SRCS})

target_include_directories(services_poppler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(services_poppler PUBLIC cxx_std_20)

# Link third-party dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

# Link to api, core and debug
if(OpenCV_FOUND)
    target_include_directories(services_poppler PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(services_poppler PUBLIC ${OpenCV_LIBS})
endif()

target_link_libraries(services_poppler PUBLIC api core_lib debug_lib PkgConfig::POPPLER_CPP nlohmann_json::nlohmann_json)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(services_poppler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

# Ensure outputs go into config-specific directories, e.g. bin/Release and lib/Release
set_target_properties(services_poppler PROPERTIES
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

# Install
install(TARGETS services_poppler
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)