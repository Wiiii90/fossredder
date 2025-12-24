#pragma once

#include <string>

class Actor {
public:
    std::string id;
    std::string name;
    std::string type;
    std::string description;

    Actor() = default;
    Actor(const std::string& name, const std::string& type = "", const std::string& description = "");
};