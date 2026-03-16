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

# Some callers (VS/CMake custom commands) may pass values already quoted.
# Normalize to plain paths.
if(DEFINED VCPKG_INSTALLED_DIR)
    string(REPLACE "\"" "" VCPKG_INSTALLED_DIR "${VCPKG_INSTALLED_DIR}")
endif()
if(DEFINED VCPKG_TARGET_TRIPLET)
    string(REPLACE "\"" "" VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}")
endif()
if(DEFINED TARGET_DIR)
    string(REPLACE "\"" "" TARGET_DIR "${TARGET_DIR}")
endif()
if(DEFINED BUILD_CONFIG)
    string(REPLACE "\"" "" BUILD_CONFIG "${BUILD_CONFIG}")
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
if(_cfg MATCHES "\\$<CONFIG>" OR _cfg STREQUAL "")
    string(TOUPPER "${TARGET_DIR}" _target_dir_upper)
    if(_target_dir_upper MATCHES "[/\\]DEBUG$")
        set(_cfg "DEBUG")
    elseif(_target_dir_upper MATCHES "[/\\]RELEASE$")
        set(_cfg "RELEASE")
    endif()
endif()
string(TOUPPER "${_cfg}" _cfg)

set(_vroot "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(_cfg STREQUAL "DEBUG")
    set(_src_bin "${_vroot}/debug/bin")
    set(_qt_dll_suffix "d")
    set(_qt_plugin_suffix "d")

    set(_src_qml "${_vroot}/debug/qml")
    if(NOT EXISTS "${_src_qml}")
        set(_src_qml "${_vroot}/debug/Qt6/qml")
    endif()

    # vcpkg layouts differ depending on port/version; check both
    set(_src_platforms "${_vroot}/debug/plugins/platforms")
    if(NOT EXISTS "${_src_platforms}")
        set(_src_platforms "${_vroot}/debug/Qt6/plugins/platforms")
    endif()

    set(_src_imageformats "${_vroot}/debug/plugins/imageformats")
    if(NOT EXISTS "${_src_imageformats}")
        set(_src_imageformats "${_vroot}/debug/Qt6/plugins/imageformats")
    endif()
else()
    set(_src_bin "${_vroot}/bin")
    set(_qt_dll_suffix "")
    set(_qt_plugin_suffix "")

    set(_src_qml "${_vroot}/qml")
    if(NOT EXISTS "${_src_qml}")
        set(_src_qml "${_vroot}/Qt6/qml")
    endif()

    set(_src_platforms "${_vroot}/plugins/platforms")
    if(NOT EXISTS "${_src_platforms}")
        set(_src_platforms "${_vroot}/Qt6/plugins/platforms")
    endif()

    set(_src_imageformats "${_vroot}/plugins/imageformats")
    if(NOT EXISTS "${_src_imageformats}")
        set(_src_imageformats "${_vroot}/Qt6/plugins/imageformats")
    endif()
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
file(REMOVE_RECURSE "${TARGET_DIR}/qml")

file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
file(MAKE_DIRECTORY "${TARGET_DIR}/qml")
file(MAKE_DIRECTORY "${TARGET_DIR}/imageformats")

# Make plugin/qml lookup relative to the executable
file(WRITE "${TARGET_DIR}/qt.conf" "[Paths]\nPlugins=./\nQml2Imports=./qml\n")

# Runtime DLLs (minimal set required by current QML usage)
set(_dlls
    Qt6Core${_qt_dll_suffix}.dll
    Qt6Gui${_qt_dll_suffix}.dll
    Qt6OpenGL${_qt_dll_suffix}.dll
    Qt6Qml${_qt_dll_suffix}.dll
    Qt6QmlMeta${_qt_dll_suffix}.dll
    Qt6QmlModels${_qt_dll_suffix}.dll
    Qt6Quick${_qt_dll_suffix}.dll
    Qt6QuickEffects${_qt_dll_suffix}.dll
    Qt6QuickControls2${_qt_dll_suffix}.dll
    Qt6QuickControls2Basic${_qt_dll_suffix}.dll
    Qt6QuickControls2Fusion${_qt_dll_suffix}.dll
    Qt6QuickControls2FusionStyleImpl${_qt_dll_suffix}.dll
    Qt6QuickControls2Impl${_qt_dll_suffix}.dll
    Qt6QuickLayouts${_qt_dll_suffix}.dll
    Qt6QuickTemplates2${_qt_dll_suffix}.dll
    Qt6Svg${_qt_dll_suffix}.dll
    Qt6Widgets${_qt_dll_suffix}.dll
    MSVCP140.dll
)
foreach(_dll IN LISTS _dlls)
    if(EXISTS "${_src_bin}/${_dll}")
        file(COPY "${_src_bin}/${_dll}" DESTINATION "${TARGET_DIR}")
    endif()
endforeach()

# Platform plugin
if(EXISTS "${_src_platforms}/qwindows${_qt_plugin_suffix}.dll")
    file(COPY "${_src_platforms}/qwindows${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/platforms")
else()
    message(WARNING "qtdeploy: qwindows${_qt_plugin_suffix}.dll not found under ${_src_platforms}")
endif()

# Image format plugins
if(EXISTS "${_src_imageformats}/qsvg${_qt_plugin_suffix}.dll")
    file(COPY "${_src_imageformats}/qsvg${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/imageformats")
endif()

if(EXISTS "${_src_imageformats}/qjpeg${_qt_plugin_suffix}.dll")
    file(COPY "${_src_imageformats}/qjpeg${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/imageformats")
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

# Ensure Qt.labs.folderlistmodel plugin DLL is deployed (used by AppFilePicker)
set(_target_folderlist_dir "${TARGET_DIR}/qml/Qt/labs/folderlistmodel")
file(MAKE_DIRECTORY "${_target_folderlist_dir}")

set(_found_folderlist 0)
set(_folderlist_candidates
    "${_src_qml}/Qt/labs/folderlistmodel/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
    "${_src_bin}/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
    "${_vroot}/plugins/labs/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
)
foreach(_ld IN LISTS _folderlist_candidates)
    if(EXISTS "${_ld}")
        file(COPY "${_ld}" DESTINATION "${_target_folderlist_dir}")
        set(_found_folderlist 1)
        break()
    endif()
endforeach()

if(NOT _found_folderlist)
    message(WARNING "qtdeploy: qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll not found; Qt.labs.folderlistmodel may fail to load at runtime")
else()
    message(STATUS "qtdeploy: copied qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll to ${_target_folderlist_dir}")
endif()

# Ensure QtQuick Effects plugin DLL is deployed (consistent with other plugin copies)
set(_target_effects_dir "${TARGET_DIR}/qml/QtQuick/Effects")
file(MAKE_DIRECTORY "${_target_effects_dir}")

set(_effects_candidates
    "${_src_qml}/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
    "${_vroot}/plugins/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
    "${_src_bin}/qml/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
)

set(_found_effects 0)
foreach(_ed IN LISTS _effects_candidates)
    if(EXISTS "${_ed}")
        file(COPY "${_ed}" DESTINATION "${_target_effects_dir}")
        set(_found_effects 1)
        break()
    endif()
endforeach()

if(NOT _found_effects)
    message(WARNING "qtdeploy: effectsplugin${_qt_plugin_suffix}.dll not found; QtQuick.Effects may fail to load at runtime")
else()
    message(STATUS "qtdeploy: copied effectsplugin${_qt_plugin_suffix}.dll to ${_target_effects_dir}")
endif()

message(STATUS "qtdeploy: completed to ${TARGET_DIR}")
