#pragma once
#include "pch.h"
#include "Property.h"
#include "Category.h"

class Expense {
public:
    double amount;
    std::string date; // Format: YYYY-MM-DD
    std::shared_ptr<Category> category;
    std::shared_ptr<Property> property;
    bool isAllocatable;

    Expense(double amount, const std::string& date, std::shared_ptr<Category> category, std::shared_ptr<Property> property, bool isAllocatable);
};