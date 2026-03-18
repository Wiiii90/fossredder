# Debug infrastructure sources.
set(DEBUG_SRCS
    src/ErrorReporter.cpp
    src/FileDebugger.cpp
    src/RunManager.cpp
    src/SpdlogDebugger.cpp
)

add_library(fossredder_debug STATIC ${DEBUG_SRCS})
add_library(FossRedder::Debug ALIAS fossredder_debug)

set_target_properties(fossredder_debug PROPERTIES EXPORT_NAME Debug)

target_compile_features(fossredder_debug PUBLIC cxx_std_20)

target_include_directories(fossredder_debug PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public debug headers depend on core interfaces.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(fossredder_debug
    PUBLIC
        FossRedder::Core
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

fossredder_set_library_output_dirs(fossredder_debug)

fossredder_install_library_target(fossredder_debug)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Optional module-local tests.
option(BUILD_DEBUG_TESTS "Build tests for debug module" OFF)
if (BUILD_DEBUG_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    set(DEBUG_TEST_SRCS
        tests/unit/TestDebuggerErrorReporter.cpp
    )
    add_executable(debug_unit_tests ${DEBUG_TEST_SRCS})
    target_include_directories(debug_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(debug_unit_tests PRIVATE
        GTest::gtest_main
        FossRedder::Debug
    )
    target_compile_features(debug_unit_tests PRIVATE cxx_std_20)

    fossredder_set_runtime_output_dirs(debug_unit_tests)

    gtest_discover_tests(debug_unit_tests PROPERTIES LABELS "DEBUG_UNIT")
endif()
