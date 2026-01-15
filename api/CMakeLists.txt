cmake_minimum_required(VERSION 4.0.1)
project(api)

# INTERFACE library containing only headers (DTOs & Interfaces)
add_library(api INTERFACE)

target_include_directories(api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(api INTERFACE cxx_std_20)

# Make nlohmann_json available for API headers that include it
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(api INTERFACE nlohmann_json::nlohmann_json)

# export/install
install(TARGETS api EXPORT apiTargets)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
install(EXPORT apiTargets FILE apiTargets.cmake NAMESPACE FossRedder:: DESTINATION lib/cmake/api)