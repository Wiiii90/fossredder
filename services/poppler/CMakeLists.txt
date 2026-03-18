# Service implementation sources.
set(SERVICE_POPPLER_SRCS
    src/PopplerAdapter.cpp
    src/PopplerEngine.cpp
    src/PopplerService.cpp
)

add_library(fossredder_poppler_service STATIC ${SERVICE_POPPLER_SRCS})
add_library(FossRedder::PopplerService ALIAS fossredder_poppler_service)
fossredder_enable_target_quality(fossredder_poppler_service)

set_target_properties(fossredder_poppler_service PROPERTIES EXPORT_NAME PopplerService)

target_include_directories(fossredder_poppler_service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_poppler_service PUBLIC cxx_std_20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(fossredder_poppler_service
    PUBLIC
        FossRedder::Api
    PRIVATE
        FossRedder::Core
        FossRedder::Debug
        PkgConfig::POPPLER_CPP
        nlohmann_json::nlohmann_json
        ${OpenCV_LIBS}
)

fossredder_set_library_output_dirs(fossredder_poppler_service)

fossredder_install_library_target(fossredder_poppler_service)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
