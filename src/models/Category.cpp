#include "pch.h"
#include "models/Category.h"

Category::Category(const std::string& name, bool isAllocatable)
    : name(name), isAllocatable(isAllocatable) {}
