#include "core/pch.h"
#include "core/models/Property.h"

Property::Property(const std::string& name, const std::string& address, const std::string& description)
    : name(name), address(address), description(description) {
}
