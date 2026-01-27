/**
 * @file core/include/core/managers/AppStateManager.h
 * @brief Loader/saver for AppState using repository abstractions.
 *
 * Responsible for converting between repository data and the in-memory
 * AppState representation. Provides load/save and validation/rehydration logic.
 */

#pragma once

#include <memory>
#include <string>

#include "core/models/AppState.h"

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;
class IAnalysisRepository;
class IAnnualRepository;

class AppStateManager {
public:
    struct Repositories {
        std::shared_ptr<IActorRepository> actors;
        std::shared_ptr<IPropertyRepository> properties;
        std::shared_ptr<IContractRepository> contracts;
        std::shared_ptr<IStatementRepository> statements;
        std::shared_ptr<ITransactionRepository> transactions;
        std::shared_ptr<IAnalysisRepository> analyses;
        std::shared_ptr<IAnnualRepository> annuals;
    };

    /**
     * @brief Construct the manager with the repository set used for load/save.
     * @param repos Repository collection for persistence operations.
     */
    explicit AppStateManager(Repositories repos);

    /**
     * @brief Load the full AppState from the configured repositories.
     * @return The rehydrated and validated AppState instance.
     */
    AppState load();

    /**
     * @brief Persist the given AppState into the configured repositories.
     * @param state The AppState to save.
     */
    void save(const AppState& state);

    /**
     * @brief Enable or disable strict validation when loading/saving.
     * @param enabled If true, validation failures throw exceptions.
     */
    void setStrictValidation(bool enabled) noexcept { strictValidation_ = enabled; }

private:
    Repositories repos_;
    bool strictValidation_ = false;

    /**
     * @brief Rehydrate references and relationships inside the AppState.
     * @param state The AppState to mutate in-place.
     */
    void rehydrate(AppState& state);

    /**
     * @brief Validate internal invariants of the AppState.
     * @param state The AppState to validate.
     * @throws std::runtime_error when validation fails and strictValidation_ is true.
     */
    void validate(const AppState& state) const;
};
