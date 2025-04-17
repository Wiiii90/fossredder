#pragma once
#include <string>

class Category {
public:
    std::string name;
    bool isAllocatable; // "umlegbar" oder "nicht umlegbar"

    Category(const std::string& name, bool isAllocatable);
};
