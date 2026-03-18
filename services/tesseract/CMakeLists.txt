# Service implementation sources.
set(SERVICE_TESSERACT_SRCS
    src/TesseractAdapter.cpp
    src/TesseractEngine.cpp
    src/TesseractService.cpp
)

add_library(fossredder_tesseract_service STATIC ${SERVICE_TESSERACT_SRCS})
add_library(FossRedder::TesseractService ALIAS fossredder_tesseract_service)

set_target_properties(fossredder_tesseract_service PROPERTIES EXPORT_NAME TesseractService)

target_include_directories(fossredder_tesseract_service PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_tesseract_service PUBLIC cxx_std_20)

find_package(Tesseract REQUIRED CONFIG)
target_link_libraries(fossredder_tesseract_service
    PUBLIC
        FossRedder::Api
    PRIVATE
        FossRedder::Core
        FossRedder::Debug
        Tesseract::libtesseract
)

fossredder_set_library_output_dirs(fossredder_tesseract_service)

fossredder_install_library_target(fossredder_tesseract_service)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
