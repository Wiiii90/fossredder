# Service implementation sources.
set(SERVICE_OPENCV_SRCS
    src/CropEngine.cpp
    src/DenoiseEngine.cpp
    src/DetectEngine.cpp
    src/MaskEngine.cpp
    src/OpenCvAdapter.cpp
    src/OpenCvService.cpp
)

add_library(fossredder_opencv_service STATIC ${SERVICE_OPENCV_SRCS})
add_library(FossRedder::OpenCvService ALIAS fossredder_opencv_service)
fossredder_enable_target_quality(fossredder_opencv_service)

set_target_properties(fossredder_opencv_service PROPERTIES EXPORT_NAME OpenCvService)

target_include_directories(fossredder_opencv_service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_opencv_service PUBLIC cxx_std_20)

# OpenCV types are part of the public service headers.
find_package(OpenCV REQUIRED)
target_link_libraries(fossredder_opencv_service
    PUBLIC
        FossRedder::Api
        ${OpenCV_LIBS}
    PRIVATE
        FossRedder::Core
        FossRedder::Debug
)

fossredder_set_library_output_dirs(fossredder_opencv_service)

fossredder_install_library_target(fossredder_opencv_service)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
