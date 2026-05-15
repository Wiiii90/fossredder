/**
 * @file ui/include/ui/state/SessionModels.h
 * @brief Declarations for the UI SessionModels component.
 */

#pragma once

#include <memory>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "ui/viewmodels/catalog/ActorListModel.h"
#include "ui/viewmodels/reporting/AnalysisListModel.h"
#include "ui/viewmodels/reporting/AnnualListModel.h"
#include "ui/viewmodels/catalog/ContractListModel.h"
#include "ui/viewmodels/catalog/PropertyListModel.h"
#include "ui/viewmodels/booking/StatementListModel.h"
#include "ui/viewmodels/booking/TransactionListModel.h"

class QObject;

namespace ui {

class SessionModels {
public:
    explicit SessionModels(QObject* objectParent = nullptr);

    ActorList& actors() noexcept { return *actors_; }
    PropertyList& properties() noexcept { return *properties_; }
    ContractList& contracts() noexcept { return *contracts_; }
    StatementList& statements() noexcept { return *statements_; }
    TransactionList& transactions() noexcept { return *transactions_; }
    AnalysisList& analyses() noexcept { return *analyses_; }
    AnnualList& annuals() noexcept { return *annuals_; }

    const ActorList& actors() const noexcept { return *actors_; }
    const PropertyList& properties() const noexcept { return *properties_; }
    const ContractList& contracts() const noexcept { return *contracts_; }
    const StatementList& statements() const noexcept { return *statements_; }
    const TransactionList& transactions() const noexcept { return *transactions_; }
    const AnalysisList& analyses() const noexcept { return *analyses_; }
    const AnnualList& annuals() const noexcept { return *annuals_; }

    void loadFromState(const core::domain::catalog::WorkspaceCatalog& state);
    void refreshContractTypes();

private:
    std::unique_ptr<ActorList> actors_;
    std::unique_ptr<AnalysisList> analyses_;
    std::unique_ptr<PropertyList> properties_;
    std::unique_ptr<ContractList> contracts_;
    std::unique_ptr<StatementList> statements_;
    std::unique_ptr<TransactionList> transactions_;
    std::unique_ptr<AnnualList> annuals_;
};

}
