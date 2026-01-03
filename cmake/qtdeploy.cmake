# qtdeploy.cmake - deterministic copy fallback
# Copies Qt runtime DLLs, selected Qt plugins, and QML modules from vcpkg into TARGET_DIR.

if(NOT DEFINED TARGET_DIR AND DEFINED ENV{TARGET_DIR})
    set(TARGET_DIR "$ENV{TARGET_DIR}")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR AND DEFINED ENV{VCPKG_INSTALLED_DIR})
    set(VCPKG_INSTALLED_DIR "$ENV{VCPKG_INSTALLED_DIR}")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET AND DEFINED ENV{VCPKG_TARGET_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}")
endif()

if(NOT DEFINED TARGET_DIR OR TARGET_DIR STREQUAL "")
    message(FATAL_ERROR "qtdeploy: TARGET_DIR not provided")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR OR VCPKG_INSTALLED_DIR STREQUAL "")
    message(FATAL_ERROR "qtdeploy: VCPKG_INSTALLED_DIR not provided")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET OR VCPKG_TARGET_TRIPLET STREQUAL "")
    message(FATAL_ERROR "qtdeploy: VCPKG_TARGET_TRIPLET not provided")
endif()

# Determine config
set(_cfg "${BUILD_CONFIG}")
if(_cfg STREQUAL "" AND DEFINED CMAKE_BUILD_TYPE)
    set(_cfg "${CMAKE_BUILD_TYPE}")
endif()
string(TOUPPER "${_cfg}" _cfg)

set(_vroot "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(_cfg STREQUAL "DEBUG")
    set(_src_bin "${_vroot}/debug/bin")
    set(_src_qml "${_vroot}/debug/Qt6/qml")
    set(_src_platforms "${_vroot}/debug/Qt6/plugins/platforms")
    set(_src_imageformats "${_vroot}/debug/Qt6/plugins/imageformats")
else()
    set(_src_bin "${_vroot}/bin")
    set(_src_qml "${_vroot}/Qt6/qml")
    set(_src_platforms "${_vroot}/Qt6/plugins/platforms")
    set(_src_imageformats "${_vroot}/Qt6/plugins/imageformats")
endif()

if(NOT EXISTS "${_src_bin}")
    message(STATUS "qtdeploy: source bin ${_src_bin} not found; skipping")
    return()
endif()

# Output directories
file(MAKE_DIRECTORY "${TARGET_DIR}")

# Always reset plugin directories to avoid mixing Debug/Release artifacts between builds.
file(REMOVE_RECURSE "${TARGET_DIR}/platforms")
file(REMOVE_RECURSE "${TARGET_DIR}/imageformats")

file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
file(MAKE_DIRECTORY "${TARGET_DIR}/qml")
file(MAKE_DIRECTORY "${TARGET_DIR}/imageformats")

# Make plugin/qml lookup relative to the executable
file(WRITE "${TARGET_DIR}/qt.conf" "[Paths]\nPlugins=./\nQml2Imports=./qml\n")

# Runtime DLLs (minimal set required by current QML usage)
set(_dlls
    Qt6Core.dll
    Qt6Gui.dll
    Qt6OpenGL.dll
    Qt6Qml.dll
    Qt6QmlMeta.dll
    Qt6QmlModels.dll
    Qt6Quick.dll
    Qt6QuickControls2.dll
    Qt6QuickControls2Basic.dll
    Qt6QuickControls2Fusion.dll
    Qt6QuickControls2FusionStyleImpl.dll
    Qt6QuickControls2Impl.dll
    Qt6QuickLayouts.dll
    Qt6QuickTemplates2.dll
    Qt6Svg.dll
    Qt6Widgets.dll
    MSVCP140.dll
)
foreach(_dll IN LISTS _dlls)
    if(EXISTS "${_src_bin}/${_dll}")
        file(COPY "${_src_bin}/${_dll}" DESTINATION "${TARGET_DIR}")
    endif()
endforeach()

# Platform plugin
if(EXISTS "${_src_platforms}/qwindows.dll")
    file(COPY "${_src_platforms}/qwindows.dll" DESTINATION "${TARGET_DIR}/platforms")
endif()

# Image format plugins
if(EXISTS "${_src_imageformats}/qsvg.dll")
    file(COPY "${_src_imageformats}/qsvg.dll" DESTINATION "${TARGET_DIR}/imageformats")
endif()

if(EXISTS "${_src_imageformats}/qjpeg.dll")
    file(COPY "${_src_imageformats}/qjpeg.dll" DESTINATION "${TARGET_DIR}/imageformats")
endif()

# QML imports (+ plugin DLLs)
if(EXISTS "${_src_qml}")
    file(GLOB_RECURSE _qml "${_src_qml}/*")
    foreach(_q IN LISTS _qml)
        file(RELATIVE_PATH _rrel "${_src_qml}" "${_q}")
        get_filename_component(_dstdir "${TARGET_DIR}/qml/${_rrel}" DIRECTORY)
        if(NOT EXISTS "${_dstdir}")
            file(MAKE_DIRECTORY "${_dstdir}")
        endif()

        string(REGEX MATCH "\\.pdb$" _ispdb "${_q}")
        if(NOT _ispdb)
            file(COPY "${_q}" DESTINATION "${_dstdir}")
        endif()
    endforeach()
endif()

message(STATUS "qtdeploy: completed to ${TARGET_DIR}")
