#pragma once

#include "core/models/DraftStatement.h"

#include <string>

namespace core::domain {
struct AppState;
}

namespace core::application {

class DraftFinalizer {
public:
    static std::string finalize(core::domain::AppState& state, const DraftStatement& draft);
};

}
