find_package(Qt6 COMPONENTS Quick Qml QuickControls2 Widgets Svg Concurrent QUIET)

# UI library sources and public headers.
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
    src/controllers/DraftProjection.cpp
    src/dialogs/FileDialogs.cpp
    src/export/AppStateSnapshot.cpp
    src/export/ExportRunner.cpp
    src/import/ImportDraftMapper.cpp
    src/import/ImportJobBridge.cpp
    src/import/ImportRunStore.cpp
    src/import/ImportSuggestionService.cpp
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
    src/state/SessionSelection.cpp
    src/state/SelectionState.cpp
    src/state/SelectionStateSync.cpp
    src/state/StateFacadeProjection.cpp
    src/state/SessionModels.cpp
    src/state/SessionModelProjection.cpp
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
    include/ui/controllers/DraftProjection.h
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
    include/ui/import/ImportSuggestionService.h
    include/ui/import/ImportState.h
    include/ui/models/ActorList.h
    include/ui/models/AnalysisList.h
    include/ui/models/AnnualList.h
    include/ui/models/ContractList.h
    include/ui/models/ImportRunList.h
    include/ui/models/ImportSuggestion.h
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
    include/ui/state/SessionSelection.h
    include/ui/state/SelectionState.h
    include/ui/state/SelectionStateSync.h
    include/ui/state/StateFacadeProjection.h
    include/ui/state/SessionModels.h
    include/ui/state/SessionModelProjection.h
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

add_library(fossredder_ui STATIC ${UI_SOURCES} ${UI_HEADERS})
add_library(FossRedder::Ui ALIAS fossredder_ui)
fossredder_enable_target_quality(fossredder_ui)

set_target_properties(fossredder_ui PROPERTIES EXPORT_NAME Ui)

set_target_properties(fossredder_ui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_include_directories(fossredder_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_ui PUBLIC cxx_std_20)
fossredder_set_library_output_dirs(fossredder_ui)

# Optional Qt UI feature blocks.
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder_ui PUBLIC USE_QML=1)
else()
    message(STATUS "Qt6 Quick/Qml not found; QML features disabled in fossredder_ui")
endif()

if(TARGET Qt6::QuickControls2)
    target_link_libraries(fossredder_ui PUBLIC Qt6::QuickControls2)
else()
    message(WARNING "Qt6 QuickControls2 not found; QML Controls may be missing at runtime")
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Widgets)
endif()

if(TARGET Qt6::Concurrent)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Concurrent)
else()
    message(WARNING "Qt6 Concurrent not found; QtConcurrent features may be unavailable")
endif()

if(TARGET Qt6::Svg)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Svg)
else()
    message(WARNING "Qt6 Svg not found; QML QtQuick.Svg module may be unavailable at runtime")
endif()

target_link_libraries(fossredder_ui
    PUBLIC
        FossRedder::Core
        FossRedder::Debug
)

fossredder_enable_target_pch(fossredder_ui "${CMAKE_CURRENT_SOURCE_DIR}/include/ui/pch.h")

# Optional module-local UI test blocks.
# Enable this to build QML-near Quick Test suites against the real UI module.
option(BUILD_UI_TESTS "Build tests for ui module" OFF)
if(BUILD_UI_TESTS)
    find_package(Qt6 COMPONENTS QuickTest Test REQUIRED)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qml" UI_QML_IMPORT_DIR)
    file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/qml" UI_QML_TEST_SOURCE_DIR)

    set(UI_UNIT_TEST_SRCS
        tests/unit/TestAnalysisPayloadMapper.cpp
        tests/unit/TestImportSuggestionService.cpp
    )

    add_executable(ui_unit_tests ${UI_UNIT_TEST_SRCS})
    target_link_libraries(ui_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::Ui
    )
    target_compile_features(ui_unit_tests PRIVATE cxx_std_20)
    fossredder_set_runtime_output_dirs(ui_unit_tests)
    fossredder_enable_target_quality(ui_unit_tests)
    gtest_discover_tests(ui_unit_tests PROPERTIES LABELS "UNIT")

    set(UI_INTERACTION_TEST_SRCS
        tests/interaction/TestImportState.cpp
    )

    add_executable(ui_interaction_tests ${UI_INTERACTION_TEST_SRCS})
    target_link_libraries(ui_interaction_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::Ui
    )
    target_compile_features(ui_interaction_tests PRIVATE cxx_std_20)
    fossredder_set_runtime_output_dirs(ui_interaction_tests)
    fossredder_enable_target_quality(ui_interaction_tests)
    gtest_discover_tests(ui_interaction_tests PROPERTIES LABELS "INTERACTION")

    add_executable(ui_qml_tests
        tests/qml/qmltests.cpp
    )

    set_target_properties(ui_qml_tests PROPERTIES
        AUTOMOC ON
    )

    target_compile_features(ui_qml_tests PRIVATE cxx_std_20)
    target_compile_definitions(ui_qml_tests PRIVATE
        FOSSREDDER_UI_QML_IMPORT_DIR="${UI_QML_IMPORT_DIR}"
        QUICK_TEST_SOURCE_DIR="${UI_QML_TEST_SOURCE_DIR}"
    )

    target_link_libraries(ui_qml_tests
        PRIVATE
            Qt6::QuickTest
            Qt6::Test
            FossRedder::Ui
    )

    fossredder_set_runtime_output_dirs(ui_qml_tests)
    fossredder_enable_target_quality(ui_qml_tests)

    add_test(NAME ui_qml_tests COMMAND ui_qml_tests -platform windows)
    set_tests_properties(ui_qml_tests PROPERTIES
        LABELS "UI;QML"
        ENVIRONMENT "QML_DISABLE_DISK_CACHE=1"
    )
endif()

fossredder_install_library_target(fossredder_ui)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
