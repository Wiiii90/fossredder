#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "Property.h"
#include "Expense.h"
#include "PdfExtractedData.h"
#include "Transaction.h"

class ConsoleView {
public:
    static void displayPdfData(const std::shared_ptr<PdfExtractedData>& data);
};