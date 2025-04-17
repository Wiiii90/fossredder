#pragma once
#include <vector>
#include <memory>
#include "Property.h"
#include "Expense.h"

class ConsoleView {
public:
    static void displayProperties(const std::vector<std::shared_ptr<Property>>& properties);
    static void displayExpenses(const std::vector<std::shared_ptr<Expense>>& expenses);
};

