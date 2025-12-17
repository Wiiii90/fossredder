# Minimal qtdeploy.cmake - deterministic deploy from vcpkg installed tree

# Allow variables to be passed either as CMake variables or via environment
if(NOT DEFINED TARGET_DIR AND DEFINED ENV{TARGET_DIR})
    set(TARGET_DIR "$ENV{TARGET_DIR}")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR AND DEFINED ENV{VCPKG_INSTALLED_DIR})
    set(VCPKG_INSTALLED_DIR "$ENV{VCPKG_INSTALLED_DIR}")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET AND DEFINED ENV{VCPKG_TARGET_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}")
endif()
if(NOT DEFINED BUILD_CONFIG AND DEFINED ENV{BUILD_CONFIG})
    set(BUILD_CONFIG "$ENV{BUILD_CONFIG}")
endif()

if(NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "qtdeploy: TARGET_DIR not provided")
endif()

# VCPKG_INSTALLED_DIR and VCPKG_TARGET_TRIPLET are optional; if missing we'll skip copying
if(NOT DEFINED VCPKG_INSTALLED_DIR)
    message(STATUS "qtdeploy: VCPKG_INSTALLED_DIR not defined; skipping vcpkg-based deploy steps")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    message(STATUS "qtdeploy: VCPKG_TARGET_TRIPLET not defined; skipping vcpkg-based deploy steps")
endif()

set(_vroot "")
if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    set(_vroot "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
endif()
set(_imgfmt_src "${_vroot}/Qt6/plugins/imageformats")
set(_qml_src "${_vroot}/share/Qt6/qml")

# Ensure target dirs
file(MAKE_DIRECTORY "${TARGET_DIR}")
file(MAKE_DIRECTORY "${TARGET_DIR}/imageformats")
file(MAKE_DIRECTORY "${TARGET_DIR}/qml")
file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")

# Copy core Qt DLLs from vcpkg bin (if present)
set(_src_bin "${_vroot}/bin")
set(_dlls
    "Qt6Core.dll" "Qt6Qml.dll" "Qt6Quick.dll" "Qt6QmlModels.dll" "Qt6QmlMeta.dll"
    "Qt6Gui.dll" "Qt6Widgets.dll" "Qt6OpenGL.dll"
)
foreach(_dll IN LISTS _dlls)
    if(EXISTS "${_src_bin}/${_dll}")
        file(COPY "${_src_bin}/${_dll}" DESTINATION "${TARGET_DIR}")
    endif()
endforeach()

# Copy imageformats
if(EXISTS "${_imgfmt_src}")
    file(GLOB _imgfmts "${_imgfmt_src}/*.dll")
    foreach(_if IN LISTS _imgfmts)
        file(COPY "${_if}" DESTINATION "${TARGET_DIR}/imageformats")
    endforeach()
    message(STATUS "qtdeploy: copied imageformats from ${_imgfmt_src}")
else()
    message(STATUS "qtdeploy: imageformats source not found (${_imgfmt_src}); skipping imageformats copy")
endif()

# Copy QML modules
if(EXISTS "${_qml_src}")
    file(GLOB_RECURSE _qml_files "${_qml_src}/*")
    foreach(_q IN LISTS _qml_files)
        string(REGEX MATCH ".*\\.dll$" _isdll "${_q}")
        if(NOT _isdll)
            file(RELATIVE_PATH _rrel "${_qml_src}" "${_q}")
            get_filename_component(_dstdir "${TARGET_DIR}/qml/${_rrel}" DIRECTORY)
            if(NOT EXISTS "${_dstdir}")
                file(MAKE_DIRECTORY "${_dstdir}")
            endif()
            file(COPY "${_q}" DESTINATION "${_dstdir}")
        endif()
    endforeach()
    message(STATUS "qtdeploy: copied QML tree from ${_qml_src} to ${TARGET_DIR}/qml")
else()
    message(STATUS "qtdeploy: QML source not found (${_qml_src}); skipping QML copy")
endif()

# Copy platform plugins if present
set(_platform_src "${_vroot}/Qt6/plugins/platforms")
if(EXISTS "${_platform_src}")
    file(GLOB _plats "${_platform_src}/*.dll")
    foreach(_p IN LISTS _plats)
        file(COPY "${_p}" DESTINATION "${TARGET_DIR}/platforms")
    endforeach()
    message(STATUS "qtdeploy: copied platform plugins from ${_platform_src}")
else()
    message(STATUS "qtdeploy: platform plugins source not found (${_platform_src}); skipping platforms copy")
endif()

message(STATUS "qtdeploy: deploy finished to ${TARGET_DIR}")
