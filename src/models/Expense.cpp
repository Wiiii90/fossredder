#include "Expense.h"

Expense::Expense(double amount, const std::string& date, std::shared_ptr<Category> category, std::shared_ptr<Property> property, bool isAllocatable)
    : amount(amount), date(date), category(category), property(property), isAllocatable(isAllocatable) {}
