#pragma once

#include <memory>

#include "core/ports/repositories/IActorRepository.h"
#include "core/ports/repositories/IAnalysisRepository.h"
#include "core/ports/repositories/IAnnualRepository.h"
#include "core/ports/repositories/IContractRepository.h"
#include "core/ports/repositories/IExportLogRepository.h"
#include "core/ports/repositories/IImportLogRepository.h"
#include "core/ports/repositories/IPropertyRepository.h"
#include "core/ports/repositories/IStatementDraftRepository.h"
#include "core/ports/repositories/IStatementRepository.h"
#include "core/ports/repositories/ITransactionDraftRepository.h"
#include "core/ports/repositories/ITransactionRepository.h"

namespace core::storage {

struct RepositoryBundle {
    std::shared_ptr<core::ports::repositories::IActorRepository> actors;
    std::shared_ptr<core::ports::repositories::IPropertyRepository> properties;
    std::shared_ptr<core::ports::repositories::IContractRepository> contracts;
    std::shared_ptr<core::ports::repositories::IStatementRepository> statements;
    std::shared_ptr<core::ports::repositories::IStatementDraftRepository> statementDrafts;
    std::shared_ptr<core::ports::repositories::ITransactionRepository> transactions;
    std::shared_ptr<core::ports::repositories::ITransactionDraftRepository> transactionDrafts;
    std::shared_ptr<core::ports::repositories::IAnalysisRepository> analyses;
    std::shared_ptr<core::ports::repositories::IAnnualRepository> annuals;
    std::shared_ptr<core::ports::repositories::IImportLogRepository> importLogs;
    std::shared_ptr<core::ports::repositories::IExportLogRepository> exportLogs;
};

}
