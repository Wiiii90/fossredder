#pragma once
#include "controllers/ICategoryController.h"
#include <vector>
#include <memory>
#include <string>

class Category;

class CategoryController : public ICategoryController {
public:
    void addCategory(const std::string& name, bool isAllocatable, const std::string& description = "") override;
    std::vector<std::shared_ptr<Category>> getCategories() const override;
    void removeCategory(const std::string& name) override;
    void updateCategory(const std::string& name, const std::string& newName, bool isAllocatable, const std::string& newDescription = "") override;
private:
    std::vector<std::shared_ptr<Category>> categories_;
};