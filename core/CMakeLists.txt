# Core implementation sources.
set(CORE_SRCS
    src/analysis/AnalysisEngine.cpp
    src/analysis/Filter.cpp
    src/analysis/OutputTypeResolver.cpp
    src/analysis/TransactionCollector.cpp
    src/analysis/strategies/CalcAnalysisStrategy.cpp
    src/analysis/strategies/PlotAnalysisStrategy.cpp
    src/analysis/strategies/TabAnalysisStrategy.cpp
    src/application/AnalysisService.cpp
    src/application/AppStateManager.cpp
    src/application/AppStateFacade.cpp
    src/application/CatalogService.cpp
    src/application/DraftFinalizer.cpp
    src/application/StateHydrator.cpp
    src/application/StateProjector.cpp
    src/application/WorkspaceSession.cpp
    src/errors/ErrorReporterRegistry.cpp
    src/export/CsvExporter.cpp
    src/export/ExportService.cpp
    src/export/XlsxExporter.cpp
    src/import/DefaultImportStatementStrategy.cpp
    src/import/ImportPipelineHelpers.cpp
    src/import/ImportStatement.cpp
    src/import/ImportStrategySupport.cpp
    src/import/parsing/AmountParser.cpp
    src/import/parsing/DefaultStatementParser.cpp
    src/import/parsing/DefaultTransactionParser.cpp
    src/import/parsing/ParserDateUtils.cpp
    src/import/parsing/ParserHelpers.cpp
    src/import/parsing/ParserHeuristics.cpp
    src/import/parsing/StatementParseHelpers.cpp
    src/jobs/JobManager.cpp
    src/jobs/JobSystem.cpp
    src/jobs/Scheduler.cpp
    src/storage/StorageManager.cpp
    src/utils/Util.cpp
)

add_library(fossredder_core STATIC ${CORE_SRCS})
add_library(FossRedder::Core ALIAS fossredder_core)
fossredder_enable_target_quality(fossredder_core)

set_target_properties(fossredder_core PROPERTIES EXPORT_NAME Core)

target_compile_features(fossredder_core PUBLIC cxx_std_20)

target_include_directories(fossredder_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Internal dependencies used by core implementation code.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(xlnt CONFIG QUIET)

target_link_libraries(fossredder_core PUBLIC FossRedder::Api)
target_link_libraries(fossredder_core PRIVATE nlohmann_json::nlohmann_json)

if(TARGET xlnt::xlnt)
    target_link_libraries(fossredder_core PRIVATE xlnt::xlnt)
elseif(TARGET xlnt)
    target_link_libraries(fossredder_core PRIVATE xlnt)
else()
    message(WARNING "xlnt library not found; XLSX export may fail to link")
endif()

fossredder_set_library_output_dirs(fossredder_core)
fossredder_enable_target_pch(fossredder_core "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")

# Optional module-local tests.
option(BUILD_CORE_TESTS "Build tests for core module" OFF)
if (BUILD_CORE_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    set(CORE_UNIT_TEST_SRCS
        tests/unit/TestAnalysisEngine.cpp
        tests/unit/TestApplicationServices.cpp
        tests/unit/TestAppStateFacade.cpp
        tests/unit/TestAppStateManager.cpp
        tests/unit/TestAppStateManagerExtended.cpp
        tests/unit/TestDraftFinalizer.cpp
        tests/unit/TestErrorReporterRegistry.cpp
        tests/unit/TestExportService.cpp
        tests/unit/TestFilter.cpp
        tests/unit/TestJobSystem.cpp
        tests/unit/TestStateTransforms.cpp
        tests/unit/TestStorageManager.cpp
        tests/unit/TestWorkspaceSession.cpp
    )
    add_executable(core_unit_tests ${CORE_UNIT_TEST_SRCS})
    target_include_directories(core_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(core_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::Core
    )
    target_compile_features(core_unit_tests PRIVATE cxx_std_20)

    fossredder_set_runtime_output_dirs(core_unit_tests)
    fossredder_enable_target_quality(core_unit_tests)

    gtest_discover_tests(core_unit_tests PROPERTIES LABELS "UNIT")

    set(CORE_INTERACTION_TEST_SRCS
        tests/interaction/TestAppStateFacadeInteraction.cpp
        tests/interaction/TestAppStateManagerInteraction.cpp
        tests/interaction/TestStorageManagerInteraction.cpp
    )
    add_executable(core_interaction_tests ${CORE_INTERACTION_TEST_SRCS})
    target_include_directories(core_interaction_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(core_interaction_tests
        PRIVATE
            GTest::gmock_main
            FossRedder::Core
    )
    target_compile_features(core_interaction_tests PRIVATE cxx_std_20)

    fossredder_set_runtime_output_dirs(core_interaction_tests)
    fossredder_enable_target_quality(core_interaction_tests)

    gtest_discover_tests(core_interaction_tests PROPERTIES LABELS "INTERACTION")
endif()

fossredder_install_library_target(fossredder_core)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
