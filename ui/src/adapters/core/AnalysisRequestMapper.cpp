/**
 * @file ui/src/adapters/core/AnalysisRequestMapper.cpp
 * @brief Implements helpers that map analysis-related UI input into core-friendly values.
 */

#include "ui/adapters/core/AnalysisRequestMapper.h"

#include <utility>

namespace ui::analysis::input {

std::vector<std::string> toSelectedTransactionIds(const QVariantList& selectedTransactionIds)
{
    std::vector<std::string> out;
    out.reserve(selectedTransactionIds.size());
    for (const auto& item : selectedTransactionIds) {
        const auto value = item.toString().trimmed();
        if (!value.isEmpty()) {
            out.push_back(value.toStdString());
        }
    }
    return out;
}

core::application::analysis::AnalysisRequest toAnalysisRequest(const QString& analysisId,
                                                               const QString& filterSpecification)
{
    core::application::analysis::AnalysisRequest request;
    request.analysisId = analysisId.trimmed().toStdString();
    request.filterSpecification = filterSpecification.trimmed().toStdString();
    return request;
}

} // namespace ui::analysis::input
