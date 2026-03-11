cmake_minimum_required(VERSION 4.0.1)
project(core)

file(GLOB_RECURSE CORE_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(core_lib STATIC ${CORE_SRCS})

target_include_directories(core_lib PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(core_lib PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/debug/include>
)

if(TARGET api)
    target_link_libraries(core_lib PUBLIC api)
endif()

if(TARGET debug_lib)
    target_link_libraries(core_lib PRIVATE debug_lib)
endif()

target_compile_features(core_lib PUBLIC cxx_std_20)

# xlnt is used by the Xlsx export implementation (xlnt workbook/cell APIs)
find_package(xlnt CONFIG QUIET)
if(TARGET xlnt::xlnt)
    message(STATUS "xlnt found; linking to core_lib")
    target_link_libraries(core_lib PRIVATE xlnt::xlnt)
elseif(TARGET xlnt)
    message(STATUS "xlnt target found; linking to core_lib")
    target_link_libraries(core_lib PRIVATE xlnt)
else()
    message(WARNING "xlnt library not found; XLSX export may fail to link")
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")
        target_precompile_headers(core_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")
    endif()
endif()

# Option to build core tests (OFF by default)
option(BUILD_CORE_TESTS "Build tests for core module" OFF)
if (BUILD_CORE_TESTS)
    find_package(GTest CONFIG REQUIRED)

    file(GLOB UNIT_TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/*.cpp)
    add_executable(core_unit_tests ${UNIT_TEST_SRCS})
    target_include_directories(core_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_SOURCE_DIR}/core/include
        ${CMAKE_SOURCE_DIR}/persistence/include
    )
    target_link_libraries(core_unit_tests
        PRIVATE
            GTest::gtest_main
            core_lib
    )
    target_compile_features(core_unit_tests PRIVATE cxx_std_20)

    set_target_properties(core_unit_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )

    include(GoogleTest)
    gtest_discover_tests(core_unit_tests PROPERTIES LABELS "UNIT")

    file(GLOB INTERACTION_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/interaction/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tests/interaction/*.cc)
    if (INTERACTION_SRCS)
        add_executable(core_interaction_tests ${INTERACTION_SRCS})
        target_include_directories(core_interaction_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
            ${CMAKE_SOURCE_DIR}/core/include
            ${CMAKE_SOURCE_DIR}/persistence/include
        )
        target_link_libraries(core_interaction_tests
            PRIVATE
                GTest::gmock_main
                core_lib
        )
        target_compile_features(core_interaction_tests PRIVATE cxx_std_20)

        set_target_properties(core_interaction_tests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        )

        include(GoogleTest)
        gtest_discover_tests(core_interaction_tests PROPERTIES LABELS "INTERACTION")
    endif()
endif()

install(TARGETS core_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
