#pragma once

#include <memory>
#include <string>
#include <vector>

namespace core::domain {
class Property;
class Actor;
class Transaction;
class Statement;
class Contract;
class Analysis;
class Annual;
}

namespace core::application::importing {
class ImportLog;
namespace draft {
class StatementDraft;
class TransactionDraft;
}
}

namespace core::application::exporting {
class ExportLog;
}

namespace core::domain {

struct WorkspaceState {
    template <typename T>
    using EntityList = std::vector<std::shared_ptr<T>>;

    using PropertyList = EntityList<core::domain::Property>;
    using ActorList = EntityList<core::domain::Actor>;
    using ContractList = EntityList<core::domain::Contract>;
    using StatementList = EntityList<core::domain::Statement>;
    using TransactionList = EntityList<core::domain::Transaction>;
    using AnalysisList = EntityList<core::domain::Analysis>;
    using AnnualList = EntityList<core::domain::Annual>;
    using StatementDraftList = EntityList<core::application::importing::draft::StatementDraft>;
    using TransactionDraftList = EntityList<core::application::importing::draft::TransactionDraft>;
    using ImportLogList = EntityList<core::application::importing::ImportLog>;
    using ExportLogList = EntityList<core::application::exporting::ExportLog>;

    PropertyList properties;
    ActorList actors;
    ContractList contracts;
    StatementList statements;
    TransactionList transactions;
    AnalysisList analyses;
    AnnualList annuals;
    StatementDraftList statementDrafts;
    TransactionDraftList transactionDrafts;
    ImportLogList importLogs;
    ExportLogList exportLogs;

    [[nodiscard]] bool empty() const noexcept
    {
        return properties.empty() && actors.empty() && contracts.empty() && statements.empty() &&
               transactions.empty() && analyses.empty() && annuals.empty() &&
               statementDrafts.empty() && transactionDrafts.empty() &&
               importLogs.empty() && exportLogs.empty();
    }
};

using AppState = WorkspaceState;

} // namespace core::domain

namespace core::application {
using WorkspaceState = core::domain::WorkspaceState;
using AppState = core::domain::WorkspaceState;
} // namespace core::application
