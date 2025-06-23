#include "managers/CategoryManager.h"
#include "models/Category.h"
#include <algorithm>
#include <memory>

void CategoryManager::addCategory(const std::string& name, bool isAllocatable, const std::string& description) {
    categories_.push_back(std::make_shared<Category>(name, isAllocatable, description));
}

std::vector<std::shared_ptr<Category>> CategoryManager::getCategories() const {
    return categories_;
}

void CategoryManager::removeCategory(const std::string& name) {
    categories_.erase(
        std::remove_if(categories_.begin(), categories_.end(),
            [&](const std::shared_ptr<Category>& c) { return c && c->name == name; }),
        categories_.end());
}

void CategoryManager::updateCategory(const std::string& name, const std::string& newName, bool isAllocatable, const std::string& newDescription) {
    for (auto& c : categories_) {
        if (c && c->name == name) {
            c->name = newName;
            c->isAllocatable = isAllocatable;
            c->description = newDescription;
            break;
        }
    }
}