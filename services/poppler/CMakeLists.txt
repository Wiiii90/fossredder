cmake_minimum_required(VERSION 3.15)
project(services_poppler)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(services_poppler STATIC ${SRCS})

target_include_directories(services_poppler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(services_poppler PUBLIC cxx_std_20)

# Link third-party dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
find_package(nlohmann_json CONFIG REQUIRED)

# Link to api, core and debug
target_link_libraries(services_poppler PUBLIC api core_lib debug_lib PkgConfig::POPPLER_CPP nlohmann_json::nlohmann_json)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(services_poppler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

# Install
install(TARGETS services_poppler
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)