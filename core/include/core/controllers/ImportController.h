#pragma once

#include <memory>
#include <string>
#include <functional>

class Statement;
class StatementController;

class ImportController {
public:
    enum class ImportType {
        Statement
    };

    explicit ImportController(std::shared_ptr<StatementController> statementController);

    std::shared_ptr<Statement> import(ImportType type, const std::string& filePath, const std::string& runRoot = {}, const std::string& runIdPrefix = {}, std::function<void(double, const std::string&)> progressCallback = {});

private:
    std::shared_ptr<StatementController> statementController_;
};
