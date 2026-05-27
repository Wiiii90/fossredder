/**
 * @file ui/include/ui/adapters/core/AnnualRequestMapper.h
 * @brief Declares mapping helpers for annual application requests in UI.
 */

#pragma once

#include <QString>
#include <QStringList>

#include "core/application/annual/AnnualRequest.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::annual {

core::application::annual::AnnualRequest toRequest(const QString& annualId);
core::ports::workspace::WorkspaceSnapshot withPreviewAnnual(
    core::ports::workspace::WorkspaceSnapshot workspace,
    const QString& annualId,
    const QStringList& analysisIds,
    int year);

} // namespace ui::annual

