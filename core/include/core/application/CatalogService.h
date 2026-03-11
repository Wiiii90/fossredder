#pragma once

#include "core/models/AppState.h"
#include "core/models/Transaction.h"

#include <string>
#include <vector>

namespace core::application {

class CatalogService {
public:
    std::string addActor(AppState& state, const std::string& name, const std::string& type, const std::string& description) const;
    bool updateActor(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& description) const;
    bool deleteActor(AppState& state, const std::string& id) const;

    std::string addProperty(AppState& state, const std::string& name, const std::string& address, const std::string& description) const;
    bool updateProperty(AppState& state, const std::string& id, const std::string& name, const std::string& address, const std::string& description) const;
    bool deleteProperty(AppState& state, const std::string& id) const;

    std::string addContract(AppState& state, const std::string& name, const std::string& type, const std::string& description,
                            const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds) const;
    bool updateContract(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& description,
                        const std::vector<std::string>& actorIds, const std::vector<std::string>& propertyIds) const;
    bool deleteContract(AppState& state, const std::string& id) const;

    std::string addStatement(AppState& state, const std::string& name) const;
    bool updateStatement(AppState& state, const std::string& id, const std::string& name) const;
    bool deleteStatement(AppState& state, const std::string& id) const;

    std::string addTransaction(AppState& state, const std::string& name, const std::string& bookingDate, double amount,
                               const std::string& description, const std::string& statementId, Transaction::Status status,
                               const std::string& actorId, bool allocatable, const std::vector<std::string>& propertyIds) const;
    bool updateTransaction(AppState& state, const std::string& id, const std::string& name, const std::string& bookingDate, double amount,
                           const std::string& description, const std::string& statementId, Transaction::Status status,
                           const std::string& actorId, bool allocatable, const std::vector<std::string>& propertyIds) const;
    bool deleteTransaction(AppState& state, const std::string& id) const;

    std::string addAnalysis(AppState& state, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec) const;
    bool updateAnalysis(AppState& state, const std::string& id, const std::string& name, const std::string& type, const std::string& configJson, const std::string& filterSpec) const;
    bool deleteAnalysis(AppState& state, const std::string& id) const;

    std::string addAnnual(AppState& state, int year) const;
    bool updateAnnual(AppState& state, const std::string& id, int year) const;
    bool deleteAnnual(AppState& state, const std::string& id) const;

    std::vector<std::string> contractTypes(const AppState& state) const;
};

}
