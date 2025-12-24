#pragma once

#include <memory>
#include <string>

#include "core/models/AppState.h"

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;

class AppStateManager {
public:
    struct Repositories {
        std::shared_ptr<IActorRepository> actors;
        std::shared_ptr<IPropertyRepository> properties;
        std::shared_ptr<IContractRepository> contracts;
        std::shared_ptr<IStatementRepository> statements;
        std::shared_ptr<ITransactionRepository> transactions;
    };

    explicit AppStateManager(Repositories repos);

    AppState load();

    void setStrictValidation(bool enabled) noexcept { strictValidation_ = enabled; }

private:
    Repositories repos_;
    bool strictValidation_ = false;

    void rehydrate(AppState& state);
    void validate(const AppState& state) const;
};
