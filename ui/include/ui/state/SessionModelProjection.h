/**
 * @file ui/include/ui/state/SessionModelProjection.h
 * @brief Declares helper functions that project core app state into UI session models.
 */

#pragma once

#include <QHash>
#include <QString>

#include "core/models/AppState.h"

namespace ui {

class ContractList;
class SessionModels;

QHash<QString, QString> buildContractTypeIndex(const ContractList& contracts);
void projectSessionModelsFromState(SessionModels& models, const core::domain::AppState& state);
void refreshSessionModelContractTypes(SessionModels& models);

} // namespace ui
