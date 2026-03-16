#pragma once

#include "core/models/AppState.h"
#include "ui/models/ActorList.h"
#include "ui/models/AnalysisList.h"
#include "ui/models/AnnualList.h"
#include "ui/models/ContractList.h"
#include "ui/models/PropertyList.h"
#include "ui/models/StatementList.h"
#include "ui/models/TransactionList.h"

class QObject;

namespace ui {

class SessionModels {
public:
    SessionModels();

    ActorList& actors() noexcept { return actors_; }
    PropertyList& properties() noexcept { return properties_; }
    ContractList& contracts() noexcept { return contracts_; }
    StatementList& statements() noexcept { return statements_; }
    TransactionList& transactions() noexcept { return transactions_; }
    AnalysisList& analyses() noexcept { return analyses_; }
    AnnualList& annuals() noexcept { return annuals_; }

    const ActorList& actors() const noexcept { return actors_; }
    const PropertyList& properties() const noexcept { return properties_; }
    const ContractList& contracts() const noexcept { return contracts_; }
    const StatementList& statements() const noexcept { return statements_; }
    const TransactionList& transactions() const noexcept { return transactions_; }
    const AnalysisList& analyses() const noexcept { return analyses_; }
    const AnnualList& annuals() const noexcept { return annuals_; }

    void loadFromState(const core::domain::AppState& state);
    void refreshContractTypes();

private:
    ActorList actors_;
    AnalysisList analyses_;
    PropertyList properties_;
    ContractList contracts_;
    StatementList statements_;
    TransactionList transactions_;
    AnnualList annuals_;
};

}
