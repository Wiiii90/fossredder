cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add all source and header files
add_executable(fossredder
    src/main.cpp
    src/controllers/PropertyController.cpp
    src/controllers/PdfImportController.cpp
    src/models/Property.cpp
    src/models/PdfExtractedData.cpp
    src/models/Block.cpp
    src/models/Paragraph.cpp
    src/models/Line.cpp
    src/models/Word.cpp
    src/models/TextElement.cpp
    src/models/Header.cpp
    src/models/Page.cpp
    src/models/Transaction.cpp
    src/views/ConsoleView.cpp
    src/ocr/TesseractOcrEngine.cpp
    src/poppler/PopplerPdfRenderer.cpp
)

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build)

# Include directories
target_include_directories(fossredder PRIVATE
    include
    include/models
    include/controllers
    include/views
    include/ocr
    include/poppler
)

# Link Poppler
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
target_link_libraries(fossredder PRIVATE PkgConfig::POPPLER_CPP)

# Link Tesseract
find_package(Tesseract REQUIRED CONFIG)
target_link_libraries(fossredder PRIVATE Tesseract::libtesseract)

# Link Fontconfig
find_package(Fontconfig REQUIRED)
target_include_directories(fossredder PRIVATE ${Fontconfig_INCLUDE_DIRS})
target_link_libraries(fossredder PRIVATE ${Fontconfig_LIBRARIES})

# Link tinyxml2
find_package(tinyxml2 REQUIRED)
target_link_libraries(fossredder PRIVATE tinyxml2::tinyxml2)

# Link ICU
find_package(ICU REQUIRED COMPONENTS uc i18n)
target_link_libraries(fossredder PRIVATE ICU::uc ICU::i18n)