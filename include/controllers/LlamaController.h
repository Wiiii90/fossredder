#pragma once
#include "llama/LlamaEngine.h"
#include "models/Transaction.h"
#include "llama/prompts/Prompts.h"
#include "util/OutputParser.h"
#include <vector>
#include <memory>
#include <QString>
#include <QDebug>

class LlamaController {
public:
    static void enrichTransactions(LlamaEngine& engine, std::vector<std::shared_ptr<Transaction>>& transactions);
};