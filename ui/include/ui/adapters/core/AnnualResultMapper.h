/**
 * @file ui/include/ui/adapters/core/AnnualResultMapper.h
 * @brief Declares annual presenter/mapping helpers for UI payloads.
 */

#pragma once

#include <QVariantMap>

#include "core/application/annual/AnnualResult.h"

namespace ui::annual {

core::application::annual::AnnualResult present(const core::application::annual::AnnualResult& result);
QVariantMap toPayload(const core::application::annual::AnnualResult& result);

} // namespace ui::annual

