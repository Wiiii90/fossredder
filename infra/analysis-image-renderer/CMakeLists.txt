# Infra analysis image renderer adapter.
set(INFRA_ANALYSIS_IMAGE_RENDERER_SRCS
    src/OpenCvAnalysisImageRendererAdapter.cpp
)

add_library(fossredder_infra_analysis_image_renderer STATIC ${INFRA_ANALYSIS_IMAGE_RENDERER_SRCS})
add_library(FossRedder::InfraAnalysisImageRenderer ALIAS fossredder_infra_analysis_image_renderer)
fossredder_enable_target_quality(fossredder_infra_analysis_image_renderer)

set_target_properties(fossredder_infra_analysis_image_renderer PROPERTIES EXPORT_NAME InfraAnalysisImageRenderer)

target_include_directories(fossredder_infra_analysis_image_renderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_analysis_image_renderer PUBLIC cxx_std_20)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(fossredder_infra_analysis_image_renderer
    PUBLIC
        FossRedder::Core
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

fossredder_set_library_output_dirs(fossredder_infra_analysis_image_renderer)

fossredder_install_library_target(fossredder_infra_analysis_image_renderer)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
