cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

set(CMAKE_CXX_STANDARD 20)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR)
    set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/third_party/vcpkg/installed" CACHE PATH "vcpkg installed dir")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(QT_QML_GENERATE_QMLLS_INI ON)

add_subdirectory(api)
add_subdirectory(core)
add_subdirectory(debug)
add_subdirectory(services/poppler)
add_subdirectory(services/opencv)
add_subdirectory(services/tesseract)
add_subdirectory(persistence)
add_subdirectory(ui)
add_subdirectory(app)

option(BUILD_THIRD_PARTY_LLAMACPP "Build third_party/llama.cpp subproject" OFF)
if(BUILD_THIRD_PARTY_LLAMACPP)
    add_subdirectory(third_party/llama.cpp)
endif()