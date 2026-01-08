cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

set(CMAKE_CXX_STANDARD 20)

# Allow optional build of services as SHARED; default OFF to preserve previous behavior
option(BUILD_SERVICES_SHARED "Build services/* as SHARED libraries instead of STATIC" OFF)

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

# Packaging target for Windows using Inno Setup. Build the 'package' target from Visual Studio
if (WIN32)
    set(_PACKAGE_VERSION "${PROJECT_VERSION}")
    if (_PACKAGE_VERSION STREQUAL "")
        set(_PACKAGE_VERSION "0.1.0")
    endif()

    set(PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/ci/package-inno.ps1")
    set(STAGING_DIR "${CMAKE_BINARY_DIR}/staging")
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/dist")

    set(_POWERSHELL_EXE "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe")
    set(_PACKAGE_LOG "${CMAKE_BINARY_DIR}/package.log")

    # Variables consumed by cmake/run-package.cmd.in (@ONLY)
    set(BUILD_DIR "${CMAKE_BINARY_DIR}")
    set(POWERSHELL_EXE "${_POWERSHELL_EXE}")
    set(VERSION "${_PACKAGE_VERSION}")
    set(LOG_FILE "${_PACKAGE_LOG}")

    # Generate a wrapper .cmd to avoid MSBuild quoting/escaping issues.
    set(_RUN_PACKAGE_CMD "${CMAKE_BINARY_DIR}/run-package.cmd")
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/run-package.cmd.in"
        "${_RUN_PACKAGE_CMD}"
        @ONLY
    )

    add_custom_target(package
        COMMENT "Install and create Inno Setup installer"
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --config $<CONFIG> --prefix "${STAGING_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "Running packaging wrapper (log: ${_PACKAGE_LOG})..."
        COMMAND cmd.exe /c call "${_RUN_PACKAGE_CMD}" $<CONFIG>
        DEPENDS fossredder
        BYPRODUCTS "${STAGING_DIR}" "${OUTPUT_DIR}" "${_PACKAGE_LOG}" "${_RUN_PACKAGE_CMD}"
        VERBATIM
    )
endif()