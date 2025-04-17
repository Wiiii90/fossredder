#pragma once
#include <vector>
#include <memory>
#include "Property.h"

class PropertyController {
private:
    std::vector<std::shared_ptr<Property>> properties;

public:
    void addProperty(const std::string& name, const std::string& address, const std::string& description = "");
    std::vector<std::shared_ptr<Property>> getProperties() const;
};
