cmake_minimum_required(VERSION 4.0.1)
project(FossRedder)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# 🔧 Erzwinge konsistente CRT unter MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Qt Integration (Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add all source and header files
add_executable(fossredder
    src/main.cpp

    # Models
    src/models/Property.cpp
    src/models/StatementData.cpp
    
    src/models/layout/BookingGroup.cpp
    src/models/layout/Header.cpp
    src/models/layout/Statement.cpp
    src/models/layout/Transaction.cpp

    src/models/Actor.cpp
    src/models/Category.cpp

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
    src/views/TransactionReviewDialog.cpp

    # Controllers
    src/controllers/StatementController.cpp

    # Persistence
    src/managers/StatementManager.cpp
    src/managers/TransactionManager.cpp
    src/managers/ConfigManager.cpp
    src/managers/BookingGroupManager.cpp
    src/managers/ActorManager.cpp
    src/managers/CategoryManager.cpp
    src/managers/PropertyManager.cpp

    src/persistence/JsonSaver.cpp

    # Services
    src/services/tesseract/TesseractAdapter.cpp
    src/services/tesseract/TesseractService.cpp
    src/services/tesseract/TesseractEngine.cpp

    src/services/poppler/PopplerAdapter.cpp
    src/services/poppler/PopplerService.cpp
    src/services/poppler/PopplerEngine.cpp

    src/services/opencv/OpenCvAdapter.cpp
    src/services/opencv/OpenCvService.cpp
    src/services/opencv/OpenCvEngine.cpp

    # Statement extraction service & strategy
    src/services/StatementExtractionService.cpp
    src/services/DefaultExtractionStrategy.cpp

    # Llama
    src/llama/LlamaEngine.cpp

    # Utils
    src/utils/OutputParser.cpp
    src/utils/Env.cpp
    src/utils/RuleBasedStatementParser.cpp

    # ONNX Cleaner
    src/onnx/OnnxTextCleaner.cpp
    src/onnx/Byt5Tokenizer.cpp
    src/onnx/FlanT5Tokenizer.cpp
    src/onnx/OnnxConfigFactory.cpp

    # Debug
    src/debug/FileDebugger.cpp
    src/debug/SpdlogDebugger.cpp

    # Optional: Header-Dateien für IDEs
    include/views/MainWindow.h
    include/views/ManageStatementsWidget.h
    include/views/ManageActorsWidget.h
    include/views/ManageCategoriesWidget.h
    include/views/ManagePropertiesWidget.h
    include/views/DashboardWidget.h
    include/views/ManageAnnualCostWidget.h
    include/views/ImportDialog.h
    include/views/TransactionReviewDialog.h
    include/managers/ActorManager.h
    include/managers/IActorManager.h
    include/managers/CategoryManager.h
    include/managers/ICategoryManager.h
    include/managers/PropertyManager.h
    include/utils/OutputParser.h
    include/utils/Env.h
    include/utils/StatementParser.h
    include/llama/prompts/Prompts.h
    include/onnx/ITextCleaner.h
    include/onnx/OnnxTextCleaner.h
    include/onnx/OnnxConfigFactory.h
    include/models/StatementData.h    
    include/models/layout/BookingGroup.h
    include/models/layout/Header.h    
    include/models/layout/Transaction.h
    include/models/layout/Statement.h
    include/services/tesseract/ITesseractAdapter.h
    include/services/tesseract/ITesseractService.h
    include/services/tesseract/TesseractEngine.h
    include/services/tesseract/TesseractDTO.h
    include/services/poppler/IPopplerAdapter.h
    include/services/poppler/IPopplerService.h
    include/services/poppler/PopplerEngine.h
    include/services/poppler/PopplerDTO.h
    include/services/poppler/PopplerServiceTypes.h
    include/services/opencv/IOpenCvAdapter.h
    include/services/opencv/IOpenCvService.h
    include/services/opencv/OpenCvEngine.h
    include/services/opencv/OpenCvDTO.h
    include/debug/IDebugger.h
    include/debug/FileDebugger.h
    include/debug/SpdlogDebugger.h
)

# Include directories
target_include_directories(fossredder PRIVATE
    include
    include/models
    include/models/layout
    include/controllers
    include/views
    include/managers
    include/services/tesseract
    include/services/poppler
    include/services/opencv
    include/onnx
    extern/llama.cpp/include
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
qt6_wrap_cpp(MOC_SRCS include/controllers/StatementController.h)
target_sources(fossredder PRIVATE ${MOC_SRCS})

# Link Llama
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(extern/llama.cpp)
target_link_libraries(fossredder PRIVATE llama)

# nlohmann_json for FlanT5Tokenizer
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(fossredder PRIVATE nlohmann_json::nlohmann_json)

# spdlog (vcpkg)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(fossredder PRIVATE spdlog::spdlog)

# ==========================
# ONNX Runtime (manual link via vcpkg)
# ==========================
set(_ORT_ROOT "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
find_path(ONNXRUNTIME_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h
    PATHS "${_ORT_ROOT}/include" "${_ORT_ROOT}/include/onnxruntime"
    NO_DEFAULT_PATH REQUIRED)
find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime
    PATHS "${_ORT_ROOT}/lib" "${_ORT_ROOT}/lib/manual-link"
    NO_DEFAULT_PATH REQUIRED)
message(STATUS "ONNX Runtime include: ${ONNXRUNTIME_INCLUDE_DIR}")
message(STATUS "ONNX Runtime lib    : ${ONNXRUNTIME_LIBRARY}")
target_include_directories(fossredder PRIVATE "${ONNXRUNTIME_INCLUDE_DIR}")
target_link_libraries(fossredder PRIVATE "${ONNXRUNTIME_LIBRARY}")

# LlamaModelTest (Standalone Test für LLM)
add_executable(LlamaModelTest src/llama/LlamaModelTest.cpp src/llama/LlamaEngine.cpp include/onnx/OnnxConfigFactory.h)
target_include_directories(LlamaModelTest PRIVATE
    include
    include/models
    include/controllers
    include/views
    include/managers
    include/tesseract
    include/poppler
    include/services/poppler
    include/poppler
    extern/llama.cpp/include
)
target_link_libraries(LlamaModelTest PRIVATE
    llama
    Qt6::Widgets
    Tesseract::libtesseract
    tinyxml2::tinyxml2
    ICU::uc ICU::i18n
    PkgConfig::POPPLER_CPP
    ${Fontconfig_LIBRARIES}
)

# OpenCV einbinden
find_package(OpenCV REQUIRED)
target_link_libraries(fossredder PRIVATE ${OpenCV_LIBS})
target_include_directories(fossredder PRIVATE ${OpenCV_INCLUDE_DIRS})
# expose service includes
target_include_directories(fossredder PRIVATE include/services/opencv)

# Setze Zielverzeichnis für die Plugins
set(QT_PLUGINS_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms")
set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/platforms")

add_custom_command(TARGET fossredder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLUGINS_DIR}/qwindows.dll"
        "${DEPLOY_DIR}/qwindows.dll"
)