cmake_minimum_required(VERSION 4.0.1)
project(ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt Quick/Qml and Controls so QML modules are available
find_package(Qt6 COMPONENTS Quick Qml QuickControls2 Widgets QuickWidgets Svg QUIET)
find_package(Qt6 COMPONENTS Concurrent QUIET)

set(UI_SOURCES
    src/MainWindow.cpp
    src/actions/Actions.cpp
    src/analysis/AnalysisPayloadMapper.cpp
    src/bootstrap/QmlRuntime.cpp
    src/controllers/ActorController.cpp
    src/controllers/AnalysisController.cpp
    src/controllers/AnnualController.cpp
    src/controllers/ContractController.cpp
    src/controllers/DraftController.cpp
    src/controllers/ExportController.cpp
    src/controllers/FileSystemController.cpp
    src/controllers/ImportController.cpp
    src/controllers/LanguageController.cpp
    src/controllers/PropertyController.cpp
    src/controllers/StatementController.cpp
    src/controllers/StorageController.cpp
    src/controllers/TransactionController.cpp
    src/dialogs/FileDialogs.cpp
    src/export/AppStateSnapshot.cpp
    src/export/ExportRunner.cpp
    src/import/ImportDraftMapper.cpp
    src/import/ImportJobBridge.cpp
    src/import/ImportRunStore.cpp
    src/import/ImportState.cpp
    src/models/ActorList.cpp
    src/models/AnalysisList.cpp
    src/models/AnnualList.cpp
    src/models/ContractList.cpp
    src/models/ImportRunList.cpp
    src/models/PropertyList.cpp
    src/models/StatementDraft.cpp
    src/models/StatementList.cpp
    src/models/TransactionDraftList.cpp
    src/models/TransactionFilter.cpp
    src/models/TransactionList.cpp
    src/providers/DraftProofProvider.cpp
    src/state/FilterState.cpp
    src/state/MetricsState.cpp
    src/state/NavigationState.cpp
    src/state/PropertyNameCatalog.cpp
    src/state/SelectionState.cpp
    src/state/SessionModels.cpp
    src/state/SessionMutationState.cpp
    src/state/SessionStore.cpp
    src/state/StateFacade.cpp
    src/window/CloseWorkflow.cpp
    src/window/DropController.cpp
    src/window/MainWindowContext.cpp
    src/workflows/FileWorkflow.cpp
    qml/qml.qrc
)

set(UI_HEADERS
    include/MainWindow.h
    include/ui/pch.h
    include/ui/actions/Actions.h
    include/ui/analysis/AnalysisPayloadMapper.h
    include/ui/bootstrap/QmlContracts.h
    include/ui/bootstrap/QmlRuntime.h
    include/ui/config/Defaults.h
    include/ui/controllers/ActorController.h
    include/ui/controllers/AnalysisController.h
    include/ui/controllers/AnnualController.h
    include/ui/controllers/ContractController.h
    include/ui/controllers/ControllerContracts.h
    include/ui/controllers/ControllerGuard.h
    include/ui/controllers/ControllerStrings.h
    include/ui/controllers/DraftController.h
    include/ui/controllers/ExportController.h
    include/ui/controllers/FileSystemController.h
    include/ui/controllers/ImportController.h
    include/ui/controllers/LanguageController.h
    include/ui/controllers/PropertyController.h
    include/ui/controllers/StatementController.h
    include/ui/controllers/StorageController.h
    include/ui/controllers/TransactionController.h
    include/ui/dialogs/FileDialogs.h
    include/ui/export/AppStateSnapshot.h
    include/ui/export/ExportRunner.h
    include/ui/import/ImportDraftMapper.h
    include/ui/import/ImportJobBridge.h
    include/ui/import/ImportRunStore.h
    include/ui/import/ImportState.h
    include/ui/models/ActorList.h
    include/ui/models/AnalysisList.h
    include/ui/models/AnnualList.h
    include/ui/models/ContractList.h
    include/ui/models/ImportRunList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/models/IndexedListModel.h
    include/ui/models/PropertyList.h
    include/ui/models/RowListModel.h
    include/ui/models/StatementDraft.h
    include/ui/models/StatementList.h
    include/ui/models/TransactionDraft.h
    include/ui/models/TransactionDraftList.h
    include/ui/models/TransactionFilter.h
    include/ui/models/TransactionList.h
    include/ui/observability/Origins.h
    include/ui/observability/Trace.h
    include/ui/payload/PayloadKeys.h
    include/ui/providers/DraftProofProvider.h
    include/ui/state/FilterState.h
    include/ui/state/MetricsState.h
    include/ui/state/NavigationState.h
    include/ui/state/AppStateClone.h
    include/ui/state/PropertyNameCatalog.h
    include/ui/state/SelectionState.h
    include/ui/state/SessionModels.h
    include/ui/state/SessionMutationState.h
    include/ui/state/SessionStore.h
    include/ui/state/StateFacade.h
    include/ui/state/StatusState.h
    include/ui/text/Text.h
    include/ui/window/CloseWorkflow.h
    include/ui/window/DropController.h
    include/ui/window/MainWindowContext.h
    include/ui/window/MainWindowTrace.h
    include/ui/workflows/FileWorkflow.h
)

add_library(ui_lib STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(ui_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ui_lib PUBLIC cxx_std_20)

# Log which Qt components were found for easier debugging and link them
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    message(STATUS "Qt6 Quick and Qml found; enabling USE_QML for ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::Quick Qt6::Qml)
    target_compile_definitions(ui_lib PUBLIC USE_QML=1)
else()
    message(STATUS "Qt6 Quick/Qml not found; QML features disabled in ui_lib")
endif()

# Link Quick Controls 2 so QML imports QtQuick.Controls and QtQuick.Layouts work
if(TARGET Qt6::QuickControls2)
    message(STATUS "Qt6 QuickControls2 found; linking to ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::QuickControls2)
else()
    message(WARNING "Qt6 QuickControls2 not found; QML Controls may be missing at runtime")
endif()

if(TARGET Qt6::Widgets)
    message(STATUS "Qt6 Widgets found; linking Widgets to ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::Widgets)
endif()

if(TARGET Qt6::Concurrent)
    message(STATUS "Qt6 Concurrent found; linking Concurrent to ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::Concurrent)
else()
    message(WARNING "Qt6 Concurrent not found; QtConcurrent features may be unavailable")
endif()

if(TARGET Qt6::QuickWidgets)
    message(STATUS "Qt6 QuickWidgets found; linking QuickWidgets to ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::QuickWidgets)
endif()

if(TARGET Qt6::Svg)
    message(STATUS "Qt6 Svg found; linking Svg to ui_lib")
    target_link_libraries(ui_lib PUBLIC Qt6::Svg)
else()
    message(WARNING "Qt6 Svg not found; QML QtQuick.Svg module may be unavailable at runtime")
endif()

# link project libs
target_link_libraries(ui_lib PUBLIC core_lib debug_lib)

# Precompiled header
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
        target_precompile_headers(ui_lib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")
    endif()
endif()

install(TARGETS ui_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
