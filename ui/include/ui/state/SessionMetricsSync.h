/**
 * @file ui/include/ui/state/SessionMetricsSync.h
 * @brief Declares helpers for session metric recomputation and signal binding.
 */

#pragma once

#include <functional>

#include <QString>

class QObject;

namespace ui {

class AnalysisList;
class MetricsState;
class SessionModels;

using PropertyChangedCallback = std::function<void(const QString&)>;

void bindSessionMetricSignals(SessionModels& models,
                              ::QObject* context,
                              const std::function<void()>& recomputeAll,
                              const std::function<void(int, int)>& recomputeRange,
                              const std::function<void()>& clearAnalysisMetrics,
                              const std::function<void()>& notifyAllProperties);

void recomputeAllSessionMetrics(const SessionModels& models,
                                MetricsState& metrics,
                                const PropertyChangedCallback& notifyChanged);

void recomputeSessionMetricsForRows(int firstRow,
                                    int lastRow,
                                    const SessionModels& models,
                                    MetricsState& metrics,
                                    const PropertyChangedCallback& notifyChanged);

void notifySessionMetricsForAllProperties(const SessionModels& models,
                                          const PropertyChangedCallback& notifyChanged);

} // namespace ui
