#pragma once
#include <memory>
#include <string>
#include <vector>

class Category;

class ICategoryController {
public:
    virtual ~ICategoryController() = default;
    virtual void addCategory(const std::string& name, bool isAllocatable, const std::string& description = "") = 0;
    virtual std::vector<std::shared_ptr<Category>> getCategories() const = 0;
    virtual void removeCategory(const std::string& name) = 0;
    virtual void updateCategory(const std::string& name, const std::string& newName, bool isAllocatable, const std::string& newDescription = "") = 0;
};