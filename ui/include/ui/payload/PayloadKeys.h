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

namespace transaction {
inline const auto kBookingDate = QStringLiteral("bookingDate");
inline const auto kValuta = QStringLiteral("valuta");
inline const auto kActorId = QStringLiteral("actorId");
inline const auto kActorProposal = QStringLiteral("actorProposal");
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
inline const auto kTransactions = QStringLiteral("transactions");
inline const auto kArtifacts = QStringLiteral("artifacts");
inline const auto kGeneratedAt = QStringLiteral("generatedAt");
}

namespace fileSystem {
inline const auto kPath = QStringLiteral("path");
inline const auto kIsDir = QStringLiteral("isDir");
}

namespace metrics {
inline const auto kTotal = QStringLiteral("total");
inline const auto kAllocatable = QStringLiteral("allocatable");
inline const auto kNonAllocatable = QStringLiteral("nonAllocatable");
}

}
