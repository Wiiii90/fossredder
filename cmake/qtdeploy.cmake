# qtdeploy.cmake - deterministic copy fallback
# Copies Qt runtime DLLs, platform plugins and QML modules from vcpkg_installed into TARGET_DIR.

if(NOT DEFINED TARGET_DIR)
    if(DEFINED ENV{TARGET_DIR})
        set(TARGET_DIR $ENV{TARGET_DIR})
    endif()
endif()

if(NOT DEFINED VCPKG_INSTALLED_DIR)
    if(DEFINED ENV{VCPKG_INSTALLED_DIR})
        set(VCPKG_INSTALLED_DIR $ENV{VCPKG_INSTALLED_DIR})
    endif()
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(DEFINED ENV{VCPKG_TARGET_TRIPLET})
        set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_TARGET_TRIPLET})
    endif()
endif()

if(NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "qtdeploy: TARGET_DIR not provided")
endif()

# Choose src paths deterministically based on BUILD_CONFIG or CMAKE_BUILD_TYPE
set(_cfg "${BUILD_CONFIG}")
if(_cfg STREQUAL "")
    if(DEFINED CMAKE_BUILD_TYPE)
        set(_cfg "${CMAKE_BUILD_TYPE}")
    endif()
endif()
string(TOUPPER _cfg "${_cfg}")

set(_vroot "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(_cfg STREQUAL "DEBUG")
    set(_src_bin "${_vroot}/debug/bin")
    set(_src_qml "${_vroot}/debug/Qt6/qml")
    set(_src_plugins "${_vroot}/debug/Qt6/plugins/platforms")
else()
    set(_src_bin "${_vroot}/bin")
    set(_src_qml "${_vroot}/Qt6/qml")
    set(_src_plugins "${_vroot}/Qt6/plugins/platforms")
endif()

if(NOT EXISTS "${_src_bin}")
    message(STATUS "qtdeploy: source bin ${_src_bin} not found; skipping copy")
    return()
endif()

file(MAKE_DIRECTORY "${TARGET_DIR}")
file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
file(MAKE_DIRECTORY "${TARGET_DIR}/qml")

# copy core DLLs - minimal list
set(_dlls
    "Qt6Core.dll" "Qt6Qml.dll" "Qt6Quick.dll" "Qt6QmlModels.dll" "Qt6QmlMeta.dll"
    "Qt6Gui.dll" "Qt6Widgets.dll" "Qt6OpenGL.dll"
)
foreach(_dll IN LISTS _dlls)
    if(EXISTS "${_src_bin}/${_dll}")
        file(COPY "${_src_bin}/${_dll}" DESTINATION "${TARGET_DIR}")
    endif()
endforeach()

# copy platform plugins
file(GLOB _plats "${_src_plugins}/*.dll")
foreach(_p IN LISTS _plats)
    file(COPY "${_p}" DESTINATION "${TARGET_DIR}/platforms")
endforeach()

# copy QML tree contents (avoid nested qml/qml)
file(GLOB_RECURSE _qml "${_src_qml}/*")
foreach(_q IN LISTS _qml)
    get_filename_component(_rel ${_q} PATH)
    file(RELATIVE_PATH _rrel "${_src_qml}" "${_q}")
    get_filename_component(_dstdir "${TARGET_DIR}/qml/${_rrel}" DIRECTORY)
    if(NOT EXISTS "${_dstdir}")
        file(MAKE_DIRECTORY "${_dstdir}")
    endif()
    # skip dlls (platform plugins handled earlier)
    string(REGEX MATCH ".*\\.dll$" _isdll "${_q}")
    if(NOT _isdll)
        file(COPY "${_q}" DESTINATION "${_dstdir}")
    endif()
endforeach()

message(STATUS "qtdeploy: copy completed to ${TARGET_DIR}")
