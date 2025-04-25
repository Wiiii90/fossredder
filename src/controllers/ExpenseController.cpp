#include "pch.h"
#include "controllers/ExpenseController.h"

void ExpenseController::addExpense(double amount, const std::string& date, std::shared_ptr<Category> category, std::shared_ptr<Property> property, bool isAllocatable) {
    expenses.push_back(std::make_shared<Expense>(amount, date, category, property, isAllocatable));
}

const std::vector<std::shared_ptr<Expense>>& ExpenseController::getExpenses() const {
    return expenses;
}

