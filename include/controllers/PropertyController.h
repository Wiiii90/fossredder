#pragma once
#include "controllers/IPropertyController.h"
#include <memory>
#include <vector>
#include <string>

class Property;

class PropertyController : public IPropertyController {
public:
    void addProperty(const std::string& name, const std::string& address, const std::string& description) override;
    std::vector<std::shared_ptr<Property>> getProperties() const override;
    void removeProperty(const std::string& name) override;
    void updateProperty(const std::string& name, const std::string& address, const std::string& description) override;
private:
    std::vector<std::shared_ptr<Property>> properties;
};