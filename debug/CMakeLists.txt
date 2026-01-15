cmake_minimum_required(VERSION 4.0.1)
project(debug)

file(GLOB_RECURSE DEBUG_SRCS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(debug_lib STATIC ${DEBUG_SRCS})

target_include_directories(debug_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
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
