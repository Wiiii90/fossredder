#pragma once
#include <vector>
#include <memory>
#include <string>

class StatementData;
class Transaction;

class ConsoleView {
public:
    void displayPdfData(const std::shared_ptr<StatementData>& data);
    void displayError(const std::string& errorMessage);
};