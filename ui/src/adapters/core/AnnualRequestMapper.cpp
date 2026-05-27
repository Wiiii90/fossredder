/**
 * @file ui/src/adapters/core/AnnualRequestMapper.cpp
 * @brief Maps UI annual inputs into annual application requests.
 */

#include "ui/adapters/core/AnnualRequestMapper.h"

#include "ui/shared/util/StringConversions.h"

namespace ui::annual {

core::application::annual::AnnualRequest toRequest(const QString& annualId)
{
    core::application::annual::AnnualRequest request;
    request.annualId = strings::toStdString(annualId);
    return request;
}

core::ports::workspace::WorkspaceSnapshot withPreviewAnnual(
    core::ports::workspace::WorkspaceSnapshot workspace,
    const QString& annualId,
    const QStringList& analysisIds,
    int year)
{
    core::ports::workspace::AnnualSnapshot previewAnnual;
    bool found = false;
    const std::string previewId = annualId.isEmpty()
        ? std::string("__annual_preview__")
        : strings::toStdString(annualId);

    for (const auto& row : workspace.annuals) {
        if (row.id != previewId) continue;
        previewAnnual = row;
        found = true;
        break;
    }

    previewAnnual.id = previewId;
    if (year > 0) previewAnnual.year = year;
    previewAnnual.analysisIds.clear();
    previewAnnual.analysisIds.reserve(static_cast<size_t>(analysisIds.size()));
    for (const auto& id : analysisIds) {
        previewAnnual.analysisIds.push_back(strings::toStdString(id));
    }

    if (found) {
        for (auto& row : workspace.annuals) {
            if (row.id != previewId) continue;
            row = previewAnnual;
            return workspace;
        }
    }

    workspace.annuals.push_back(previewAnnual);
    return workspace;
}

} // namespace ui::annual

