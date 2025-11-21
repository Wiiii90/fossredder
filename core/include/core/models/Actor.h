#pragma once

class Actor {
public:
    std::string name;
    std::string type;
    std::string description;

    Actor(const std::string& name, const std::string& type = "", const std::string& description = "");
};