#pragma once
#include "controllers/ICategoryController.h"
#include <memory>
#include <vector>
#include <string>

class Category;

class CategoryController : public ICategoryController {
public:
    void addCategory(const std::string& name, bool isAllocatable) override;
    std::vector<std::shared_ptr<Category>> getCategories() const override;
    void removeCategory(const std::string& name) override;
    void updateCategory(const std::string& name, bool isAllocatable) override;
private:
    std::vector<std::shared_ptr<Category>> categories;
};