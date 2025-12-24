#include "core/pch.h"
#include "core/models/Transaction.h"

Transaction::Transaction(std::string name,
    std::string bookingDate,
    std::string valuta,
    double amount,
    Contract* contract,
    Actor* actor,
    std::string description)
    : name(std::move(name)),
      bookingDate(std::move(bookingDate)),
      valuta(std::move(valuta)),
      amount(amount),
      contract(contract),
      actor(actor),
      description(std::move(description)) {
}

bool Transaction::operator==(const Transaction& other) const noexcept {
    return id == other.id &&
           name == other.name &&
           bookingDate == other.bookingDate &&
           valuta == other.valuta &&
           amount == other.amount &&
           contractId == other.contractId &&
           contract == other.contract &&
           actorId == other.actorId &&
           actor == other.actor &&
           statementId == other.statementId &&
           description == other.description;
}
