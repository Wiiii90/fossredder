#pragma once
#include "managers/IPropertyManager.h"
#include <vector>
#include <memory>
#include <string>

class Property;

class PropertyManager : public IPropertyManager {
public:
    void addProperty(const std::string& name, const std::string& address, const std::string& description) override;
    std::vector<std::shared_ptr<Property>> getProperties() const override;
    void removeProperty(const std::string& name) override;
    void updateProperty(const std::string& name, const std::string& address, const std::string& description) override;
private:
    std::vector<std::shared_ptr<Property>> properties_;
};