#pragma once
#include <vector>
#include <memory>
#include "Property.h"

class ConsoleView {
public:
    static void displayProperties(const std::vector<std::shared_ptr<Property>>& properties);
};
