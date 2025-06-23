#pragma once
#include <string>
#include <vector>
#include <memory>

class Property;
class Transaction;

class Statement {
public:
    Statement(const std::string& period, std::shared_ptr<Property> property);

    const std::string& getPeriod() const;
    std::shared_ptr<Property> getProperty() const;
    const std::vector<std::shared_ptr<Transaction>>& getTransactions() const;
    void addTransaction(const std::shared_ptr<Transaction>& transaction);

    double getTotal() const;

private:
    std::string period;
    std::shared_ptr<Property> property;
    std::vector<std::shared_ptr<Transaction>> transactions;
};