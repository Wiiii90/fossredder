#pragma once
#include <string>

class Category {
public:
    std::string name;
    bool isAllocatable; // "umlegbar" oder "nicht umlegbar"
    std::string description;

    Category(const std::string& name, bool isAllocatable, const std::string& description = "");
};