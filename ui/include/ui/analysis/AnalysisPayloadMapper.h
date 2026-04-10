/**
 * @file ui/include/ui/analysis/AnalysisPayloadMapper.h
 * @brief Declares mapping from core analysis results to UI QVariant payloads.
 */

#pragma once

#include <QVariantMap>

#include "core/models/AnalysisResult.h"

namespace ui::analysis {

/**
 * @brief Map a core analysis result into a QML-friendly QVariantMap payload.
 * @param result AnalysisResult from core domain.
 * @return QVariantMap containing keys defined in ui::payload::keys::analysis.
 */
QVariantMap toPayload(const core::domain::AnalysisResult& result);

} // namespace ui::analysis
