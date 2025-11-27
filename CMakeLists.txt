cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# 🔧 Erzwinge konsistente CRT unter MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Qt Integration (Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# API module must be available early
add_subdirectory(api)

# Add core and debug modules early
add_subdirectory(core)
add_subdirectory(debug)

# Add service modules
add_subdirectory(services/poppler)
add_subdirectory(services/opencv)
add_subdirectory(services/tesseract)

# Add persistence and ui modules
add_subdirectory(persistence)
add_subdirectory(ui)

# Add app subdirectory
add_subdirectory(app)

# Third-party llama (moved to third_party)
option(BUILD_THIRD_PARTY_LLAMACPP "Build third_party/llama.cpp subproject" OFF)
if(BUILD_THIRD_PARTY_LLAMACPP)
    if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/llama.cpp/CMakeLists.txt")
        add_subdirectory(third_party/llama.cpp)
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/extern/llama.cpp/CMakeLists.txt")
        add_subdirectory(extern/llama.cpp)
    endif()
else()
    message(STATUS "Skipping third_party/llama.cpp (BUILD_THIRD_PARTY_LLAMACPP=OFF)")
endif()

# Third-party packages
find_package(Tesseract QUIET CONFIG)
find_package(Fontconfig QUIET)
find_package(tinyxml2 QUIET)
find_package(ICU QUIET COMPONENTS uc i18n)
find_package(nlohmann_json QUIET CONFIG)
find_package(spdlog QUIET CONFIG)
find_package(OpenCV QUIET)

# ONNX Runtime (manual link via vcpkg)
set(_ORT_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATHS "${_ORT_ROOT}/include" "${_ORT_ROOT}/include/onnxruntime"
    NO_DEFAULT_PATH)
find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS "${_ORT_ROOT}/lib" "${_ORT_ROOT}/lib/manual-link"
    NO_DEFAULT_PATH)

# Setze Zielverzeichnis für die Plugins
set(QT_PLUGINS_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms")
set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/platforms")

# Dummy LlamaModelTest post-build
if(TARGET LlamaModelTest)
    add_custom_command(TARGET LlamaModelTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "LlamaModelTest built"
    )
endif()