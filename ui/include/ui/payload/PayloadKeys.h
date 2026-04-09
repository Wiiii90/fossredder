/**
 * @file ui/include/ui/payload/PayloadKeys.h
 * @brief Declarations for the UI PayloadKeys component.
 */

#pragma once

#include <QString>

namespace ui::payload::keys {

namespace common {
inline const auto kId = QStringLiteral("id");
inline const auto kName = QStringLiteral("name");
inline const auto kType = QStringLiteral("type");
inline const auto kDescription = QStringLiteral("description");
inline const auto kAmount = QStringLiteral("amount");
inline const auto kStatus = QStringLiteral("status");
inline const auto kMetadata = QStringLiteral("metadata");
}

namespace actor {
inline const auto kAliases = QStringLiteral("aliases");
}

namespace property {
inline const auto kAddress = QStringLiteral("address");
inline const auto kConsumption = QStringLiteral("consumption");
inline const auto kConsumptionUnit = QStringLiteral("consumptionUnit");
inline const auto kAliases = QStringLiteral("aliases");
}

namespace contract {
inline const auto kStartDate = QStringLiteral("startDate");
inline const auto kEndDate = QStringLiteral("endDate");
inline const auto kBasePrice = QStringLiteral("basePrice");
inline const auto kConsumptionPrice = QStringLiteral("consumptionPrice");
inline const auto kMonthlyAdvance = QStringLiteral("monthlyAdvance");
inline const auto kActorIds = QStringLiteral("actorIds");
inline const auto kPropertyIds = QStringLiteral("propertyIds");
inline const auto kAliases = QStringLiteral("aliases");
}

namespace statement {
inline const auto kStatementId = QStringLiteral("statementId");
}

namespace transaction {
inline const auto kBookingDate = QStringLiteral("bookingDate");
inline const auto kValuta = QStringLiteral("valuta");
inline const auto kActorId = QStringLiteral("actorId");
inline const auto kProofImagePath = QStringLiteral("proofImagePath");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kPropertyIds = QStringLiteral("propertyIds");
inline const auto kContractId = QStringLiteral("contractId");
inline const auto kContractType = QStringLiteral("contractType");
inline const auto kDate = QStringLiteral("date");
}

namespace analysis {
inline const auto kMetrics = QStringLiteral("metrics");
inline const auto kTable = QStringLiteral("table");
inline const auto kConfig = QStringLiteral("config");
inline const auto kFilter = QStringLiteral("filterSpec");
inline const auto kAdjustments = QStringLiteral("adjustments");
inline const auto kTransactions = QStringLiteral("transactions");
inline const auto kArtifacts = QStringLiteral("artifacts");
inline const auto kGeneratedAt = QStringLiteral("generatedAt");
}

namespace annual {
inline const auto kYear = QStringLiteral("year");
inline const auto kVerificationState = QStringLiteral("verificationState");
inline const auto kTransactionIds = QStringLiteral("transactionIds");
inline const auto kAssignedAnalysisIds = QStringLiteral("assignedAnalysisIds");
}

namespace fileSystem {
inline const auto kPath = QStringLiteral("path");
inline const auto kIsDir = QStringLiteral("isDir");
}

namespace importRun {
inline const auto kTime = QStringLiteral("time");
inline const auto kFile = QStringLiteral("file");
inline const auto kMessage = QStringLiteral("message");
}

namespace draft {
inline const auto kActorText = QStringLiteral("actorText");
inline const auto kNewActorSelected = QStringLiteral("newActorSelected");
inline const auto kNewContractSelected = QStringLiteral("newContractSelected");
inline const auto kActorSuggestions = QStringLiteral("actorSuggestions");
inline const auto kPropertySuggestions = QStringLiteral("propertySuggestions");
inline const auto kContractSuggestions = QStringLiteral("contractSuggestions");
inline const auto kAllocatableManualOverride = QStringLiteral("allocatableManualOverride");
}

namespace language {
inline const auto kCode = QStringLiteral("code");
inline const auto kLabel = QStringLiteral("label");
inline const auto kAvailable = QStringLiteral("available");
}

namespace metrics {
inline const auto kTotal = QStringLiteral("total");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kNonAllocatable = QStringLiteral("nonAllocatable");
}

}
