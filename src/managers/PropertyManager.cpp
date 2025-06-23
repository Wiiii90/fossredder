#include "managers/PropertyManager.h"
#include "models/Property.h"
#include <algorithm>
#include <memory>

void PropertyManager::addProperty(const std::string& name, const std::string& address, const std::string& description) {
    properties_.push_back(std::make_shared<Property>(name, address, description));
}

std::vector<std::shared_ptr<Property>> PropertyManager::getProperties() const {
    return properties_;
}

void PropertyManager::removeProperty(const std::string& name) {
    properties_.erase(
        std::remove_if(properties_.begin(), properties_.end(),
            [&](const std::shared_ptr<Property>& p) { return p && p->name == name; }),
        properties_.end());
}

void PropertyManager::updateProperty(const std::string& name, const std::string& address, const std::string& description) {
    for (auto& p : properties_) {
        if (p && p->name == name) {
            p->address = address;
            p->description = description;
            break;
        }
    }
}