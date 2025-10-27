#pragma once
#include "onnx/ITextCleaner.h"
#include <memory>
#include <vector>

class CompositeTextCleaner : public ITextCleaner {
public:
    CompositeTextCleaner() = default;
    ~CompositeTextCleaner() override = default;

    void add(std::unique_ptr<ITextCleaner> cleaner) {
        cleaners_.emplace_back(std::move(cleaner));
    }

    std::string clean(const std::string& input) override {
        std::string text = input;
        for (auto& c : cleaners_) {
            text = c->clean(text);
        }
        return text;
    }

private:
    std::vector<std::unique_ptr<ITextCleaner>> cleaners_;
};
