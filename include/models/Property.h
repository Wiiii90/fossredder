#pragma once
#include "pch.h"

class Property {
public:
    std::string name;
    std::string address;
    std::string description;

    Property(const std::string& name, const std::string& address, const std::string& description = "");
};