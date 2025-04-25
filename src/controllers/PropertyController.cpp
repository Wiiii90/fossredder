#include "pch.h"
#include "controllers/PropertyController.h"

void PropertyController::addProperty(const std::string& name, const std::string& address, const std::string& description) {
    properties.push_back(std::make_shared<Property>(name, address, description));
}

std::vector<std::shared_ptr<Property>> PropertyController::getProperties() const {
    return properties;
}
