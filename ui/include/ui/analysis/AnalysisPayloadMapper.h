#pragma once

#include <QVariantMap>

#include "core/models/AnalysisResult.h"

namespace ui::analysis {

QVariantMap toPayload(const AnalysisResult& result);

}
