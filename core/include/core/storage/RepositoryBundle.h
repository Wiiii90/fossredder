#pragma once

#include <memory>

class IActorRepository;
class IPropertyRepository;
class IContractRepository;
class IStatementRepository;
class ITransactionRepository;
class IStatementDraftRepository;
class ITransactionDraftRepository;
class IAnalysisRepository;
class IAnnualRepository;
class IImportLogRepository;
class IExportLogRepository;

namespace core::storage {

struct RepositoryBundle {
    std::shared_ptr<IActorRepository> actors;
    std::shared_ptr<IPropertyRepository> properties;
    std::shared_ptr<IContractRepository> contracts;
    std::shared_ptr<IStatementRepository> statements;
    std::shared_ptr<IStatementDraftRepository> statementDrafts;
    std::shared_ptr<ITransactionRepository> transactions;
    std::shared_ptr<ITransactionDraftRepository> transactionDrafts;
    std::shared_ptr<IAnalysisRepository> analyses;
    std::shared_ptr<IAnnualRepository> annuals;
    std::shared_ptr<IImportLogRepository> importLogs;
    std::shared_ptr<IExportLogRepository> exportLogs;
};

}
