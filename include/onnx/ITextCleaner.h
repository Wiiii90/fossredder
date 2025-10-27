#pragma once
#include <string>

class ITextCleaner {
public:
    virtual ~ITextCleaner() = default;
    virtual std::string clean(const std::string& input) = 0;
};
