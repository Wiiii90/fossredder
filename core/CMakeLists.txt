set(CORE_PUBLIC_HEADERS
    include/core/application/analysis/AnalysisEngine.h
    include/core/application/analysis/ComposeAnalysisRequest.h
    include/core/application/analysis/Filter.h
    include/core/application/analysis/RunAnalysis.h
    include/core/application/analysis/RunAnalysisRequest.h
    include/core/application/analysis/RunAnalysisResult.h
    include/core/application/export/AnalysisExportService.h
    include/core/application/export/CsvExporter.h
    include/core/application/export/ExportLog.h
    include/core/application/export/ExportRequest.h
    include/core/application/export/ExportResult.h
    include/core/application/export/ExportService.h
    include/core/application/export/ExportTypes.h
    include/core/application/export/IArchive.h
    include/core/application/export/IArchivePackager.h
    include/core/application/export/XlsxExporter.h
    include/core/application/import/IImportStatement.h
    include/core/application/import/ImportLog.h
    include/core/application/import/ImportRequest.h
    include/core/application/import/ImportResult.h
    include/core/application/import/draft/DraftFinalizer.h
    include/core/application/import/draft/DraftMatcher.h
    include/core/application/import/draft/StatementDraft.h
    include/core/application/import/draft/TransactionDraft.h
    include/core/application/import/internal/ParserConfig.h
    include/core/application/import/internal/ParserDateUtils.h
    include/core/application/import/internal/ParserHelpers.h
    include/core/application/import/internal/ParserHeuristics.h
    include/core/application/import/statement/DefaultStatementParser.h
    include/core/application/import/statement/StatementParseHelpers.h
    include/core/application/import/statement/StatementParserTypes.h
    include/core/application/import/transaction/AmountParser.h
    include/core/application/import/transaction/DefaultTransactionParser.h
    include/core/application/import/transaction/TransactionParserTypes.h
    include/core/application/storage/DeletionImpact.h
    include/core/application/storage/RepositoryBundle.h
    include/core/application/storage/StorageManager.h
    include/core/application/workspace/AppState.h
    include/core/application/workspace/WorkspaceFacade.h
    include/core/application/workspace/AppStateManager.h
    include/core/application/workspace/WorkspaceCatalogService.h
    include/core/application/workspace/DeletionImpact.h
    include/core/application/workspace/StateHydrator.h
    include/core/application/workspace/StateProjector.h
    include/core/application/workspace/WorkspaceDraftService.h
    include/core/application/workspace/WorkspaceSession.h
    include/core/application/workspace/WorkspaceState.h
    include/core/constants/analysis.h
    include/core/constants/app.h
    include/core/constants/app_state.h
    include/core/constants/comparison.h
    include/core/constants/config.h
    include/core/constants/export.h
    include/core/constants/filters.h
    include/core/constants/ids.h
    include/core/constants/import.h
    include/core/constants/json.h
    include/core/constants/jobs.h
    include/core/constants/localization.h
    include/core/constants/parser.h
    include/core/constants/preferences.h
    include/core/constants/runtime.h
    include/core/domain/entities/Actor.h
    include/core/domain/entities/Analysis.h
    include/core/domain/entities/Annual.h
    include/core/domain/entities/Contract.h
    include/core/domain/entities/Property.h
    include/core/domain/entities/Statement.h
    include/core/domain/entities/Transaction.h
    include/core/domain/policies/DraftMatchingPolicy.h
    include/core/domain/values/Alias.h
    include/core/domain/values/AliasUsage.h
    include/core/errors/ErrorCodes.h
    include/core/errors/ErrorEvent.h
    include/core/errors/ErrorEventFactory.h
    include/core/errors/ErrorReporterRegistry.h
    include/core/errors/ErrorReporting.h
    include/core/errors/IErrorReporter.h
    include/core/jobs/ImportJobSpec.h
    include/core/jobs/JobSystem.h
    include/core/jobs/JobTypes.h
    include/core/jobs/Scheduler.h
    include/core/pch.h
    include/core/ports/presenters/IAnalysisPresenter.h
    include/core/ports/presenters/IExportPresenter.h
    include/core/ports/presenters/IImportPresenter.h
    include/core/ports/presenters/IWorkspacePresenter.h
    include/core/ports/repositories/IActorRepository.h
    include/core/ports/repositories/IAnalysisRepository.h
    include/core/ports/repositories/IAnnualRepository.h
    include/core/ports/repositories/IContractRepository.h
    include/core/ports/repositories/IExportLogRepository.h
    include/core/ports/repositories/IImportLogRepository.h
    include/core/ports/repositories/IPropertyRepository.h
    include/core/ports/repositories/IStatementDraftRepository.h
    include/core/ports/repositories/IStatementRepository.h
    include/core/ports/repositories/ITransactionDraftRepository.h
    include/core/ports/repositories/ITransactionRepository.h
    include/core/ports/services/IOpenCvService.h
    include/core/ports/services/IImportMatcherService.h
    include/core/ports/services/IPopplerService.h
    include/core/ports/services/ITesseractService.h
    include/core/ports/storage/IRegistry.h
    include/core/ports/storage/IStorageManager.h
    include/core/utils/Time.h
)

