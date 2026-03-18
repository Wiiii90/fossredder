# Public header-only contracts shared by the implementation libraries.
add_library(fossredder_api INTERFACE)
add_library(FossRedder::Api ALIAS fossredder_api)

set_target_properties(fossredder_api PROPERTIES EXPORT_NAME Api)

target_compile_features(fossredder_api INTERFACE cxx_std_20)

target_include_directories(fossredder_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS fossredder_api EXPORT ${FOSSREDDER_EXPORT_SET} INCLUDES DESTINATION include)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
