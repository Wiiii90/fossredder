/**
 * @file core/include/core/application/DraftFinalizer.h
 */
#pragma once

#include "core/models/AppState.h"
#include "core/models/DraftStatement.h"

#include <string>

namespace core::application {

class DraftFinalizer {
public:
    static std::string finalize(core::domain::AppState& state, const core::domain::DraftStatement& draft);
};

}
