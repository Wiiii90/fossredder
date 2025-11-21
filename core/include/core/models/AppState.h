#pragma once

#include <vector>
#include <memory>
#include <string>

class Property;
class Category;
class Actor;
class Transaction;

struct AppState {
    std::vector<std::shared_ptr<Property>> properties;
    std::vector<std::shared_ptr<Category>> categories;
    std::vector<std::shared_ptr<Actor>> actors;
    std::vector<std::shared_ptr<Transaction>> transactions;
};