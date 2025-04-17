#include "ConsoleView.h"
#include <iostream>

void ConsoleView::displayProperties(const std::vector<std::shared_ptr<Property>>& properties) {
    for (const auto& property : properties) {
        std::cout << "Name: " << property->name << ", Address: " << property->address
                  << ", Description: " << property->description << std::endl;
    }
}
