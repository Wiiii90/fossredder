#pragma once
#include <vector>
#include <memory>
#include "models/Transaction.h"

class ILlamaEngine {
public:
    virtual ~ILlamaEngine() = default;
    virtual void enrichTransactions(std::vector<std::shared_ptr<Transaction>>& transactions) = 0;
};