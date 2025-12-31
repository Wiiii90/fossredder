#pragma once

#include <memory>
#include <string>

class Statement;
class StatementController;

class ImportController {
public:
    enum class ImportType {
        Statement
    };

    explicit ImportController(std::shared_ptr<StatementController> statementController);

    std::shared_ptr<Statement> import(ImportType type, const std::string& filePath);

private:
    std::shared_ptr<StatementController> statementController_;
};
