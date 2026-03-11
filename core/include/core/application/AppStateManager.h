/**
 * @file core/include/core/application/AppStateManager.h
 * @brief Loader/saver for AppState using repository abstractions.
 *
 * Responsible for converting between repository data and the in-memory
 * AppState representation. Provides load/save and validation/rehydration logic.
 */

#pragma once

#include <memory>
#include <string>

#include "core/models/AppState.h"
#include "core/storage/RepositoryBundle.h"

namespace core::application {

class AppStateManager {
public:
    using Repositories = core::storage::RepositoryBundle;

    explicit AppStateManager(Repositories repos);

    AppState load();
    void save(const AppState& state);

    void setStrictValidation(bool enabled) noexcept { strictValidation_ = enabled; }

private:
    Repositories repos_;
    bool strictValidation_ = false;
};

}
