#include "ConsoleView.h"
#include <iostream>

void ConsoleView::displayProperties(const std::vector<std::shared_ptr<Property>>& properties) {
    for (const auto& property : properties) {
        std::cout << "Name: " << property->name
            << ", Address: " << property->address
            << ", Description: " << property->description << std::endl;
    }
}

void ConsoleView::displayExpenses(const std::vector<std::shared_ptr<Expense>>& expenses) {
    for (const auto& expense : expenses) {
        std::cout << "Amount: " << expense->amount
            << ", Date: " << expense->date
            << ", Category: " << expense->category->name
            << ", Property: " << expense->property->name
            << ", Allocatable: " << (expense->isAllocatable ? "Yes" : "No") << std::endl;
    }
}

