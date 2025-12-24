#pragma once

#include <vector>
#include <memory>
#include <string>

class Property;
class Actor;
class Transaction;
class Statement;
class Contract;

struct AppState {
    std::vector<std::shared_ptr<Property>> properties;
    std::vector<std::shared_ptr<Actor>> actors;
    std::vector<std::shared_ptr<Contract>> contracts;
    std::vector<std::shared_ptr<Statement>> statements;
    std::vector<std::shared_ptr<Transaction>> transactions;
};