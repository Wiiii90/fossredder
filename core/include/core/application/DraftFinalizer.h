/**
 * @file core/include/core/application/DraftFinalizer.h
 */
#pragma once

#include "core/models/AppState.h"
#include "core/models/StatementDraft.h"

#include <string>

namespace core::application {

class DraftFinalizer {
public:
    static std::string finalize(core::domain::AppState& state, const core::domain::StatementDraft& draft);
};

}
