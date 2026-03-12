cmake_minimum_required(VERSION 4.0.1)
project(debug)

file(GLOB_RECURSE DEBUG_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(debug_lib STATIC ${DEBUG_SRCS})

target_include_directories(debug_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/core/include>
)

target_compile_features(debug_lib PUBLIC cxx_std_20)

find_package(spdlog QUIET CONFIG)
if (spdlog_FOUND)
    target_link_libraries(debug_lib PRIVATE spdlog::spdlog)
endif()

find_package(nlohmann_json QUIET CONFIG)
if (nlohmann_json_FOUND)
    target_link_libraries(debug_lib PRIVATE nlohmann_json::nlohmann_json)
endif()

# Precompiled header (if supported)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(debug_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

install(TARGETS debug_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

option(BUILD_DEBUG_TESTS "Build tests for debug module" OFF)
if (BUILD_DEBUG_TESTS)
    find_package(GTest CONFIG REQUIRED)

    file(GLOB DEBUG_TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.cpp)
    add_executable(debug_unit_tests ${DEBUG_TEST_SRCS})
    target_include_directories(debug_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(debug_unit_tests PRIVATE
        GTest::gtest_main
        debug_lib
        core_lib
    )
    target_compile_features(debug_unit_tests PRIVATE cxx_std_20)

    set_target_properties(debug_unit_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )

    include(GoogleTest)
    gtest_discover_tests(debug_unit_tests PROPERTIES LABELS "DEBUG_UNIT")
endif()
