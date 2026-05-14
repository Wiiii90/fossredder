/**
 * @file core/include/core/application/analysis/AnalysisService.h
 * @brief Declares the application service that resolves and executes analyses.
 */

#pragma once

#include "core/application/analysis/AnalysisRequest.h"
#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Transaction.h"

#include <memory>
#include <string>
#include <vector>

namespace core::application::analysis {

/**
 * @brief Coordinates analysis lookup, execution, and transaction preview filtering.
 */
class AnalysisService {
public:
    AnalysisService() = default;
    ~AnalysisService() = default;

    AnalysisService(const AnalysisService&) = delete;
    AnalysisService& operator=(const AnalysisService&) = delete;
    AnalysisService(AnalysisService&&) noexcept = default;
    AnalysisService& operator=(AnalysisService&&) noexcept = default;

    /**
     * @brief Runs one analysis request against the supplied workspace state.
     * @param state Current workspace catalog snapshot.
     * @param request Analysis request containing identifier and optional filter override.
     * @return Materialized analysis result, or an unfound result when the identifier is missing.
     */
    AnalysisResult runAnalysis(const core::domain::catalog::WorkspaceCatalog& state, const AnalysisRequest& request) const;

    /**
     * @brief Resolves an analysis by identifier and executes it.
     * @param state Current workspace catalog snapshot.
     * @param analysisId Identifier of the stored analysis definition.
     * @param filterSpec Optional filter override applied instead of the stored filter.
     * @return Materialized analysis result, or an unfound result when the identifier is missing.
     */
    AnalysisResult runAnalysisById(const core::domain::catalog::WorkspaceCatalog& state,
                                   const std::string& analysisId,
                                   const std::string& filterSpec = {}) const;

    /**
     * @brief Computes a concrete analysis definition against the supplied workspace state.
     * @param analysis Analysis definition to execute.
     * @param state Current workspace catalog snapshot.
     * @param filterSpec Optional filter override applied during execution.
     * @return Fully materialized analysis result.
     */
    AnalysisResult computeAnalysis(const core::domain::Analysis& analysis,
                                   const core::domain::catalog::WorkspaceCatalog& state,
                                   const std::string& filterSpec = {}) const;

    /**
     * @brief Returns the transactions matched by one analysis filter specification.
     * @param state Current workspace catalog snapshot.
     * @param filterSpec Raw analysis filter specification.
     * @return Transactions that satisfy the parsed filter.
     */
    std::vector<std::shared_ptr<core::domain::Transaction>> filterTransactions(const core::domain::catalog::WorkspaceCatalog& state,
                                                                               const std::string& filterSpec = {}) const;
};

} // namespace core::application::analysis

namespace core::application {
using AnalysisService = analysis::AnalysisService;
}
