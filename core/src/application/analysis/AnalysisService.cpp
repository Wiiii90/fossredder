/**
 * @file core/src/application/analysis/AnalysisService.cpp
 * @brief Implements application-level analysis execution services.
 */

#include "core/application/analysis/AnalysisService.h"

#include "core/constants/analysis.h"
#include "core/application/analysis/AnalysisResult.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Transaction.h"
#include "internal/AnalysisFilter.h"
#include "adjustment/AdjustmentCalculation.h"
#include "presentation/PlotAnalysis.h"
#include "presentation/TableAnalysis.h"

#include <unordered_map>

#include <utility>

namespace core::application::analysis {

namespace {

std::vector<AnalysisTransaction> projectAnalysisTransactions(const core::domain::catalog::WorkspaceCatalog& state,
                                                             const std::vector<std::shared_ptr<core::domain::Transaction>>& transactions)
{
    std::vector<AnalysisTransaction> out;
    out.reserve(transactions.size());

    std::unordered_map<std::string, std::shared_ptr<core::domain::Contract>> contractById;
    contractById.reserve(state.contracts().size());
    for (const auto& contract : state.contracts()) {
        if (!contract) {
            continue;
        }
        contractById.emplace(contract->id(), contract);
    }

    for (const auto& transaction : transactions) {
        if (!transaction) {
            continue;
        }

        AnalysisTransaction projected;
        projected.id = transaction->id();
        projected.name = transaction->name();
        projected.bookingDate = transaction->bookingDate();
        projected.amount = transaction->amount();
        projected.contractId = transaction->contractId();
        projected.propertyIds = transaction->propertyIds();
        if (!transaction->contractId().empty()) {
            const auto it = contractById.find(transaction->contractId());
            if (it != contractById.end() && it->second) {
                projected.contractType = it->second->type();
            }
        }
        out.push_back(std::move(projected));
    }

    return out;
}

} // namespace

AnalysisResult AnalysisService::runAnalysis(const core::domain::catalog::WorkspaceCatalog& state,
                                           const AnalysisRequest& request) const
{
    return runAnalysisById(state, request.analysisId, request.filterSpecification);
}

AnalysisResult AnalysisService::runAnalysisById(const core::domain::catalog::WorkspaceCatalog& state,
                                               const std::string& analysisId,
                                               const std::string& filterSpec) const
{
    for (const auto& analysis : state.analyses()) {
        if (!analysis) {
            continue;
        }
        if (analysis->id() != analysisId) {
            continue;
        }

        const std::string effectiveFilter =
            filterSpec.empty() ? analysis->filterSpec() : filterSpec;
        return computeAnalysis(*analysis, state, effectiveFilter);
    }

    return AnalysisResult{};
}

AnalysisResult AnalysisService::computeAnalysis(const core::domain::Analysis& analysis,
                                               const core::domain::catalog::WorkspaceCatalog& state,
                                               const std::string& filterSpec) const
{
    AnalysisResult out;
    const core::application::analysis::AnalysisFilter filter =
        core::application::analysis::parseAnalysisFilterSpec(filterSpec);

    const std::string key = analysis.executionType();

    if (key == core::constants::analysis::kTypePlot) {
        out = core::application::analysis::computePlotAnalysis(analysis, state, filter);
    } else if (key == core::constants::analysis::kTypeTab) {
        out = core::application::analysis::computeTableAnalysis(analysis, state, filter);
    } else if (key == core::constants::analysis::kTypeCalculation) {
        out = core::application::analysis::computeAdjustmentAnalysis(analysis, state, filter);
    } else {
        out = core::application::analysis::computeTableAnalysis(analysis, state, filter);
    }

    out.type = analysis.outputType();
    out.configJson = analysis.configJson();
    out.transactions = core::application::analysis::projectAnalysisTransactions(
        state,
        core::application::analysis::collectAnalysisTransactions(state, filter));
    out.generatedAt = analysis.updatedAt().empty()
        ? analysis.createdAt()
        : analysis.updatedAt();
    out.found = true;
    return out;
}

std::vector<std::shared_ptr<core::domain::Transaction>> AnalysisService::filterTransactions(
    const core::domain::catalog::WorkspaceCatalog& state,
    const std::string& filterSpec) const
{
    return collectAnalysisTransactions(state, parseAnalysisFilterSpec(filterSpec));
}

} // namespace core::application::analysis
