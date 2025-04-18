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

void ConsoleView::displayPdfData(const std::shared_ptr<PdfExtractedData>& data) {
    std::cout << "PDF Source File: " << data->sourceFile << std::endl;
    std::cout << "Extracted Text Blocks:" << std::endl;

    for (const auto& block : data->textBlocks) {
        std::cout << "Text: " << block.text << std::endl;
        std::cout << "Position: (x: " << block.x << ", y: " << block.y
            << ", width: " << block.width << ", height: " << block.height << ")" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}
