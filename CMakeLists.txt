cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Qt Integration (Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add all source and header files
add_executable(fossredder
    src/main.cpp

    # Controllers
    src/controllers/PdfImportController.cpp
    src/controllers/CsvImportController.cpp

    # Models
    src/models/Property.cpp
    src/models/PdfExtractedData.cpp
    src/models/Block.cpp
    src/models/Paragraph.cpp
    src/models/Line.cpp
    src/models/Word.cpp
    src/models/TextElement.cpp
    src/models/Header.cpp
    src/models/Page.cpp
    src/models/BookingGroup.cpp
    src/models/Transaction.cpp
    src/models/Actor.cpp
    src/models/Category.cpp
    src/models/Statement.cpp

    # Views
    src/views/MainWindow.cpp
    src/views/ConsoleView.cpp
    src/views/ManageStatementsWidget.cpp
    src/views/ManageActorsWidget.cpp
    src/views/ManageCategoriesWidget.cpp
    src/views/ManagePropertiesWidget.cpp
    src/views/DashboardWidget.cpp
    src/views/ManageAnnualCostWidget.cpp
    src/views/ImportDialog.cpp

    # Managers
    src/managers/StatementManager.cpp
    src/managers/TransactionManager.cpp
    src/managers/ConfigManager.cpp
    src/managers/BookingGroupManager.cpp
    src/managers/ActorManager.cpp
    src/managers/CategoryManager.cpp
    src/managers/PropertyManager.cpp

    # Persistence
    src/persistence/JsonSaver.cpp

    # OCR & PDF
    src/ocr/TesseractOcrEngine.cpp
    src/poppler/PopplerPdfRenderer.cpp

    # Optional: Header-Dateien für IDEs
    include/views/MainWindow.h
    include/views/ManageStatementsWidget.h
    include/views/ManageActorsWidget.h
    include/views/ManageCategoriesWidget.h
    include/views/ManagePropertiesWidget.h
    include/views/DashboardWidget.h
    include/views/ManageAnnualCostWidget.h
    include/views/ImportDialog.h
    include/managers/ActorManager.h
    include/managers/IActorManager.h
    include/managers/CategoryManager.h
    include/managers/ICategoryManager.h
    include/managers/PropertyManager.h
    include/managers/IPropertyManager.h
)

# === Llama.cpp einbinden ===
add_subdirectory(extern/llama.cpp)
target_link_libraries(fossredder PRIVATE llama)

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

# Link Qt6 Widgets
target_link_libraries(fossredder PRIVATE Qt6::Widgets)

# Setze Zielverzeichnis für die Plugins
set(QT_PLUGINS_DIR "${CMAKE_SOURCE_DIR}/out/build/x64-Release-vcpkg/vcpkg_installed/x64-windows/Qt6/plugins/platforms")
set(DEPLOY_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/platforms")

add_custom_command(TARGET fossredder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLUGINS_DIR}/qwindows.dll"
        "${DEPLOY_DIR}/qwindows.dll"
)