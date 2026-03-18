include_guard(GLOBAL)

set(FOSSREDDER_EXPORT_SET "FossRedderTargets")
set(FOSSREDDER_INSTALL_CMAKEDIR "lib/cmake/FossRedder")

function(fossredder_set_runtime_output_dirs _target)
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )
endfunction()

function(fossredder_set_library_output_dirs _target)
    set_target_properties(${_target} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )
endfunction()

function(fossredder_enable_target_pch _target _header)
    if(EXISTS "${_header}")
        target_precompile_headers(${_target} PRIVATE "${_header}")
    endif()
endfunction()

function(fossredder_install_library_target _target)
    install(TARGETS ${_target}
        EXPORT ${FOSSREDDER_EXPORT_SET}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
endfunction()

function(fossredder_install_headers _include_dir)
    install(DIRECTORY "${_include_dir}/" DESTINATION include)
endfunction()
