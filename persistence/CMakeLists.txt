cmake_minimum_required(VERSION 4.0.1)
project(persistence)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(persistence_lib STATIC ${SRCS})

target_include_directories(persistence_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/core/include>
)

target_link_libraries(persistence_lib PUBLIC core_lib)

target_compile_features(persistence_lib PUBLIC cxx_std_20)

find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    target_link_libraries(persistence_lib PRIVATE SQLite::SQLite3)
else()
    find_library(SQLITE3_LIBRARY sqlite3)
    if(SQLITE3_LIBRARY)
        target_link_libraries(persistence_lib PRIVATE ${SQLITE3_LIBRARY})
    endif()
endif()

find_package(nlohmann_json CONFIG QUIET)
if (TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(persistence_lib PRIVATE nlohmann_json::nlohmann_json)
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence/pch.h")
        target_precompile_headers(persistence_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence/pch.h")
    endif()
endif()

install(TARGETS persistence_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

option(BUILD_PERSISTENCE_TESTS "Build tests for persistence module" OFF)
if (BUILD_PERSISTENCE_TESTS)
    find_package(GTest CONFIG REQUIRED)

    file(GLOB PERSISTENCE_TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.cpp)
    add_executable(persistence_unit_tests ${PERSISTENCE_TEST_SRCS})
    target_include_directories(persistence_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(persistence_unit_tests PRIVATE
        GTest::gtest_main
        persistence_lib
        core_lib
    )
    target_compile_features(persistence_unit_tests PRIVATE cxx_std_20)

    set_target_properties(persistence_unit_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )

    include(GoogleTest)
    gtest_discover_tests(persistence_unit_tests PROPERTIES LABELS "PERSISTENCE_UNIT")
endif()
