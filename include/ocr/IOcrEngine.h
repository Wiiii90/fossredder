#pragma once
#include <string>
#include <memory>
#include <vector>

class IOcrEngine {
public:
    virtual ~IOcrEngine() = default;
    virtual std::string recognizeAltoXml(const std::string& imageFilePath, const std::string& tessdataPath) = 0;
};