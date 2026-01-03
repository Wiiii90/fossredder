#pragma once

#include <string>
#include <vector>

class Actor {
public:
    std::string id;
    std::string name;
    std::string type;
    std::string description;

    std::vector<std::string> aliases;

    Actor() = default;
    Actor(const std::string& name, const std::string& type = "", const std::string& description = "");
};