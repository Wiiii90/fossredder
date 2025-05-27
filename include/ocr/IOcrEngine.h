#pragma once
#include <string>

class IOcrEngine {
public:
    virtual ~IOcrEngine() = default;
    virtual std::string recognizeAltoXml(const std::string& imageFilePath, const std::string& tessdataPath) = 0;
};