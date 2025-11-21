#pragma once

#include <memory>
#include <string>
#include <vector>

class Property;

class IPropertyRepository {
public:
    virtual ~IPropertyRepository() = default;
    virtual void addProperty(const std::string& name, const std::string& address, const std::string& description) = 0;
    virtual std::vector<std::shared_ptr<Property>> getProperties() const = 0;
    virtual void removeProperty(const std::string& name) = 0;
    virtual void updateProperty(const std::string& name, const std::string& address, const std::string& description) = 0;
};
