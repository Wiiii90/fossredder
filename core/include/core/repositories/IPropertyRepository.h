#pragma once

#include <memory>
#include <string>
#include <vector>

class Property;

class IPropertyRepository {
public:
    virtual ~IPropertyRepository() = default;

    virtual void addProperty(const std::shared_ptr<Property>& property) = 0;
    virtual std::vector<std::shared_ptr<Property>> getProperties() const = 0;
    virtual std::optional<std::shared_ptr<Property>> getPropertyById(const std::string& id) const = 0;
    virtual void removeProperty(const std::string& id) = 0;
    virtual void updateProperty(const std::shared_ptr<Property>& property) = 0;
};
