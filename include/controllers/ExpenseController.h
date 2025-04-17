#pragma once
#include <vector>
#include <memory>
#include "Expense.h"

class ExpenseController {
private:
    std::vector<std::shared_ptr<Expense>> expenses;

public:
    void addExpense(double amount, const std::string& date, std::shared_ptr<Category> category, std::shared_ptr<Property> property, bool isAllocatable);
    const std::vector<std::shared_ptr<Expense>>& getExpenses() const;
};

