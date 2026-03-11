#pragma once

#include <string>

struct ImportedTransaction {
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string description;
    std::string metadata;
    std::string proofImagePath;
};
