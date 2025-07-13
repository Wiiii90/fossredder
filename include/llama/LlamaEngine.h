#pragma once
#include "llama/ILlamaEngine.h"
#include "models/Transaction.h"
#include "llama.h"
#include <memory>
#include <string>

class LlamaEngine : public ILlamaEngine {
public:
    explicit LlamaEngine(const std::string& modelPath);
    ~LlamaEngine() override;

    void enrichTransactions(std::vector<std::shared_ptr<Transaction>>& transactions) override;

private:
    llama_model* model_;
    llama_context* ctx_;
    std::string modelPath_;
};