set(CORE_PRIVATE_HEADERS
    src/application/analysis/AnalysisStrategy.h
    src/application/analysis/FilteredTransactions.h
    src/application/analysis/OutputTypeResolver.h
    src/application/analysis/TransactionCollector.h
    src/application/analysis/strategies/CalcAnalysisStrategy.h
    src/application/analysis/strategies/PlotAnalysisStrategy.h
    src/application/analysis/strategies/TabAnalysisStrategy.h
    src/application/export/PropertyContractMatrix.h
    src/application/import/ImportPageRequests.h
    src/application/import/ImportPipelineHelpers.h
    src/application/import/ImportStatementStrategy.h
    src/application/import/ImportStrategySupport.h
    src/application/import/internal/ParserConfig.h
    src/application/import/internal/ParserDateUtils.h
    src/application/import/internal/ParserHelpers.h
    src/application/import/internal/ParserHeuristics.h
    src/application/import/statement/DefaultStatementParser.h
    src/application/import/statement/StatementParseHelpers.h
    src/application/import/statement/StatementParserTypes.h
    src/application/import/transaction/AmountParser.h
    src/application/import/transaction/DefaultTransactionParser.h
    src/application/import/transaction/ParserTypes.h
    src/application/workspace/CatalogDraftAppliers.h
    src/application/workspace/CatalogMutationHelpers.h
    src/jobs/JobManager.h
    src/utils/StableId.h
    src/utils/Time.h
    src/utils/UniqId.h
    src/utils/Util.h
)

# Core implementation sources.
set(CORE_SRCS
    src/application/analysis/AnalysisEngine.cpp
    src/application/analysis/ComposeAnalysisRequest.cpp
    src/application/analysis/Filter.cpp
    src/application/analysis/OutputTypeResolver.cpp
    src/application/analysis/RunAnalysis.cpp
    src/application/analysis/TransactionCollector.cpp
    src/application/analysis/strategies/CalcAnalysisStrategy.cpp
    src/application/analysis/strategies/PlotAnalysisStrategy.cpp
    src/application/analysis/strategies/TabAnalysisStrategy.cpp
    src/application/export/AnalysisExportService.cpp
    src/application/export/CsvExporter.cpp
    src/application/export/ExportService.cpp
    src/application/export/XlsxExporter.cpp
    src/application/import/DefaultImportStatementStrategy.cpp
    src/application/import/draft/DraftFinalizer.cpp
    src/application/import/draft/DraftMatcherProjection.cpp
    src/application/import/draft/DraftMatcherResolution.cpp
    src/application/import/draft/DraftMatcherSignals.cpp
    src/application/import/ImportPipelineHelpers.cpp
    src/application/import/ImportStatement.cpp
    src/application/import/ImportStrategySupport.cpp
    src/application/import/internal/ParserDateUtils.cpp
    src/application/import/internal/ParserHelpers.cpp
    src/application/import/internal/ParserHeuristics.cpp
    src/application/import/statement/DefaultStatementParser.cpp
    src/application/import/statement/StatementParseHelpers.cpp
    src/application/import/transaction/AmountParser.cpp
    src/application/import/transaction/DefaultTransactionParser.cpp
    src/application/storage/StorageManager.cpp
    src/application/workspace/WorkspaceFacade.cpp
    src/application/workspace/AppStateManager.cpp
    src/application/workspace/WorkspaceCatalogService.cpp
    src/application/workspace/StateHydrator.cpp
    src/application/workspace/StateProjector.cpp
    src/application/workspace/WorkspaceDraftService.cpp
    src/application/workspace/WorkspaceSession.cpp
    src/errors/ErrorReporterRegistry.cpp
    src/jobs/JobManager.cpp
    src/jobs/JobSystem.cpp
    src/jobs/Scheduler.cpp
    src/domain/policies/DraftMatchingPolicy.cpp
    src/utils/Util.cpp
    ${CORE_PUBLIC_HEADERS}
    ${CORE_PRIVATE_HEADERS}
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
find_package(OpenCV REQUIRED)
find_package(libzip CONFIG REQUIRED)

target_link_libraries(fossredder_core PUBLIC FossRedder::Api)
target_link_libraries(fossredder_core PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(fossredder_core PRIVATE ${OpenCV_LIBS})
target_link_libraries(fossredder_core PRIVATE libzip::zip)

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
        tests/unit/TestDefaultStatementParser.cpp
        tests/unit/TestDraftLinking.cpp
        tests/unit/TestDraftFinalizer.cpp
        tests/unit/TestErrorReporterRegistry.cpp
        tests/unit/TestExportService.cpp
        tests/unit/TestFilter.cpp
        tests/unit/TestJobSystem.cpp
        tests/unit/TestParserHelpers.cpp
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
