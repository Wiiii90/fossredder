#include "core/pch.h"
#include "core/models/Transaction.h"

Transaction::Transaction(std::string name,
    std::string bookingDate,
    std::string valuta,
    double amount,
    Contract* contract,
    Actor* actor,
    std::string description,
    bool allocatable)
    : name(std::move(name)),
      bookingDate(std::move(bookingDate)),
      valuta(std::move(valuta)),
      amount(amount),
      contract(contract),
      actor(actor),
      description(std::move(description)),
      allocatable(allocatable) {
}

bool Transaction::operator==(const Transaction& other) const noexcept {
    return id == other.id &&
           name == other.name &&
           bookingDate == other.bookingDate &&
           valuta == other.valuta &&
           amount == other.amount &&
           status == other.status &&
           contractId == other.contractId &&
           actorId == other.actorId &&
           statementId == other.statementId &&
           description == other.description &&
           actorProposal == other.actorProposal &&
           metadata == other.metadata &&
           proofImagePath == other.proofImagePath &&
           allocatable == other.allocatable;
}
